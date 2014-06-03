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

int check_device(struct Interface *iface)
{
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, iface->Name, IFNAMSIZ - 1);

	if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
		if (!iface->IgnoreIfMissing)
			flog(LOG_ERR, "ioctl(SIOCGIFFLAGS) failed for %s: %s", iface->Name, strerror(errno));
		return -1;
	}

	if (!(ifr.ifr_flags & IFF_UP)) {
		if (!iface->IgnoreIfMissing)
			flog(LOG_ERR, "interface %s is not UP", iface->Name);
		return -1;
	}

	if (!(ifr.ifr_flags & IFF_RUNNING)) {
		if (!iface->IgnoreIfMissing)
			flog(LOG_ERR, "interface %s is not RUNNING", iface->Name);
		return -1;
	}

	if (!iface->UnicastOnly && !(ifr.ifr_flags & IFF_MULTICAST)) {
		flog(LOG_INFO, "interface %s does not support multicast, forcing UnicastOnly", iface->Name);
		iface->UnicastOnly = 1;
	}

	return 0;
}

int get_v4addr(const char *ifn, unsigned int *dst)
{

	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		flog(LOG_ERR, "create socket for IPv4 ioctl failed for %s: %s", ifn, strerror(errno));
		return -1;
	}

	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifn, IFNAMSIZ - 1);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	ifr.ifr_addr.sa_family = AF_INET;

	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
		flog(LOG_ERR, "ioctl(SIOCGIFADDR) failed for %s: %s", ifn, strerror(errno));
		close(fd);
		return -1;
	}

	struct sockaddr_in *addr = (struct sockaddr_in *)(&ifr.ifr_addr);

	dlog(LOG_DEBUG, 3, "IPv4 address for %s is %s", ifn, inet_ntoa(addr->sin_addr));

	*dst = addr->sin_addr.s_addr;

	close(fd);

	return 0;
}

int check_ip6_forwarding(void)
{
	int value;
	FILE *fp = NULL;

#ifdef __linux__
	fp = fopen(PROC_SYS_IP6_FORWARDING, "r");
	if (fp) {
		int rc = fscanf(fp, "%d", &value);
		if (rc != 1) {
			flog(LOG_ERR, "cannot read value from %s: %s", PROC_SYS_IP6_FORWARDING, strerror(errno));
			exit(1);
		}
		fclose(fp);
	} else {
		flog(LOG_DEBUG,
		     "Correct IPv6 forwarding procfs entry not found, " "perhaps the procfs is disabled, "
		     "or the kernel interface has changed?");
		value = -1;
	}
#endif				/* __linux__ */

#ifdef HAVE_SYS_SYSCTL_H
	int forw_sysctl[] = { SYSCTL_IP6_FORWARDING };
	size_t size = sizeof(value);
	if (!fp && sysctl(forw_sysctl, sizeof(forw_sysctl) / sizeof(forw_sysctl[0]), &value, &size, NULL, 0) < 0) {
		flog(LOG_DEBUG,
		     "Correct IPv6 forwarding sysctl branch not found, " "perhaps the kernel interface has changed?");
		return 0;	/* this is of advisory value only */
	}
#endif

	/* Linux allows the forwarding value to be either 1 or 2.
	 * https://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/Documentation/networking/ip-sysctl.txt?id=ae8abfa00efb8ec550f772cbd1e1854977d06212#n1078
	 *
	 * The value 2 indicates forwarding is enabled and that *AS* *WELL* router solicitions are being done.
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
