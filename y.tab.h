typedef union {
	int			num;
	int			bool;
	struct in6_addr		*addr;
	char			*str;
	struct AdvPrefix	*pinfo;
} YYSTYPE;
#define	T_INTERFACE	257
#define	T_PREFIX	258
#define	STRING	259
#define	NUMBER	260
#define	SWITCH	261
#define	IPV6ADDR	262
#define	INFINITY	263
#define	T_AdvSendAdvert	264
#define	T_MaxRtrAdvInterval	265
#define	T_MinRtrAdvInterval	266
#define	T_AdvManagedFlag	267
#define	T_AdvOtherConfigFlag	268
#define	T_AdvLinkMTU	269
#define	T_AdvReachableTime	270
#define	T_AdvRetransTimer	271
#define	T_AdvCurHopLimit	272
#define	T_AdvDefaultLifetime	273
#define	T_AdvSourceLLAddress	274
#define	T_AdvOnLink	275
#define	T_AdvAutonomous	276
#define	T_AdvValidLifetime	277
#define	T_AdvPreferredLifetime	278
#define	T_BAD_TOKEN	279


extern YYSTYPE yylval;
