/*
 *   $Id: pathnames.h,v 1.2 2000/11/26 22:17:12 lf Exp $
 *
 *   Authors:
 *    Pedro Roque		<roque@di.fc.ul.pt>
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

#ifndef PATHNAMES_H
#define PATHNAMES_H

#ifndef PATH_RADVD_CONF
#define PATH_RADVD_CONF "/etc/radvd.conf"
#endif

#ifndef PATH_RADVD_PID
#define PATH_RADVD_PID "/var/run/radvd.pid"
#endif

#ifndef PATH_RADVD_LOG
#define PATH_RADVD_LOG "/var/log/radvd.log"
#endif

#define PATH_PROC_NET_IF_INET6 "/proc/net/if_inet6"

#endif
