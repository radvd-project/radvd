/*
 *   $Id: radvd.c,v 1.3 1997/10/16 23:02:28 lf Exp $
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

#include <config.h>
#include <includes.h>
#include <radvd.h>
#include <pathnames.h>

struct Interface *IfaceList = NULL;

char usage_str[] =
	"[-vh] [-d level] [-C config_file] [-m log_method] [-l log_file]\n"
	"\t[-f facility]";

#ifdef HAVE_GETOPT_LONG
struct option prog_opt[] = {
	{"debug", 1, 0, 'd'},
	{"config", 1, 0, 'C'},
	{"logfile", 1, 0, 'l'},
	{"logmethod", 1, 0, 'm'},
	{"facility", 1, 0, 'f'},
	{"version", 0, 0, 'v'},
	{"help", 0, 0, 'h'},
	{NULL, 0, 0, 0}
};
#endif

extern FILE *yyin;

char *conf_file = NULL;
char *pname;
int sock = -1;

void sighup_handler(int sig);
void timer_handler(void *data);
int readin_config(char *);
void version(void);
void usage(void);

int
main(int argc, char *argv[])
{
	unsigned char msg[MSG_SIZE];
	struct Interface *iface;
	int c, log_method;
	char *logfile;
	int facility;
#ifdef HAVE_GETOPT_LONG
	int opt_idx;
#endif

	pname = ((pname=strrchr(argv[0],'/')) != NULL)?pname+1:argv[0];

	srand((unsigned int)time(NULL));

	log_method = L_SYSLOG;
	logfile = PATH_RADVD_LOG;
	conf_file = PATH_RADVD_CONF;
	facility = LOG_FACILITY;

	/* parse args */
#ifdef HAVE_GETOPT_LONG
	while ((c = getopt_long(argc, argv, "d:C:l:m:vh", prog_opt, &opt_idx)) > 0)
#else
	while ((c = getopt(argc, argv, "d:C:l:m:vh")) > 0)
#endif
	{
		switch (c) {
		case 'C':
			conf_file = optarg;
			break;
		case 'd':
			set_debuglevel(atoi(optarg));
			break;
		case 'f':
			facility = atoi(optarg);
			break;
		case 'l':
			logfile = optarg;
			break;
		case 'm':
			if (!strcmp(optarg, "syslog"))
			{
				log_method = L_SYSLOG;
			}
			else if (!strcmp(optarg, "stderr"))
			{
				log_method = L_STDERR;
			}
			else if (!strcmp(optarg, "logfile"))
			{
				log_method = L_LOGFILE;
			}
			else if (!strcmp(optarg, "none"))
			{
				log_method = L_NONE;
			}
			else
			{
				fprintf(stderr, "%s: unknown log method: %s\n", pname, optarg);
				exit(1);
			}
			break;
		case 'v':
			version();
			break;
		case 'h':
			usage();
#ifdef HAVE_GETOPT_LONG
		case ':':
			fprintf(stderr, "%s: option %s: parameter expected\n", pname,
				prog_opt[opt_idx].name);
			exit(1);
#endif
		case '?':
			exit(1);
		}
	}
	
	if (log_open(log_method, pname, logfile, facility) < 0)
		exit(1);

	log(LOG_INFO, "version %s started", VERSION);

	/* get a raw socket for sending and receiving ICMPv6 messages */
	sock = open_icmpv6_socket();
	if (sock < 0)
		exit(1);

	/* parse config file */
	if (readin_config(conf_file) < 0)
		exit(1);

	/*
	 * okay, config file is read in, socket and stuff is setup, so
	 * lets fork now...
	 */

	if (get_debuglevel() == 0) {

		switch (fork()) {
			case 0:
				/* child falls through */
				break;
			case -1:
				log(LOG_ERR, "fork: %s", strerror(errno));
				exit(1);
			default:
				/* parent exists */
				log_close();
				exit(0);	
		}

		/*
		 * reopen logfile, so that we get the process id right in the syslog
		 */
		if (log_reopen() < 0)
			exit(1);

		/*
		 * detach from our controlling terminal
		 */
	 
		setsid();
	}

	/*
	 *	config signal handlers
	 */

	signal(SIGHUP, sighup_handler);

	/*
	 *	send initial advertisement and set timers
	 */

	for(iface=IfaceList; iface; iface=iface->next)
	{
		init_timer(&iface->tm, timer_handler, (void *) iface);
		if (iface->AdvSendAdvert)
		{
			/* send an initial advertisement */
			send_ra(sock, iface, NULL);

			set_timer(&iface->tm, iface->MaxRtrAdvInterval);
		}
	}


	/* enter loop */

	for (;;)
	{
		int len, hoplimit;
		struct sockaddr_in6 rcv_addr;
		struct in6_pktinfo *pkt_info = NULL;
		
		len = recv_rs_ra(sock, msg, &rcv_addr, &pkt_info, &hoplimit);
		if (len > 0)
			process(sock, IfaceList, msg, len, 
				&rcv_addr, pkt_info, hoplimit);
	}
}

void
timer_handler(void *data)
{
	struct Interface *iface = (struct Interface *) data;
	int next;

	dlog(LOG_DEBUG, 4, "timer_handler called for %s", iface->Name);

	send_ra(sock, iface, NULL);

	next = rand_between(iface->MinRtrAdvInterval, iface->MaxRtrAdvInterval); 
	set_timer(&iface->tm, next);
}

void
sighup_handler(int sig)
{
	struct Interface *iface;

	/* Linux has "one-shot" signals, reinstall the signal handler */
	signal(SIGHUP, sighup_handler);

	log(LOG_INFO, "attempting to reread config file");

	dlog(LOG_DEBUG, 4, "reopening log");
	if (log_reopen() < 0)
		exit(1);

	/* disable timers, free interface and prefix structures */
	for(iface=IfaceList; iface; iface=iface->next)
	{
		dlog(LOG_DEBUG, 4, "disabling timer for %s", iface->Name);
		clear_timer(&iface->tm);
	}

	iface=IfaceList; 
	while(iface)
	{
		struct Interface *next_iface = iface->next;
		struct AdvPrefix *prefix;

		dlog(LOG_DEBUG, 4, "freeing interface %s", iface->Name);
		
		prefix = iface->AdvPrefixList;
		while (prefix)
		{
			struct AdvPrefix *next_prefix = prefix->next;
			
			free(prefix);
			prefix = next_prefix;
		}
		
		free(iface);
		iface = next_iface;
	}

	IfaceList = NULL;

	/* reread config file */
	if (readin_config(conf_file) < 0)
		exit(1);

	/*
	 *	send initial advertisement and set timers
	 */
	for(iface=IfaceList; iface; iface=iface->next)
	{
		init_timer(&iface->tm, timer_handler, (void *)iface);
		if (iface->AdvSendAdvert)
		{
			/* send an initial advertisement */
			send_ra(sock, iface, NULL);
		
			set_timer(&iface->tm, iface->MaxRtrAdvInterval);
		}
	}

	log(LOG_INFO, "resuming normal operation");
}

int
readin_config(char *fname)
{
	if ((yyin = fopen(fname, "r")) == NULL)
	{
		log(LOG_ERR, "can't open %s: %s", fname, strerror(errno));
		return (-1);
	}

	if (yyparse() != 0)
		return (-1);

	return 0;
}

void
version(void)
{
	fprintf(stderr, "Version: %s\n\n", VERSION);
	fprintf(stderr, "Compiled in settings:\n");
	fprintf(stderr, "  default config file		\"%s\"\n", PATH_RADVD_CONF);
	fprintf(stderr, "  default logfile		\"%s\"\n", PATH_RADVD_LOG);
	fprintf(stderr, "  default syslog facililty	%d\n", LOG_FACILITY);
#ifdef EUI_64_SUPPORT
	fprintf(stderr, "  EUI-64 support		enabled\n\n"); 	
#else
	fprintf(stderr, "  EUI-64 support		disabled\n\n");
#endif	
	fprintf(stderr, "Please send bug reports or suggestions to %s.\n",
		CONTACT_EMAIL);

	exit(1);	
}

void
usage(void)
{
	fprintf(stderr,"usage: %s %s\n", pname, usage_str);
	exit(1);	
}
