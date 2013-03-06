
#pragma once

#include "radvd.h"

/* parser */
struct yydata
{
	yyscan_t scaninfo;
	char const * filename;
	struct Interface *iface;
	struct AdvPrefix *prefix;
	struct AdvRoute *route;
	struct AdvRDNSS *rdnss;
	struct AdvDNSSL *dnssl;
};

