/*
 *   $Id: radvd.h,v 1.1 1997/10/14 17:17:40 lf Exp $
 *
 *   Authors:
 *    Pedro Roque		<roque@di.fc.ul.pt>
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

#ifndef RADV_H
#define RADV_H

#include <config.h>
#include <includes.h>

#define CONTACT_EMAIL	"Lars Fenneberg <lf@elemental.net>"

/* protocol constants as specified by RFC 1970 */
#define MAX_INITIAL_RTR_ADVERT_INTERVAL	       16 /* seconds */
#define MAX_INITIAL_RTR_ADVERTISEMENTS		3 /* transmissions */
#define MAX_FINAL_RTR_ADVERTISEMENTS      	3 /* transmissions */
#define MAX_RA_DELAY_TIME                   500.0 /* milliseconds */
#define MIN_DELAY_BETWEEN_RAS			3 /* seconds */

/* for log.c */
#define	L_NONE		0
#define L_SYSLOG	1
#define L_STDERR	2
#define L_LOGFILE	3

#define LOG_TIME_FORMAT "%b %d %H:%M:%S"

/* maximum message size for incoming and outgoing RSs and RAs */
#define MSG_SIZE	4096

struct timer_lst {
	unsigned long		expires;
	void			(*handler)(void *);
	void *			data;
	struct timer_lst	*next;
	struct timer_lst	*prev;	
};

struct AdvPrefix;

#define HWADDR_MAX 16

struct Interface {
	char			Name[IFNAMSIZ+1];	/* interface name */

	struct in6_addr		if_addr;
	int			if_index;

	u_int8_t		if_hwaddr[HWADDR_MAX];
	int			if_hwaddr_len;
	int			if_prefix_len;
	int			if_maxmtu;

	int			AdvSendAdvert;
	int			MaxRtrAdvInterval;
	int			MinRtrAdvInterval;
	int			AdvManagedFlag;
	int			AdvOtherConfigFlag;
	int			AdvLinkMTU;
	int			AdvReachableTime;
	int			AdvRetransTimer;
	int			AdvCurHopLimit;
	int			AdvDefaultLifetime;
	int			AdvSourceLLAddress;

	struct AdvPrefix	*AdvPrefixList;
	struct timer_lst	tm;
	unsigned long		last_multicast;
	struct Interface	*next;
};

struct AdvPrefix {
	struct in6_addr		Prefix;
	int			PrefixLen;
	
	int			AdvOnLinkFlag;
	int			AdvAutonomousFlag;
	u_int32_t		AdvValidLifetime;
	u_int32_t		AdvPreferredLifetime;

	struct AdvPrefix	*next;
};

/* gram.y */
int yyparse(void);

/* scanner.l */
int yylex(void);

/* timer.c */
void set_timer(struct timer_lst *tm, int secs);
unsigned long clear_timer(struct timer_lst *tm);
void init_timer(struct timer_lst *, void (*)(void *), void *); 

/* log.c */
int log_open(int, char *, char*, int);
int log(int, char *, ...);
int dlog(int, int, char *, ...);
int log_close(void);
int log_reopen(void);
void set_debuglevel(int);
int get_debuglevel(void);

/* device.c */
int setup_deviceinfo(int sock, struct Interface *);
int check_device(int sock, struct Interface *);
int setup_linklocal_addr(int sock, struct Interface *);

/* interface.c */
void iface_init_defaults(struct Interface *);
void prefix_init_defaults(struct AdvPrefix *);
int check_iface(struct Interface *);

/* socket.c */
int open_icmpv6_socket(void);

/* send.c */
void send_ra(int, struct Interface *iface, struct in6_addr *dest);

/* process.c */
void process(int sock, struct Interface *, unsigned char *, int,
	struct sockaddr_in6 *, struct in6_pktinfo *, int);

/* recv.c */
int recv_rs_ra(int, char *, struct sockaddr_in6 *, struct in6_pktinfo **, int *);

/* util.c */
void mdelay(int);
int rand_between(int, int);
void print_addr(struct in6_addr *, char *);

#endif
