
/*  A Bison parser, made from gram.y
 by  GNU Bison version 1.25
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	T_INTERFACE	258
#define	T_PREFIX	259
#define	STRING	260
#define	NUMBER	261
#define	SWITCH	262
#define	IPV6ADDR	263
#define	INFINITY	264
#define	T_AdvSendAdvert	265
#define	T_MaxRtrAdvInterval	266
#define	T_MinRtrAdvInterval	267
#define	T_AdvManagedFlag	268
#define	T_AdvOtherConfigFlag	269
#define	T_AdvLinkMTU	270
#define	T_AdvReachableTime	271
#define	T_AdvRetransTimer	272
#define	T_AdvCurHopLimit	273
#define	T_AdvDefaultLifetime	274
#define	T_AdvSourceLLAddress	275
#define	T_AdvOnLink	276
#define	T_AdvAutonomous	277
#define	T_AdvValidLifetime	278
#define	T_AdvPreferredLifetime	279
#define	T_BAD_TOKEN	280

#line 16 "gram.y"

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

static int palloc_check(void);

static void cleanup(void);
static void yyerror(char *msg);

#define ABORT	do { cleanup(); YYABORT; } while (0);


#line 72 "gram.y"
typedef union {
	int			num;
	int			bool;
	struct in6_addr		*addr;
	char			*str;
	struct AdvPrefix	*pinfo;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		78
#define	YYFLAG		-32768
#define	YYNTBASE	30

#define YYTRANSLATE(x) ((unsigned)(x) <= 280 ? yytranslate[x] : 44)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,    29,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    28,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    26,     2,    27,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     5,    12,    14,    18,    19,    21,    24,    26,
    30,    34,    38,    42,    46,    50,    54,    58,    62,    66,
    70,    72,    75,    84,    85,    87,    90,    92,    96,   100,
   104,   108,   110
};

static const short yyrhs[] = {    30,
    31,     0,    31,     0,     3,    32,    26,    33,    27,    28,
     0,     5,     0,    36,    34,    38,     0,     0,    35,     0,
    35,    37,     0,    37,     0,    10,     7,    28,     0,    12,
     6,    28,     0,    11,     6,    28,     0,    13,     7,    28,
     0,    14,     7,    28,     0,    15,     6,    28,     0,    16,
     6,    28,     0,    17,     6,    28,     0,    19,     6,    28,
     0,    18,     6,    28,     0,    20,     7,    28,     0,    39,
     0,    38,    39,     0,     4,     8,    29,     6,    26,    40,
    27,    28,     0,     0,    41,     0,    41,    42,     0,    42,
     0,    21,     7,    28,     0,    22,     7,    28,     0,    23,
    43,    28,     0,    24,    43,    28,     0,     6,     0,     9,
     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    82,    83,    86,   122,   129,   135,   136,   139,   140,   143,
   156,   160,   164,   168,   172,   176,   180,   184,   188,   192,
   198,   202,   209,   239,   240,   242,   243,   246,   253,   260,
   267,   276,   280
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","T_INTERFACE",
"T_PREFIX","STRING","NUMBER","SWITCH","IPV6ADDR","INFINITY","T_AdvSendAdvert",
"T_MaxRtrAdvInterval","T_MinRtrAdvInterval","T_AdvManagedFlag","T_AdvOtherConfigFlag",
"T_AdvLinkMTU","T_AdvReachableTime","T_AdvRetransTimer","T_AdvCurHopLimit","T_AdvDefaultLifetime",
"T_AdvSourceLLAddress","T_AdvOnLink","T_AdvAutonomous","T_AdvValidLifetime",
"T_AdvPreferredLifetime","T_BAD_TOKEN","'{'","'}'","';'","'/'","grammar","ifacedef",
"name","ifaceparams","optional_ifacevlist","ifacevlist","iface_advt","ifaceval",
"prefixlist","prefixdef","optional_prefixplist","prefixplist","prefixparms",
"number_or_infinity", NULL
};
#endif

static const short yyr1[] = {     0,
    30,    30,    31,    32,    33,    34,    34,    35,    35,    36,
    37,    37,    37,    37,    37,    37,    37,    37,    37,    37,
    38,    38,    39,    40,    40,    41,    41,    42,    42,    42,
    42,    43,    43
};

static const short yyr2[] = {     0,
     2,     1,     6,     1,     3,     0,     1,     2,     1,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     1,     2,     8,     0,     1,     2,     1,     3,     3,     3,
     3,     1,     1
};

static const short yydefact[] = {     0,
     0,     0,     2,     4,     0,     1,     0,     0,     0,     6,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     7,     9,    10,     3,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     5,    21,
     8,    12,    11,    13,    14,    15,    16,    17,    19,    18,
    20,     0,    22,     0,     0,    24,     0,     0,     0,     0,
     0,    25,    27,     0,     0,    32,    33,     0,     0,     0,
    26,    28,    29,    30,    31,    23,     0,     0
};

static const short yydefgoto[] = {     2,
     3,     5,     9,    23,    24,    10,    25,    39,    40,    61,
    62,    63,    68
};

static const short yypact[] = {     7,
    12,    15,-32768,-32768,    -6,-32768,    11,    16,    -5,   -11,
    -4,    -3,    20,    21,    22,    23,    25,    26,    27,    28,
    29,    30,    24,   -11,-32768,-32768,-32768,     8,    13,    14,
    17,    18,    19,    31,    32,    33,    34,    35,    24,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,     9,-32768,    38,    37,   -10,    41,    42,    10,    10,
    39,   -10,-32768,    36,    40,-32768,-32768,    43,    44,    45,
-32768,-32768,-32768,-32768,-32768,-32768,    50,-32768
};

static const short yypgoto[] = {-32768,
    49,-32768,-32768,-32768,-32768,-32768,    46,-32768,     0,-32768,
-32768,   -22,    -8
};


#define	YYLAST		73


static const short yytable[] = {    13,
    14,    15,    16,    17,    18,    19,    20,    21,    22,     1,
    57,    58,    59,    60,    77,    66,     4,     1,    67,     7,
     8,    12,    11,    26,    27,    28,    29,    38,    30,    31,
    32,    33,    34,    35,    36,    42,    37,    54,    53,    71,
    43,    44,    52,    55,    45,    46,    47,    64,    65,    78,
     6,    69,     0,     0,     0,     0,     0,     0,    48,    49,
    50,    51,    56,    72,     0,    70,     0,    73,     0,    41,
    74,    75,    76
};

static const short yycheck[] = {    11,
    12,    13,    14,    15,    16,    17,    18,    19,    20,     3,
    21,    22,    23,    24,     0,     6,     5,     3,     9,    26,
    10,    27,     7,    28,    28,     6,     6,     4,     7,     7,
     6,     6,     6,     6,     6,    28,     7,    29,    39,    62,
    28,    28,     8,     6,    28,    28,    28,     7,     7,     0,
     2,    60,    -1,    -1,    -1,    -1,    -1,    -1,    28,    28,
    28,    28,    26,    28,    -1,    27,    -1,    28,    -1,    24,
    28,    28,    28
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 196 "/usr/lib/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 3:
#line 87 "gram.y"
{
			struct Interface *iface2;

			strcpy(iface->Name, yyvsp[-4].str);

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

			iface->next = IfaceList;
			IfaceList = iface;

			dlog(LOG_DEBUG, 4, "interface definition for %s is ok", iface->Name);

			iface = NULL;
		;
    break;}
case 4:
#line 123 "gram.y"
{
			/* check vality */
			yyval.str = yyvsp[0].str;
		;
    break;}
case 5:
#line 130 "gram.y"
{
			iface->AdvPrefixList = yyvsp[0].pinfo;
		;
    break;}
case 10:
#line 144 "gram.y"
{
			iface = malloc(sizeof(struct Interface));

			if (iface == NULL) {
				log(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			iface_init_defaults(iface);
			iface->AdvSendAdvert = yyvsp[-1].bool;
		;
    break;}
case 11:
#line 157 "gram.y"
{
			iface->MinRtrAdvInterval = yyvsp[-1].num;
		;
    break;}
case 12:
#line 161 "gram.y"
{
			iface->MaxRtrAdvInterval = yyvsp[-1].num;
		;
    break;}
case 13:
#line 165 "gram.y"
{
			iface->AdvManagedFlag = yyvsp[-1].bool;
		;
    break;}
case 14:
#line 169 "gram.y"
{
			iface->AdvOtherConfigFlag = yyvsp[-1].bool;
		;
    break;}
case 15:
#line 173 "gram.y"
{
			iface->AdvLinkMTU = yyvsp[-1].num;
		;
    break;}
case 16:
#line 177 "gram.y"
{
			iface->AdvReachableTime = yyvsp[-1].num;
		;
    break;}
case 17:
#line 181 "gram.y"
{
			iface->AdvRetransTimer = yyvsp[-1].num;
		;
    break;}
case 18:
#line 185 "gram.y"
{
			iface->AdvDefaultLifetime = yyvsp[-1].num;
		;
    break;}
case 19:
#line 189 "gram.y"
{
			iface->AdvCurHopLimit = yyvsp[-1].num;
		;
    break;}
case 20:
#line 193 "gram.y"
{
			iface->AdvSourceLLAddress = yyvsp[-1].bool;
		;
    break;}
case 21:
#line 199 "gram.y"
{
			yyval.pinfo = yyvsp[0].pinfo;
		;
    break;}
case 22:
#line 203 "gram.y"
{
			yyvsp[0].pinfo->next = yyvsp[-1].pinfo;
			yyval.pinfo = yyvsp[0].pinfo;
		;
    break;}
case 23:
#line 210 "gram.y"
{
			if (palloc_check() < 0)
				ABORT;

			if (yyvsp[-4].num > MAX_PrefixLen)
			{
				log(LOG_ERR, "invalid prefix length in %s, line %d", conf_file, num_lines);
				ABORT;
			}
			

			prefix->PrefixLen = yyvsp[-4].num;

			if (prefix->AdvPreferredLifetime >
			    prefix->AdvValidLifetime)
			{
				log(LOG_ERR, "AdvValidLifeTime must be "
					"greater than AdvPreferredLifetime in %s, line %d", 
					conf_file, num_lines);
				ABORT;
			}

			memcpy(&prefix->Prefix, yyvsp[-6].addr, sizeof(struct in6_addr));
			
			yyval.pinfo = prefix;
			prefix = NULL;
		;
    break;}
case 28:
#line 247 "gram.y"
{
			if (palloc_check() < 0)
				ABORT;

			prefix->AdvOnLinkFlag = yyvsp[-1].bool;
		;
    break;}
case 29:
#line 254 "gram.y"
{
			if (palloc_check() < 0)
				ABORT;

			prefix->AdvAutonomousFlag = yyvsp[-1].bool;
		;
    break;}
case 30:
#line 261 "gram.y"
{
			if (palloc_check() < 0)
				ABORT;

			prefix->AdvValidLifetime = yyvsp[-1].num;
		;
    break;}
case 31:
#line 268 "gram.y"
{
			if (palloc_check() < 0)
				ABORT;

			prefix->AdvPreferredLifetime = yyvsp[-1].num;
		;
    break;}
case 32:
#line 277 "gram.y"
{
                                yyval.num = yyvsp[0].num; 
                        ;
    break;}
case 33:
#line 281 "gram.y"
{
                                yyval.num = (uint32_t)~0;
                        ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 498 "/usr/lib/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 286 "gram.y"


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

static int
palloc_check(void)
{
	if (prefix == NULL)
	{
		prefix = malloc(sizeof(struct AdvPrefix));
		
		if (prefix == NULL) {
			log(LOG_CRIT, "malloc failed: %s", strerror(errno));
			return (-1);
		}

		prefix_init_defaults(prefix);
	}

	return 0;
}
