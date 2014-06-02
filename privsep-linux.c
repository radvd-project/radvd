/*
 *
 *   Authors:
 *    Jim Paris			<jim@jtan.com>
 *    Pedro Roque		<roque@di.fc.ul.pt>
 *    Lars Fenneberg		<lf@elemental.net>
 *
 *   This software is Copyright 1996,1997,2008 by the above mentioned author(s),
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

static int set_interface_var(const char *iface, const char *var, const char *name, uint32_t val);
static void privsep_read_loop(void);

/* For reading or writing, depending on process */
static int pfd = -1;

/* Command types */
enum privsep_type {
	SET_INTERFACE_LINKMTU,
	SET_INTERFACE_CURHLIM,
	SET_INTERFACE_REACHTIME,
	SET_INTERFACE_RETRANSTIMER,
};

/* Command sent over pipe is a fixed size binary structure. */
struct privsep_command {
	int type;
	char iface[IFNAMSIZ];
	uint32_t val;
};

/* Privileged read loop */
static void privsep_read_loop(void)
{
	while (1) {
		struct privsep_command cmd;
		int ret = readn(pfd, &cmd, sizeof(cmd));
		if (ret <= 0) {
			/* Error or EOF, give up */
			if (ret < 0) {
				flog(LOG_ERR, "Exiting, privsep_read_loop had readn error: %s", strerror(errno));
			} else {
				flog(LOG_ERR, "Exiting, privsep_read_loop had readn return 0 bytes");
			}
		}
		if (ret != sizeof(cmd)) {
			/* Short read, ignore */
			return;
		}

		cmd.iface[IFNAMSIZ - 1] = '\0';

		switch (cmd.type) {

		case SET_INTERFACE_LINKMTU:
			if (cmd.val < MIN_AdvLinkMTU || cmd.val > MAX_AdvLinkMTU) {
				flog(LOG_ERR, "(privsep) %s: LinkMTU (%u) is not within the defined bounds, ignoring",
				     cmd.iface, cmd.val);
				break;
			}
			ret = set_interface_var(cmd.iface, PROC_SYS_IP6_LINKMTU, "LinkMTU", cmd.val);
			break;

		case SET_INTERFACE_CURHLIM:
			if (cmd.val < MIN_AdvCurHopLimit || cmd.val > MAX_AdvCurHopLimit) {
				flog(LOG_ERR, "(privsep) %s: CurHopLimit (%u) is not within the defined bounds, ignoring",
				     cmd.iface, cmd.val);
				break;
			}
			ret = set_interface_var(cmd.iface, PROC_SYS_IP6_CURHLIM, "CurHopLimit", cmd.val);
			break;

		case SET_INTERFACE_REACHTIME:
			if (cmd.val < MIN_AdvReachableTime || cmd.val > MAX_AdvReachableTime) {
				flog(LOG_ERR,
				     "(privsep) %s: BaseReachableTimer (%u) is not within the defined bounds, ignoring",
				     cmd.iface, cmd.val);
				break;
			}
			ret =
			    set_interface_var(cmd.iface, PROC_SYS_IP6_BASEREACHTIME_MS, "BaseReachableTimer (ms)", cmd.val);
			if (ret == 0)
				break;
			set_interface_var(cmd.iface, PROC_SYS_IP6_BASEREACHTIME, "BaseReachableTimer", cmd.val / 1000);
			break;

		case SET_INTERFACE_RETRANSTIMER:
			if (cmd.val < MIN_AdvRetransTimer || cmd.val > MAX_AdvRetransTimer) {
				flog(LOG_ERR, "(privsep) %s: RetransTimer (%u) is not within the defined bounds, ignoring",
				     cmd.iface, cmd.val);
				break;
			}
			ret = set_interface_var(cmd.iface, PROC_SYS_IP6_RETRANSTIMER_MS, "RetransTimer (ms)", cmd.val);
			if (ret == 0)
				break;
			set_interface_var(cmd.iface, PROC_SYS_IP6_RETRANSTIMER, "RetransTimer", cmd.val / 1000 * USER_HZ);	/* XXX user_hz */
			break;

		default:
			/* Bad command */
			break;
		}
	}
}

/* Fork to create privileged process connected by a pipe */
int privsep_init(void)
{
	int pipefds[2];

	if (pipe(pipefds) != 0) {
		flog(LOG_ERR, "Couldn't create privsep pipe.");
		return -1;
	}

	pid_t pid = fork();
	if (pid == -1) {
		flog(LOG_ERR, "Couldn't fork for privsep.");
		return -1;
	}

	if (pid == 0) {
		/* This will be the privileged child */
		close(pipefds[1]);
		pfd = pipefds[0];

		/* Detach from stdio */
		int nullfd = open("/dev/null", O_RDONLY);
		if (nullfd < 0) {
			perror("/dev/null");
			close(pfd);
			_exit(1);
		}
		dup2(nullfd, 0);
		dup2(nullfd, 1);
		/* XXX: we'll keep stderr open in debug mode for better logging */
		if (get_debuglevel() == 0)
			dup2(nullfd, 2);

		privsep_read_loop();
		close(pfd);
		flog(LOG_ERR, "Exiting, privsep_read_loop is complete.");
		_exit(0);
	}

	dlog(LOG_DEBUG, 3, "radvd privsep PID is %d", pid);

	/* Continue execution (will drop privileges soon) */
	close(pipefds[0]);
	pfd = pipefds[1];

	return 0;
}

/* Interface calls for the unprivileged process */
int privsep_interface_linkmtu(const char *iface, uint32_t mtu)
{
	struct privsep_command cmd;
	cmd.type = SET_INTERFACE_LINKMTU;
	strncpy(cmd.iface, iface, sizeof(cmd.iface));
	cmd.val = mtu;

	if (writen(pfd, &cmd, sizeof(cmd)) != sizeof(cmd))
		return -1;
	return 0;
}

int privsep_interface_curhlim(const char *iface, uint32_t hlim)
{
	struct privsep_command cmd;
	cmd.type = SET_INTERFACE_CURHLIM;
	strncpy(cmd.iface, iface, sizeof(cmd.iface));
	cmd.val = hlim;
	if (writen(pfd, &cmd, sizeof(cmd)) != sizeof(cmd))
		return -1;
	return 0;
}

int privsep_interface_reachtime(const char *iface, uint32_t rtime)
{
	struct privsep_command cmd;
	cmd.type = SET_INTERFACE_REACHTIME;
	strncpy(cmd.iface, iface, sizeof(cmd.iface));
	cmd.val = rtime;
	if (writen(pfd, &cmd, sizeof(cmd)) != sizeof(cmd))
		return -1;
	return 0;
}

int privsep_interface_retranstimer(const char *iface, uint32_t rettimer)
{
	struct privsep_command cmd;
	cmd.type = SET_INTERFACE_RETRANSTIMER;
	strncpy(cmd.iface, iface, sizeof(cmd.iface));
	cmd.val = rettimer;
	if (writen(pfd, &cmd, sizeof(cmd)) != sizeof(cmd))
		return -1;
	return 0;
}

/* note: also called from the root context */
static int set_interface_var(const char *iface, const char *var, const char *name, uint32_t val)
{
	int retval = -1;
	FILE * fp = 0;
	char * spath = strdupf(var, iface);

	/* No path traversal */
	if (!iface[0] || !strcmp(iface, ".") || !strcmp(iface, "..") || strchr(iface, '/'))
		goto cleanup;

	if (access(spath, F_OK) != 0)
		goto cleanup;

	fp = fopen(spath, "w");
	if (!fp) {
		if (name)
			flog(LOG_ERR, "failed to set %s (%u) for %s: %s", name, val, iface, strerror(errno));
		goto cleanup;
	}

	if (0 > fprintf(fp, "%u", val)) {
		goto cleanup;
	}

	retval = 0;

cleanup:
	if (fp)
		fclose(fp);

	free(spath);

	return retval;
}

