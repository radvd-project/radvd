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


#include "config.h"
#include "includes.h"
#include "radvd.h"
#include "pathnames.h"

#ifdef HAVE_NETLINK
#include "netlink.h"
#endif

#include <poll.h>
#include <libdaemon/dfork.h>
#include <libdaemon/dpid.h>

struct Interface *IfaceList = NULL;

#ifdef HAVE_GETOPT_LONG

char usage_str[] = {
"\n"
"  -c, --configtest       Parse the config file and exit.\n"
"  -C, --config=PATH      Sets the config file.  Default is /etc/radvd.conf.\n"
"  -d, --debug=NUM        Sets the debug level.  Values can be 1, 2, 3, 4 or 5.\n"
"  -f, --facility=NUM     Sets the logging facility.\n"
"  -h, --help             Show this help screen.\n"
"  -l, --logfile=PATH     Sets the log file.\n"
"  -m, --logmethod=X      Sets the log method to one of: syslog, stderr, stderr_syslog, logfile, or none.\n"
"  -p, --pidfile=PATH     Sets the pid file.\n"
"  -t, --chrootdir=PATH   Chroot to the specified path.\n"
"  -u, --username=USER    Switch to the specified user.\n"
"  -n, --nodaemon         Prevent the daemonizing.\n"
"  -v, --version          Print the version and quit.\n"
};

struct option prog_opt[] = {
	{"debug", 1, 0, 'd'},
	{"configtest", 0, 0, 'c'},
	{"config", 1, 0, 'C'},
	{"pidfile", 1, 0, 'p'},
	{"logfile", 1, 0, 'l'},
	{"logmethod", 1, 0, 'm'},
	{"facility", 1, 0, 'f'},
	{"username", 1, 0, 'u'},
	{"chrootdir", 1, 0, 't'},
	{"version", 0, 0, 'v'},
	{"help", 0, 0, 'h'},
	{"singleprocess", 0, 0, 's'},
	{"nodaemon", 0, 0, 'n'},
	{NULL, 0, 0, 0}
};

#else

char usage_str[] =
	"[-hsvcn] [-d level] [-C config_file] [-m log_method] [-l log_file]\n"
	"\t[-f facility] [-p pid_file] [-u username] [-t chrootdir]";

#endif

char *conf_file = NULL;
char *pidfile = NULL;
char *pname;
int sock = -1;

volatile int sighup_received = 0;
volatile int sigterm_received = 0;
volatile int sigint_received = 0;
volatile int sigusr1_received = 0;

void sighup_handler(int sig);
void sigterm_handler(int sig);
void sigint_handler(int sig);
void sigusr1_handler(int sig);
void timer_handler(void *data);
void config_interface(struct Interface * iface);
void config_interfaces(void);
void stop_adverts(void);
void validate_configuration_or_die(void);
void version(void);
void usage(void);
int drop_root_privileges(const char *);
int check_conffile_perm(const char *, const char *);
const char *get_pidfile(void);
void extract_ipv6_address(char const * addr_str, struct in6_addr * addr);
struct ipv6_route * get_route_table(void);
void free_route_table(struct ipv6_route * route_table);
int already_advrting(struct in6_addr const * addr, int len, struct PrefixSpec const * spec);
void main_loop(void);

int
main(int argc, char *argv[])
{
	int c, log_method;
	char *logfile;
	int facility;
	char *username = NULL;
	char *chrootdir = NULL;
	int configtest = 0;
	int daemonize = 1;
#ifdef HAVE_GETOPT_LONG
	int opt_idx;
#endif
	pid_t pid;

	pname = ((pname=strrchr(argv[0],'/')) != NULL)?pname+1:argv[0];

	srand((unsigned int)time(NULL));

	log_method = L_STDERR_SYSLOG;
	logfile = PATH_RADVD_LOG;
	conf_file = PATH_RADVD_CONF;
	facility = LOG_FACILITY;
	pidfile = PATH_RADVD_PID;

	/* parse args */
#define OPTIONS_STR "d:C:l:m:p:t:u:vhcsn"
#ifdef HAVE_GETOPT_LONG
	while ((c = getopt_long(argc, argv, OPTIONS_STR, prog_opt, &opt_idx)) > 0)
#else
	while ((c = getopt(argc, argv, OPTIONS_STR)) > 0)
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
		case 'p':
			pidfile = optarg;
			break;
		case 'm':
			if (!strcmp(optarg, "syslog"))
			{
				log_method = L_SYSLOG;
			}
			else if (!strcmp(optarg, "stderr_syslog"))
			{
				log_method = L_STDERR_SYSLOG;
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
		case 't':
			chrootdir = strdup(optarg);
			break;
		case 'u':
			username = strdup(optarg);
			break;
		case 'v':
			version();
			break;
		case 'c':
			configtest = 1;
			break;
		case 's':
			fprintf(stderr, "privsep is not optional.  This options will be removed in a near future release.");
			break;
		case 'n':
			daemonize = 0;
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

	if (chrootdir) {
		if (!username) {
			fprintf(stderr, "Chroot as root is not safe, exiting\n");
			exit(1);
		}

		if (chroot(chrootdir) == -1) {
			perror("chroot");
			exit (1);
		}

		if (chdir("/") == -1) {
			perror("chdir");
			exit (1);
		}
		/* username will be switched later */
	}

	if (configtest) {
		log_method = L_STDERR;
	}

	if (log_open(log_method, pname, logfile, facility) < 0) {
		perror("log_open");
		exit(1);
	}

	if (!configtest) {
		flog(LOG_INFO, "version %s started", VERSION);
	}

	/* check that 'other' cannot write the file
         * for non-root, also that self/own group can't either
         */
	if (check_conffile_perm(username, conf_file) < 0) {
		if (get_debuglevel() == 0) {
			flog(LOG_ERR, "Exiting, permissions on conf_file invalid.\n");
			exit(1);
		}
		else
			flog(LOG_WARNING, "Insecure file permissions, but continuing anyway");
	}

	/* parse config file */
	if (readin_config(conf_file) < 0) {
		flog(LOG_ERR, "Exiting, failed to read config file.\n");
		exit(1);
	}

	if (configtest) {
		fprintf(stderr, "Syntax OK\n");
		exit(0);
	}

	/* get a raw socket for sending and receiving ICMPv6 messages */
	sock = open_icmpv6_socket();
	if (sock < 0) {
		perror("open_icmpv6_socket");
		exit(1);
	}

	/* if we know how to do it, check whether forwarding is enabled */
	if (check_ip6_forwarding()) {
		flog(LOG_WARNING, "IPv6 forwarding seems to be disabled, but continuing anyway.");
	}


#ifdef USE_PRIVSEP
	dlog(LOG_DEBUG, 3, "Initializing privsep");
	if (privsep_init() < 0) {
		perror("Failed to initialize privsep.");
		exit(1);
	}
#endif

	/* drop root privileges if requested. */
	if (username) {
		if (drop_root_privileges(username) < 0) {
			perror("drop_root_privileges");
			exit(1);
		}
	}

	/*
	 * okay, config file is read in, socket and stuff is setup, so
	 * lets fork now...
	 */

	if (get_debuglevel() > 0) {
		daemonize = 0;
	}

	if (daemonize) {
		if (daemon_retval_init()) {
			flog(LOG_ERR, "Could not initialize daemon IPC.");
			exit(1);
		}

		pid = daemon_fork();
		if (-1 == pid) {
			flog(LOG_ERR, "Could not fork: %s", strerror(errno));
			daemon_retval_done();
			exit(1);
		}

		if (0 < pid) {
			if (daemon_retval_wait(0)) {
				flog(LOG_ERR, "Could not daemonize.");
				exit(1);
			}
			exit(0);
		}

		daemon_pid_file_proc = get_pidfile;
		if (daemon_pid_file_is_running() >= 0) {
			flog(LOG_ERR, "radvd already running, terminating.");
			daemon_retval_send(1);
			exit(1);
		}
		if (daemon_pid_file_create()) {
			flog(LOG_ERR, "Cannot create radvd PID file, terminating: %s",
					strerror(errno));
			daemon_retval_send(2);
			exit(1);
		}
		daemon_retval_send(0);
	}

	/*
	 *	config signal handlers
	 */
	signal(SIGHUP, sighup_handler);
	signal(SIGTERM, sigterm_handler);
	signal(SIGINT, sigint_handler);
	signal(SIGUSR1, sigusr1_handler);

	config_interfaces();
	/* Must do this after sock is opened */
	validate_configuration_or_die();

	main_loop();
	flog(LOG_INFO, "sending stop adverts", pidfile);
	stop_adverts();

	if (daemonize) {
		flog(LOG_INFO, "removing %s", pidfile);
		unlink(pidfile);
	}

	return 0;
}


const char *get_pidfile(void) {
	return pidfile;
}

void main_loop(void)
{
	struct pollfd fds[2];

	memset(fds, 0, sizeof(fds));

	fds[0].fd = sock;
	fds[0].events = POLLIN;
	fds[0].revents = 0;

#if HAVE_NETLINK
	fds[1].fd = netlink_socket();
	fds[1].events = POLLIN;
	fds[1].revents = 0;
#else
	fds[1].fd = -1;
	fds[1].events = 0;
	fds[1].revents = 0;
#endif

	for (;;) {
		struct Interface *next = NULL;
		struct Interface *iface;
		int timeout = -1;
		int alive_count = 0;
		int rc;

		if (IfaceList) {
			timeout = next_time_msec(IfaceList);
			next = IfaceList;
			for (iface = IfaceList; iface; iface = iface->next) {
				int t;
				t = next_time_msec(iface);
				if (timeout > t) {
					timeout = t;
					next = iface;
				}
				if (!iface->is_dead) {
					++alive_count;
				}
			}
		}

		if (alive_count > 0) {
			dlog(LOG_DEBUG, 5, "polling for %g seconds.", timeout/1000.0);
		} else {
			dlog(LOG_DEBUG, 3, "all interfaces dead...going to sleep.");
			timeout = -1;
		}

		rc = poll(fds, sizeof(fds)/sizeof(fds[0]), timeout);

		if (rc > 0) {
			if (fds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) {
				flog(LOG_WARNING, "socket error on fds[0].fd");
			}
			else if (fds[0].revents & POLLIN) {
				int len, hoplimit;
				struct sockaddr_in6 rcv_addr;
				struct in6_pktinfo *pkt_info = NULL;
				unsigned char msg[MSG_SIZE_RECV];

				len = recv_rs_ra(msg, &rcv_addr, &pkt_info, &hoplimit);
				if (len > 0) {
					process(IfaceList, msg, len,
						&rcv_addr, pkt_info, hoplimit);
				}
			}
#ifdef HAVE_NETLINK
			if (fds[1].revents & (POLLERR | POLLHUP | POLLNVAL)) {
				flog(LOG_WARNING, "socket error on fds[1].fd");
			}
			else if (fds[1].revents & POLLIN) {
				process_netlink_msg(fds[1].fd);
			}
#endif
		}
		else if ( rc == 0 ) {
			if (next)
				timer_handler(next);
		}
		else if ( rc == -1 ) {
			dlog(LOG_INFO, 3, "poll returned early: %s", strerror(errno));
		}

		if (sigterm_received || sigint_received) {
			flog(LOG_WARNING, "Exiting, sigterm or sigint received.\n");
			break;
		}

		if (sighup_received)
		{
			dlog(LOG_INFO, 3, "sig hup received.\n");
			sighup_received = 0;
		}

		if (sigusr1_received)
		{
			dlog(LOG_INFO, 3, "sig usr1 received.\n");
			reset_prefix_lifetimes();
			sigusr1_received = 0;
		}
	}
}


void
timer_handler(void *data)
{
	struct Interface *iface = (struct Interface *) data;
	double next;

	dlog(LOG_DEBUG, 4, "timer_handler called for %s", iface->Name);

	/* First we need to check that the interface hasn't been removed or deactivated */
	if (check_device(iface) < 0) {
		if (!iface->is_dead) {
			if (iface->IgnoreIfMissing) {
				dlog(LOG_DEBUG, 4, "interface %s does not exist, ignoring the interface", iface->Name);
			}
			else {
				flog(LOG_WARNING, "interface %s does not exist, ignoring the interface", iface->Name);
			}
			iface->is_dead = 1;
		}
	}
	else if (iface->is_dead == 1) {
		/* check_device was successful, act if it has failed previously */
		flog(LOG_WARNING, "interface %s seems to have come back up, proceeding normally", iface->Name);
		iface->is_dead = 0;
		iface->init_racount = 0;
	}


	next = rand_between(iface->MinRtrAdvInterval, iface->MaxRtrAdvInterval);

	if (!iface->is_dead) {
		int rc;

		rc = send_ra_forall(iface, NULL);

		if (iface->init_racount < MAX_INITIAL_RTR_ADVERTISEMENTS) {
			iface->init_racount++;
			next = min(MAX_INITIAL_RTR_ADVERT_INTERVAL, next);
			if (rc == 0) {
				dlog(LOG_DEBUG, 4, "init RA %d sent on %s", iface->init_racount, iface->Name);
			}
		}
	}
	iface->next_multicast = next_timeval(next);
}


void
config_interface(struct Interface * iface)
{
	if (iface->AdvLinkMTU)
		set_interface_linkmtu(iface->Name, iface->AdvLinkMTU);
	if (iface->AdvCurHopLimit)
		set_interface_curhlim(iface->Name, iface->AdvCurHopLimit);
	if (iface->AdvReachableTime)
		set_interface_reachtime(iface->Name, iface->AdvReachableTime);
	if (iface->AdvRetransTimer)
		set_interface_retranstimer(iface->Name, iface->AdvRetransTimer);
}


void
config_interfaces(void)
{
	struct Interface *iface;
	for (iface=IfaceList; iface; iface=iface->next)
	{
		config_interface(iface);
	}
}


void
stop_adverts(void)
{
	struct Interface *iface;

	/*
	 *	send final RA (a SHOULD in RFC4861 section 6.2.5)
	 */

	for (iface=IfaceList; iface; iface=iface->next) {
		if( ! iface->UnicastOnly ) {
			if (iface->AdvSendAdvert) {
				/* send a final advertisement with zero Router Lifetime */
				iface->cease_adv = 1;
				send_ra_forall(iface, NULL);
			}
		}
	}
}


void
sighup_handler(int sig)
{
	/* Linux has "one-shot" signals, reinstall the signal handler */
	signal(SIGHUP, sighup_handler);
	sighup_received = 1;
}

void
sigterm_handler(int sig)
{
	/* Linux has "one-shot" signals, reinstall the signal handler */
	signal(SIGTERM, sigterm_handler);

	++sigterm_received;

	if(sigterm_received > 1){
		abort();
	}
}

void
sigint_handler(int sig)
{
	/* Linux has "one-shot" signals, reinstall the signal handler */
	signal(SIGINT, sigint_handler);

	++sigint_received;

	if(sigint_received > 1){
		abort();
	}
}

void sigusr1_handler(int sig)
{
	/* Linux has "one-shot" signals, reinstall the signal handler */
	signal(SIGUSR1, sigusr1_handler);

	sigusr1_received = 1;
}

void reset_prefix_lifetimes(void)
{
	struct Interface *iface;

	flog(LOG_INFO, "Resetting prefix lifetimes");
	
	for (iface = IfaceList; iface; iface = iface->next) 
	{
		struct PrefixSpec *spec;

		for (spec = iface->PrefixSpec; spec; spec = spec->next) 
		{
			if (spec->options->DecrementLifetimesFlag)
			{
				struct Prefix * prefix = spec->prefix;

				while (prefix) {
					char pfx_str[INET6_ADDRSTRLEN];
					print_addr(&prefix->addr, pfx_str, sizeof(pfx_str));

					dlog(LOG_DEBUG, 4, "%s/%u%%%s plft reset from %u to %u secs", 
						pfx_str, prefix->len, iface->Name,
						spec->options->curr_preferredlft, spec->options->AdvPreferredLifetime);

					dlog(LOG_DEBUG, 4, "%s/%u%%%s vlft reset from %u to %u secs",
						pfx_str, prefix->len, iface->Name,
						spec->options->curr_validlft, spec->options->AdvValidLifetime);
					prefix = prefix->next;
				}

				spec->options->curr_validlft = spec->options->AdvValidLifetime;
				spec->options->curr_preferredlft = spec->options->AdvPreferredLifetime;

			}
		}
		
	}

}

int
drop_root_privileges(const char *username)
{
	struct passwd *pw = NULL;
	pw = getpwnam(username);
	if (pw) {
		if (initgroups(username, pw->pw_gid) != 0 || setgid(pw->pw_gid) != 0 || setuid(pw->pw_uid) != 0) {
			flog(LOG_ERR, "Couldn't change to '%.32s' uid=%d gid=%d",
					username, pw->pw_uid, pw->pw_gid);
			return (-1);
		}
	}
	else {
		flog(LOG_ERR, "Couldn't find user '%.32s'", username);
		return (-1);
	}
	return 0;
}

int
check_conffile_perm(const char *username, const char *conf_file)
{
	struct stat stbuf;
	struct passwd *pw = NULL;
	FILE *fp = fopen(conf_file, "r");

	if (fp == NULL) {
		flog(LOG_ERR, "can't open %s: %s", conf_file, strerror(errno));
		return (-1);
	}
	fclose(fp);

	if (!username)
		username = "root";

	pw = getpwnam(username);

	if (stat(conf_file, &stbuf) || pw == NULL)
		return (-1);

	if (stbuf.st_mode & S_IWOTH) {
                flog(LOG_ERR, "Insecure file permissions (writable by others): %s", conf_file);
		return (-1);
        }

	/* for non-root: must not be writable by self/own group */
	if (strncmp(username, "root", 5) != 0 &&
	    ((stbuf.st_mode & S_IWGRP && pw->pw_gid == stbuf.st_gid) ||
	     (stbuf.st_mode & S_IWUSR && pw->pw_uid == stbuf.st_uid))) {
                flog(LOG_ERR, "Insecure file permissions (writable by self/group): %s", conf_file);
		return (-1);
        }

        return 0;
}

void extract_ipv6_address(char const * addr_str, struct in6_addr * addr)
{
	int i;
	memset(addr, 0, 16);
	for (i = 0; i < 32; ++i) {
		int const j = i / 2;
		int const k = 4 * ((i+1) % 2); 
		unsigned char x;
		if (addr_str[i] >= '0' && addr_str[i] <= '9') {
			x = addr_str[i] - '0';
		}   
		else if (addr_str[i] >= 'A' && addr_str[i] <= 'F') {
			x = addr_str[i] - 'A' + 10; 
		}   
		else {
			x = addr_str[i] - 'a' + 10; 
		}   
		addr->s6_addr[j] |= (x << k); 
	}   
}

struct ipv6_route
{
    struct ipv6_route * next;
    int if_index;
    struct in6_addr addr;
    int len;
};

struct ipv6_route * get_route_table(void)
{
   	struct ipv6_route * route_table = 0;
    FILE * in = fopen(PROC_SYS_IP6_ROUTE, "r");

	if (in) {

    	char dest[33], iface_name[64];
    	int len;
		int rc = fscanf(in, " %s %x %*s %*s %*s %*s %*s %*s %*s %s", dest, &len, iface_name);

		while (rc == 3) {

    		struct in6_addr addr;

			extract_ipv6_address(dest, &addr);

			if (!IN6_IS_ADDR_LINKLOCAL(&addr) && !IN6_IS_ADDR_MULTICAST(&addr)) {
				struct ipv6_route * route_table_rec;
				route_table_rec = malloc(sizeof(struct ipv6_route));
				if (!route_table_rec) {
					flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
					abort();
				}
				memset(route_table_rec, 0, sizeof(struct ipv6_route));
				route_table_rec->if_index = if_nametoindex(iface_name);
				route_table_rec->addr = addr;
				route_table_rec->len = len;
				route_table_rec->next = route_table;
				route_table = route_table_rec;
			}

			rc = fscanf(in, " %s %x %*s %*s %*s %*s %*s %*s %*s %s", dest, &len, iface_name);
		}

		fclose(in);
	}

	return route_table;
}

void free_route_table(struct ipv6_route * route_table)
{
	while (route_table) {
		struct ipv6_route * next = route_table->next;
		free(route_table);
		route_table = next;
	}
}

int already_advrting(struct in6_addr const * addr, int len, struct PrefixSpec const * spec)
{
	while (spec) {
		struct Prefix const * prefix = spec->prefix;

		while (prefix) {
			if (prefix->len == len && memcmp(&prefix->addr, addr, sizeof(struct in6_addr)) == 0) {
				return 1;
			}
			prefix = prefix->next;
		}
		spec = spec->next;
	}

	return 0;
}

void
validate_configuration_or_die(void)
{
	struct Interface *iface;
	struct ipv6_route * route_table = 0;

	for (iface = IfaceList; iface; iface = iface->next) {
        struct PrefixSpec * spec = iface->PrefixSpec;

		iface->if_index = if_nametoindex(iface->Name);

		while (spec) {
			char pfx_str[INET6_ADDRSTRLEN];

			if (!spec->prefix) {
				struct ipv6_route * route_table_rec;
				dlog(LOG_DEBUG, 4, "initilializing auto prefix on %s", iface->Name);

				if (!route_table) {
					route_table = get_route_table();
				}

				route_table_rec = route_table;
				while (route_table_rec) {
					if (route_table_rec->if_index == iface->if_index) {
						if (!already_advrting(&route_table_rec->addr, route_table_rec->len, iface->PrefixSpec)) {
							struct Prefix * prefix = malloc(sizeof(struct Prefix));
							if (!prefix) {
								flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
								abort();
							}
							prefix->addr = route_table_rec->addr;
							prefix->len = route_table_rec->len;
							prefix->next = spec->prefix;
							spec->prefix = prefix;
							print_addr(&route_table_rec->addr, pfx_str, sizeof(pfx_str));
							dlog(LOG_DEBUG, 4, "interface %s will advertise auto prefix %s/%d", iface->Name, pfx_str, route_table_rec->len);
						}
					}
					route_table_rec = route_table_rec->next;
				}
			}
			else {
				print_addr(&spec->prefix->addr, pfx_str, sizeof(pfx_str));
				dlog(LOG_DEBUG, 3, "interface %s will advertise %s/%d", iface->Name, pfx_str, spec->prefix->len);
			}
			spec = spec->next;
		}
	}

	if (route_table) {
		free_route_table(route_table);
	}

	for (iface = IfaceList; iface; iface = iface->next) {
        struct PrefixSpec * spec = iface->PrefixSpec;
        struct PrefixSpec ** spec_ptr = &iface->PrefixSpec;

		while (spec) {
			struct PrefixSpec * next_spec = spec->next;

			if (!spec->prefix) {
				*spec_ptr = spec->next;
				free(spec);
				dlog(LOG_DEBUG, 5, "trimmed an overridden auto prefix on interface %s", iface->Name);
			}
			else {
				spec_ptr = &spec->next;
			}
			spec = next_spec;
		}
	}

	for (iface = IfaceList; iface; iface = iface->next) {
		if (check_device(iface) < 0) {
			if (iface->IgnoreIfMissing) {
				dlog(LOG_DEBUG, 4, "interface %s did not exist, ignoring the interface", iface->Name);
			}
			else {
				flog(LOG_ERR, "interface %s does not exist", iface->Name);
				exit(1);
			}
		}

		if (setup_deviceinfo(iface) < 0) {
			if (!iface->IgnoreIfMissing) {
				flog(LOG_ERR, "setup_deviceinfo on %s failed", iface->Name);
				exit(1);
			}
		}

		if (check_iface(iface) < 0) {
			if (!iface->IgnoreIfMissing) {
				flog(LOG_ERR, "check_iface on %s failed", iface->Name);
				exit(1);
			}
		}

		if (setup_linklocal_addr(iface) < 0) {
			if (!iface->IgnoreIfMissing) {
				flog(LOG_ERR, "setup_linklocal_addr on %s failed", iface->Name);
				exit(1);
			}
		}

		if (setup_allrouters_membership(iface) < 0) {
			if (!iface->IgnoreIfMissing) {
				flog(LOG_ERR, "setup_allrouters_membership on %s failed", iface->Name);
				exit(1);
			}
		}
	}
}


int
check_ip6_forwarding(void)
{
#ifdef HAVE_SYS_SYSCTL_H
	int forw_sysctl[] = { SYSCTL_IP6_FORWARDING };
#endif
	int value;
	size_t size = sizeof(value);
	FILE *fp = NULL;
	static int warned = 0;

#ifdef __linux__
	fp = fopen(PROC_SYS_IP6_FORWARDING, "r");
	if (fp) {
		int rc = fscanf(fp, "%d", &value);
		if(rc != 1){
			flog(LOG_ERR, "cannot read value from %s: %s", PROC_SYS_IP6_FORWARDING, strerror(errno));
			exit(1);
		}
		fclose(fp);
	}
	else {
		flog(LOG_DEBUG, "Correct IPv6 forwarding procfs entry not found, "
	                       "perhaps the procfs is disabled, "
	                        "or the kernel interface has changed?");
		value = -1;
	}
#endif /* __linux__ */

#ifdef HAVE_SYS_SYSCTL_H
	if (!fp && sysctl(forw_sysctl, sizeof(forw_sysctl)/sizeof(forw_sysctl[0]),
	    &value, &size, NULL, 0) < 0) {
		flog(LOG_DEBUG, "Correct IPv6 forwarding sysctl branch not found, "
			"perhaps the kernel interface has changed?");
		return(0);	/* this is of advisory value only */
	}
#endif

	if (value != 1 && !warned) {
		warned = 1;
		flog(LOG_DEBUG, "IPv6 forwarding setting is: %u, should be 1", value);
		return(-1);
	}

	return(0);
}

void
version(void)
{
	fprintf(stderr, "Version: %s\n\n", VERSION);
	fprintf(stderr, "Compiled in settings:\n");
	fprintf(stderr, "  default config file		\"%s\"\n", PATH_RADVD_CONF);
	fprintf(stderr, "  default pidfile		\"%s\"\n", PATH_RADVD_PID);
	fprintf(stderr, "  default logfile		\"%s\"\n", PATH_RADVD_LOG);
	fprintf(stderr, "  default syslog facility	%d\n", LOG_FACILITY);
	fprintf(stderr, "Please send bug reports or suggestions to %s.\n",
		CONTACT_EMAIL);

	exit(1);
}

void
usage(void)
{
	fprintf(stderr, "usage: %s %s\n", pname, usage_str);
	exit(1);
}

