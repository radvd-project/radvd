/*
 *   $Id: includes.h,v 1.6 1999/06/15 21:42:03 lf Exp $
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

#ifndef INCLUDES_H
#define INCLUDES_H

#include <config.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <netdb.h>

#include <sys/types.h>
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/uio.h>

#include <netinet/in.h>

#include <netinet/ip6.h>
#include <netinet/icmp6.h>

#include <arpa/inet.h>

#include <net/if.h>

#ifdef HAVE_NET_IF_DL_H
#include <net/if_dl.h>
#endif
#ifdef HAVE_NET_IF_TYPES_H
#include <net/if_types.h>
#endif
#if defined(HAVE_NET_IF_ARP_H) && !defined(ARPHRD_ETHER)
#include <net/if_arp.h>
#endif /* defined(HAVE_NET_IF_ARP_H) && !defined(ARPHRD_ETHER) */

#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#endif /* INCLUDES_H */
