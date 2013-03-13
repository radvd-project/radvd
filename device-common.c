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

	if (! iface->UnicastOnly && !(ifr.ifr_flags & IFF_MULTICAST))
	{
		flog(LOG_WARNING, "interface %s does not support multicast",
			iface->Name);
		flog(LOG_WARNING, "   do you need to add the UnicastOnly flag?");
	}

	if (! iface->UnicastOnly && !(ifr.ifr_flags & IFF_BROADCAST))
	{
		flog(LOG_WARNING, "interface %s does not support broadcast",
			iface->Name);
		flog(LOG_WARNING, "   do you need to add the UnicastOnly flag?");
	}

	return 0;
}

