/*
 *   $Id: recv.c,v 1.4 1997/10/19 18:39:14 lf Exp $
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

int
recv_rs_ra(int sock, unsigned char *msg, struct sockaddr_in6 *addr,
                 struct in6_pktinfo **pkt_info, int *hoplimit)
{                 
	struct msghdr mhdr;
	struct cmsghdr *cmsg;
	struct iovec iov;
	static unsigned char chdr[CMSG_SPACE(sizeof(struct in6_pktinfo)) + \
				  CMSG_SPACE(sizeof(int))];
	int len;
	
	iov.iov_len = MSG_SIZE;
	iov.iov_base = (caddr_t) msg;

	mhdr.msg_name = (caddr_t)addr;
	mhdr.msg_namelen = sizeof(struct sockaddr_in6);
	mhdr.msg_iov = &iov;
	mhdr.msg_iovlen = 1;
	mhdr.msg_control = (void *)chdr;
	mhdr.msg_controllen = sizeof(chdr);

	len = recvmsg(sock, &mhdr, 0);

	if (len < 0)
	{
		if (errno != EINTR)
			log(LOG_ERR, "recvmsg: %s", strerror(errno));
			
		return len;
	}

	*hoplimit = 255;

        for (cmsg = CMSG_FIRSTHDR(&mhdr); cmsg; cmsg = CMSG_NXTHDR(&mhdr, cmsg))
	{
          if (cmsg->cmsg_level != IPPROTO_IPV6)
          	continue;
          
          switch(cmsg->cmsg_type)
          {
#ifdef IPV6_HOPLIMIT
              case IPV6_HOPLIMIT:
                if ((cmsg->cmsg_len == CMSG_LEN(sizeof(int))) && 
                    (*(int *)CMSG_DATA(cmsg) >= 0) && 
                    (*(int *)CMSG_DATA(cmsg) < 256))
                {
                  *hoplimit = *(int *)CMSG_DATA(cmsg);
                }
                else
                {
                  log(LOG_ERR, "received a bogus IPV6_HOPLIMIT from the kernel! len=%d, data=%d",
                  	cmsg->cmsg_len, *(int *)CMSG_DATA(cmsg));
                  return (-1);	
                }  
                break;
#endif /* IPV6_HOPLIMIT */
              case IPV6_PKTINFO:
                if ((cmsg->cmsg_len == CMSG_LEN(sizeof(struct in6_pktinfo))) &&
                    ((struct in6_pktinfo *)CMSG_DATA(cmsg))->ipi6_ifindex)
                {
                  *pkt_info = (struct in6_pktinfo *)CMSG_DATA(cmsg);
                }
                else
                {
                  log(LOG_ERR, "received a bogus IPV6_PKTINFO from the kernel! len=%d, index=%d", 
                  	cmsg->cmsg_len, ((struct in6_pktinfo *)CMSG_DATA(cmsg))->ipi6_ifindex);
                  return (-1);
                } 
                break;
          }
	}
	
	dlog(LOG_DEBUG, 4, "recvmsg len=%d", len);

	return len;
}
