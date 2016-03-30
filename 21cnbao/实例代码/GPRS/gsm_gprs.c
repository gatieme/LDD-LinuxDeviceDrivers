// this is a test about GPRS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>	// open() close() 
#include <unistd.h>	// read() write()

#include <termios.h>	// set baud rate

#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

#define FUNC_RUN		0
#define FUNC_NOT_RUN		1

#define SIMPLE_TEST 		1
#define READ_SIM_CARD_ID 	2
#define MAKE_A_CALL 		3
#define WAIT_A_CALL	 	4
#define SHORT_MESSAGE 		5
#define FUNC_QUIT 		6

#define SEND_SHORT_MESSAGE		1
#define READ_SHORT_MESSAGE		2
#define CONFIG_SHORT_MESSAGE_ENV        3
#define QUIT_SHORT_MESSAGE		4

#define DEVICE_TTYS "/dev/ttyS1"
#define MAX_LEN_OF_SHORT_MESSAGE	140


#define RECEIVE_BUF_WAIT_1S 1
#define RECEIVE_BUF_WAIT_2S 2
#define RECEIVE_BUF_WAIT_3S 3
#define RECEIVE_BUF_WAIT_4S 4
#define RECEIVE_BUF_WAIT_5S 5

//------------------------------------- read datas from GSM/GPRS --------------------------------------------
// succese return 1
// error   return 0
int read_GSM_GPRS_datas(int fd, char *rcv_buf,int rcv_wait)
{
	int retval;
	fd_set rfds;
	struct timeval tv;

	int ret,pos;

	tv.tv_sec = rcv_wait;	// wait 2.5s
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
                        printf("No data\n");
                        break;
                }
	}

	return 1;
} // end read_GSM_GPRS_datas


//------------------------------------- send cmd ------------------------------------------------------------
// succese return 1
// error   return 0
int send_GSM_GPRS_cmd(int fd, char *send_buf)
{
	ssize_t ret;
	
	ret = write(fd,send_buf,strlen(send_buf));
	if (ret == -1)
        {
                printf ("write device %s error\n", DEVICE_TTYS);
                return -1;
        }

	return 1;
} // end send_GSM_GPRS_cmd

//------------------------------------- send cmd and read back result ---------------------------------------
void GSM_GPRS_send_cmd_read_result(int fd, char *send_buf, int rcv_wait)
{
        char rcv_buf[2048];

	if((send_buf==NULL) || (send_GSM_GPRS_cmd(fd,send_buf)))
	{	// send success , then read
		bzero(rcv_buf,sizeof(rcv_buf));
		if (read_GSM_GPRS_datas(fd,rcv_buf,rcv_wait))
                {
                        printf ("%s\n",rcv_buf);
                }
                else
                {
                        printf ("read error\n");
                }

	}
	else
	{
		printf("write error\n");
	}
} // end GSM_GPRS_send_cmd_read_result

//------------------------------------- send cmd : "at" to GSM/GPRS MODEM -----------------------------------
void GSM_simple_test(int fd)
{
	char *send_buf="at\r";
	
	GSM_GPRS_send_cmd_read_result(fd,send_buf,RECEIVE_BUF_WAIT_1S);

} // end GSM_simple_test

//------------------------------------- send cmd : "at+ccid" to GSM/GPRS MODEM ------------------------------
void GSM_read_sim_card_id(int fd)
{
        char *send_buf="at+ccid\r";

	GSM_GPRS_send_cmd_read_result(fd,send_buf,RECEIVE_BUF_WAIT_1S);

} // end GSM_read_sim_card_id

//------------------------------------- print ---------------------------------------------------------------
void print_prompt(void)
{
        printf ("Select what you want to do:\n");
        printf ("1 : Simple Test\n");
        printf ("2 : Read SIM CARD ID\n");
        printf ("3 : Make A Call\n");
        printf ("4 : Wait A Call\n");
        printf ("5 : Short message\n");
        printf ("6 : Quit\n");
        printf (">");
} // end print_prompt

//------------------------------------- Control GSM/GPRS MODULE ---------------------------------------------
void func_GSM(int fd)
{
        int flag_func_run;
        int flag_select_func;
        ssize_t ret;

        flag_func_run = FUNC_RUN;

        while (flag_func_run == FUNC_RUN)
        {
                print_prompt();			// print select functions
                scanf("%d",&flag_select_func);	// user input select
                getchar();

                switch(flag_select_func)
                {
                        case SIMPLE_TEST        : {GSM_simple_test(fd);         break;}
                        case READ_SIM_CARD_ID   : {GSM_read_sim_card_id(fd);    break;}
                        case MAKE_A_CALL        : {GSM_call(fd);                break;}
                        case WAIT_A_CALL        : {GSM_wait_call(fd);           break;}
                        case SHORT_MESSAGE      : {GSM_short_mesg(fd);          break;}
                        case FUNC_QUIT          :
                                                {
                                                        flag_func_run = FUNC_NOT_RUN;
                                                        printf("Quit GSM/GPRS function.  byebye\n");
                                                        break;
                                                }
                        default :
                        {
                                printf("please input your select use 1 to 7\n");
                        }
                }

        }
}// end func_GPRS

//------------------------------------- init seriel port  ---------------------------------------------------
void init_ttyS(int fd)
{
        struct termios options;

        bzero(&options, sizeof(options));       // clear options

        cfsetispeed(&options,B9600);            // setup baud rate
        cfsetospeed(&options,B9600);

        options.c_cflag |= (CRTSCTS | CS8 | CLOCAL | CREAD);
	options.c_iflag = IGNPAR;

	tcflush(fd, TCIFLUSH);

        tcsetattr(fd, TCSANOW, &options);

}//end init_ttyS


//------------------------------------- main ----------------------------------------------------------------
int main(void)
{
        int fd;

        printf("\nGSM/GPRS TESTS\n\n");

        // open seriel port
        fd = open(DEVICE_TTYS, O_RDWR);

        if (fd == -1)
        {
                printf("open device %s error\n",DEVICE_TTYS);
        }
        else
        {
		init_ttyS(fd);	// init device
		func_GSM(fd);	// GSM/GPRS functions
                
		// close ttyS0
                if (close(fd)!=0) printf("close device %s error",DEVICE_TTYS);
        }

        return 0;
}// end main
