/*
 *   $Id: privsep-linux.c,v 1.2 2008/01/24 10:10:18 psavola Exp $
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
 *   may request it from <pekkas@netcore.fi>.
 *
 */

#include <config.h>
#include <includes.h>
#include <radvd.h>
#include <pathnames.h>

int privsep_set(const char *iface, const char *var, uint32_t val);
void privsep_read_loop(void);

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
void
privsep_read_loop(void)
{
	struct privsep_command cmd;
	int ret;

	while (1) {
		ret = readn(pfd, &cmd, sizeof(cmd));
		if (ret <= 0) {
			/* Error or EOF, give up */
			close(pfd);
			_exit(0);
		}
		if (ret != sizeof(cmd)) {
			/* Short read, ignore */
			continue;
		}

		cmd.iface[IFNAMSIZ-1] = '\0';

		switch(cmd.type) {

		case SET_INTERFACE_LINKMTU:
			if (cmd.val < MIN_AdvLinkMTU || cmd.val > MAX_AdvLinkMTU)
				break;
			ret = privsep_set(cmd.iface, PROC_SYS_IP6_LINKMTU, cmd.val);
			 dlog(LOG_DEBUG, 4, "privsep: set link %s mtu to %u", cmd.iface, cmd.val);
			break;

		case SET_INTERFACE_CURHLIM:
			if (cmd.val < MIN_AdvCurHopLimit || cmd.val > MAX_AdvCurHopLimit)
				break;
			ret = privsep_set(cmd.iface, PROC_SYS_IP6_CURHLIM, cmd.val);
			break;

		case SET_INTERFACE_REACHTIME:
			if (cmd.val < MIN_AdvReachableTime || cmd.val > MAX_AdvReachableTime) 
				break;
			ret = privsep_set(cmd.iface, PROC_SYS_IP6_BASEREACHTIME_MS, cmd.val);
			if (ret == 0)
				break;
			privsep_set(cmd.iface, PROC_SYS_IP6_BASEREACHTIME, cmd.val / 1000); /* sec */
			break;

		case SET_INTERFACE_RETRANSTIMER:
			if (cmd.val < MIN_AdvRetransTimer || cmd.val > MAX_AdvRetransTimer)
				break;
			ret = privsep_set(cmd.iface, PROC_SYS_IP6_RETRANSTIMER_MS, cmd.val);
			if (ret == 0)
				break;
			privsep_set(cmd.iface, PROC_SYS_IP6_RETRANSTIMER, cmd.val / 1000 * USER_HZ); /* XXX user_hz */
			break;

		default:
			/* Bad command */
			break;
		}
	}
}

int
privsep_set(const char *iface, const char *var, uint32_t val)
{
	FILE *fp;
	char spath[64+IFNAMSIZ];	/* XXX: magic constant */

	if (snprintf(spath, sizeof(spath), var, iface) >= sizeof(spath))
		return (-1);

	if (access(spath, F_OK) != 0)
		return -1;

	fp = fopen(spath, "w");
	if (!fp)
		return -1;

	fprintf(fp, "%u", val);
	fclose(fp);
	return 0;
}

/* Return 1 if privsep is currently enabled */
int
privsep_enabled(void)
{
	if (pfd < 0)
		return 0;
	return 1;
}

/* Fork to create privileged process connected by a pipe */
int
privsep_init(void)
{
	int pipefds[2];
	pid_t pid;

	if (privsep_enabled())
		return 0;

	if (pipe(pipefds) != 0) {
		flog(LOG_ERR, "Couldn't create privsep pipe\n");
		return (-1);
	}

	pid = fork();
	if (pid == -1) {
		flog(LOG_ERR, "Couldn't fork for privsep\n");
		return (-1);
	}

	if (pid == 0) {
		int nullfd;

		/* This will be the privileged child */
		close(pipefds[1]);
		pfd = pipefds[0];

		/* Detach from stdio */
		nullfd = open("/dev/null", O_RDONLY);
		if (nullfd < 0) {
			perror("/dev/null");
			close(pfd);
			_exit(1);
		}
		dup2(nullfd, 0);
		dup2(nullfd, 1);
		dup2(nullfd, 2);

		privsep_read_loop();
		close(pfd);
		_exit(0);
	}

	/* Continue execution (will drop privileges soon) */
	close(pipefds[0]);
	pfd = pipefds[1];

	return 0;
}

/* Interface calls for the unprivileged process */
int
privsep_interface_linkmtu(const char *iface, uint32_t mtu)
{
	struct privsep_command cmd;
	cmd.type = SET_INTERFACE_LINKMTU;
	strncpy(cmd.iface, iface, sizeof(cmd.iface));
	cmd.val = mtu;

	if (writen(pfd, &cmd, sizeof(cmd)) != sizeof(cmd))
		return (-1);
	return 0;
}

int
privsep_interface_curhlim(const char *iface, uint32_t hlim)
{
	struct privsep_command cmd;
	cmd.type = SET_INTERFACE_CURHLIM;
	strncpy(cmd.iface, iface, sizeof(cmd.iface));
	cmd.val = hlim;
	if (writen(pfd, &cmd, sizeof(cmd)) != sizeof(cmd))
		return (-1);
	return 0;
}

int
privsep_interface_reachtime(const char *iface, uint32_t rtime)
{
	struct privsep_command cmd;
	cmd.type = SET_INTERFACE_REACHTIME;
	strncpy(cmd.iface, iface, sizeof(cmd.iface));
	cmd.val = rtime;
	if (writen(pfd, &cmd, sizeof(cmd)) != sizeof(cmd))
		return (-1);
	return 0;
}

int
privsep_interface_retranstimer(const char *iface, uint32_t rettimer)
{
	struct privsep_command cmd;
	cmd.type = SET_INTERFACE_RETRANSTIMER;
	strncpy(cmd.iface, iface, sizeof(cmd.iface));
	cmd.val = rettimer;
	if (writen(pfd, &cmd, sizeof(cmd)) != sizeof(cmd))
		return (-1);
	return 0;
}
