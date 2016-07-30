#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>

unsigned char offset[]={0x3E,0x3D,0x3B,0x37,0x2F,0x1F};

int main(void)
{
	int buttons_fd;
	int i=0;
	unsigned char key_value;
	fd_set rds;
	
	buttons_fd = open("/dev/buttons", 0);
	if (buttons_fd < 0) 
	{
		perror("open device buttons");
		exit(1);
	}
	
	while(1)
	{
		int ret;
		FD_ZERO(&rds);
		FD_SET(buttons_fd, &rds);
		
		ret = select(buttons_fd+1, &rds, NULL, NULL, NULL);
		if (ret < 0) 
		{
			perror("select");
			exit(1);
		}
		else if (ret == 0)
		{
			printf("select timeout.\n");
		} 
		else if (FD_ISSET(buttons_fd, &rds))
		{
			int ret = read(buttons_fd, &key_value, sizeof(key_value));
			if (ret != sizeof(key_value))
			{
				if (errno != EAGAIN)
					perror("read buttons\n");
				continue;
			}
			else
			{
				for(i=0;i<6;i++)
				{
					if(offset[i]==key_value)
					{
						printf("key%d is pressed\n",i+1);
					}
				}
			}
		}
	}
	close(buttons_fd);
	return 0;
}
