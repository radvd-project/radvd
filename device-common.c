/*
 *
 *   Authors:
 *    Lars Fenneberg	<lf@elemental.net>
 *    Pekka Savola		<pekkas@netcore.fi>
 *    Craig Metz		<cmetz@inner.net>
 *    Jim Paris			<jim@jtan.com>
 *    Marko Myllynen	<myllynen@lut.fi>
 *    Pedro Roque		<roque@di.fc.ul.pt>
 *    Reuben Hawkins	<reubenhwk@gmail.com>
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

int
check_device(struct Interface *iface)
{
	struct ifreq	ifr;

	strncpy(ifr.ifr_name, iface->Name, IFNAMSIZ-1);
	ifr.ifr_name[IFNAMSIZ-1] = '\0';

	if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0)
	{
		if (!iface->IgnoreIfMissing)
			flog(LOG_ERR, "ioctl(SIOCGIFFLAGS) failed for %s: %s",
				iface->Name, strerror(errno));
		return (-1);
	}

	if (!(ifr.ifr_flags & IFF_UP))
	{
		if (!iface->IgnoreIfMissing)
                	flog(LOG_ERR, "interface %s is not UP", iface->Name);
		return (-1);
	}

	if (!(ifr.ifr_flags & IFF_RUNNING))
	{
		if (!iface->IgnoreIfMissing)
                	flog(LOG_ERR, "interface %s is not RUNNING", iface->Name);
		return (-1);
	}

	if (!iface->UnicastOnly && !(ifr.ifr_flags & IFF_MULTICAST))
	{
		flog(LOG_INFO, "interface %s does not support multicast, forcing UnicastOnly", iface->Name);
		iface->UnicastOnly = 1;
	}

	return 0;
}


/*
 * Saves the first link local address seen on the specified interface to iface->if_addr
 *
 */
int setup_linklocal_addr(struct Interface *iface)
{
	struct ifaddrs *addresses = 0;
	struct ifaddrs const *ifa;

	if (getifaddrs(&addresses) != 0)
	{
		flog(LOG_ERR, "getifaddrs failed: %s(%d)", strerror(errno), errno);
		goto ret;
	}

	for (ifa = addresses; ifa; ifa = ifa->ifa_next)
	{
		if (!ifa->ifa_addr)
			continue;

		if (ifa->ifa_addr->sa_family != AF_INET6)
			continue;

		struct sockaddr_in6 const *a6 = (struct sockaddr_in6 const*)ifa->ifa_addr;

		if (!IN6_IS_ADDR_LINKLOCAL(&a6->sin6_addr))
			continue;

		if (strcmp(ifa->ifa_name, iface->Name) != 0)
			continue;

		memcpy(&iface->if_addr, &(a6->sin6_addr), sizeof(iface->if_addr));

		freeifaddrs(addresses);

		dlog(LOG_INFO, 4, "linklocal address configured for %s", iface->Name);

		return 0;
	}

ret:

	if(addresses)
		freeifaddrs(addresses);

	flog(LOG_ERR, "no linklocal address configured for %s", iface->Name);

	return -1;
}


