/*
 *   $Id: gram.y,v 1.6 2002/01/02 11:01:11 psavola Exp $
 *
 *   Authors:
 *    Pedro Roque		<roque@di.fc.ul.pt>
 *    Lars Fenneberg		<lf@elemental.net>	 
 *
 *   This software is Copyright 1996-2000 by the above mentioned author(s), 
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <lutchann@litech.org>.
 *
 */
%{
#include <config.h>
#include <includes.h>
#include <radvd.h>
#include <defaults.h>

extern struct Interface *IfaceList;
struct Interface *iface = NULL;
struct AdvPrefix *prefix = NULL;

extern char *conf_file;
extern int num_lines;
extern char *yytext;
extern int sock;

static void cleanup(void);
static void yyerror(char *msg);

#if 0 /* no longer necessary? */
#ifndef HAVE_IN6_ADDR_S6_ADDR
# ifdef __FreeBSD__
#  define s6_addr32 __u6_addr.__u6_addr32
#  define s6_addr16 __u6_addr.__u6_addr16
# endif
#endif
#endif

#define ABORT	do { cleanup(); YYABORT; } while (0);

%}

%token		T_INTERFACE
%token		T_PREFIX

%token	<str>	STRING
%token	<num>	NUMBER
%token	<snum>	SIGNEDNUMBER
%token	<dec>	DECIMAL
%token	<bool>	SWITCH
%token	<addr>	IPV6ADDR
%token 		INFINITY

%token		T_AdvSendAdvert
%token		T_MaxRtrAdvInterval
%token		T_MinRtrAdvInterval
%token		T_AdvManagedFlag
%token		T_AdvOtherConfigFlag
%token		T_AdvLinkMTU
%token		T_AdvReachableTime
%token		T_AdvRetransTimer
%token		T_AdvCurHopLimit
%token		T_AdvDefaultLifetime
%token		T_AdvSourceLLAddress

%token		T_AdvOnLink
%token		T_AdvAutonomous
%token		T_AdvValidLifetime
%token		T_AdvPreferredLifetime

%token		T_AdvRouterAddr
%token		T_AdvHomeAgentFlag
%token		T_AdvIntervalOpt
%token		T_AdvHomeAgentInfo

%token		T_Base6to4Interface
%token		T_UnicastOnly

%token		T_HomeAgentPreference
%token		T_HomeAgentLifetime

%token		T_BAD_TOKEN

%type	<str>	name
%type	<pinfo> prefixdef prefixlist
%type   <num>	number_or_infinity

%union {
	int			num;
	int			snum;
	double			dec;
	int			bool;
	struct in6_addr		*addr;
	char			*str;
	struct AdvPrefix	*pinfo;
};

%%

grammar		: grammar ifacedef
		| ifacedef
		;

ifacedef	: ifacehead '{' ifaceparams  '}' ';'
		{
			struct Interface *iface2;

			iface2 = IfaceList;
			while (iface2)
			{
				if (!strcmp(iface2->Name, iface->Name))
				{
					log(LOG_ERR, "duplicate interface "
						"definition for %s", iface->Name);

					ABORT;
				}
				iface2 = iface2->next;
			}			

			if (check_device(sock, iface) < 0)
				ABORT;
			if (setup_deviceinfo(sock, iface) < 0)
				ABORT;
			if (check_iface(iface) < 0)
				ABORT;
			if (setup_linklocal_addr(sock, iface) < 0)
				ABORT;
			if (setup_allrouters_membership(sock, iface) < 0)
				ABORT;

			iface->next = IfaceList;
			IfaceList = iface;

			dlog(LOG_DEBUG, 4, "interface definition for %s is ok", iface->Name);

			iface = NULL;
		};

ifacehead	: T_INTERFACE name
		{
			iface = malloc(sizeof(struct Interface));

			if (iface == NULL) {
				log(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			iface_init_defaults(iface);
			strncpy(iface->Name, $2, IFNAMSIZ-1);
			iface->Name[IFNAMSIZ-1] = '\0';
		}
		;
	
name		: STRING
		{
			/* check vality */
			$$ = $1;
		}
		;

ifaceparams	: optional_ifacevlist prefixlist
		{
			iface->AdvPrefixList = $2;
		}
		;

optional_ifacevlist: /* empty */
		   | ifacevlist
		   ;

ifacevlist	: ifacevlist ifaceval
		| ifaceval
		;

ifaceval	: T_MinRtrAdvInterval NUMBER ';'
		{
			iface->MinRtrAdvInterval = $2;
		}
		| T_MaxRtrAdvInterval NUMBER ';'
		{
			iface->MaxRtrAdvInterval = $2;
		}
		| T_MinRtrAdvInterval DECIMAL ';'
		{
			iface->MinRtrAdvInterval = $2;
		}
		| T_MaxRtrAdvInterval DECIMAL ';'
		{
			iface->MaxRtrAdvInterval = $2;
		}
		| T_AdvSendAdvert SWITCH ';'
		{
			iface->AdvSendAdvert = $2;
		}
		| T_AdvManagedFlag SWITCH ';'
		{
			iface->AdvManagedFlag = $2;
		}
		| T_AdvOtherConfigFlag SWITCH ';'
		{
			iface->AdvOtherConfigFlag = $2;
		}
		| T_AdvLinkMTU NUMBER ';'
		{
			iface->AdvLinkMTU = $2;
		}
		| T_AdvReachableTime NUMBER ';'
		{
			iface->AdvReachableTime = $2;
		}
		| T_AdvRetransTimer NUMBER ';'
		{
			iface->AdvRetransTimer = $2;
		}
		| T_AdvDefaultLifetime NUMBER ';'
		{
			iface->AdvDefaultLifetime = $2;
		}
		| T_AdvCurHopLimit NUMBER ';'
		{
			iface->AdvCurHopLimit = $2;
		}
		| T_AdvSourceLLAddress SWITCH ';'
		{
			iface->AdvSourceLLAddress = $2;
		}
		| T_AdvIntervalOpt SWITCH ';'
		{
			iface->AdvIntervalOpt = $2;
		}
		| T_AdvHomeAgentInfo SWITCH ';'
		{
			iface->AdvHomeAgentInfo = $2;
		}
		| T_AdvHomeAgentFlag SWITCH ';'
		{
			iface->AdvHomeAgentFlag = $2;
		}
		| T_HomeAgentPreference NUMBER ';'
		{
			iface->HomeAgentPreference = $2;
		}
		| T_HomeAgentPreference SIGNEDNUMBER ';'
		{
			iface->HomeAgentPreference = $2;
		}
		| T_HomeAgentLifetime NUMBER ';'
		{
			iface->HomeAgentLifetime = $2;
		}
		| T_UnicastOnly SWITCH ';'
		{
			iface->UnicastOnly = $2;
		}
		;
		
prefixlist	: prefixdef
		{
			$$ = $1;
		}
		| prefixlist prefixdef
		{
			$2->next = $1;
			$$ = $2;
		}
		;

prefixdef	: prefixhead '{' optional_prefixplist '}' ';'
		{
			unsigned int dst;

			if (prefix->AdvPreferredLifetime >
			    prefix->AdvValidLifetime)
			{
				log(LOG_ERR, "AdvValidLifeTime must be "
					"greater than AdvPreferredLifetime in %s, line %d", 
					conf_file, num_lines);
				ABORT;
			}

			if( prefix->if6to4[0] )
			{
				if (get_v4addr(prefix->if6to4, &dst) < 0)
				{
					log(LOG_ERR, "interface %s has no IPv4 addresses, disabling 6to4 prefix", prefix->if6to4 );
					prefix->enabled = 0;
				} else
				{
					*((uint32_t *)(prefix->Prefix.s6_addr)) = htons(0x2002);
					memcpy( prefix->Prefix.s6_addr + 2, &dst, sizeof( dst ) );
				}
			}

			$$ = prefix;
			prefix = NULL;
		}
		;

prefixhead	: T_PREFIX IPV6ADDR '/' NUMBER
		{
			prefix = malloc(sizeof(struct AdvPrefix));
			
			if (prefix == NULL) {
				log(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			prefix_init_defaults(prefix);

			if ($4 > MAX_PrefixLen)
			{
				log(LOG_ERR, "invalid prefix length in %s, line %d", conf_file, num_lines);
				ABORT;
			}

			prefix->PrefixLen = $4;

			memcpy(&prefix->Prefix, $2, sizeof(struct in6_addr));
		}
		;

optional_prefixplist: /* empty */
		    | prefixplist 

prefixplist	: prefixplist prefixparms
		| prefixparms
		;

prefixparms	: T_AdvOnLink SWITCH ';'
		{
			prefix->AdvOnLinkFlag = $2;
		}
		| T_AdvAutonomous SWITCH ';'
		{
			prefix->AdvAutonomousFlag = $2;
		}
		| T_AdvRouterAddr SWITCH ';'
		{
			prefix->AdvRouterAddr = $2;
		}
		| T_AdvValidLifetime number_or_infinity ';'
		{
			prefix->AdvValidLifetime = $2;
		}
		| T_AdvPreferredLifetime number_or_infinity ';'
		{
			prefix->AdvPreferredLifetime = $2;
		}
		| T_Base6to4Interface name ';'
		{
			dlog(LOG_DEBUG, 4, "using interface %s for 6to4", $2);
			strncpy(prefix->if6to4, $2, IFNAMSIZ-1);
			prefix->if6to4[IFNAMSIZ-1] = '\0';
		}
		;

number_or_infinity      : NUMBER
                        {
                                $$ = $1; 
                        }
                        | INFINITY
                        {
                                $$ = (uint32_t)~0;
                        }
                        ;

%%

static
void cleanup(void)
{
	if (iface)
		free(iface);
	
	if (prefix)
		free(prefix);
}

static void
yyerror(char *msg)
{
	cleanup();
	log(LOG_ERR, "%s in %s, line %d: %s", msg, conf_file, num_lines, yytext);
}
