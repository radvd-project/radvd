/*
 *   $Id: device-common.c,v 1.1 1997/10/14 17:17:40 lf Exp $
 *
 *   Authors:
 *    Lars Fenneberg		<lf@elemental.net>	 
 *
 *   This software is Copyright 1996,1997 by the above mentioned author(s), 
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <lf@elemental.net>.
 *
 */

#include <config.h>
#include <includes.h>
#include <radvd.h>
#include <defaults.h>

int
check_device(int sock, struct Interface *iface)
{
	struct ifreq	ifr;
	
	strcpy(ifr.ifr_name, iface->Name);
	
	if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0)
	{
		log(LOG_ERR, "ioctl(SIOCGIFFLAGS) failed for %s: %s", 
			iface->Name, strerror(errno));
		return (-1);
	}

	if (!(ifr.ifr_flags & IFF_UP))
	{
		log(LOG_ERR, "interface %s is not UP", iface->Name);
		return (-1);
	}
	
	if (!(ifr.ifr_flags & IFF_MULTICAST))
	{
		log(LOG_WARNING, "interface %s does not support multicast",
			iface->Name);
	}

	if (!(ifr.ifr_flags & IFF_BROADCAST))
	{
		log(LOG_WARNING, "interface %s does not support broadcast",
			iface->Name);
	}

	return 0;
}
