typedef union {
	int			num;
	int			bool;
	struct in6_addr		*addr;
	char			*str;
	struct AdvPrefix	*pinfo;
} YYSTYPE;
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


extern YYSTYPE yylval;
