/*
 *   $Id: send.c,v 1.1 1997/10/14 17:17:40 lf Exp $
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

void
send_ra(int sock, struct Interface *iface, struct in6_addr *dest)
{
	u_int8_t all_hosts_addr[] = {0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	struct sockaddr_in6 addr;
	struct in6_pktinfo *pkt_info;
	struct msghdr mhdr;
	struct cmsghdr *cmsg;
	struct iovec iov;
	char chdr[sizeof(struct cmsghdr) + sizeof(struct in6_pktinfo)];
	struct nd_router_advert *radvert;
	struct AdvPrefix *prefix;
	unsigned char buff[MSG_SIZE];
	int len = 0;
	int err;

	dlog(LOG_DEBUG, 3, "sending RA on %s", iface->Name);

	if (dest == NULL)
	{
		struct timeval tv;

		dest = (struct in6_addr *)all_hosts_addr;
		gettimeofday(&tv, NULL);

		iface->last_multicast = tv.tv_sec;
	}
	
	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(IPPROTO_ICMPV6);
	memcpy(&addr.sin6_addr, dest, sizeof(struct in6_addr));

	radvert = (struct nd_router_advert *) buff;

	radvert->radv_type  = ND6_ROUTER_ADVERTISEMENT;
	radvert->radv_code  = 0;
	radvert->radv_cksum = 0;

	radvert->radv_maxhoplimit		= iface->AdvCurHopLimit;
	radvert->radv_m_o_res		= 
		(iface->AdvManagedFlag)?ND6_RADV_M_BIT:0;
	radvert->radv_m_o_res		|= 
		(iface->AdvOtherConfigFlag)?ND6_RADV_O_BIT:0;
	radvert->radv_router_lifetime	= htons(iface->AdvDefaultLifetime);

	radvert->radv_reachable  = htonl(iface->AdvReachableTime);
	radvert->radv_retransmit = htonl(iface->AdvRetransTimer);

	len = sizeof(struct nd_router_advert);

	prefix = iface->AdvPrefixList;

	/*
	 *	add prefix options
	 */

	while(prefix)
	{
		struct nd6_opt_prefix_info *pinfo;
		
		pinfo = (struct nd6_opt_prefix_info *) (buff + len);

		pinfo->opt_type		  = ND6_OPT_PREFIX_INFORMATION;
		pinfo->opt_length	  = 4;
		pinfo->opt_prefix_length  = prefix->PrefixLen;
		pinfo->opt_l_a_res	  = 
			(prefix->AdvOnLinkFlag)?ND6_OPT_PI_L_BIT:0;
		pinfo->opt_l_a_res	  |=
			(prefix->AdvAutonomousFlag)?ND6_OPT_PI_A_BIT:0;
		pinfo->opt_valid_life	  = htonl(prefix->AdvValidLifetime);
		pinfo->opt_preferred_life = htonl(prefix->AdvPreferredLifetime);
		pinfo->opt_reserved2	  = 0;
		
		memcpy(&pinfo->opt_prefix, &prefix->Prefix,
		       sizeof(struct in6_addr));

		len += sizeof(struct nd6_opt_prefix_info);

		prefix = prefix->next;
	}
	
	/*
	 *	add MTU option
	 */

	if (iface->AdvLinkMTU != 0) {
		struct nd6_opt_mtu *mtu;
		
		mtu = (struct nd6_opt_mtu *) (buff + len);
	
		mtu->opt_type	  = ND6_OPT_MTU;
		mtu->opt_length   = 1;
		mtu->opt_reserved = 0; 
		mtu->opt_mtu	  = htonl(iface->AdvLinkMTU);

		len += sizeof(struct nd6_opt_mtu);
	}

	/*
	 * add Source Link-layer Address option
	 */

	if (iface->AdvSourceLLAddress && iface->if_hwaddr_len != -1)
	{
		u_int8_t *ucp;
		int i;

		ucp = (u_int8_t *) (buff + len);
	
		*ucp++  = ND6_OPT_SOURCE_LINKADDR;
		*ucp++  = (u_int8_t) ((iface->if_hwaddr_len + 16 + 63) >> 6);

		len += 2 * sizeof(u_int8_t);

		i = (iface->if_hwaddr_len + 7) >> 3;
		memcpy(buff + len, iface->if_hwaddr, i);
		len += i;
	}

	iov.iov_len  = len;
	iov.iov_base = buff;
	
	cmsg = (struct cmsghdr *) chdr;
	
	cmsg->cmsg_len   = (sizeof(struct cmsghdr) +
			    sizeof(struct in6_pktinfo));
	cmsg->cmsg_level = IPPROTO_IPV6;
	cmsg->cmsg_type  = IPV6_PKTINFO;
	
	pkt_info = (struct in6_pktinfo *)((void *)cmsg + sizeof(struct cmsghdr));
	memset(pkt_info, 0, sizeof(struct in6_pktinfo));
	pkt_info->ipi6_ifindex = iface->if_index;
	memcpy(&pkt_info->ipi6_addr, &iface->if_addr, sizeof(struct in6_addr));

	mhdr.msg_name = (caddr_t)&addr;
	mhdr.msg_namelen = sizeof(struct sockaddr_in6);
	mhdr.msg_iov = &iov;
	mhdr.msg_iovlen = 1;
	mhdr.msg_control = (void *) cmsg;
	mhdr.msg_controllen = (sizeof(struct cmsghdr) +
			       sizeof(struct in6_pktinfo));

	err = sendmsg(sock, &mhdr, 0);
	
	if (err < 0) {
		log(LOG_WARNING, "sendmsg: %s", strerror(errno));
	}
}
