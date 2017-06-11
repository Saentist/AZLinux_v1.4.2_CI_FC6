
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include <linux/dvb/frontend.h>

#ifndef TRUE
#define TRUE (1==1)
#endif
#ifndef FALSE
#define FALSE (1==0)
#endif

#define FRONTENDDEVICE "/dev/dvb/adapter%d/frontend%d"

static char *usage_str =
    "usage: switch -q\n"
    "      switch [options] \n"
    "      -a number : use given adapter (default 0)\n"
    "      -f number : use given frontend (default 0)\n"
    "      -t DvbsMode : select dvbs or dvbs2\n";

void bad_usage(char *pname)
{
	fprintf (stderr, usage_str, pname);
}

int set_standard(unsigned int adapter, unsigned int frontend, unsigned int dvb_standard)
{
	char fedev[128];
	static int fefd;
	int result;

	if(!fefd)
	{
		snprintf(fedev, sizeof(fedev), FRONTENDDEVICE, adapter, frontend);
		printf("using '%s'\n", fedev);

		if((fefd = open(fedev, O_RDWR | O_NONBLOCK)) < 0)
		{
	 		perror("opening frontend failed");
	 		return 0;
      		}
		
		result = ioctl(fefd, FE_SET_STANDARD, &dvb_standard);
		if (result < 0) 
		{
			perror("ioctl FE_SET_STANDARD failed");
	 		close(fefd);
	 		return FALSE;
     		 }
	}
	printf("set DvbsMode ok! \n");
	return TRUE;
}

int main(int argc, char *argv[])
{
	unsigned int adapter = 0, frontend = 0, dvb_standard = 1;
	int dvbmode = 0;
	int opt;
  
   	while ((opt = getopt(argc, argv, "t:a:f:h")) != -1)
	{
      		switch (opt)
      		{
			case 'a':
	    			adapter = strtoul(optarg, NULL, 0);
	   		break;
	 		case 'f':
	    			frontend = strtoul(optarg, NULL, 0);
	   		break;
			case 't':
				dvbmode = 1;
                        	if (strcmp(optarg, "dvbs") == 0) 
					dvb_standard = 0;
                       		else if (strcmp(optarg, "dvbs2") == 0)
					dvb_standard = 1;
                       		else 
				{
					bad_usage(argv[0]);
					return -1;
				}
			break;
			case '?':
	 		case 'h':
			default:
	    			bad_usage(argv[0]);
			break;
		}
   	}
	if(dvbmode == 1)
	{
		if(set_standard(adapter, frontend, dvb_standard))
			return TRUE;
	}else{
		bad_usage(argv[0]);
		return -1;
	}

   	return FALSE;
}
















