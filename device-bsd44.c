/*
 *   $Id: device-bsd44.c,v 1.3 1997/10/14 21:52:46 lf Exp $
 *
 *   Authors:
 *    Craig Metz		<cmetz@inner.net>
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

static u_int8_t ll_prefix[] = { 0xfe, 0x80 };

/*
 * this function gets the hardware type and address of an interface,
 * determines the link layer token length and checks it against
 * the defined prefixes
 */
int
setup_deviceinfo(int sock, struct Interface *iface)
{
	struct ifconf ifconf;
	int i, nlen;
	u_int8_t *p, *end;
	struct AdvPrefix *prefix;

	for (i = 0; i < 32; i++)
	{
		if (i)
			free(ifconf.ifc_buf);

		if (!(ifconf.ifc_buf = malloc(ifconf.ifc_len = (i << 8))))
		{
			log(LOG_CRIT, "malloc(%d) failed: %s", i, strerror(errno));
			goto ret;
		}

		if (ioctl(sock, SIOCGIFCONF, &ifconf) < 0)
		{
			log(LOG_ERR, "ioctl(SIOCGIFCONF) failed: %s(%d)", strerror(errno), errno);
			goto ret;
		};

		if (((i << 8) - ifconf.ifc_len) > sizeof(struct ifreq))
			break;
	}

	p = (uint8_t *)ifconf.ifc_buf;
	end = p + ifconf.ifc_len;
	nlen = strlen(iface->Name);

	while(p < end)
	{
		p += IFNAMSIZ;
		
		if ((p + 2) >= end)
			break;
			
		if ((p + *p) >= end)
			break;
			
		if ((*(p + 1) == AF_LINK) &&
		    (((struct sockaddr_dl *)p)->sdl_nlen == nlen) &&
		    (!memcmp(iface->Name, ((struct sockaddr_dl *)p)->sdl_data, nlen)))
		{
		
			if (((struct sockaddr_dl *)p)->sdl_alen > HWADDR_MAX)
			{
				log(LOG_ERR, "address length %d too big for",
					((struct sockaddr_dl *)p)->sdl_alen,
					iface->Name);
				goto ret;
			}
		
			memcpy(iface->if_hwaddr, LLADDR((struct sockaddr_dl *)p), ((struct sockaddr_dl *)p)->sdl_alen);
			iface->if_hwaddr_len = ((struct sockaddr_dl *)p)->sdl_alen << 3;

          		switch(((struct sockaddr_dl *)p)->sdl_type) {
            		case IFT_ETHER:
            		case IFT_ISO88023:
#ifdef EUI_64_SUPPORT
            			iface->if_prefix_len = 64;
#else
				iface->if_prefix_len = 80;
#endif            		
              			iface->if_maxmtu = 1500;
              			break;
            		case IFT_FDDI:
#ifdef EUI_64_SUPPORT
            			iface->if_prefix_len = 64;
#else
				iface->if_prefix_len = 80;
#endif            		
              			iface->if_maxmtu = 4352;
              			break;
            		default:
            			iface->if_prefix_len = -1;
				iface->if_maxmtu = -1;
				break;
          		}

			dlog(LOG_DEBUG, 3, "link layer token length for %s is %d", iface->Name,
				iface->if_hwaddr_len);

			dlog(LOG_DEBUG, 3, "prefix length for %s is %d", iface->Name,
				iface->if_prefix_len);

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
          		
          		free(ifconf.ifc_buf);
          		return 0;
        	}
        
    		p += *p;	
	}

ret:
	iface->if_maxmtu = -1;
	iface->if_hwaddr_len = -1;
	iface->if_prefix_len = -1;
	free(ifconf.ifc_buf);
	return 0;
}

int setup_linklocal_addr(int sock, struct Interface *iface)
{
	struct ifconf ifconf;
	int i, nlen;
	u_int8_t *p, *end;
	int index = 0;

	for (i = 0; i < 32; i++)
	{
		if (i)
			free(ifconf.ifc_buf);

		if (!(ifconf.ifc_buf = malloc(ifconf.ifc_len = (i << 8))))
		{
			log(LOG_ERR, "malloc(%d) failed: %s", i, strerror(errno));
			goto ret;
		}

		if (ioctl(sock, SIOCGIFCONF, &ifconf) < 0)
		{
      			log(LOG_ERR, "ioctl(SIOCGIFCONF) failed: %s(%d)", strerror(errno), errno);
			goto ret;
		}

		if (((i << 8) - ifconf.ifc_len) > sizeof(struct ifreq))
			break;
  	}

	p = (uint8_t *)ifconf.ifc_buf;
	end = p + ifconf.ifc_len;
	nlen = strlen(iface->Name);

	while(p < end)
  	{
		p += IFNAMSIZ;
	
		if ((p + 2) >= end)
			break;
			
		if ((p + *p) >= end)
			break;
			
		if ((*(p + 1) == AF_LINK) &&
		    (((struct sockaddr_dl *)p)->sdl_nlen == nlen) &&
		    (!memcmp(iface->Name, ((struct sockaddr_dl *)p)->sdl_data, nlen)))
		{
			index = ((struct sockaddr_dl *)p)->sdl_index;
		}
		
   	 	if (index && (*(p + 1) == AF_INET6))
		  if (!memcmp(&((struct sockaddr_in6 *)p)->sin6_addr, ll_prefix, sizeof(ll_prefix)))
		  {
			memcpy(&iface->if_addr, &((struct sockaddr_in6 *)p)->sin6_addr, sizeof(struct in6_addr));
			iface->if_index = index;

			free(ifconf.ifc_buf);
			return 0;
      	  	  }
      	  
		p += *p;

	}

ret:
	log(LOG_ERR, "no linklocal address configured for %s", iface->Name);
	free(ifconf.ifc_buf);
	return -1;
}
