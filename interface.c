/*
 *   $Id: interface.c,v 1.1 1997/10/14 17:17:40 lf Exp $
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

void
iface_init_defaults(struct Interface *iface)
{
	memset(iface, 0, sizeof(struct Interface));

	iface->MaxRtrAdvInterval  = DFLT_MaxRtrAdvInterval;
	iface->AdvSourceLLAddress = DFLT_AdvSourceLLAddress;
	iface->AdvReachableTime	  = DFLT_AdvReachableTime;
	iface->AdvRetransTimer    = DFLT_AdvRetransTimer;
	iface->AdvLinkMTU	  = DFLT_AdvLinkMTU;
	iface->AdvCurHopLimit	  = DFLT_AdvCurHopLimit;

	iface->MinRtrAdvInterval  = -1;
	iface->AdvDefaultLifetime = -1;
}

void
prefix_init_defaults(struct AdvPrefix *prefix)
{
	memset(prefix, 0, sizeof(struct AdvPrefix));
		
	prefix->AdvOnLinkFlag = DFLT_AdvOnLinkFlag;
	prefix->AdvAutonomousFlag = DFLT_AdvAutonomousFlag;
	prefix->AdvValidLifetime = DFLT_AdvValidLifetime;
	prefix->AdvPreferredLifetime = DFLT_AdvPreferredLifetime;
}

int
check_iface(struct Interface *iface)
{
	struct AdvPrefix *prefix;
	int res = 0;

	if (iface->MinRtrAdvInterval == -1)
		iface->MinRtrAdvInterval = DFLT_MinRtrAdvInterval(iface);

	if ((iface->MinRtrAdvInterval < MIN_MinRtrAdvInterval) || 
	    (iface->MinRtrAdvInterval > MAX_MinRtrAdvInterval(iface)))
	{
		log(LOG_ERR, 
			"MinRtrAdvInterval must be between %d and %d for %s",
			 MIN_MinRtrAdvInterval,  MAX_MinRtrAdvInterval(iface),
			 iface->Name);
		res = -1;
	}

	if ((iface->MaxRtrAdvInterval < MIN_MaxRtrAdvInterval) 
		|| (iface->MaxRtrAdvInterval > MAX_MaxRtrAdvInterval))
	{
		log(LOG_ERR, 
			"MaxRtrAdvInterval must be between %d and %d for %s",
			MIN_MaxRtrAdvInterval, MAX_MaxRtrAdvInterval, iface->Name);
		res = -1;
	}

	if (iface->if_maxmtu != -1)
	{
		if ((iface->AdvLinkMTU != 0) &&
		   ((iface->AdvLinkMTU < MIN_AdvLinkMTU) || 
		   (iface->AdvLinkMTU > iface->if_maxmtu)))
		{
			log(LOG_ERR,  "AdvLinkMTU must be zero or between %d and %d for %s",
			MIN_AdvLinkMTU, iface->if_maxmtu, iface->Name);
			res = -1;
		}
	}
	else
	{
		if ((iface->AdvLinkMTU != 0) 
			&& (iface->AdvLinkMTU < MIN_AdvLinkMTU))
		{
			log(LOG_ERR,  "AdvLinkMTU must be zero or greater than %d for %s",
			MIN_AdvLinkMTU, iface->Name);
			res = -1;
		}
	}

	if (iface->AdvReachableTime >  MAX_AdvReachableTime)
	{
		log(LOG_ERR, 
			"AdvReachableTime must be less than %d for %s",
			MAX_AdvReachableTime, iface->Name);
		res = -1;
	}

	if (iface->AdvCurHopLimit > MAX_AdvCurHopLimit)
	{
		log(LOG_ERR, "AdvCurHopLimit must not be greater than %d for %s",
			MAX_AdvCurHopLimit, iface->Name);
		res = -1;
	}
	
	if (iface->AdvDefaultLifetime == -1)
	{
		iface->AdvDefaultLifetime = DFLT_AdvDefaultLifetime(iface);
	}

	if ((iface->AdvDefaultLifetime != 0) &&
	   ((iface->AdvDefaultLifetime > MAX_AdvDefaultLifetime) ||
	    (iface->AdvDefaultLifetime < MIN_AdvDefaultLifetime(iface))))
	{
		log(LOG_ERR, 
			"AdvDefaultLifetime must be zero or between %d and %d for %s",
			MIN_AdvDefaultLifetime(iface), MAX_AdvDefaultLifetime,
			iface->Name);
		res = -1;
	}
	
	prefix = iface->AdvPrefixList;	
	while (prefix)
	{
		if (prefix->PrefixLen > MAX_PrefixLen)
		{
			log(LOG_ERR, "invalid prefix length for %s", iface->Name);
			res = -1;
		}

		if (prefix->AdvPreferredLifetime > prefix->AdvValidLifetime)
		{
			log(LOG_ERR, "AdvValidLifetime must be "
				"greater than AdvPreferredLifetime for %s", 
				iface->Name);
			res = -1;
		}

		prefix = prefix->next;
	}

	return res;
}
