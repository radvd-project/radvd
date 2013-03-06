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

