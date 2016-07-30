
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(void)
{
	int fd;
	int rt;
	int cmd;
        char enter_c;
        unsigned long arg_G,arg_B,arg_R,arg_Y,arg_W,arg_K,arg_CY;

	struct arg 
	{
		unsigned long a;
		unsigned long b;
		unsigned long c;
		unsigned long d;
	};
       struct arg arg1 = {0,120,300,0}; 
       struct arg arg2 = {140,0,239,0};
       struct arg arg3 = {100,100,50,0};
       struct arg arg4 = {0,0,319,239};
       struct arg arg5 = {240,100,60,0};
       struct arg arg6 = {0,0,319,239};
       struct arg arg7 = {40,170,100,200};

       arg_G = 0x00FF00;
       arg_R = 0xFF0000;
       arg_B = 0x0000FF;
       arg_Y = 0xAAAA00;
       arg_W = 0xFFFFFF;
       arg_K = 0x000000;
       arg_CY = 0x808080;
	
	if ((fd = open("/dev/S3C2410LCD", 0)) < 0) 
	{
		printf("cannot open /dev/S3C2410LCD\n");
		exit(0);
	};
	
       do{
		cmd = getchar();
	 
		switch (cmd)
		{
			case 49:
	
                                enter_c = getchar();
                                rt = ioctl(fd, 0,arg_R);   // set RED
                                cmd = 0;
				break;

			case 50:
                                enter_c = getchar();
				rt = ioctl(fd, 0,arg_G);   //  set GREEN
				break;
		
	                case 51:
                                enter_c = getchar();
				rt = ioctl(fd, 0,arg_B);   // set BLUE
			        cmd = 0;
			        break;
		
	                case 52:
                                enter_c = getchar();
				rt = ioctl(fd, 0,arg_Y); // set YELLOW
			
				cmd = 0;
				break;
		
	                case 53:
                                enter_c = getchar();
				rt = ioctl(fd, 0,arg_W); // set WHITE
				cmd = 0;
				break;
		
	                case 54:
                                enter_c = getchar();
				rt = ioctl(fd, 0,arg_K); // set BLACK
				cmd = 0;
				break;
             	   
                        case 55:
                                enter_c = getchar();
				rt = ioctl(fd, 0,arg_CY); // set CYNE
				cmd = 0;
				break;
			
                        case 'a':
                                enter_c = getchar();
				rt = ioctl(fd, 1,(unsigned long )&arg1); // draw h_line
				cmd = 0;
				break;
			case 'b':
                                enter_c = getchar();
				rt = ioctl(fd, 2,(unsigned long )&arg2); // draw v_line
				cmd = 0;
				break;
			case 'c':
                                enter_c = getchar();
				rt = ioctl(fd, 3,(unsigned long )&arg3); // draw circle
				cmd = 0;
				break;
			case 'd':
                                enter_c = getchar();
				rt = ioctl(fd, 4,(unsigned long )&arg4); // draw rect
				cmd = 0;
				break;
			case 'e':
                                enter_c = getchar();
				rt = ioctl(fd, 5,(unsigned long )&arg5); // draw fillcircle
				cmd = 0;
				break;
			case 'f':
                                enter_c = getchar();
				rt = ioctl(fd, 6,(unsigned long )&arg6); // clear screen
                                cmd = 0;
				break;
			case 'g':
                                enter_c = getchar();
				rt = ioctl(fd, 7,(unsigned long )&arg7); // draw rect
                                cmd = 0;
				break;
			default:
			    break;
		}
	}while(cmd != 'q');                                  // "q" is quit command
	close(fd);
        return 0;
}
