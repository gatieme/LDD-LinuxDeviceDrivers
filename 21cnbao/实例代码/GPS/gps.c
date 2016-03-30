// this is a test about GPS Receiver 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>      // open() close()
#include <unistd.h>     // read() write()

#include <termios.h>    // set baud rate

#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

#define DEVICE_TTYS "/dev/ttyS2"
#define BAUD_RATE_9600 B9600

#define FUNC_RUN                0
#define FUNC_NOT_RUN            1

#define ORG_GPS		1
#define SEL_GPGGA	2
#define SEL_GPGLL	3
#define SEL_GPGSA	4
#define SEL_GPGSV	5
#define SEL_GPRMC	6
#define SEL_GPVTG	7
#define FUNC_QUIT	8

//------------------------------------- read datas from GPS -------------------------------------------------
// succese return 1
// error   return 0
int read_GPS_datas(int fd, char *rcv_buf)
{
	int retval;
        fd_set rfds;
        struct timeval tv;

        int ret,pos;

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        pos = 0; // point to rceeive buf

        while (1)
        {
                FD_ZERO(&rfds);
                FD_SET(fd, &rfds);

                retval = select(fd+1 , &rfds, NULL, NULL, &tv);

                if (retval == -1)
                {
                        perror("select()");
                        break;
                }
                else if (retval)
                {// pan duan shi fou hai you shu ju
                        ret = read(fd, rcv_buf+pos, 2048);
                        pos += ret;
                        if (rcv_buf[pos-2] == '\r' && rcv_buf[pos-1] == '\n')
                        {
                                FD_ZERO(&rfds);
                                FD_SET(fd, &rfds);

                                retval = select(fd+1 , &rfds, NULL, NULL, &tv);

                                if (!retval) break;// if no datas, break
                        }
                }
                else
                {
                        printf("No data\n");
                        break;
                }
        }

        return 1;
} // end read_GPS_datas

//------------------------------------- print ---------------------------------------------------------------
void print_prompt(void)
{
        printf ("Select what you want to read:\n");
        printf ("1 : Original GPS datas\n");
        printf ("2 : GPGGA\n");
        printf ("3 : GPGLL\n");
        printf ("4 : GPGSA\n");
        printf ("5 : GPGSV\n");
        printf ("6 : GPRMC\n");
	printf ("7 : GPVTG\n");
	printf ("8 : Quit\n");
        printf (">");
} // end print_prompt

//------------------------------------- FUNCTIONS ABOUT GPS -------------------------------------------------
//------------------------------------- FUNCTION 1 show all receive signal ----------------------------------
void GPS_original_signal(int fd)
{
	char rcv_buf[2048];

	while (1)
	{
		bzero(rcv_buf,sizeof(rcv_buf));
		{
			if (read_GPS_datas(fd,rcv_buf))
			{
				printf("%s",rcv_buf);
			}
		}
	}
} // end GPS_original_signal

//------------------------------------- FUNCTION 2 resolve GPS GPGGA information ----------------------------
void GPS_resolve_gpgga(int fd)
{
} // end GPS_resolve_gpgga

//------------------------------------- FUNCTION 3 resolve GPS GPGLL information ----------------------------
void GPS_resolve_gpgll(int fd)
{
} // end GPS_resolve_gpgll

//------------------------------------- FUNCTION 4 resolve GPS GPGSA information ----------------------------
void GPS_resolve_gpgsa(int fd)
{
} // end GPS_resolve_gpgsa

//------------------------------------- FUNCTION 5 resolve GPS GPGSV information ----------------------------
void GPS_resolve_gpgsv(int fd)
{
} // end GPS_resolve_gpgsv

//------------------------------------- FUNCTION 6 resolve GPS GPRMC information ----------------------------
void GPS_resolve_gprmc(int fd)
{
} // end GPS_resolve_gprmc

//------------------------------------- FUNCTION 7 resolve GPS GPVTG information ----------------------------
void GPS_resolve_gpvtg(int fd)
{
} // end GPS_resolve_gpvtg

//------------------------------------- Receive GPS Signal --------------------------------------------------
void func_GPS(int fd)
{
        int flag_func_run;
        int flag_select_func;
        ssize_t ret;

        flag_func_run = FUNC_RUN;

        while (flag_func_run == FUNC_RUN)
        {
                print_prompt();                 // print select functions
                scanf("%d",&flag_select_func);  // user input select
                getchar(); // get ENTER <LF>

                switch(flag_select_func)
                {
			case ORG_GPS	: {GPS_original_signal(fd);	break; }
                        case SEL_GPGGA	: {GPS_resolve_gpgga(fd);	break; }
                        case SEL_GPGLL	: {GPS_resolve_gpgll(fd);	break; }
                        case SEL_GPGSA	: {GPS_resolve_gpgsa(fd);	break; }
                        case SEL_GPGSV  : {GPS_resolve_gpgsv(fd);       break; }
                        case SEL_GPRMC	: {GPS_resolve_gprmc(fd);       break; }
			case SEL_GPVTG	: {GPS_resolve_gpvtg(fd);	break; }
                        case FUNC_QUIT          :
                                                {
                                                        flag_func_run = FUNC_NOT_RUN;
                                                        printf("Quit GPS function.  byebye\n");
                                                        break;
                                                }
                        default :
                        {
                                printf("please input your select use 1 to 8\n");
                        }
                }

        }
}// end func_GPS

//------------------------------------- init seriel port  ---------------------------------------------------
void init_ttyS(int fd)
{
	struct termios newtio;

	bzero(&newtio, sizeof(newtio));

	newtio.c_lflag &= ~(ECHO | ICANON);
	newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_oflag &= ~(OPOST);

	newtio.c_cc[VTIME]    = 5;   /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 0;   /* blocking read until 9 chars received */

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);

}//end init_ttyS


//------------------------------------- main ----------------------------------------------------------------
int main(void)
{
	int fd;

	printf ("\nThis is a test about GPS : receive GPS signal\n\n");

	// open seriel port
        fd = open(DEVICE_TTYS, O_RDONLY);

	if (fd == -1)
        {
                printf("open device %s error\n",DEVICE_TTYS);
        }
        else
        {
                init_ttyS(fd);  // init device
        	func_GPS(fd);   // GPS functions

                // close ttyS0
                if (close(fd)!=0) printf("close device %s error",DEVICE_TTYS);
        }

	return 0;
 
} // end main
