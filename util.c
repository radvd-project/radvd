/*
 *   $Id: util.c,v 1.1 1997/10/14 17:17:40 lf Exp $
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
               
void
mdelay(int msecs)
{
	struct timeval tv;
                
	tv.tv_sec = (int) msecs / 1000;
	tv.tv_usec = (msecs % 1000) * 1000;
                                
	select(0,(fd_set *)NULL,(fd_set *)NULL,(fd_set *)NULL, &tv);
}

int
rand_between(int lower, int upper)
{
	return  (lower+(int)((double)(upper-lower+1)*rand()/(RAND_MAX+1.0)));
}

void
print_addr(struct in6_addr *addr, char *str)
{
	const char *res;

	res = inet_ntop(AF_INET6, (void *)addr, str, INET6_ADDRSTRLEN);
	
	if (res == NULL) 
	{
		strcpy(str, "[invalid address]");	
	}
}
