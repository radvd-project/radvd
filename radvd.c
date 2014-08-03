/*
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
#include <sys/file.h>

#ifdef HAVE_GETOPT_LONG

/* *INDENT-OFF* */
static char usage_str[] = {
"\n"
"  -c, --configtest        Parse the config file and exit.\n"
"  -C, --config=PATH       Sets the config file.  Default is /etc/radvd.d.\n"
"  -d, --debug=NUM         Sets the debug level.  Values can be 1, 2, 3, 4 or 5.\n"
"  -f, --facility=NUM      Sets the logging facility.\n"
"  -h, --help              Show this help screen.\n"
"  -l, --logfile=PATH      Sets the log file.\n"
"  -m, --logmethod=X       Sets the log method to one of: syslog, stderr, stderr_syslog, logfile, or none.\n"
"  -p, --pidfile=PATH      Sets the pid file.\n"
"  -t, --chrootdir=PATH    Chroot to the specified path.\n"
"  -u, --username=USER     Switch to the specified user.\n"
"  -n, --nodaemon          Prevent the daemonizing.\n"
"  -v, --version           Print the version and quit.\n"
};

static struct option prog_opt[] = {
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
	{"nodaemon", 0, 0, 'n'},
	{NULL, 0, 0, 0}
};

#else

static char usage_str[] = {
"[-hsvcn] [-d level] [-C config_path] [-m log_method] [-l log_file]\n"
"\t[-f facility] [-p pid_file] [-u username] [-t chrootdir]"

};
/* *INDENT-ON* */

#endif

static volatile int sighup_received = 0;
static volatile int sigterm_received = 0;
static volatile int sigint_received = 0;
static volatile int sigusr1_received = 0;

static void sighup_handler(int sig);
static void sigterm_handler(int sig);
static void sigint_handler(int sig);
static void sigusr1_handler(int sig);
static void timer_handler(int sock, struct Interface *iface);
static void config_interface(struct Interface *iface);
static void kickoff_adverts(int sock, struct Interface *iface);
static void stop_advert_foo(struct Interface *iface, void *data);
static void stop_adverts(int sock, struct Interface *ifaces);
static void version(void);
static void usage(char const *pname);
static int drop_root_privileges(const char *);
static int check_conffile_perm(const char *, const char *);
static void setup_iface_foo(struct Interface *iface, void *data);
static void setup_ifaces(int sock, struct Interface *ifaces);
static void main_loop(int sock, struct Interface *ifaces, char const *conf_path);
static void reset_prefix_lifetimes_foo(struct Interface *iface, void *data);
static void reset_prefix_lifetimes(struct Interface *ifaces);
static struct Interface *reload_config(int sock, struct Interface *ifaces, char const *conf_path);
static void do_daemonize(int log_method);
static int open_and_lock_pid_file(char const * daemon_pid_file_ident);
static int write_pid_file(int pid_fd);

int main(int argc, char *argv[])
{
	int c;
	int log_method = L_STDERR_SYSLOG;
	char *logfile = PATH_RADVD_LOG;
	int facility = LOG_FACILITY;
	char *username = NULL;
	char *chrootdir = NULL;
	int configtest = 0;
	int daemonize = 1;

	char const *pname = ((pname = strrchr(argv[0], '/')) != NULL) ? pname + 1 : argv[0];

	srand((unsigned int)time(NULL));

	char const *conf_path = PATH_RADVD_CONF;
	char const *daemon_pid_file_ident = PATH_RADVD_PID;

	/* parse args */
#define OPTIONS_STR "d:C:l:m:p:t:u:vhcn"
#ifdef HAVE_GETOPT_LONG
	int opt_idx;
	while ((c = getopt_long(argc, argv, OPTIONS_STR, prog_opt, &opt_idx)) > 0)
#else
	while ((c = getopt(argc, argv, OPTIONS_STR)) > 0)
#endif
	{
		switch (c) {
		case 'C':
			conf_path = optarg;
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
			daemon_pid_file_ident = optarg;
			break;
		case 'm':
			if (!strcmp(optarg, "syslog")) {
				log_method = L_SYSLOG;
			} else if (!strcmp(optarg, "stderr_syslog")) {
				log_method = L_STDERR_SYSLOG;
			} else if (!strcmp(optarg, "stderr")) {
				log_method = L_STDERR;
			} else if (!strcmp(optarg, "logfile")) {
				log_method = L_LOGFILE;
			} else if (!strcmp(optarg, "none")) {
				log_method = L_NONE;
			} else {
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
		case 'n':
			daemonize = 0;
			break;
		case 'h':
			usage(pname);
#ifdef HAVE_GETOPT_LONG
		case ':':
			fprintf(stderr, "%s: option %s: parameter expected\n", pname, prog_opt[opt_idx].name);
			exit(1);
#endif
		case '?':
			exit(1);
		}
	}

	/* TODO: Seems like this chroot'ing should happen *after* daemonizing for
	 * the sake of the PID file. */
	if (chrootdir) {
		if (!username) {
			fprintf(stderr, "Chroot as root is not safe, exiting\n");
			exit(1);
		}

		if (chroot(chrootdir) == -1) {
			perror("chroot");
			exit(1);
		}

		if (chdir("/") == -1) {
			perror("chdir");
			exit(1);
		}
		/* username will be switched later */
	}

	if (configtest) {
		set_debuglevel(1);
		log_method = L_STDERR;
	}

	if (log_open(log_method, pname, logfile, facility) < 0) {
		perror("log_open");
		exit(1);
	}

	if (!configtest) {
		flog(LOG_INFO, "version %s started", VERSION);

		/* Calling privsep here, before opening the socket and reading the config
		 * file, ensures we're not going to be wasting resources in the privsep
		 * process. */
		dlog(LOG_DEBUG, 3, "Initializing privsep");
		if (privsep_init(username, chrootdir) < 0) {
			flog(LOG_INFO, "Failed to initialize privsep.");
			exit(1);
		}
	}

	/* check that 'other' cannot write the file
	 * for non-root, also that self/own group can't either
	 */
	if (check_conffile_perm(username, conf_path) != 0) {
		if (get_debuglevel() == 0) {
			flog(LOG_ERR, "Exiting, permissions on conf_file invalid.");
			exit(1);
		} else
			flog(LOG_WARNING, "Insecure file permissions, but continuing anyway");
	}

	/* parse config file */
	struct Interface *ifaces = NULL;
	if ((ifaces = readin_config(conf_path)) == 0) {
		flog(LOG_ERR, "Exiting, failed to read config file.");
		exit(1);
	}

	if (configtest) {
		free_ifaces(ifaces);
		exit(0);
	}

	/* get a raw socket for sending and receiving ICMPv6 messages */
	int sock = open_icmpv6_socket();
	if (sock < 0) {
		perror("open_icmpv6_socket");
		exit(1);
	}

	/* if we know how to do it, check whether forwarding is enabled */
	if (check_ip6_forwarding()) {
		flog(LOG_WARNING, "IPv6 forwarding seems to be disabled, but continuing anyway.");
	}

	int const pid_fd = open_and_lock_pid_file(daemon_pid_file_ident);

	/*
	 * okay, config file is read in, socket and stuff is setup, so
	 * lets fork now...
	 */
	if (daemonize) {
		do_daemonize(log_method);
	}

	if (0 != write_pid_file(pid_fd)) {
		flog(LOG_ERR, "Unable to write PID to %s", daemon_pid_file_ident);
		exit(-1);
	}

	if (username) {
		if (drop_root_privileges(username) < 0) {
			perror("drop_root_privileges");
			flog(LOG_ERR, "unable to drop root privileges");
			exit(1);
		}
	}

	setup_ifaces(sock, ifaces);
	main_loop(sock, ifaces, conf_path);
	stop_adverts(sock, ifaces);

	flog(LOG_INFO, "removing %s", daemon_pid_file_ident);
	unlink(daemon_pid_file_ident);
	close(pid_fd);

	if (ifaces)
		free_ifaces(ifaces);

	if (chrootdir)
		free(chrootdir);

	if (username)
		free(username);

	flog(LOG_INFO, "returning from radvd main");
	log_close();

	return 0;
}

static void main_loop(int sock, struct Interface *ifaces, char const *conf_path)
{
	struct pollfd fds[2];
	sigset_t sigmask;
	sigset_t sigempty;
	struct sigaction sa;

	sigemptyset(&sigempty);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGHUP);
	sigaddset(&sigmask, SIGTERM);
	sigaddset(&sigmask, SIGINT);
	sigaddset(&sigmask, SIGUSR1);
	sigprocmask(SIG_BLOCK, &sigmask, NULL);

	sa.sa_handler = sighup_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGHUP, &sa, 0);

	sa.sa_handler = sigterm_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGTERM, &sa, 0);

	sa.sa_handler = sigint_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, 0);

	sa.sa_handler = sigusr1_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGUSR1, &sa, 0);

	memset(fds, 0, sizeof(fds));

	fds[0].fd = sock;
	fds[0].events = POLLIN;

#if HAVE_NETLINK
	fds[1].fd = netlink_socket();
	fds[1].events = POLLIN;
#else
	fds[1].fd = -1;
#endif

	for (;;) {
		struct timespec *tsp = 0;

		struct Interface *next_iface_to_expire = find_iface_by_time(ifaces);
		if (next_iface_to_expire) {
			static struct timespec ts;
			int timeout = next_time_msec(next_iface_to_expire);
			ts.tv_sec = timeout / 1000;
			ts.tv_nsec = (timeout - 1000 * ts.tv_sec) * 1000000;
			tsp = &ts;
			dlog(LOG_DEBUG, 1, "polling for %g seconds. Next iface is %s.", timeout / 1000.0,
			     next_iface_to_expire->props.name);
		} else {
			dlog(LOG_DEBUG, 1, "No iface is next. Polling indefinitely.");
		}

		int rc = ppoll(fds, sizeof(fds) / sizeof(fds[0]), tsp, &sigempty);

		if (rc > 0) {
#ifdef HAVE_NETLINK
			if (fds[1].revents & (POLLERR | POLLHUP | POLLNVAL)) {
				flog(LOG_WARNING, "socket error on fds[1].fd");
			} else if (fds[1].revents & POLLIN) {
				process_netlink_msg(fds[1].fd, ifaces);
			}
#endif

			if (fds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) {
				flog(LOG_WARNING, "socket error on fds[0].fd");
			} else if (fds[0].revents & POLLIN) {
				int len, hoplimit;
				struct sockaddr_in6 rcv_addr;
				struct in6_pktinfo *pkt_info = NULL;
				unsigned char msg[MSG_SIZE_RECV];

				len = recv_rs_ra(sock, msg, &rcv_addr, &pkt_info, &hoplimit);
				if (len > 0 && pkt_info) {
					process(sock, ifaces, msg, len, &rcv_addr, pkt_info, hoplimit);
				} else if (!pkt_info) {
					dlog(LOG_INFO, 4, "recv_rs_ra returned null pkt_info.");
				} else if (len <= 0) {
					dlog(LOG_INFO, 4, "recv_rs_ra returned len <= 0: %d", len);
				}
			}
		} else if (rc == 0) {
			if (next_iface_to_expire)
				timer_handler(sock, next_iface_to_expire);
		} else if (rc == -1) {
			dlog(LOG_INFO, 3, "poll returned early: %s", strerror(errno));
		}

		if (sigint_received) {
			flog(LOG_WARNING, "Exiting, %d sigint(s) received.", sigint_received);
			break;
		}

		if (sigterm_received) {
			flog(LOG_WARNING, "Exiting, %d sigterm(s) received.", sigterm_received);
			break;
		}

		if (sighup_received) {
			dlog(LOG_INFO, 3, "sig hup received.");
			ifaces = reload_config(sock, ifaces, conf_path);
			sighup_received = 0;
		}

		if (sigusr1_received) {
			dlog(LOG_INFO, 3, "sig usr1 received.");
			reset_prefix_lifetimes(ifaces);
			sigusr1_received = 0;
		}

	}
}

static void do_daemonize(int log_method)
{
	int rc = -1;

	if (L_STDERR_SYSLOG == log_method || L_STDERR == log_method) {
		rc = daemon(1, 1);
	} else {
		rc = daemon(0, 0);
	}

	if (-1 == rc) {
		flog(LOG_ERR, "Unable to daemonize: %s", strerror(errno));
		exit(-1);
	}
}

static int open_and_lock_pid_file(char const * daemon_pid_file_ident)
{
	dlog(LOG_DEBUG, 3, "radvd startup PID is %d", getpid());

	int pid_fd = open(daemon_pid_file_ident, O_CREAT | O_RDWR, 0644);
	if (-1 == pid_fd) {
		flog(LOG_ERR, "Unable to open pid file, %s: %s", daemon_pid_file_ident, strerror(errno));
		exit(-1);
	} else {
		dlog(LOG_DEBUG, 4, "opened pid file %s", daemon_pid_file_ident);
	}

	int lock = flock(pid_fd, LOCK_EX | LOCK_NB);
	if (0 != lock) {
		flog(LOG_ERR, "Unable to lock pid file, %s: %s", daemon_pid_file_ident, strerror(errno));
		exit(-1);
	} else {
		dlog(LOG_DEBUG, 4, "locked pid file %s", daemon_pid_file_ident);
	}

	return pid_fd;
}

static int write_pid_file(int pid_fd)
{
	char pid_str[20] = {""};
	sprintf(pid_str, "%d", getpid());
	dlog(LOG_DEBUG, 3, "radvd PID is %s", pid_str);
	size_t len = strlen(pid_str);
	int rc = write(pid_fd, pid_str, len);
	if (rc != (int)len) {
		return -1;
	}
	char newline[] = {"\n"};
	len = strlen(newline);
	rc = write(pid_fd, newline, len);
	if (rc != (int)len) {
		return -1;
	}
	return 0;

}

static void timer_handler(int sock, struct Interface *iface)
{
	dlog(LOG_DEBUG, 1, "timer_handler called for %s", iface->props.name);

	if (send_ra_forall(sock, iface, NULL) != 0) {
		dlog(LOG_DEBUG, 4, "send_ra_forall failed on interface %s", iface->props.name);
	}

	double next = rand_between(iface->MinRtrAdvInterval, iface->MaxRtrAdvInterval);

	reschedule_iface(iface, next);
}

static void config_interface(struct Interface *iface)
{
	if (iface->AdvLinkMTU)
		set_interface_linkmtu(iface->props.name, iface->AdvLinkMTU);
	if (iface->ra_header_info.AdvCurHopLimit)
		set_interface_curhlim(iface->props.name, iface->ra_header_info.AdvCurHopLimit);
	if (iface->ra_header_info.AdvReachableTime)
		set_interface_reachtime(iface->props.name, iface->ra_header_info.AdvReachableTime);
	if (iface->ra_header_info.AdvRetransTimer)
		set_interface_retranstimer(iface->props.name, iface->ra_header_info.AdvRetransTimer);
}

/*
 *      send initial advertisement and set timers
 */
static void kickoff_adverts(int sock, struct Interface *iface)
{
	clock_gettime(CLOCK_MONOTONIC, &iface->times.last_ra_time);

	if (iface->UnicastOnly)
		return;

	clock_gettime(CLOCK_MONOTONIC, &iface->times.last_multicast);

	/* send an initial advertisement */
	if (send_ra_forall(sock, iface, NULL) != 0) {
		dlog(LOG_DEBUG, 4, "send_ra_forall failed on interface %s", iface->props.name);
	}

	double next = min(MAX_INITIAL_RTR_ADVERT_INTERVAL, iface->MaxRtrAdvInterval);
	reschedule_iface(iface, next);
}

static void stop_advert_foo(struct Interface *iface, void *data)
{
	if (!iface->UnicastOnly) {
		/* send a final advertisement with zero Router Lifetime */
		dlog(LOG_DEBUG, 4, "stopping all adverts on %s.", iface->props.name);
		iface->state_info.cease_adv = 1;
		int sock = *(int *)data;
		send_ra_forall(sock, iface, NULL);
	}
}

static void stop_adverts(int sock, struct Interface *ifaces)
{
	flog(LOG_INFO, "sending stop adverts");
	/*
	 *      send final RA (a SHOULD in RFC4861 section 6.2.5)
	 */
	for_each_iface(ifaces, stop_advert_foo, &sock);
}

static void setup_iface_foo(struct Interface *iface, void *data)
{
	int sock = *(int *)data;

	if (setup_iface(sock, iface) < 0) {
		if (iface->IgnoreIfMissing) {
			dlog(LOG_DEBUG, 4, "interface %s does not exist or is not set up properly, ignoring the interface",
			     iface->props.name);
			return;
		} else {
			flog(LOG_ERR, "interface %s does not exist or is not set up properly", iface->props.name);
			exit(1);
		}
	}

	config_interface(iface);
	kickoff_adverts(sock, iface);
}

static void setup_ifaces(int sock, struct Interface *ifaces)
{
	for_each_iface(ifaces, setup_iface_foo, &sock);
}

static struct Interface *reload_config(int sock, struct Interface *ifaces, char const *conf_path)
{
	free_ifaces(ifaces);

	flog(LOG_INFO, "attempting to reread config file");

	/* reread config file */
	ifaces = readin_config(conf_path);
	if (!ifaces) {
		flog(LOG_ERR, "Exiting, failed to read config file.");
		exit(1);
	}
	setup_ifaces(sock, ifaces);

	flog(LOG_INFO, "resuming normal operation");

	return ifaces;
}

static void sighup_handler(int sig)
{
	sighup_received = 1;
}

static void sigterm_handler(int sig)
{
	++sigterm_received;

	if (sigterm_received > 2) {
		abort();
	}
}

static void sigint_handler(int sig)
{
	++sigint_received;

	if (sigint_received > 2) {
		abort();
	}
}

static void sigusr1_handler(int sig)
{
	sigusr1_received = 1;
}

static void reset_prefix_lifetimes_foo(struct Interface *iface, void *data)
{
	flog(LOG_INFO, "Resetting prefix lifetimes on %s", iface->props.name);

	for (struct AdvPrefix * prefix = iface->AdvPrefixList; prefix; prefix = prefix->next) {
		if (prefix->DecrementLifetimesFlag) {
			char pfx_str[INET6_ADDRSTRLEN];
			addrtostr(&prefix->Prefix, pfx_str, sizeof(pfx_str));
			dlog(LOG_DEBUG, 4, "%s/%u%%%s plft reset from %u to %u secs", pfx_str, prefix->PrefixLen,
			     iface->props.name, prefix->curr_preferredlft, prefix->AdvPreferredLifetime);
			dlog(LOG_DEBUG, 4, "%s/%u%%%s vlft reset from %u to %u secs", pfx_str, prefix->PrefixLen,
			     iface->props.name, prefix->curr_validlft, prefix->AdvValidLifetime);
			prefix->curr_validlft = prefix->AdvValidLifetime;
			prefix->curr_preferredlft = prefix->AdvPreferredLifetime;
		}
	}
}

static void reset_prefix_lifetimes(struct Interface *ifaces)
{
	for_each_iface(ifaces, reset_prefix_lifetimes_foo, 0);
}

static int drop_root_privileges(const char *username)
{
	struct passwd *pw = getpwnam(username);
	if (pw) {
		if (initgroups(username, pw->pw_gid) != 0 || setgid(pw->pw_gid) != 0 || setuid(pw->pw_uid) != 0) {
			flog(LOG_ERR, "Couldn't change to '%.32s' uid=%d gid=%d", username, pw->pw_uid, pw->pw_gid);
			return -1;
		}
	} else {
		flog(LOG_ERR, "Couldn't find user '%.32s'", username);
		return -1;
	}
	return 0;
}

static int check_conffile_perm(const char *username, const char *conf_file)
{
	FILE *fp = fopen(conf_file, "r");
	if (fp == NULL) {
		flog(LOG_ERR, "can't open %s: %s", conf_file, strerror(errno));
		return -1;
	}
	fclose(fp);

	if (!username)
		username = "root";

	struct passwd *pw = getpwnam(username);
	if (!pw) {
		return -1;
	}

	struct stat stbuf;
	if (0 != stat(conf_file, &stbuf)) {
		return -1;
	}

	if (stbuf.st_mode & S_IWOTH) {
		flog(LOG_ERR, "Insecure file permissions (writable by others): %s", conf_file);
		return -1;
	}

	/* for non-root: must not be writable by self/own group */
	if (strncmp(username, "root", 5) != 0 && ((stbuf.st_mode & S_IWGRP && pw->pw_gid == stbuf.st_gid)
						  || (stbuf.st_mode & S_IWUSR && pw->pw_uid == stbuf.st_uid))) {
		flog(LOG_ERR, "Insecure file permissions (writable by self/group): %s", conf_file);
		return -1;
	}

	return 0;
}

static void version(void)
{
	fprintf(stderr, "Version: %s\n\n", VERSION);
	fprintf(stderr, "Compiled in settings:\n");
	fprintf(stderr, "  default config file		\"%s\"\n", PATH_RADVD_CONF);
	fprintf(stderr, "  default pidfile		\"%s\"\n", PATH_RADVD_PID);
	fprintf(stderr, "  default logfile		\"%s\"\n", PATH_RADVD_LOG);
	fprintf(stderr, "  default syslog facility	%d\n", LOG_FACILITY);
	fprintf(stderr, "Please send bug reports or suggestions to %s.\n", CONTACT_EMAIL);

	exit(1);
}

static void usage(char const *pname)
{
	fprintf(stderr, "usage: %s %s\n", pname, usage_str);
	exit(1);
}
