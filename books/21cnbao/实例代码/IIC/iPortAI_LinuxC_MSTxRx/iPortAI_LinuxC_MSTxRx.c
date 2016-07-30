/*
	iPort/AI Asynchronous Communication Sample Application for Linux

	Version 2 - 19-JUN-06

	Provided by MCC (Micro Computer Control Corporation)
	www.mcc-us.com

	This application supports the following I2C bus operations:
		1.  Master transmit
		2.  Master recieve
		3.  Master TxRx
		4.  Slave transmit
		5.  Slave recieve

	System requirements:
		1.  GCC (>=3.6)
		2.  User has write access to serial ports (more information below)
		3.  iPort/AI I2C Bus host adapter
		4.  Linux kernel 2.4 or newer
	
	Serial port information:
		Considerations and trouble shooting for using the serial port can
		be found in the file "serial_help.txt".

	How to compile this software:
		1.  Run "make clean" to remove any past compiled versions of this software
		2.  Run "make" to compile the software
		3.  Run the program by entering "./iPortAI_LinuxC_MSTxRx /dev/ttySn", where n is
		    the number of the desired serial port (see "serial_help.txt" for more info)
		    Example: ./iPortAI_LinuxC_MSTxRx /dev/ttyS0

	Notes:
		This software operates asynchronously, meaning that operations can be initiated
		either by the software itself, or by incoming data.  In addition, functions that
		perform master operations can be modified to avoid waiting for a reply, and return 
		immediately, allowing the software to go about it's business and check for replies
		when it gets a chance.  Slave commands are automatically detected and interpreted,
		without requiring the software to know anything about their happening (beyond some
		predefined routines to handle them).  Because of this added flexibility, this method
		is recommended over the synchronous form, and is necessary for full operation as
		a slave.

		Also, this code uses software flow control.  Hardware flow control would require a different
		configuration in configureSerial(), as well as the adjusted iPortAI_setFlow() call.

	Revision history:
		19-JUN-06	Fixed incorrrect "reset" message - released version 2
		02-JUN-06	First revision

*/


//Minimum necessary header files
#include <stdio.h>		//Generic input/output
#include <fcntl.h>		//Modes and flags for IO
#include <string.h>		//String routines
#include <termios.h>		//Terminal control
#include <sys/signal.h>		//Operating system signals
#include <sys/time.h>		//Timer definitions

//Function prototypes
void configureSerial ();
int writeSerial (char *);
void handleReply (int);
void identifySignal ();
void printAction (char *);
int validateAddress (char *);
void iPortAI_reset ();
void iPortAI_setFormat (int);
void iPortAI_setFlow (int);
void iPortAI_setAddress (char *);
void iPortAI_selectSlave (char *);
int iPortAI_open ();
int iPortAI_close ();
int iPortAI_MTx (char *);
int iPortAI_MRx (int);
int iPortAI_STx (char *s);
int iPortAI_M_Send (char *, char *);
int iPortAI_M_Recieve (char *, int);
int iPortAI_M_SendRecieve (char *, char *, int);
void iPortAI_S_Recieve ();
void iPortAI_S_Send ();
void configureWatchdog ();
void handleWatchdog (int);
void resetWatchdog ();

//Watchdog variables
#define WATCHDOG_FREQUENCY 10		//The frequency of the watchdog timer, in Hz
#define WATCHDOG_TIMEOUT_COUNT 100	//The number of ticks before the watchdog announces a timeout
volatile int WATCHDOG;			//Global flag to enable/disable the watchdog timer
volatile int TIMEOUT;			//Global flag to determine if the watchdog timer announced a timeout
volatile int wt;			//Global watchdog timer counter

//If the user doesn't supply a serial port when he/she runs the program,
//this default port will be used.
char DEFAULT_SERIAL_PORT[] = "/dev/ttyS0";

//Global descriptor for the serial port
int port_descriptor;

//Global flag to announce when a signal is recieved
volatile int DATA_READY;

//Space for the signal name and any accompanying data
#define MAX_DATA_SIZE 255
char sig[5] = {0, 0, 0, 0, 0};
volatile char *data;
char buffer[MAX_DATA_SIZE];
volatile int bptr;

//Space for the slave's data
char SLAVE_BUFFER_IN[MAX_DATA_SIZE];
char SLAVE_BUFFER_OUT[4] = {'~', '0', '0', 0};

//Handy defines
#define ON 1
#define OFF 0
#define SOFTWARE 0
#define HARDWARE 1
#define HEX_ONLY 0
#define ALLOW_ASCII 1

//Main routine
int main (int argc, char **argv) {
	//If the user supplied a serial port, use that, otherwise
	//use our predefined default.
	char *serial_port;
	if (argc > 1) serial_port = argv[1];
	else serial_port = DEFAULT_SERIAL_PORT;
	printf("Using %s as our serial port.\n", serial_port);

	//Attempt to open the port for read+write access, without needing to 
	//be a controlling terminal on the port.
	port_descriptor = open(serial_port, O_RDWR | O_NOCTTY);
	if (port_descriptor == -1) {
		printf("Failed to open %s for read+write access.\n", serial_port);
		return 1;
	}

	//Configure the serial port
	configureSerial();

	//Configure the watchdog timer
	configureWatchdog();

	//Configure the iPortAI
	iPortAI_reset();
	iPortAI_setFlow(SOFTWARE);
	iPortAI_setFormat(HEX_ONLY);
	iPortAI_setAddress("70");
	iPortAI_open();

	//Select a slave, then send it four messages in a row
	iPortAI_selectSlave("4E");
	iPortAI_MTx("~FF~12~44");
	iPortAI_MTx("~00~67");
	iPortAI_MTx("~FF~10~00~EC~E4~30");
	iPortAI_MTx("~00");

	//Send two messages to different slaves, using the high level wrapper
	iPortAI_M_Send("4E", "~77~17~27~88");
	iPortAI_M_Send("40", "ASCII data!");

	//Recieve a message from a slave, using the high level wrapper
	iPortAI_M_Recieve("4E", 2);

	//Send a message, then recieve a message from a slave in one step
	iPortAI_M_SendRecieve("4E", "~FF~FF~7C", 4);

	//Send a big message, then read a single byte
	char big[100];
	int i;
	for (i = 0; i < 98; i++) big[i] = '+';
	strcpy(&big[96], "~55");
	big[99] = 0;
	iPortAI_M_SendRecieve("4E", big, 1);

	//All done!  Close the iPort's connection to the I2C bus and
	//the operating systems serial port before we exit
	iPortAI_close();
	close(port_descriptor);
	return 0;
}



/************************************************
	Serial Port Communication Routines
************************************************/

//Configure a serial port
//Takes:
//	Nothing
//Returns:
//	Nothing
void configureSerial () {
	//Attach our handler to the IO signal
	struct sigaction sh;
	sh.sa_handler = handleReply;
	sh.sa_flags = 0;
	sh.sa_restorer = NULL;
	sigaction(SIGIO, &sh, NULL);

	//Tell the OS to let this process catch the IO signal
	fcntl(port_descriptor, F_SETOWN, getpid());

	//Make the port descriptor asynchronous
	fcntl(port_descriptor, F_SETFL, FASYNC);
	
	//Read in the current port options
	struct termios options;
	tcgetattr(port_descriptor, &options);

	//Configure the port
	options.c_cflag = 0;				//Clear all flags (if you don't, many bits
	options.c_iflag = 0;				//must be cleared manually to ensure the
	options.c_oflag = 0;				//correct mode is chosen)
	options.c_lflag = 0;
	options.c_cflag |= B19200;			//baud rate: 19.2 kpbs
	options.c_cflag |= CS8;				//8 data bits (no parity and 1 stop bit are default)
	options.c_cflag |= CREAD;			//Enable the reciever
	options.c_cflag |= CLOCAL;			//Set "local" mode
	options.c_iflag |= (IXON | IXOFF | IXANY);	//Software flow control
	options.c_cc[VEOL] = 13;			//CR byte
	options.c_cc[VEOL2] = 10;			//LF byte
	options.c_cc[VSTART] = 17;			//XON byte
	options.c_cc[VSTOP] = 19;			//XOFF byte
	options.c_cc[VMIN] = 0;				//No blocking
	options.c_cc[VTIME] = 0;

	//Store the new options
	tcflush(port_descriptor, TCIFLUSH);
	tcsetattr(port_descriptor, TCSANOW, &options);
}

//Write a string to the serial port
//Takes:
//	buffer: char * - pointer to an array of characters (a string)
//Returns:
//	1 on success, 0 on failure
//Notes:
//	Success does not mean the iPortAI recieved the string, just that
//	on this end things appear to have worked.  Even if the iPortAI
//	is not connected this command will usually succeed.
int writeSerial (char *buffer) {
	//Reset the buffers
	DATA_READY = 0;
	sig[0] = 0;
	data = buffer;
	bptr = 0;

	//Determine how many bytes are in the user's string
	int c = strlen(buffer);

	//Try to write to the port
	int bytes_written = write(port_descriptor, buffer, c);

	//If we succesfully wrote all of our buffer out, we succeeded
	if (bytes_written == c) return 1;
	else return 0;
}

//Handle data coming into the serial port
//Takes:
//	status: int
//Returns:
//	Nothing
//Notes:
//	This function is called automatically when 1 or more bytes of data (a 
//	chunk) is in the serial port buffer.  This chunk could be nothing important,
//	it could be the start of a new message, or it could be extra data going
//	along with the last message.
void handleReply (int status) {
	//This flag tells us if we're reading preliminary data up until a "/", or
	//if we're reading actual content
	static int read_prelim = 1;

	//Read the chunk into a temporary buffer
	char temp[MAX_DATA_SIZE];
	int l = read(port_descriptor, temp, MAX_DATA_SIZE);

	//Append a null character to mark the end of the chunk
	temp[l] = 0;

	//Read past "filler" data (CR's, LF's, asterisks) at the beginning of the chunk.
	//If we're reading actual message data, however, we can't skip over asterisks, since
	//they might be part of an ASCII string we're being sent.
	char *reply = temp;
	if (!read_prelim)
		while ((*reply == 13 || *reply == 10) && *reply != 0) reply++;
	else
		while ((*reply == 13 || *reply == 10 || *reply == '*') && *reply != 0) reply++;
	
	//If the first non-filler byte is a "/", it's a new message
	//and we're done reading preliminary data
	int is_newmessage = 0;
	if (*reply == '/' && read_prelim == 1) {
		is_newmessage = 1;
		read_prelim = 0;
	}

	//Find the end of the chunk by finding the first filler character (will be
	//a CR or an LF, unless there's nothing to find, in which case we hit a null)
	int end_of_data = 0;
	while (reply[end_of_data] != 13 && reply[end_of_data] != 10 && reply[end_of_data] != 0) end_of_data++;

	//If the chunk is now empty, we don't have anything worth looking at
	if (reply[0] == 0) return;

	//If the chunk ended in a CR, it's the end of a message, and we need to set the
	//preliminary flag so the next read will know it's getting a brand new message.
	int is_messagedone = 0;
	if (reply[end_of_data] == 13) {
		is_messagedone = 1;
		read_prelim = 1;
	}

	//"reply" now holds the actual content that was sent to us.  It might be nothing (if all
	//we were sent was filler data), it might be a message ("/MTC" or whatever), or it could
	//be extra data to be appended to whatever we read last (such as data coming from a slave
	//transfer).
	reply[end_of_data] = 0;
	l = end_of_data;

	//Copy this data into the permanent buffer.  If it's a new message, we'll just overwrite
	//whatever is already in the buffer.  If it's not a new message (ie. it's extra data
	//for the last chunk) just tack it on the end.
	if (is_newmessage) bptr = 0;
	strncpy(&buffer[bptr], reply, l + 1);
	bptr += l;
	
	//If this data concludes a message (we checked earlier), update
	//the global variables to point to the respective parts, and set the
	//ready flag.  Also identify the message and print out some information.
	if (is_messagedone) {
		strncpy(sig, buffer, 4);
		data = buffer + 4;
		bptr = 0;
		DATA_READY = 1;
		sigrelse(SIGIO);	//Release the signal manually
		identifySignal();
	}
}

//Identify a signal.
//Takes:
//	Nothing
//Returns:
//	Nothing
//Notes:
//	Most messages are handled with a simple output string.  Slave messages, however, require
//	us to switch gears and act as a slave.
void identifySignal () {
	printf("- ");
	if (!strcmp(sig, "/OCC")) printf("Open connection complete.\n");
	else if (!strcmp(sig, "/MTC")) printf("Master transmit complete.\n");
	else if (!strcmp(sig, "/MRC")) printf("Master recieve complete.  Data: '%s'\n", data);
	else if (!strcmp(sig, "/STC")) printf("Slave transmit complete.\n");
	else if (!strcmp(sig, "/CCC")) printf("Close connection complete.\n");
	else if (!strcmp(sig, "/SRC")) {
		printf("Slave recieve complete.  Data: '%s'.  Calling handler...\n", data);
		strncpy(SLAVE_BUFFER_IN, (char *)data, MAX_DATA_SIZE);
		iPortAI_S_Recieve();
	}
	else if (!strcmp(sig, "/GRC")) printf("General recieve complete.  Data: '%s'\n", data);
	else if (!strcmp(sig, "/STR")) {
		printf("Slave transmit request.  Calling handler...\n");
		iPortAI_S_Send();
	}
	else if (!strcmp(sig, "/SNA")) printf("Slave not acknowledging.\n");
	else if (!strcmp(sig, "/I81")) printf("iPortAI busy.\n");
	else if (!strcmp(sig, "/I83")) printf("Arbitration loss.\n");
	else if (!strcmp(sig, "/I84")) printf("I2C bus error.\n");
	else if (!strcmp(sig, "/I85")) printf("I2C bus timeout.\n");
	else if (!strcmp(sig, "/I88")) printf("Connection closed.\n");
	else if (!strcmp(sig, "/I89")) printf("Invalid command argument.\n");
	else if (!strcmp(sig, "/I8A")) printf("Slave transfer request not active.\n");
	else if (!strcmp(sig, "/I8F")) printf("Invalid command.\n");
	else if (!strcmp(sig, "/I90")) printf("Recieve buffer overflow.\n");
	else printf("Unknown message: %s%s\n", sig, data);
}



/****************************************
	iPortAI Communication Routines
*****************************************/

//Display an "action" phrase - just a way to easily format what we're doing
//Takes:
//	s: char * - character string
//Returns:
//	Nothing
void printAction (char *s) {
	printf("> %s\n", s);
}

//Validate an address
//Takes:
//	s: char * - character string, first two characters are the address (no null terminator necessary)
//Returns:
//	1 on success, 0 on failure
//Notes:
//	Valid addresses are between 0x00 and 0xFF, non-inclusive, and must be even.
int validateAddress (char *s) {
	//Convert the hex string to an integer
	char temp[3] = {0, 0, 0};
	strncpy (temp, s, 2);
	int dec_num;
	sscanf(temp,"%x",&dec_num);

	//Make sure it's acceptable (between 0 and 0xFE, even)
	if (dec_num < 0 || dec_num > 0xFE || dec_num % 2 == 1) return 0;
	else return 1;
}

//Reset the iPortAI
//Takes:
//	Nothing
//Returns:
//	Nothing
void iPortAI_reset () {
	printAction("Resetting...");
	writeSerial("\x12\x12\x12");
	usleep(20000);		//20 ms
}

//Set the format of incoming data
//Takes:
//	f: int - HEX_ONLY | ALLOW_ASCII
//Returns:
//	Nothing
void iPortAI_setFormat (int f) {
	if (f == HEX_ONLY) {
		printAction("Setting incoming data to be restricted to hex only...");
		writeSerial("/h1\r");
	}
	else if (f == ALLOW_ASCII) {
		printAction("Setting incoming data to be hex or ascii...");
		writeSerial("/h0\r");
	}
	else printAction("Bad format argument.");
}

//Set flow control
//Takes:
//	f: int - HARDWARE | SOFTWARE
//Returns:
//	Nothing
void iPortAI_setFlow (int f) {
	if (f == SOFTWARE) {
		printAction("Setting software flow control...");
		writeSerial("/f0\r");
	}
	else if (f == HARDWARE) {
		printAction("Setting hardware flow control...");
		writeSerial("/f1\r");
	}
	else printAction("Bad flow control argument.");
}

//Set the address of the iPortAI
//Takes:
//	address: char * - character string, first two characters are the desired address
//Returns:
//	Nothing
void iPortAI_setAddress (char *address) {
	printAction("Setting our own address...");
	if (!validateAddress(address)) {
		printAction("Bad address.");
		return;
	}
	char temp[6] = "/i??\r";
	strncpy(&temp[2], address, 2);
	writeSerial(temp);
}

//Select a slave
//Takes:
//	address: char * - character string, first two characters are the desired address
//Returns:
//	Nothing
void iPortAI_selectSlave (char *address) {
	printAction("Selecting a slave...");
	if (!validateAddress(address)) {
		printAction("Bad slave address.");
		return;
	}
	char temp[6] = "/d??\r";
	strncpy(&temp[2], address, 2);
	writeSerial(temp);
}

//Open the I2C bus
//Takes:
//	Nothing
//Returns:
//	1 on success, 0 on failure
int iPortAI_open () {
	printAction("Opening the I2C bus...");
	writeSerial("/o\r");
	WATCHDOG = ON;
	while (!DATA_READY && !TIMEOUT);
	resetWatchdog();
	if (!strcmp(sig, "/OCC")) return 1;
	else return 0;
}

//Close the I2C bus
//Takes:
//	Nothing
//Returns:
//	1 on success, 0 on failure
int iPortAI_close () {
	printAction("Closing the I2C bus...");
	writeSerial("/c\r");
	WATCHDOG = ON;
	while(!DATA_READY && !TIMEOUT);
	resetWatchdog();
	if (!strcmp(sig, "/CCC")) return 1;
	else return 0;
}

//Master Tx
//Takes:
//	s: char * - character string (null terminated)
//Returns:
//	1 on success, 0 on failure
int iPortAI_MTx (char *s) {
	printAction("Sending data to a slave...");
	writeSerial("/t");
	writeSerial(s);
	writeSerial("\r");
	WATCHDOG = ON;
	while(!DATA_READY && !TIMEOUT);
	resetWatchdog();
	if (!strcmp(sig, "/MTC")) return 1;
	else return 0;
}

//Master Rx
//Takes:
//	c: int - number of bytes to read
//Returns:
//	1 on success, 0 on failure
int iPortAI_MRx (int c) {
	printAction("Recieving data from a slave...");

	//Validate the number of bytes we're supposed to read
	if (c < 0 || c > MAX_DATA_SIZE - 4) {
		printAction("Number of bytes to read is out of range.");
		return 0;
	}

	//Send the message
	char temp[10];
	sprintf(temp, "%d", c);
	writeSerial("/r");
	writeSerial(temp);
	writeSerial("\r");
	WATCHDOG = ON;
	while(!DATA_READY && !TIMEOUT);
	resetWatchdog();
	if (!strcmp(sig, "/MRC")) return 1;
	else return 0;
}

//Slave Tx
//Takes:
//	s: char * - character string (null terminated)
//Returns:
//	1 on success, 0 on failure
int iPortAI_STx (char *s) {
	printAction("Sending data to a master...");
	writeSerial("/s");
	writeSerial(s);
	writeSerial("\r");

	WATCHDOG = ON;
	while(!DATA_READY && !TIMEOUT);
	resetWatchdog();
	if (!strcmp(sig, "/STC")) return 1;
	else return 0;
}



/****************************************
	High Level iPortAI Wrappers
*****************************************/

//Select a slave and send an MTx in one step
//Takes:
//	address: char * - character string, first two characters are the desired address
//	s: char * - character string (null terminated)
//Returns:
//	1 on success, 0 on failure
int iPortAI_M_Send (char *address, char *s) {
	iPortAI_selectSlave(address);
	return iPortAI_MTx(s);
}

//Select a slave and send an MRx in one step
//Takes:
//	address: char * - character string, first two characters are the desired address
//	c: int - number of bytes to read
//Returns:
//	1 on success, 0 on failure
int iPortAI_M_Recieve (char *address, int c) {
	iPortAI_selectSlave(address);
	return iPortAI_MRx(c);
}

//Select a slave, send an MTx, followed by an MRx, all in one step
//Takes:
//	address: char * - character string, first two characters are the desired address
//	s: char * - character string (null terminated)
//	c: int - number of bytes to read
//Returns:
//	1 on success, 0 on failure
int iPortAI_M_SendRecieve (char *address, char *s, int c) {
	iPortAI_selectSlave(address);
	if (!iPortAI_MTx(s)) return 0;
	else if (!iPortAI_MRx(c)) return 0;
	else return 1;
}

//Handle slave recieve requests
//Takes:
//	Nothing, called automatically when a slave recieve request comes in
//Returns:
//	Nothing
//Notes:
//	The data coming in will be stored in SLAVE_BUFFER_IN as soon as it is caught.
void iPortAI_S_Recieve () {
	//Copy the first 3 bytes from the incoming buffer into the outgoing buffer
	strncpy(SLAVE_BUFFER_OUT, SLAVE_BUFFER_IN, 3);
}

//Handle slave transfer requests
//Takes:
//	Nothing, called automatically when a slave transfer request comes in
//Returns:
//	Nothing
void iPortAI_S_Send () {
	//Write our outgoing character out
	iPortAI_STx(SLAVE_BUFFER_OUT);
}


/****************************************
	Watchdog Timer Routines
*****************************************/

//Configure the watchdog timer
//Takes:
//	Nothing
//Returns:
//	Nothing
void configureWatchdog () {
	//Configure the timer to update at WATCHDOG_FREQUENCY, in Hz
	struct itimerval timer1;
	timer1.it_interval.tv_sec = 0;
	timer1.it_interval.tv_usec = (__suseconds_t) ( (1.0 / (double) WATCHDOG_FREQUENCY) * 1000000);
	timer1.it_value.tv_sec = timer1.it_interval.tv_sec;
	timer1.it_value.tv_usec = timer1.it_interval.tv_usec;

	//Attach our handler to the ALRM signal
	signal(SIGALRM, handleWatchdog);

	//Start the timer
	setitimer(ITIMER_REAL, &timer1, NULL);
}

//Handle the watchdog timer
//Takes:
//	status: int
//Returns:
//	Nothing
//Notes:
//	If WATCHDOG is on, then the watchdog's internal counter will increase.  Once it
//	reaches WATCHDOG_TIMEOUT_COUNT, TIMEOUT will be set to 1, and the watchdog will
//	turn itself off.
void handleWatchdog (int status) {
	if (WATCHDOG) {
		wt++;
		if (wt == WATCHDOG_TIMEOUT_COUNT) {
			wt = 0;
			TIMEOUT = 1;
			WATCHDOG = OFF;
			printf("! Timed out\n");
		}
	}
}

//Reset the watchdog timer
//Takes:
//	Nothing
//Returns:
//	Nothing
//Notes:
//	This resets the watchdog's internal counter, as well as turning TIMEOUT and the
//	watchdog itself off.
void resetWatchdog () {
	wt = 0;
	TIMEOUT = 0;
	WATCHDOG = OFF;
}


