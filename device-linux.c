/*
 *   $Id: device-linux.c,v 1.1 1997/10/14 17:17:40 lf Exp $
 *
 *   Authors:
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
#include <defaults.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */

#ifndef IPV6_ADDR_LINKLOCAL
#define IPV6_ADDR_LINKLOCAL   0x0020U
#endif

/*
 * this function gets the hardware type and address of an interface,
 * determines the link layer token length and checks it against
 * the defined prefixes
 */
int
setup_deviceinfo(int sock, struct Interface *iface)
{
	struct ifreq	ifr;
	struct AdvPrefix *prefix;
	
	strcpy(ifr.ifr_name, iface->Name);
	
	if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
	{
		log(LOG_ERR, "ioctl(SIOCGIFHWADDR) failed for %s: %s",
			iface->Name, strerror(errno));
		return (-1);
	}

	dlog(LOG_DEBUG, 3, "hardware type for %s is %d", iface->Name,
		ifr.ifr_hwaddr.sa_family); 

	switch(ifr.ifr_hwaddr.sa_family)
        {
	case ARPHRD_ETHER:
		iface->if_hwaddr_len = 48;
#ifdef EUI_64_SUPPORT
		iface->if_prefix_len = 64;
#else
		iface->if_prefix_len = 80;
#endif
		iface->if_maxmtu = 1500;
		break;
#ifdef ARPHRD_FDDI
	case ARPHRD_FDDI:
		iface->if_hwaddr_len = 48;
#ifdef EUI_64_SUPPORT
		iface->if_prefix_len = 64;
#else
		iface->if_prefix_len = 80;
#endif
		iface->if_maxmtu = 4352;
		break;
#endif /* ARPHDR_FDDI */
#ifdef ARPHRD_ARCNET
	case ARPHRD_ARCNET:
		iface->if_hwaddr_len = 8;
		iface->if_prefix_len = -1;
		iface->if_maxmtu = -1;
		break;
#endif /* ARPHDR_ARCNET */
	default:
		iface->if_hwaddr_len = -1;
		iface->if_prefix_len = -1;
		iface->if_maxmtu = -1;
		break;
	}

	dlog(LOG_DEBUG, 3, "link layer token length for %s is %d", iface->Name,
		iface->if_hwaddr_len);

	dlog(LOG_DEBUG, 3, "prefix length for %s is %d", iface->Name,
		iface->if_prefix_len);

	if (iface->if_hwaddr_len != -1)
		memcpy(iface->if_hwaddr, ifr.ifr_hwaddr.sa_data, (iface->if_hwaddr_len + 7) >> 3);

	prefix = iface->AdvPrefixList;
	while (prefix)
	{
		if ((iface->if_prefix_len != -1) &&
		   (iface->if_prefix_len != prefix->PrefixLen))
		{
			log(LOG_WARNING, "prefix length should be %d for %s",
				iface->if_prefix_len, iface->Name);
 			return (-1);
 		}
 			
 		prefix = prefix->next;
	}
                
	return (0);
}

/*
 * this function extracts the link local address and interface index
 * from PATH_PROC_NET_IF_INET6
 */
int setup_linklocal_addr(int sock, struct Interface *iface)
{
	FILE *fp;
	char str_addr[40];
	int plen, scope, dad_status, if_idx;
	char devname[IFNAMSIZ];

	if ((fp = fopen(PATH_PROC_NET_IF_INET6, "r")) == NULL)
	{
		log(LOG_ERR, "can't open %s: %s", PATH_PROC_NET_IF_INET6,
			strerror(errno));
		return (-1);	
	}
	
	while (fscanf(fp, "%32s %02x %02x %02x %02x %s\n",
		      str_addr, &if_idx, &plen, &scope, &dad_status,
		      devname) != EOF)
	{
		if (scope == IPV6_ADDR_LINKLOCAL &&
		    strcmp(devname, iface->Name) == 0)
		{
			struct in6_addr addr;
			unsigned int ap;
			int i;
			
			for (i=0; i<16; i++)
			{
				sscanf(str_addr + i * 2, "%02x", &ap);
				addr.s6_addr[i] = (unsigned char)ap;
			}
			memcpy(&iface->if_addr, &addr,
			       sizeof(struct in6_addr));

			iface->if_index = if_idx;
			fclose(fp);
			return 0;
		}
	}

	log(LOG_ERR, "no linklocal address configured for %s", iface->Name);
	fclose(fp);
	return (-1);
}
