/*
 *
 *   Authors:
 *    Lars Fenneberg		<lf@elemental.net>
 *
 *   This software is Copyright 1996,1997 by the above mentioned author(s),
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <reubenhwk@gmail.com>.
 *
 */

#include "config.h"
#include "defaults.h"
#include "includes.h"
#include "netlink.h"
#include "pathnames.h"
#include "radvd.h"

#ifndef IPV6_ADDR_LINKLOCAL
#define IPV6_ADDR_LINKLOCAL 0x0020U
#endif

#ifndef ARPHRD_6LOWPAN
#define ARPHRD_6LOWPAN 825 /* IPv6 over LoWPAN */
#endif

static char const *hwstr(unsigned short sa_family);
uint32_t get_interface_linkmtu(const char *);

/*
 * this function gets the hardware type and address of an interface,
 * determines the link layer token length and checks it against
 * the defined prefixes
 */
int update_device_info(int sock, struct Interface *iface)
{
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, iface->props.name, IFNAMSIZ - 1);

	if (ioctl(sock, SIOCGIFMTU, &ifr) < 0) {
		flog(LOG_ERR, "ioctl(SIOCGIFMTU) failed on %s: %s", iface->props.name, strerror(errno));
		return -1;
	}

	iface->sllao.if_maxmtu = ifr.ifr_mtu;
	dlog(LOG_DEBUG, 3, "%s mtu: %d", iface->props.name, ifr.ifr_mtu);

	/* RFC 2460: 5. Packet Size Issues */
	/* Get the smallest MTU between the SIOCGIFMTU value and the protocol MTU
	 * /proc/sys/net/ipv6/conf/eth0/mtu
	 * Because the protocol MTU _may_ be different than the physical link MTU
	 *
	 * If Link-layer MTU <= 1280: use 1280 (enforced by iface->AdvRAMTU)
	 * If Link-layer MTU > 1280: use the lower of:
	 *   - RA MTU
	 *   - link-layer MTU
	 *   - per-protocol MTU
	 */
	iface->props.max_ra_option_size = iface->AdvRAMTU;
	iface->props.max_ra_option_size = MIN(iface->props.max_ra_option_size, MAX(iface->sllao.if_maxmtu, RFC2460_MIN_MTU));
	iface->props.max_ra_option_size =
	    MIN(iface->props.max_ra_option_size, MAX(get_interface_linkmtu(iface->props.name), RFC2460_MIN_MTU));

	if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
		flog(LOG_ERR, "ioctl(SIOCGIFHWADDR) failed on %s: %s", iface->props.name, strerror(errno));
		return -1;
	}

	dlog(LOG_DEBUG, 3, "%s hardware type: %s", iface->props.name, hwstr(ifr.ifr_hwaddr.sa_family));

	switch (ifr.ifr_hwaddr.sa_family) {
	case ARPHRD_ETHER:
		iface->sllao.if_hwaddr_len = 48;
		iface->sllao.if_prefix_len = 64;
		char hwaddr[3 * 6];
		sprintf(hwaddr, "%02x:%02x:%02x:%02x:%02x:%02x", (unsigned char)ifr.ifr_hwaddr.sa_data[0],
			(unsigned char)ifr.ifr_hwaddr.sa_data[1], (unsigned char)ifr.ifr_hwaddr.sa_data[2],
			(unsigned char)ifr.ifr_hwaddr.sa_data[3], (unsigned char)ifr.ifr_hwaddr.sa_data[4],
			(unsigned char)ifr.ifr_hwaddr.sa_data[5]);
		dlog(LOG_DEBUG, 3, "%s hardware address: %s", iface->props.name, hwaddr);
		iface->props.max_ra_option_size -= 14; /* RFC 2464 */
		break;
#ifdef ARPHRD_FDDI
	case ARPHRD_FDDI:
		iface->sllao.if_hwaddr_len = 48;
		iface->sllao.if_prefix_len = 64;
		iface->props.max_ra_option_size -= 22; /* RFC 2109 */
		break;
#endif /* ARPHDR_FDDI */
#ifdef ARPHRD_ARCNET
	case ARPHRD_ARCNET:
		iface->sllao.if_hwaddr_len = 8;
		iface->sllao.if_prefix_len = -1;
		iface->sllao.if_maxmtu = -1;
		/* RFC1201: fragmentation handled at a lower layer.
		 * native packet size is 256-512 bytes */
		iface->props.max_ra_option_size -= 0;
		break;
#endif /* ARPHDR_ARCNET */
	case ARPHRD_6LOWPAN:
#ifdef HAVE_NETLINK
		/* hwaddr length differs on some L2 type lets detect them */
		iface->sllao.if_hwaddr_len = netlink_get_device_addr_len(iface);
		if (iface->sllao.if_hwaddr_len != -1) {
			iface->sllao.if_hwaddr_len *= 8;
			iface->sllao.if_prefix_len = 64;
		} else {
			iface->sllao.if_prefix_len = -1;
		}
#else
		iface->sllao.if_hwaddr_len = -1;
		iface->sllao.if_prefix_len = -1;
#endif
		/* RFC4944: fragmentation handled at a lower layer.
		 * native packet size maxes at 127 bytes */
		iface->props.max_ra_option_size -= 0;
		break;
	default:
		iface->sllao.if_hwaddr_len = -1;
		iface->sllao.if_prefix_len = -1;
		iface->sllao.if_maxmtu = -1;
		/* Assume fragmentation handled at a lower layer. */
		iface->props.max_ra_option_size -= 0;
		break;
	}

	dlog(LOG_DEBUG, 3, "%s link layer token length: %d", iface->props.name, iface->sllao.if_hwaddr_len);

	dlog(LOG_DEBUG, 3, "%s prefix length: %d", iface->props.name, iface->sllao.if_prefix_len);

	if (iface->sllao.if_hwaddr_len != -1) {
		unsigned int if_hwaddr_len_bytes = (iface->sllao.if_hwaddr_len + 7) >> 3;

		if (if_hwaddr_len_bytes > sizeof(iface->sllao.if_hwaddr)) {
			flog(LOG_ERR, "%s address length too big: %d", iface->props.name, if_hwaddr_len_bytes);
			return -2;
		}
		memcpy(iface->sllao.if_hwaddr, ifr.ifr_hwaddr.sa_data, if_hwaddr_len_bytes);

		char zero[sizeof(iface->props.if_addr)];
		memset(zero, 0, sizeof(zero));
		if (!memcmp(iface->sllao.if_hwaddr, zero, if_hwaddr_len_bytes))
			flog(LOG_WARNING, "WARNING, MAC address on %s is all zero!", iface->props.name);
	}

	struct AdvPrefix *prefix = iface->AdvPrefixList;
	while (prefix) {
		if ((iface->sllao.if_prefix_len != -1) && (iface->sllao.if_prefix_len != prefix->PrefixLen)) {
			flog(LOG_WARNING, "%s prefix length should be: %d", iface->props.name, iface->sllao.if_prefix_len);
		}

		prefix = prefix->next;
	}

	// Regardless of link-layer, every RA message will have an IPV6 header & RA header
	iface->props.max_ra_option_size -= sizeof(struct ip6_hdr);
	iface->props.max_ra_option_size -= sizeof(struct nd_router_advert);

	return 0;
}

int setup_allrouters_membership(int sock, struct Interface *iface)
{
	struct ipv6_mreq mreq;

	memset(&mreq, 0, sizeof(mreq));
	mreq.ipv6mr_interface = iface->props.if_index;

	/* ipv6-allrouters: ff02::2 */
	mreq.ipv6mr_multiaddr.s6_addr32[0] = htonl(0xFF020000);
	mreq.ipv6mr_multiaddr.s6_addr32[3] = htonl(0x2);

	if (setsockopt(sock, SOL_IPV6, IPV6_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		/* linux-2.6.12-bk4 returns error with HUP signal but keep listening */
		if (errno != EADDRINUSE) {
			flog(LOG_ERR, "can't join ipv6-allrouters on %s", iface->props.name);
			return -1;
		}
	}

	return 0;
}

uint32_t get_interface_linkmtu(const char *iface)
{
	int value;
	FILE *fp = NULL;
	char proc_path[sizeof(PROC_SYS_IP6_LINKMTU) + IFNAMSIZ];

	snprintf(proc_path, sizeof(PROC_SYS_IP6_LINKMTU) + IFNAMSIZ, PROC_SYS_IP6_LINKMTU, iface);

	fp = fopen(proc_path, "r");
	if (fp) {
		int rc = fscanf(fp, "%d", &value);
		if (rc != 1) {
			flog(LOG_ERR, "cannot read value from %s: %s", proc_path, strerror(errno));
			exit(1);
		}
		fclose(fp);
	} else {
		flog(LOG_DEBUG, "Correct IPv6 MTU entry not found, "
				"perhaps the procfs is disabled, "
				"or the kernel interface has changed?");
		value = 1280; /* RFC2460: section 5 */
	}

	return value;
}

int set_interface_linkmtu(const char *iface, uint32_t mtu) { return privsep_interface_linkmtu(iface, mtu); }

int set_interface_curhlim(const char *iface, uint8_t hlim) { return privsep_interface_curhlim(iface, hlim); }

int set_interface_reachtime(const char *iface, uint32_t rtime) { return privsep_interface_reachtime(iface, rtime); }

int set_interface_retranstimer(const char *iface, uint32_t rettimer) { return privsep_interface_retranstimer(iface, rettimer); }

int check_ip6_iface_forwarding(const char *iface)
{
	int value = -1;
	FILE *fp = NULL;
	char path[sizeof(PROC_SYS_IP6_IFACE_FORWARDING) + IFNAMSIZ];

	snprintf(path, sizeof(PROC_SYS_IP6_IFACE_FORWARDING) + IFNAMSIZ, PROC_SYS_IP6_IFACE_FORWARDING, iface);

	fp = fopen(path, "r");
	if (fp) {
		int rc = fscanf(fp, "%d", &value);
		if (rc != 1) {
			flog(LOG_ERR, "cannot read value from %s: %s", path, strerror(errno));
			exit(1);
		}
		fclose(fp);
	} else {
		flog(LOG_DEBUG, "Correct IPv6 forwarding procfs entry for interface "
				"not found, perhaps the procfs is disabled, "
				"or the kernel interface has changed?");
		value = -1;
	}

	return value;
}

int check_ip6_forwarding(void)
{
	int value;
	FILE *fp = NULL;

	fp = fopen(PROC_SYS_IP6_FORWARDING, "r");
	if (fp) {
		int rc = fscanf(fp, "%d", &value);
		if (rc != 1) {
			flog(LOG_ERR, "cannot read value from %s: %s", PROC_SYS_IP6_FORWARDING, strerror(errno));
			exit(1);
		}
		fclose(fp);
	} else {
		flog(LOG_DEBUG, "Correct IPv6 forwarding procfs entry not found, "
				"perhaps the procfs is disabled, "
				"or the kernel interface has changed?");
		value = -1;
	}

#ifdef HAVE_SYSCTL
	int forw_sysctl[] = {SYSCTL_IP6_FORWARDING};
	size_t size = sizeof(value);
	if (!fp && sysctl(forw_sysctl, sizeof(forw_sysctl) / sizeof(forw_sysctl[0]), &value, &size, NULL, 0) < 0) {
		flog(LOG_DEBUG, "Correct IPv6 forwarding sysctl branch not found, "
				"perhaps the kernel interface has changed?");
		return 0; /* this is of advisory value only */
	}
#endif

	/* Linux allows the forwarding value to be either 1 or 2.
	 * https://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/Documentation/networking/ip-sysctl.txt?id=ae8abfa00efb8ec550f772cbd1e1854977d06212#n1078
	 *
	 * The value 2 indicates forwarding is enabled and that *AS* *WELL* router solicitations are being done.
	 *
	 * Which is sometimes used on routers performing RS on their WAN (ppp, etc.) links
	 */
	static int warned = 0;
	if (!warned && value != 1 && value != 2) {
		warned = 1;
		flog(LOG_DEBUG, "IPv6 forwarding setting is: %u, should be 1 or 2", value);
		return -1;
	}

	return 0;
}

static char const *hwstr(unsigned short sa_family)
{
	char const *rc = 0;

	switch (sa_family) {
	case ARPHRD_NETROM:
		rc = "ARPHRD_NETROM";
		break;
	case ARPHRD_ETHER:
		rc = "ARPHRD_ETHER";
		break;
	case ARPHRD_EETHER:
		rc = "ARPHRD_EETHER";
		break;
	case ARPHRD_AX25:
		rc = "ARPHRD_AX25";
		break;
	case ARPHRD_PRONET:
		rc = "ARPHRD_PRONET";
		break;
	case ARPHRD_CHAOS:
		rc = "ARPHRD_CHAOS";
		break;
	case ARPHRD_IEEE802:
		rc = "ARPHRD_IEEE802";
		break;
	case ARPHRD_APPLETLK:
		rc = "ARPHRD_APPLETLK";
		break;
	case ARPHRD_DLCI:
		rc = "ARPHRD_DLCI";
		break;
	case ARPHRD_ATM:
		rc = "ARPHRD_ATM";
		break;
	case ARPHRD_METRICOM:
		rc = "ARPHRD_METRICOM";
		break;
	case ARPHRD_IEEE1394:
		rc = "ARPHRD_IEEE1394";
		break;
	case ARPHRD_EUI64:
		rc = "ARPHRD_EUI64";
		break;
	case ARPHRD_INFINIBAND:
		rc = "ARPHRD_INFINIBAND";
		break;
	case ARPHRD_SLIP:
		rc = "ARPHRD_SLIP";
		break;
	case ARPHRD_CSLIP:
		rc = "ARPHRD_CSLIP";
		break;
	case ARPHRD_SLIP6:
		rc = "ARPHRD_SLIP6";
		break;
	case ARPHRD_CSLIP6:
		rc = "ARPHRD_CSLIP6";
		break;
	case ARPHRD_RSRVD:
		rc = "ARPHRD_RSRVD";
		break;
	case ARPHRD_ADAPT:
		return "ARPHRD_ADAPT";
		break;
	case ARPHRD_ROSE:
		rc = "ARPHRD_ROSE";
		break;
	case ARPHRD_X25:
		rc = "ARPHRD_X25";
		break;
	case ARPHRD_HWX25:
		rc = "ARPHRD_HWX25";
		break;
	case ARPHRD_PPP:
		rc = "ARPHRD_PPP";
		break;
	case ARPHRD_CISCO:
		rc = "ARPHRD_CISCO";
		break;
	case ARPHRD_LAPB:
		rc = "ARPHRD_LAPB";
		break;
	case ARPHRD_DDCMP:
		rc = "ARPHRD_DDCMP";
		break;
	case ARPHRD_RAWHDLC:
		rc = "ARPHRD_RAWHDLC";
		break;
	case ARPHRD_TUNNEL:
		rc = "ARPHRD_TUNNEL";
		break;
	case ARPHRD_TUNNEL6:
		rc = "ARPHRD_TUNNEL6";
		break;
	case ARPHRD_FRAD:
		rc = "ARPHRD_FRAD";
		break;
	case ARPHRD_SKIP:
		rc = "ARPHRD_SKIP";
		break;
	case ARPHRD_LOOPBACK:
		rc = "ARPHRD_LOOPBACK";
		break;
	case ARPHRD_LOCALTLK:
		rc = "ARPHRD_LOCALTLK";
		break;
	case ARPHRD_BIF:
		rc = "ARPHRD_BIF";
		break;
	case ARPHRD_SIT:
		rc = "ARPHRD_SIT";
		break;
	case ARPHRD_IPDDP:
		rc = "ARPHRD_IPDDP";
		break;
	case ARPHRD_IPGRE:
		rc = "ARPHRD_IPGRE";
		break;
	case ARPHRD_PIMREG:
		rc = "ARPHRD_PIMREG";
		break;
	case ARPHRD_HIPPI:
		rc = "ARPHRD_HIPPI";
		break;
	case ARPHRD_ASH:
		rc = "ARPHRD_ASH";
		break;
	case ARPHRD_ECONET:
		rc = "ARPHRD_ECONET";
		break;
	case ARPHRD_IRDA:
		rc = "ARPHRD_IRDA";
		break;
	case ARPHRD_FCPP:
		rc = "ARPHRD_FCPP";
		break;
	case ARPHRD_FCAL:
		rc = "ARPHRD_FCAL";
		break;
	case ARPHRD_FCPL:
		rc = "ARPHRD_FCPL";
		break;
	case ARPHRD_FCFABRIC:
		rc = "ARPHRD_FCFABRIC";
		break;
	case ARPHRD_IEEE802_TR:
		rc = "ARPHRD_IEEE802_TR";
		break;
	case ARPHRD_IEEE80211:
		rc = "ARPHRD_IEEE80211";
		break;
	case ARPHRD_IEEE80211_PRISM:
		rc = "ARPHRD_IEEE80211_PRISM";
		break;
	case ARPHRD_IEEE80211_RADIOTAP:
		rc = "ARPHRD_IEEE80211_RADIOTAP";
		break;
	case ARPHRD_IEEE802154:
		rc = "ARPHRD_IEEE802154";
		break;
#if ARPHRD_IEEE802154_MONITOR
	case ARPHRD_IEEE802154_MONITOR:
		rc = "ARPHRD_IEEE802154_MONITOR";
		break;
#elif ARPHRD_IEEE802154_PHY
	case ARPHRD_IEEE802154_PHY:
		rc = "ARPHRD_IEEE802154_PHY";
		break;
#endif
	case ARPHRD_6LOWPAN:
		rc = "ARPHRD_6LOWPAN";
		break;
	case ARPHRD_VOID:
		rc = "ARPHRD_VOID";
		break;
	case ARPHRD_NONE:
		rc = "ARPHRD_NONE";
		break;
	default:
		rc = "unknown";
		break;
	}

	return rc;
}
