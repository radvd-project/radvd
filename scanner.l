/*
 *   $Id: scanner.l,v 1.1 1997/10/14 17:17:40 lf Exp $
 *
 *   Authors:
 *    Pedro Roque		<roque@di.fc.ul.pt>
 *    Lars Fenneberg		<lf@elemental.net>	 
 *
 *   This software is Copyright 1996 by the above mentioned author(s), 
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <lf@elemental.net>.
 *
 */
%{
#include <config.h>
#include <includes.h>
#include <radvd.h>
#include <y.tab.h>

extern conf_file;

int num_lines = 1;
%}

digit		[0-9]
number		({digit})+
hexdigit	([a-f]|[A-F]|[0-9])
addr1		{hexdigit}{1,4}":"({hexdigit}{1,4}":")*(":"{hexdigit}{1,4})+
addr2		({hexdigit}{1,4}":"){7}{hexdigit}
addr		({addr1}|{addr2})
whitespace	([ \t])+
string		[a-z]([a-z]|{digit})*
%%

#.*$			{/* ignore comments */}
\n			{num_lines++;}
{whitespace}		{}

interface		{ return T_INTERFACE; }
prefix			{ return T_PREFIX; }

AdvSendAdvert		{ return T_AdvSendAdvert; }
MaxRtrAdvInterval	{ return T_MaxRtrAdvInterval; }
MinRtrAdvInterval	{ return T_MinRtrAdvInterval; }
AdvManagedFlag		{ return T_AdvManagedFlag; }
AdvOtherConfigFlag	{ return T_AdvOtherConfigFlag; }
AdvLinkMTU		{ return T_AdvLinkMTU; }
AdvReachableTime	{ return T_AdvReachableTime; }
AdvRetransTimer		{ return T_AdvRetransTimer; }
AdvCurHopLimit		{ return T_AdvCurHopLimit; }
AdvDefaultLifetime	{ return T_AdvDefaultLifetime; }
AdvSourceLLAddress	{ return T_AdvSourceLLAddress; }

AdvOnLink		{ return T_AdvOnLink; }
AdvAutonomous		{ return T_AdvAutonomous; }
AdvValidLifetime	{ return T_AdvValidLifetime; }
AdvPreferredLifetime	{ return T_AdvPreferredLifetime; }

{addr}		{
			static struct in6_addr addr;
			int i;
				
			i = inet_pton(AF_INET6, yytext, &addr);

			dlog(LOG_DEBUG, 4, "inet_pton returned %d", i);

			/* BSD API draft and NRL's code don't aggree on
			 * this. the draft specifies a return value of 1 on 
			 * success, NRL's code returns the address length in 
			 * bytes on success (16 for an IPv6 address)
			 */
			if (i < 1) {
				log(LOG_ERR, "invalid address in %s, line %d", conf_file,
					num_lines);
				return T_BAD_TOKEN;
			}

			yylval.addr = &addr;
			return IPV6ADDR;
		}

{number}	{ yylval.num = atoi(yytext); return NUMBER; }

infinity	{ return INFINITY; }

on			{ yylval.bool = 1; return SWITCH; }

off			{ yylval.bool = 0; return SWITCH; }

{string}	{
			static char name[IFNAMSIZ];
				
			strcpy(name, yytext);
			yylval.str = name;
			return STRING;
		}

"{"|"}"|";"|"/"	{ return *yytext; }

.		{ return T_BAD_TOKEN; }
%%
