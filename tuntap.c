
#include "config.h"
#include "tuntap.h"
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>	/* TUNSETIFF */
#include <fcntl.h>		/* O_RDWR */
#include <unistd.h>

int tun_alloc(char *dev)
{

	struct ifreq ifr;
	int fd, err;
	int sock;
	char *clonedev = "/dev/net/tun";

	/* Arguments taken by the function:
	 *
	 * char *dev: the name of an interface (or '\0'). MUST have enough
	 *       space to hold the interface name if '\0' is passed
	 */

	/* open the clone device */
	if ((fd = open(clonedev, O_RDWR)) < 0) {
		return fd;
	}

	/* preparation of the struct ifr, of type "struct ifreq" */
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TUN;	/* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */

	if (*dev) {
		/* if a device name was specified, put it in the structure; otherwise,
		 * the kernel will try to allocate the "next" device of the
		 * specified type */
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	}

	/* try to create the device */
	if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
		close(fd);
		return err;
	}

	/* if the operation was successful, write back the name of the
	 * interface to the variable "dev", so the caller can know
	 * it. Note that the caller MUST reserve space in *dev (see calling
	 * code below) */
	strcpy(dev, ifr.ifr_name);

	/* this is the special file descriptor that the caller will use to talk
	 * with the virtual interface */

	sock = open_icmpv6_socket();
	if (sock < 0) {
		perror("open_icmpv6_socket failed");
		exit(1);
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, dev, IFNAMSIZ - 1);

	if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
		perror("ioctl(SIOCGIFFLAGS) failed");
		exit(1);
	}

	ifr.ifr_flags |= IFF_UP | IFF_RUNNING;

	if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0) {
		perror("ioctl(SIOCGIFFLAGS) failed");
		exit(1);
	}

	return fd;
}
