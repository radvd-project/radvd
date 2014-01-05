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

int check_device(struct Interface *iface)
{
	struct ifreq ifr;

	strncpy(ifr.ifr_name, iface->Name, IFNAMSIZ - 1);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';

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
	struct ifreq ifr;
	struct sockaddr_in *addr;
	int fd;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		flog(LOG_ERR, "create socket for IPv4 ioctl failed for %s: %s", ifn, strerror(errno));
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifn, IFNAMSIZ - 1);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	ifr.ifr_addr.sa_family = AF_INET;

	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
		flog(LOG_ERR, "ioctl(SIOCGIFADDR) failed for %s: %s", ifn, strerror(errno));
		close(fd);
		return -1;
	}

	addr = (struct sockaddr_in *)(&ifr.ifr_addr);

	dlog(LOG_DEBUG, 3, "IPv4 address for %s is %s", ifn, inet_ntoa(addr->sin_addr));

	*dst = addr->sin_addr.s_addr;

	close(fd);

	return 0;
}

/*
 * Saves the first link local address seen on the specified interface to iface->if_addr
 *
 */
int setup_linklocal_addr(struct Interface *iface)
{
	struct ifaddrs *addresses = 0, *ifa;
	uint8_t ll_prefix[] = { 0xfe, 0x80 };

	if (getifaddrs(&addresses) != 0) {
		flog(LOG_ERR, "getifaddrs failed: %s(%d)", strerror(errno), errno);
	} else {
		for (ifa = addresses; ifa != NULL; ifa = ifa->ifa_next) {

			if (!ifa->ifa_addr)
				continue;

			/* TODO: We use to set the iface->if_index here, make sure
			 * it's getting set somewhere. (preferable with if_nametoindex because
			 * it's so easy to read that way) */

			if (ifa->ifa_addr->sa_family != AF_INET6)
				continue;

			struct sockaddr_in6 *a6 = (struct sockaddr_in6 *)ifa->ifa_addr;

			/* Skip if it is not a linklocal address */
			if (memcmp(&(a6->sin6_addr), ll_prefix, sizeof(ll_prefix)) != 0)
				continue;

			/* Skip if it is not the interface we're looking for. */
			/* TODO: Can this be made to check the ifa_index instead? */
			if (strcmp(ifa->ifa_name, iface->Name) != 0)
				continue;

			memcpy(&iface->if_addr, &(a6->sin6_addr), sizeof(struct in6_addr));

			freeifaddrs(addresses);

			return 0;
		}
	}

	if (addresses)
		freeifaddrs(addresses);

	if (iface->IgnoreIfMissing)
		dlog(LOG_DEBUG, 4, "no linklocal address configured for %s", iface->Name);
	else
		flog(LOG_ERR, "no linklocal address configured for %s", iface->Name);

	iface->if_index = 0;

	return -1;
}
