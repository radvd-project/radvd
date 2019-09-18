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

#pragma once

#include "config.h"

#include <errno.h>
#include <grp.h>
#include <netdb.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#else
#ifdef HAVE_MACHINE_PARAM_H
#include <machine/param.h>
#endif
#ifdef HAVE_MACHINE_LIMITS_H
#include <machine/limits.h>
#endif
#endif

#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/uio.h>

#include <fcntl.h>
#include <sys/stat.h>

#include <netinet/in.h>

#include <netinet/icmp6.h>
#include <netinet/ip6.h>

#include <arpa/inet.h>

#ifdef HAVE_SYSCTL
#include <sys/sysctl.h>
#endif

#if !defined(__GLIBC__) && defined(linux)
#include <linux/if.h>
#define IF_NAMESIZE IFNAMSIZ
#else
#include <net/if.h>
#endif

#ifdef HAVE_NET_IF_DL_H
#include <net/if_dl.h>
#endif

#ifdef HAVE_NET_IF_TYPES_H
#include <net/if_types.h>
#endif

#if defined(HAVE_NET_IF_ARP_H) && !defined(ARPHRD_ETHER) && !defined(HAVE_LINUX_IF_ARP_H)
#include <net/if_arp.h>
#endif /* defined(HAVE_NET_IF_ARP_H) && !defined(ARPHRD_ETHER) */

#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif

#ifdef HAVE_LINUX_IF_ARP_H
#include <linux/if_arp.h>
#endif
