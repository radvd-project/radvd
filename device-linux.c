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
#include "includes.h"
#include "radvd.h"
#include "defaults.h"
#include "pathnames.h"

#ifndef IPV6_ADDR_LINKLOCAL
#define IPV6_ADDR_LINKLOCAL   0x0020U
#endif

static char const *hwstr(unsigned short sa_family);

/*
 * this function gets the hardware type and address of an interface,
 * determines the link layer token length and checks it against
 * the defined prefixes
 */
int update_device_info(int sock, struct Interface *iface)
{
	struct ifreq ifr;
	struct AdvPrefix *prefix;
	char zero[sizeof(iface->if_addr)];
	char hwaddr[3 * 6];

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, iface->Name, IFNAMSIZ - 1);

	if (ioctl(sock, SIOCGIFMTU, &ifr) < 0) {
		flog(LOG_ERR, "ioctl(SIOCGIFMTU) failed for %s: %s", iface->Name, strerror(errno));
		return -1;
	}

	iface->if_maxmtu = ifr.ifr_mtu;
	dlog(LOG_DEBUG, 3, "mtu for %s is %d", iface->Name, ifr.ifr_mtu);

	if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
		flog(LOG_ERR, "ioctl(SIOCGIFHWADDR) failed for %s: %s", iface->Name, strerror(errno));
		return -1;
	}

	dlog(LOG_DEBUG, 3, "hardware type for %s is %s", iface->Name, hwstr(ifr.ifr_hwaddr.sa_family));

	switch (ifr.ifr_hwaddr.sa_family) {
	case ARPHRD_ETHER:
		iface->if_hwaddr_len = 48;
		iface->if_prefix_len = 64;
		sprintf(hwaddr, "%02x:%02x:%02x:%02x:%02x:%02x",
			(unsigned char)ifr.ifr_hwaddr.sa_data[0],
			(unsigned char)ifr.ifr_hwaddr.sa_data[1],
			(unsigned char)ifr.ifr_hwaddr.sa_data[2],
			(unsigned char)ifr.ifr_hwaddr.sa_data[3], (unsigned char)ifr.ifr_hwaddr.sa_data[4], (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
		dlog(LOG_DEBUG, 3, "hardware address is %s", hwaddr);
		break;

#ifdef ARPHRD_FDDI
	case ARPHRD_FDDI:
		iface->if_hwaddr_len = 48;
		iface->if_prefix_len = 64;
		break;
#endif

#ifdef ARPHRD_ARCNET
	case ARPHRD_ARCNET:
		iface->if_hwaddr_len = 8;
		iface->if_prefix_len = -1;
		iface->if_maxmtu = -1;
		break;
#endif

	default:
		iface->if_hwaddr_len = -1;
		iface->if_prefix_len = -1;
		iface->if_maxmtu = -1;
		break;
	}

	dlog(LOG_DEBUG, 3, "link layer token length for %s is %d", iface->Name, iface->if_hwaddr_len);
	dlog(LOG_DEBUG, 3, "prefix length for %s is %d", iface->Name, iface->if_prefix_len);

	if (iface->if_hwaddr_len != -1) {
		unsigned int if_hwaddr_len_bytes = (iface->if_hwaddr_len + 7) >> 3;

		if (if_hwaddr_len_bytes > sizeof(iface->if_hwaddr)) {
			flog(LOG_ERR, "address length %d too big for %s", if_hwaddr_len_bytes, iface->Name);
			return (-2);
		}
		memcpy(iface->if_hwaddr, ifr.ifr_hwaddr.sa_data, if_hwaddr_len_bytes);

		memset(zero, 0, sizeof(zero));
		if (!memcmp(iface->if_hwaddr, zero, if_hwaddr_len_bytes))
			flog(LOG_WARNING, "WARNING, MAC address on %s is all zero!", iface->Name);
	}

	prefix = iface->AdvPrefixList;
	while (prefix) {
		if ((iface->if_prefix_len != -1) && (iface->if_prefix_len != prefix->PrefixLen)) {
			flog(LOG_WARNING, "prefix length should be %d for %s", iface->if_prefix_len, iface->Name);
		}

		prefix = prefix->next;
	}

	return 0;
}

int setup_allrouters_membership(int sock, struct Interface *iface)
{
	struct ipv6_mreq mreq;

	memset(&mreq, 0, sizeof(mreq));
	mreq.ipv6mr_interface = iface->if_index;

	/* ipv6-allrouters: ff02::2 */
	mreq.ipv6mr_multiaddr.s6_addr32[0] = htonl(0xFF020000);
	mreq.ipv6mr_multiaddr.s6_addr32[3] = htonl(0x2);

	if (setsockopt(sock, SOL_IPV6, IPV6_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		/* linux-2.6.12-bk4 returns error with HUP signal but keep listening */
		if (errno != EADDRINUSE) {
			flog(LOG_ERR, "can't join ipv6-allrouters on %s", iface->Name);
			return -1;
		}
	}

	return (0);
}

/* TODO: Is check_allrouters_membership needed? */
/* TODO: I don't like how this reads files.  Can we do this with getsockopt? */
int check_allrouters_membership(int sock, struct Interface *iface)
{
#define ALL_ROUTERS_MCAST "ff020000000000000000000000000002"

	FILE *fp;
	unsigned int if_idx, allrouters_ok = 0;
	char addr[32 + 1];
	char buffer[301] = { "" }, *str;
	int ret = 0;

	if ((fp = fopen(PATH_PROC_NET_IGMP6, "r")) == NULL) {
		flog(LOG_ERR, "can't open %s: %s", PATH_PROC_NET_IGMP6, strerror(errno));
		return -1;
	}

	str = fgets(buffer, 300, fp);

	while (str && (ret = sscanf(str, "%u %*s %32[0-9A-Fa-f]", &if_idx, addr))) {
		if (ret == 2) {
			if (iface->if_index == if_idx) {
				if (strncmp(addr, ALL_ROUTERS_MCAST, sizeof(addr)) == 0) {
					allrouters_ok = 1;
					break;
				}
			}
		}
		str = fgets(buffer, 300, fp);
	}

	fclose(fp);

	if (!allrouters_ok) {
		flog(LOG_WARNING, "resetting ipv6-allrouters membership on %s", iface->Name);
		return setup_allrouters_membership(sock, iface);
	}

	return (0);
}

/* note: also called from the root context */
int set_interface_var(const char *iface, const char *var, const char *name, uint32_t val)
{
	FILE *fp;
	char spath[64 + IFNAMSIZ];	/* XXX: magic constant */
	if (snprintf(spath, sizeof(spath), var, iface) >= sizeof(spath))
		return -1;

	/* No path traversal */
	if (!iface[0] || !strcmp(iface, ".") || !strcmp(iface, "..") || strchr(iface, '/'))
		return -1;

	if (access(spath, F_OK) != 0)
		return -1;

	fp = fopen(spath, "w");
	if (!fp) {
		if (name)
			flog(LOG_ERR, "failed to set %s (%u) for %s: %s", name, val, iface, strerror(errno));
		return -1;
	}
	fprintf(fp, "%u", val);
	fclose(fp);

	return 0;
}

int set_interface_linkmtu(const char *iface, uint32_t mtu)
{
	return privsep_interface_linkmtu(iface, mtu);
}

int set_interface_curhlim(const char *iface, uint8_t hlim)
{
	return privsep_interface_curhlim(iface, hlim);
}

int set_interface_reachtime(const char *iface, uint32_t rtime)
{
	return privsep_interface_reachtime(iface, rtime);
}

int set_interface_retranstimer(const char *iface, uint32_t rettimer)
{
	return privsep_interface_retranstimer(iface, rettimer);
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
	case ARPHRD_IEEE802154_PHY:
		rc = "ARPHRD_IEEE802154_PHY";
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
