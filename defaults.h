/*
 *   $Id: defaults.h,v 1.1 1997/10/14 17:17:40 lf Exp $
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

#ifndef DEFAULTS_H
#define DEFAULTS_H

#include <config.h>
#include <includes.h>
#include <radvd.h>

#define DFLT_MaxRtrAdvInterval		600
#define DFLT_MinRtrAdvInterval(iface)	((int)(0.33 * (double)(iface)->MaxRtrAdvInterval))
#define	DFLT_AdvDefaultLifetime(iface)	(3 * (iface)->MaxRtrAdvInterval)
#define DFLT_AdvLinkMTU			0
#define DFLT_AdvSourceLLAddress		1
#define DFLT_AdvReachableTime		0
#define DFLT_AdvRetransTimer		0
#define DFLT_AdvCurHopLimit		64	/* as per RFC 1700 or the 
						   next incarnation of it :) */

#define DFLT_AdvOnLinkFlag		1
#define DFLT_AdvAutonomousFlag		1
#define DFLT_AdvValidLifetime		(~(u_int32_t)0)
#define DFLT_AdvPreferredLifetime	604800

#define MIN_MinRtrAdvInterval		3
#define MAX_MinRtrAdvInterval(iface)	((int)(0.75 * (double)(iface)->MaxRtrAdvInterval))

#define MIN_MaxRtrAdvInterval		4
#define MAX_MaxRtrAdvInterval		1800

#define MIN_AdvDefaultLifetime(iface)	((iface)->MaxRtrAdvInterval)
#define MAX_AdvDefaultLifetime		9000

#define	MIN_AdvLinkMTU			576

#define MAX_AdvReachableTime		3600000 /* 1 hour in milliseconds */

#define MAX_AdvCurHopLimit		255

#define MAX_PrefixLen			128

#endif
