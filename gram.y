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


%define api.pure
%parse-param {struct yydata * yydata}
%locations
%defines

%{
#define YYERROR_VERBOSE
static void yyerror(void const * loc, void * vp, char const * s);
#include "config.h"
#include "includes.h"
#include "radvd.h"
#include "defaults.h"

extern struct Interface *IfaceList;
extern char *conf_file;
extern int num_lines;
extern char *yytext;

#define ADD_TO_LL(type, list, value) \
	do { \
		if (yydata->iface->list == NULL) \
			yydata->iface->list = value; \
		else { \
			type *current = yydata->iface->list; \
			while (current->next != NULL) \
				current = current->next; \
			current->next = value; \
		} \
	} while (0)
%}

%token		T_INTERFACE
%token		T_PREFIX
%token		T_ROUTE
%token		T_RDNSS
%token		T_DNSSL
%token		T_CLIENTS

%token	<str>	STRING
%token	<num>	NUMBER
%token	<snum>	SIGNEDNUMBER
%token	<dec>	DECIMAL
%token	<num>	SWITCH
%token	<addr>	IPV6ADDR
%token 		INFINITY

%token		T_IgnoreIfMissing
%token		T_AdvSendAdvert
%token		T_MaxRtrAdvInterval
%token		T_MinRtrAdvInterval
%token		T_MinDelayBetweenRAs
%token		T_AdvManagedFlag
%token		T_AdvOtherConfigFlag
%token		T_AdvLinkMTU
%token		T_AdvReachableTime
%token		T_AdvRetransTimer
%token		T_AdvCurHopLimit
%token		T_AdvDefaultLifetime
%token		T_AdvDefaultPreference
%token		T_AdvSourceLLAddress

%token		T_AdvOnLink
%token		T_AdvAutonomous
%token		T_AdvValidLifetime
%token		T_AdvPreferredLifetime
%token		T_DeprecatePrefix
%token		T_DecrementLifetimes

%token		T_AdvRouterAddr
%token		T_AdvHomeAgentFlag
%token		T_AdvIntervalOpt
%token		T_AdvHomeAgentInfo

%token		T_UnicastOnly

%token		T_HomeAgentPreference
%token		T_HomeAgentLifetime

%token		T_AdvRoutePreference
%token		T_AdvRouteLifetime
%token		T_RemoveRoute

%token		T_AdvRDNSSPreference
%token		T_AdvRDNSSOpenFlag
%token		T_AdvRDNSSLifetime
%token		T_FlushRDNSS

%token		T_AdvDNSSLLifetime
%token		T_FlushDNSSL

%token		T_AdvMobRtrSupportFlag

%token		T_BAD_TOKEN

%type	<str>	name
%type	<pinfo> prefixdef
%type	<ainfo> clientslist v6addrlist
%type	<rinfo>	routedef
%type	<rdnssinfo> rdnssdef
%type	<dnsslinfo> dnssldef
%type   <num>	number_or_infinity

%union {
	unsigned int		num;
	int			snum;
	double			dec;
	struct in6_addr		*addr;
	char			*str;
	struct AdvPrefix	*pinfo;
	struct AdvRoute		*rinfo;
	struct AdvRDNSS		*rdnssinfo;
	struct AdvDNSSL		*dnsslinfo;
	struct Clients		*ainfo;
};

%{
#include "scanner.h"
#include "parser.h"

static void cleanup(struct yydata * yydata);
#define ABORT	do { cleanup(yydata); YYABORT; } while (0);

#define YYLEX_PARAM yydata->scaninfo
%}

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
				if (!strcmp(iface2->Name, yydata->iface->Name))
				{
				flog(LOG_ERR, "duplicate interface "
						"definition for %s", yydata->iface->Name);
					ABORT;
				}
				iface2 = iface2->next;
			}

			dlog(LOG_DEBUG, 4, "interface definition for %s is ok", yydata->iface->Name);

			yydata->iface->next = IfaceList;
			IfaceList = yydata->iface;

			yydata->iface = NULL;
		};

ifacehead	: T_INTERFACE name
		{
			yydata->iface = malloc(sizeof(struct Interface));

			if (yydata->iface == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			iface_init_defaults(yydata->iface);
			strncpy(yydata->iface->Name, $2, IFNAMSIZ-1);
			yydata->iface->Name[IFNAMSIZ-1] = '\0';
		}
		;

name		: STRING
		{
			/* check vality */
			$$ = $1;
		}
		;

ifaceparams :
		/* empty */
		| ifaceparam ifaceparams
		;

ifaceparam 	: ifaceval
		| prefixdef 	{ ADD_TO_LL(struct AdvPrefix, AdvPrefixList, $1); }
		| clientslist 	{ ADD_TO_LL(struct Clients, ClientList, $1); }
		| routedef 	{ ADD_TO_LL(struct AdvRoute, AdvRouteList, $1); }
		| rdnssdef 	{ ADD_TO_LL(struct AdvRDNSS, AdvRDNSSList, $1); }
		| dnssldef 	{ ADD_TO_LL(struct AdvDNSSL, AdvDNSSLList, $1); }
		;

ifaceval	: T_MinRtrAdvInterval NUMBER ';'
		{
			yydata->iface->MinRtrAdvInterval = $2;
		}
		| T_MaxRtrAdvInterval NUMBER ';'
		{
			yydata->iface->MaxRtrAdvInterval = $2;
		}
		| T_MinDelayBetweenRAs NUMBER ';'
		{
			yydata->iface->MinDelayBetweenRAs = $2;
		}
		| T_MinRtrAdvInterval DECIMAL ';'
		{
			yydata->iface->MinRtrAdvInterval = $2;
		}
		| T_MaxRtrAdvInterval DECIMAL ';'
		{
			yydata->iface->MaxRtrAdvInterval = $2;
		}
		| T_MinDelayBetweenRAs DECIMAL ';'
		{
			yydata->iface->MinDelayBetweenRAs = $2;
		}
		| T_IgnoreIfMissing SWITCH ';'
		{
			yydata->iface->IgnoreIfMissing = $2;
		}
		| T_AdvSendAdvert SWITCH ';'
		{
			yydata->iface->AdvSendAdvert = $2;
		}
		| T_AdvManagedFlag SWITCH ';'
		{
			yydata->iface->AdvManagedFlag = $2;
		}
		| T_AdvOtherConfigFlag SWITCH ';'
		{
			yydata->iface->AdvOtherConfigFlag = $2;
		}
		| T_AdvLinkMTU NUMBER ';'
		{
			yydata->iface->AdvLinkMTU = $2;
		}
		| T_AdvReachableTime NUMBER ';'
		{
			yydata->iface->AdvReachableTime = $2;
		}
		| T_AdvRetransTimer NUMBER ';'
		{
			yydata->iface->AdvRetransTimer = $2;
		}
		| T_AdvDefaultLifetime NUMBER ';'
		{
			yydata->iface->AdvDefaultLifetime = $2;
		}
		| T_AdvDefaultPreference SIGNEDNUMBER ';'
		{
			yydata->iface->AdvDefaultPreference = $2;
		}
		| T_AdvCurHopLimit NUMBER ';'
		{
			yydata->iface->AdvCurHopLimit = $2;
		}
		| T_AdvSourceLLAddress SWITCH ';'
		{
			yydata->iface->AdvSourceLLAddress = $2;
		}
		| T_AdvIntervalOpt SWITCH ';'
		{
			yydata->iface->AdvIntervalOpt = $2;
		}
		| T_AdvHomeAgentInfo SWITCH ';'
		{
			yydata->iface->AdvHomeAgentInfo = $2;
		}
		| T_AdvHomeAgentFlag SWITCH ';'
		{
			yydata->iface->AdvHomeAgentFlag = $2;
		}
		| T_HomeAgentPreference NUMBER ';'
		{
			yydata->iface->HomeAgentPreference = $2;
		}
		| T_HomeAgentLifetime NUMBER ';'
		{
			yydata->iface->HomeAgentLifetime = $2;
		}
		| T_UnicastOnly SWITCH ';'
		{
			yydata->iface->UnicastOnly = $2;
		}
		| T_AdvMobRtrSupportFlag SWITCH ';'
		{
			yydata->iface->AdvMobRtrSupportFlag = $2;
		}
		;

clientslist	: T_CLIENTS '{' v6addrlist '}' ';'
		{
			$$ = $3;
		}
		;

v6addrlist	: IPV6ADDR ';'
		{
			struct Clients *new = calloc(1, sizeof(struct Clients));
			if (new == NULL) {
				flog(LOG_CRIT, "calloc failed: %s", strerror(errno));
				ABORT;
			}

			memcpy(&(new->Address), $1, sizeof(struct in6_addr));
			$$ = new;
		}
		| v6addrlist IPV6ADDR ';'
		{
			struct Clients *new = calloc(1, sizeof(struct Clients));
			if (new == NULL) {
				flog(LOG_CRIT, "calloc failed: %s", strerror(errno));
				ABORT;
			}

			memcpy(&(new->Address), $2, sizeof(struct in6_addr));
			new->next = $1;
			$$ = new;
		}
		;


prefixdef	: prefixhead optional_prefixplist ';'
		{
			if (yydata->prefix) {
				if (yydata->prefix->AdvPreferredLifetime > yydata->prefix->AdvValidLifetime)
				{
					flog(LOG_ERR, "AdvValidLifeTime must be "
						"greater than AdvPreferredLifetime in %s, line %d",
						conf_file, num_lines);
					ABORT;
				}

			}
			$$ = yydata->prefix;
			yydata->prefix = NULL;
		}
		;

prefixhead	: T_PREFIX IPV6ADDR '/' NUMBER
		{
			yydata->prefix = malloc(sizeof(struct AdvPrefix));

			if (yydata->prefix == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			prefix_init_defaults(yydata->prefix);

			if ($4 > MAX_PrefixLen)
			{
				flog(LOG_ERR, "invalid prefix length in %s, line %d", conf_file, num_lines);
				ABORT;
			}

			yydata->prefix->PrefixLen = $4;

			memcpy(&yydata->prefix->Prefix, $2, sizeof(struct in6_addr));
		}
		;

optional_prefixplist: /* empty */
		| '{' /* somewhat empty */ '}'
		| '{' prefixplist '}'
		;

prefixplist	: prefixplist prefixparms
		| prefixparms
		;

prefixparms	: T_AdvOnLink SWITCH ';'
		{
			if (yydata->prefix) {
				yydata->prefix->AdvOnLinkFlag = $2;
			}
		}
		| T_AdvAutonomous SWITCH ';'
		{
			if (yydata->prefix) {
				yydata->prefix->AdvAutonomousFlag = $2;
			}
		}
		| T_AdvRouterAddr SWITCH ';'
		{
			if (yydata->prefix) {
				yydata->prefix->AdvRouterAddr = $2;
			}
		}
		| T_AdvValidLifetime number_or_infinity ';'
		{
			if (yydata->prefix) {
				yydata->prefix->AdvValidLifetime = $2;
				yydata->prefix->curr_validlft = $2;
			}
		}
		| T_AdvPreferredLifetime number_or_infinity ';'
		{
			if (yydata->prefix) {
				yydata->prefix->AdvPreferredLifetime = $2;
				yydata->prefix->curr_preferredlft = $2;
			}
		}
		| T_DeprecatePrefix SWITCH ';'
		{
			yydata->prefix->DeprecatePrefixFlag = $2;
		}
		| T_DecrementLifetimes SWITCH ';'
		{
			yydata->prefix->DecrementLifetimesFlag = $2;
		}
		;

routedef	: routehead '{' optional_routeplist '}' ';'
		{
			$$ = yydata->route;
			yydata->route = NULL;
		}
		;


routehead	: T_ROUTE IPV6ADDR '/' NUMBER
		{
			yydata->route = malloc(sizeof(struct AdvRoute));

			if (yydata->route == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			route_init_defaults(yydata->route, yydata->iface);

			if ($4 > MAX_PrefixLen)
			{
				flog(LOG_ERR, "invalid route prefix length in %s, line %d", conf_file, num_lines);
				ABORT;
			}

			yydata->route->PrefixLen = $4;

			memcpy(&yydata->route->Prefix, $2, sizeof(struct in6_addr));
		}
		;


optional_routeplist: /* empty */
		| routeplist
		;

routeplist	: routeplist routeparms
		| routeparms
		;


routeparms	: T_AdvRoutePreference SIGNEDNUMBER ';'
		{
			yydata->route->AdvRoutePreference = $2;
		}
		| T_AdvRouteLifetime number_or_infinity ';'
		{
			yydata->route->AdvRouteLifetime = $2;
		}
		| T_RemoveRoute SWITCH ';'
		{
			yydata->route->RemoveRouteFlag = $2;
		}
		;

rdnssdef	: rdnsshead '{' optional_rdnssplist '}' ';'
		{
			$$ = yydata->rdnss;
			yydata->rdnss = NULL;
		}
		;

rdnssaddrs	: rdnssaddrs rdnssaddr
		| rdnssaddr
		;

rdnssaddr	: IPV6ADDR
		{
			if (!yydata->rdnss) {
				/* first IP found */
				yydata->rdnss = malloc(sizeof(struct AdvRDNSS));

				if (yydata->rdnss == NULL) {
					flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
					ABORT;
				}

				rdnss_init_defaults(yydata->rdnss, yydata->iface);
			}

			switch (yydata->rdnss->AdvRDNSSNumber) {
				case 0:
					memcpy(&yydata->rdnss->AdvRDNSSAddr1, $1, sizeof(struct in6_addr));
					yydata->rdnss->AdvRDNSSNumber++;
					break;
				case 1:
					memcpy(&yydata->rdnss->AdvRDNSSAddr2, $1, sizeof(struct in6_addr));
					yydata->rdnss->AdvRDNSSNumber++;
					break;
				case 2:
					memcpy(&yydata->rdnss->AdvRDNSSAddr3, $1, sizeof(struct in6_addr));
					yydata->rdnss->AdvRDNSSNumber++;
					break;
				default:
					flog(LOG_CRIT, "Too many addresses in RDNSS section");
					ABORT;
			}

		}
		;

rdnsshead	: T_RDNSS rdnssaddrs
		{
			if (!yydata->rdnss) {
				flog(LOG_CRIT, "No address specified in RDNSS section");
				ABORT;
			}
		}
		;

optional_rdnssplist: /* empty */
		| rdnssplist
		;

rdnssplist	: rdnssplist rdnssparms
		| rdnssparms
		;


rdnssparms	: T_AdvRDNSSPreference NUMBER ';'
		{
			flog(LOG_WARNING, "Ignoring deprecated RDNSS preference.");
		}
		| T_AdvRDNSSOpenFlag SWITCH ';'
		{
			flog(LOG_WARNING, "Ignoring deprecated RDNSS open flag.");
		}
		| T_AdvRDNSSLifetime number_or_infinity ';'
		{
			if ($2 < yydata->iface->MaxRtrAdvInterval && $2 != 0) {
				flog(LOG_ERR, "AdvRDNSSLifetime must be at least MaxRtrAdvInterval");
				ABORT;
			}
			if ($2 > 2*(yydata->iface->MaxRtrAdvInterval))
				flog(LOG_WARNING, "Warning: AdvRDNSSLifetime <= 2*MaxRtrAdvInterval would allow stale DNS servers to be deleted faster");

			yydata->rdnss->AdvRDNSSLifetime = $2;
		}
		| T_FlushRDNSS SWITCH ';'
		{
			yydata->rdnss->FlushRDNSSFlag = $2;
		}
		;

dnssldef	: dnsslhead '{' optional_dnsslplist '}' ';'
		{
			$$ = yydata->dnssl;
			yydata->dnssl = NULL;
		}
		;

dnsslsuffixes	: dnsslsuffixes dnsslsuffix
		| dnsslsuffix
		;

dnsslsuffix	: STRING
		{
			char *ch;
			for (ch = $1; *ch != '\0'; ++ch) {
				if (*ch >= 'A' && *ch <= 'Z')
					continue;
				if (*ch >= 'a' && *ch <= 'z')
					continue;
				if (*ch >= '0' && *ch <= '9')
					continue;
				if (*ch == '-' || *ch == '.')
					continue;

				flog(LOG_CRIT, "Invalid domain suffix specified");
				ABORT;
			}

			if (!yydata->dnssl) {
				/* first domain found */
				yydata->dnssl = malloc(sizeof(struct AdvDNSSL));

				if (yydata->dnssl == NULL) {
					flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
					ABORT;
				}

				dnssl_init_defaults(yydata->dnssl, yydata->iface);
			}

			yydata->dnssl->AdvDNSSLNumber++;
			yydata->dnssl->AdvDNSSLSuffixes =
				realloc(yydata->dnssl->AdvDNSSLSuffixes,
					yydata->dnssl->AdvDNSSLNumber * sizeof(char*));
			if (yydata->dnssl->AdvDNSSLSuffixes == NULL) {
				flog(LOG_CRIT, "realloc failed: %s", strerror(errno));
				ABORT;
			}

			yydata->dnssl->AdvDNSSLSuffixes[yydata->dnssl->AdvDNSSLNumber - 1] = strdup($1);
		}
		;

dnsslhead	: T_DNSSL dnsslsuffixes
		{
			if (!yydata->dnssl) {
				flog(LOG_CRIT, "No domain specified in DNSSL section");
				ABORT;
			}
		}
		;

optional_dnsslplist: /* empty */
		| dnsslplist
		;

dnsslplist	: dnsslplist dnsslparms
		| dnsslparms
		;


dnsslparms	: T_AdvDNSSLLifetime number_or_infinity ';'
		{
			if ($2 < yydata->iface->MaxRtrAdvInterval && $2 != 0) {
				flog(LOG_ERR, "AdvDNSSLLifetime must be at least MaxRtrAdvInterval");
				ABORT;
			}
			if ($2 > 2*(yydata->iface->MaxRtrAdvInterval))
				flog(LOG_WARNING, "Warning: AdvDNSSLLifetime <= 2*MaxRtrAdvInterval would allow stale DNS suffixes to be deleted faster");

			yydata->dnssl->AdvDNSSLLifetime = $2;
		}
		| T_FlushDNSSL SWITCH ';'
		{
			yydata->dnssl->FlushDNSSLFlag = $2;
		}
		;

number_or_infinity	: NUMBER
			{
				$$ = $1;
			}
			| INFINITY
			{
				$$ = (uint32_t)~0;
			}
			;

%%

static void cleanup(struct yydata * yydata)
{
	if (yydata->iface)
		free(yydata->iface);

	if (yydata->prefix)
		free(yydata->prefix);

	if (yydata->route)
		free(yydata->route);

	if (yydata->rdnss)
		free(yydata->rdnss);

	if (yydata->dnssl) {
		int i;
		for (i = 0; i < yydata->dnssl->AdvDNSSLNumber; ++i)
			free(yydata->dnssl->AdvDNSSLSuffixes[i]);
		free(yydata->dnssl->AdvDNSSLSuffixes);
		free(yydata->dnssl);
	}
}

int
readin_config(char *fname)
{
	struct yydata yydata;
	FILE * in;
	int rc = 0;

	in = fopen(fname, "r");

	if (!in)
	{
		flog(LOG_ERR, "can't open %s: %s", fname, strerror(errno));
		return (-1);
	}

	memset(&yydata, 0, sizeof(yydata));
	yydata.filename = fname;
	yylex_init(&yydata.scaninfo);
	yyset_in(in, yydata.scaninfo);

	if (yyparse(&yydata) != 0)
	{
		flog(LOG_ERR, "error parsing or activating the config file: %s", fname);
		rc = -1;
	}

	yylex_destroy(yydata.scaninfo);

	fclose(in);

	return rc;
}


static void
yyerror(void const * loc, void * vp, char const * msg)
{
	char * str1 = 0;
	char * str2 = 0;
	char * str3 = 0;
	int rc = 0;
	YYLTYPE const * t = (YYLTYPE const*)loc;
	struct yydata * yydata = (struct yydata *)vp;

	cleanup(yydata);

	rc = asprintf(&str1, "%s", msg);
	if (rc == -1) {
		flog (LOG_ERR, "asprintf failed in yyerror");
	}

	rc = asprintf(&str2, "location %d.%d-%d.%d: %s",
		t->first_line, t->first_column,
		t->last_line,  t->last_column,
		yyget_text(yydata->scaninfo));
	if (rc == -1) {
		flog (LOG_ERR, "asprintf failed in yyerror");
	}

	rc = asprintf(&str3, "%s in %s, %s", str1, yydata->filename, str2);
	if (rc == -1) {
		flog (LOG_ERR, "asprintf failed in yyerror");
	}

	flog (LOG_ERR, "%s", str3);

	if (str1) {
		free(str1);
	}

	if (str2) {
		free(str2);
	}

	if (str3) {
		free(str3);
	}
}

