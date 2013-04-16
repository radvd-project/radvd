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
#include "radvd.h"
#include "log.h"
#include "netlink.h"

#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifndef SOL_NETLINK
#define SOL_NETLINK	270
#endif

void process_netlink_msg(int sock)
{
	char buf[4096];
	struct iovec iov = { buf, sizeof(buf) };
	struct sockaddr_nl sa;
	struct msghdr msg = { (void *)&sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
	struct nlmsghdr *nh;
	int len;

	len = recvmsg (sock, &msg, 0);
	if (len == -1) {
		flog(LOG_ERR, "recvmsg failed: %s", strerror(errno));
	}

	for (nh = (struct nlmsghdr *) buf; NLMSG_OK (nh, len); nh = NLMSG_NEXT (nh, len)) {
		struct ifinfomsg * ifinfo;
		char ifname[IF_NAMESIZE] = {""};
		struct Interface *iface;

		/* The end of multipart message. */
		if (nh->nlmsg_type == NLMSG_DONE)
			return;

		if (nh->nlmsg_type == NLMSG_ERROR) {
			flog(LOG_ERR, "%s:%d Some type of netlink error.\n", __FILE__, __LINE__);
			abort();
		}

		/* Continue with parsing payload. */
		ifinfo = NLMSG_DATA(nh);
		if_indextoname(ifinfo->ifi_index, ifname);
		if (ifinfo->ifi_flags & IFF_RUNNING) {
			dlog(LOG_DEBUG, 3, "%s, ifindex %d, flags is running", ifname, ifinfo->ifi_index);
		}
		else {
			dlog(LOG_DEBUG, 3, "%s, ifindex %d, flags is *NOT* running", ifname, ifinfo->ifi_index);
		}

		for (iface=IfaceList; iface; iface=iface->next) {
			if (iface->if_index == ifinfo->ifi_index) {
				double next;
				iface->is_dead = 0;
				iface->init_racount = 0;
				next = rand_between(iface->MinRtrAdvInterval, iface->MaxRtrAdvInterval);
				next = min(MAX_INITIAL_RTR_ADVERT_INTERVAL, next);
				iface->next_multicast = next_timeval(next);
			}
		}
	}
}

int netlink_socket(void)
{
	int rc, sock;
	unsigned int val = 1;
	struct sockaddr_nl snl;

	sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (sock == -1) {
		flog(LOG_ERR, "Unable to open netlink socket: %s", strerror(errno));
	}
	else if (setsockopt(sock, SOL_NETLINK, NETLINK_NO_ENOBUFS, &val, sizeof(val)) < 0 ) {
		flog(LOG_ERR, "Unable to setsockopt NETLINK_NO_ENOBUFS: %s", strerror(errno));
	}

	memset(&snl, 0, sizeof(snl));
	snl.nl_family = AF_NETLINK;
	snl.nl_groups = RTMGRP_LINK;

	rc = bind(sock, (struct sockaddr*)&snl, sizeof(snl));
	if (rc == -1) {
		flog(LOG_ERR, "Unable to bind netlink socket: %s", strerror(errno));
		close(sock);
		sock = -1;
	}

	return sock;
}

