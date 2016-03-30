// this is a test about 485 full-duplex communication
// hardware : MAX488

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
#define MY_BAUD_RATE B9600

#define DATAS_CONTENT	"HELLO"

#define RECEIVE_BUF_WAIT_1S 1
#define RECEIVE_BUF_WAIT_2S 2
#define RECEIVE_BUF_WAIT_3S 3
#define RECEIVE_BUF_WAIT_4S 4
#define RECEIVE_BUF_WAIT_5S 5

//------------------------------------- read datas from ttyS ------------------------------------------------
// succese return 1
// error   return 0
int read_datas_ttyS(int fd, char *rcv_buf,int rcv_wait)
{
        int retval;
        fd_set rfds;
        struct timeval tv;

        int ret,pos;

        tv.tv_sec = rcv_wait;   
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

                                if (!retval) break;// no datas, break
                        }
                }
                else
                {
                 //       printf("No data\n");
                        break;
                }
        }

        return 1;
} // end read_datas_ttyS


//------------------------------------- send and receive datas by RS485 -------------------------------------
int func_485_transfer(int fd)
{	
	ssize_t ret;
	char rcv_buf[1024];
	int i;
        char *send_buf = DATAS_CONTENT;

	bzero(rcv_buf,sizeof(rcv_buf));
	
	while(1) {
		// send datas
		ret = write(fd,send_buf,strlen(send_buf));
                if (ret == -1) {
                        printf ("write device %s error\n", DEVICE_TTYS);
                        return -1;
                }
		// read datas
		if (read_datas_ttyS(fd,rcv_buf,RECEIVE_BUF_WAIT_1S)) {
			printf ("%s\n",rcv_buf);
		}
		else {
			printf ("read error\n");
		}
	}
	
	return 0;

} // end func_485_receive


//------------------------------------- init seriel port  ---------------------------------------------------
void init_ttyS(int fd)
{
        struct termios options;

        bzero(&options, sizeof(options));       	// clear options

        cfsetispeed(&options,MY_BAUD_RATE);            	// setup baud rate
        cfsetospeed(&options,MY_BAUD_RATE);

        options.c_cflag |= ( CS8 | CLOCAL | CREAD);
        options.c_iflag = IGNPAR;

        tcflush(fd, TCIFLUSH);

        tcsetattr(fd, TCSANOW, &options);

}//end init_ttyS


//------------------------------------- main ----------------------------------------------------------------
int main(void)
{
        int fd;

        printf("\n RS485 TRANSFERS DATAS \n\n");

        // open seriel port
        fd = open(DEVICE_TTYS, O_RDWR);

        if (fd == -1) {
                printf("open device %s error\n",DEVICE_TTYS);
        }
        else {
                init_ttyS(fd);  	// init device
                func_485_transfer(fd); 	// 485 functions

                // close ttyS0
                if (close(fd)!=0) printf("close device %s error",DEVICE_TTYS);
        }

        return 0;
}// end main

