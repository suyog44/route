#include <net/route.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdio.h>

#define RTACTION_ADD 1
#define RTACTION_DEL 2

/* add/del route item in route table */
int inet_setroute(int action, char **args)
{
	struct rtentry route;  /* route item struct */
	char target[128] = {0};
	char gateway[128] = {0};
	char netmask[128] = {0};
	
	struct sockaddr_in *addr;
	
	int skfd;

	/* clear route struct by 0 */
	memset((char *)&route, 0x00, sizeof(route));

	/* default target is net (host)*/
	route.rt_flags = RTF_UP ;

	while(args)
	{
		if(*args == NULL)
		{
			break;
		}
		if(!strcmp(*args, "-net"))
		{/* default is a net target */
			args++;
			strcpy(target, *args);
			addr = (struct sockaddr_in*) &route.rt_dst;
			addr->sin_family = AF_INET;
			addr->sin_addr.s_addr = inet_addr(target);
			args++;
			continue;
		}
		if(!strcmp(*args, "netmask"))
		{/* netmask setting */
			args++;
			strcpy(netmask, *args);
			addr = (struct sockaddr_in*) &route.rt_genmask;
			addr->sin_family = AF_INET;
			addr->sin_addr.s_addr = inet_addr(netmask);
			args++;
			continue;
		}
		if(!strcmp(*args, "gw") || !strcmp(*args, "gateway"))
		{/* gateway setting */
			args++;
			strcpy(gateway, *args);
			addr = (struct sockaddr_in*) &route.rt_gateway;
			addr->sin_family = AF_INET;
			addr->sin_addr.s_addr = inet_addr(gateway);
		    route.rt_flags |= RTF_GATEWAY;
			args++;
			continue;
		}
		if(!strcmp(*args, "device") || !strcmp(*args, "dev"))
		{/* device setting */
			args++;
			route.rt_dev = *args;
			args++;
			continue;
		}
		if(!strcmp(*args, "mtu"))
		{/* mtu setting */
			args++;
			route.rt_flags |= RTF_MTU;
		    route.rt_mtu = atoi(*args);
			args++;
		    continue; 
		}
		/* if you have other options, please put them in this place,
		  like the options above. */
	}	

	/* create a socket */
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd < 0)
	{
		perror("socket");
		return -1;
	}

	/* tell the kernel to accept this route */
	/* del a route item */
	if(action == RTACTION_DEL)
	{
		if(ioctl(skfd, SIOCDELRT, &route) < 0)
		{
			perror("SIOCDELRT");
			close(skfd);
			return -1;
		}
	}
	else
	{/* add a route item */
		if(ioctl(skfd, SIOCADDRT, &route) < 0)
		{
			perror("SIOCADDRT");
			close(skfd);
		    return -1;
		}
	}
	(void) close(skfd);
	return 0;
}	

int inet6_setroute(int action, char **args)
{
	return 0;
}
/* print usage information */ 
void usage()
{
	printf("route -A inet add/del -net TARGET netmask \
NETMASK gw GETWAY dev DEVICE mtu MTU\n");
	printf("route -A inet6 add/del -net TARGET netmask \
NETMASK gw GETWAY dev DEVICE mtu MTU\n");
	return ;
}


/* main function */
int main(int argc, char **argv)
{
	int action = 0;
	if(argc < 5)
	{
		usage();
		return -1;
	}
    if(strcmp(argv[1], "-A"))
	{
		usage();
		return -1;
	}

	if(!strcmp(argv[3], "add"))
	{
		action = RTACTION_ADD;
	}
	if(!strcmp(argv[3], "del"))
	{
		action = RTACTION_DEL;
	}
    
	/* add or del a ipv4 route item */
	if(!strcmp(argv[2], "inet"))
	{
		inet_setroute(action, argv+4); 
	}
	/* add  or del a ipv6 route item */
	if(!strcmp(argv[2], "inet6"))
	{
		inet6_setroute(action, argv+4);
	}
	return 0;
}
