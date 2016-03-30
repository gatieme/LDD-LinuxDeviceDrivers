/*
	iPort/AI Synchronous Communication Sample Application for Linux
	
	Version 2 - 19-JUN-06

	Provided by MCC (Micro Computer Control Corporation)
	www.mcc-us.com

	This application supports the following I2C bus operations:
		1.  Master transmit
		2.  Master recieve
		3.  Master TxRx

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
		3.  Run the program by entering "./iPortAI_LinuxC_MTxRx /dev/ttySn", where n is
		    the number of the desired serial port (see "serial_help.txt" for more info)
		    Example: ./iPortAI_LinuxC_MTxRx /dev/ttyS0

	Notes:
		This software operates synchronously, meaning that all operations must be initiated by
		the software.  Master commands are, by nature, synchronous, as the software decides it
		is time to send data to a slave.  Slave operations, however, are asynchronous, because
		they can occur at any time.  This software cannot handle such operations, as it only
		examines what data is crossing the bus when the getReply() function is called.  Instead,
		the software will detect the slave operation the next time getReply() is called, and
		deal with it then.

		Also, this code uses software flow control.  Hardware flow control would require a different
		configuration in configureSerial(), as well as the adjusted iPort_setFlow() call.

	Revision history:
		19-JUN-06	Fixed incorrect "reset" message - released version 2
		02-JUN-06	First revision

*/


//Minimum necessary header files
#include <stdio.h>		//Generic input/output
#include <fcntl.h>		//Modes and flags for IO
#include <string.h>		//String routines
#include <termios.h>		//Terminal control

//Function prototypes
void configureSerial ();
int writeSerial (char *);
int getReply (int);
void identifySignal ();
int validateAddress (char *);
void iPortAI_reset ();
void iPortAI_setFlow (int);
void iPortAI_setAddress (char *);
void iPortAI_selectSlave (char *);
int iPortAI_open ();
int iPortAI_close ();
int iPortAI_MTx (char *);
int iPortAI_MRx (int);
int iPortAI_M_Send (char *, char *);
int iPortAI_M_Recieve (char *, int);
int iPortAI_M_SendRecieve (char *, char *, int);

//If the user doesn't supply a serial port when he/she runs the program,
//this default port will be used.
char DEFAULT_SERIAL_PORT[] = "/dev/ttyS0";

//Global descriptor for the serial port
int port_descriptor;

//Space for the signal name and any accompanying data
#define MAX_DATA_SIZE 255
char sig[5] = {0, 0, 0, 0, 0};
char *data;
char buffer[MAX_DATA_SIZE];

//Handy defines
#define SOFTWARE 0
#define HARDWARE 1

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

	//Configure the iPortAI
	iPortAI_reset();
	iPortAI_setFlow(SOFTWARE);
	iPortAI_setAddress("70");
	iPortAI_open();

	//Select a slave, then send it four messages in a row
	iPortAI_selectSlave("4E");
	iPortAI_MTx("~FF");
	iPortAI_MTx("~00");
	iPortAI_MTx("~FF");
	iPortAI_MTx("~00");

	//Send two messages to different slaves, using the high level wrapper
	iPortAI_M_Send("4E", "~77");
	iPortAI_M_Send("40", "Hello");

	//Recieve a message from a slave, using the high level wrapper
	iPortAI_M_Recieve("4E", 1);

	//Send a message, then recieve a message from a slave in one step
	iPortAI_M_SendRecieve("4E", "~FF", 1);

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
	//Set non blocking behaviour - read() will return immediately
	fcntl(port_descriptor, F_SETFL, FNDELAY);
	
	//Read in the current port options
	struct termios options;
	tcgetattr(port_descriptor, &options);

	//Configure the port
	options.c_cflag = 0;				//Clear all flags (if you don't, many bits
	options.c_iflag = 0;				//must be cleared manually to ensure the
	options.c_oflag = 0;				//correct mode is chosen)
	options.c_lflag = 0;
	cfsetispeed(&options, B19200);			//baud rate: 19.2 kpbs
	cfsetospeed(&options, B19200);
	options.c_cflag |= CS8;				//8 data bits (no parity and 1 stop bit are default)
	options.c_iflag |= (IXON | IXOFF | IXANY);	//Software flow control
	options.c_cflag |= CREAD;			//Enable the reciever
	options.c_cflag |= CLOCAL;			//Set "local" mode
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
	//Determine how many bytes are in the user's string
	int c = strlen(buffer);

	//Try to write to the port
	int bytes_written = write(port_descriptor, buffer, c);

	//If we succesfully wrote all of our buffer out, we succeeded
	if (bytes_written == c) return 1;
	else return 0;
}

//Listen to the IPortAI for a reply
//Takes:
//	timeout: int - time in milliseconds
//Returns:
//	int: 1 on success, 0 on timeout
int getReply (int timeout) {
	buffer[0] = 0;			//Init the buffer with a null character
	int i = 0;			//Index into the buffer
	char incoming;			//Incoming byte
	timeout /= 10;
	if (timeout == 0) timeout = 1;

	//Run continuously, until a reply is gathered or the iPortAI stops responding
	while (1) {
		//Read a single character from the IPortAI (until we get one or time out)
		int c;
		for (c = 0; c < timeout; c++) {
			if (read(port_descriptor, &incoming, 1) == 1) break;
			usleep(10000);
		}
		if (c == timeout) {
			printf("Timed out!\n");
			sig[0] = 0;
			return 0;
		}

		//Carriage returns indicate that some sort of reply has been given.  If
		//the reply isn't crucial (ie, the buffer is still empty), we just ignore the
		//CR.  If it's a "real" reply, we're done.
		if (incoming == 13 && i != 0) break;

		//Any other incoming characters that are not line feeds, carriage
		//returns, or asterisks are crucial and will be recorded.
		else if (incoming != 13 && incoming != 10 && incoming != '*') {
			buffer[i] = incoming;
			buffer[i + 1] = 0;
			i++;
			if (i == MAX_DATA_SIZE) {
				printf("Out of room to store incoming message!\n");
				break;
			}
		}
	}

	//Extract the first four characters from the reply
	sig[4] = 0;
	strncpy(sig, buffer, 4);

	//Locate any remaining data
	data = buffer + 4;

	//All done!
	return 1;
}

//Identify a signal.
//Takes:
//	Nothing
//Returns:
//	Nothing
//Notes:
//	If we get a signal, but it's not something we're expecting (such as a slave transmit request), we handle
//	it as quickly as possible and get a new message.  It means that while we were doing whatever it was we
//	were doing, something on the bus sent us a message we aren't expecting, and we have to ditch it and go
//	retrieve another message.
void identifySignal () {
	printf("- ");
	if (!strcmp(sig, "/OCC")) printf("Open connection complete.\n");
	else if (!strcmp(sig, "/MTC")) printf("Master transmit complete.\n");
	else if (!strcmp(sig, "/MRC")) printf("Master recieve complete.  '%s'\n", data);
	else if (!strcmp(sig, "/STC")) {
		printf("Slave transmit complete.  Ignoring and waiting for a new message.\n");
		getReply(1000);
		identifySignal();
	}
	else if (!strcmp(sig, "/CCC")) printf("Close connection complete.\n");
	else if (!strcmp(sig, "/SRC")) {
		printf("Slave recieve complete.  '%s'  Ignoring and waiting for a new message.\n", data);
		getReply(1000);
		identifySignal();
	}
	else if (!strcmp(sig, "/GRC")) printf("General recieve complete.  '%s'\n", data);
	else if (!strcmp(sig, "/STR")) {
		printf("Slave transfer request.  Sending data and waiting for a new message.\n", data);
		writeSerial("/s~FF\r");		//Send a byte to release the bus
		getReply(1000);			//Wait for a non-slave message to come in
		identifySignal();		//Repeat this routine once it does
	}
	else if (!strcmp(sig, "/SNA")) printf("Slave not acknowledging.\n");
	else if (!strcmp(sig, "/I81")) {
		printf("iPortAI busy.  Waiting for a new message.\n");
		getReply(1000);
		identifySignal();
	}
	else if (!strcmp(sig, "/I83")) {
		printf("Arbitration loss.  Waiting 10 ms and waiting for possible slave message.\n");
		usleep(10000);
		getReply(1000);
		identifySignal();
	}
	else if (!strcmp(sig, "/I84")) printf("I2C bus error.\n");
	else if (!strcmp(sig, "/I85")) printf("I2C bus timeout.\n");
	else if (!strcmp(sig, "/I88")) printf("Connection closed.\n");
	else if (!strcmp(sig, "/I89")) printf("Invalid command argument.\n");
	else if (!strcmp(sig, "/I8A")) printf("Slave transfer request not active.\n");
	else if (!strcmp(sig, "/I8F")) printf("Invalid command.\n");
	else if (!strcmp(sig, "/I90")) printf("Recieve buffer overflow.\n");
	else if (sig[0] == 0) return;	//empty message, we timed out
	else {
		printf("Unknown message: %s(%s) Waiting for a new message.\n", sig, data);
		getReply(1000);
		identifySignal();
	}
}



/****************************************
	iPortAI Communication Routines
*****************************************/

//Validate an address
//Takes:
//	s: char * - character string, first two characters are the address (no null terminator necessary)
//Returns:
//	1 on success, 0 on failure
int validateAddress (char *s) {
	//Convert the hex string to an integer
	char temp[3] = {0, 0, 0};
	strncpy (temp, s, 2);
	int dec_num;
	sscanf(temp,"%x",&dec_num);

	//Make sure it's acceptable
	if (dec_num < 0 || dec_num > 0xFE || dec_num % 2 == 1) return 0;
	else return 1;
}

//Reset the iPortAI
//Takes:
//	Nothing
//Returns:
//	Nothing
void iPortAI_reset () {
	printf("Resetting...\n");
	writeSerial("\x12\x12\x12");
	usleep(20000);		//20 ms
}

//Set flow control
//Takes:
//	HARDWARE | SOFTWARE: int - predefined
//Returns:
//	Nothing
void iPortAI_setFlow (int f) {
	printf("Setting %s flow control...\n", f ? "hardware" : "software");
	if (f == SOFTWARE) writeSerial("/f0\r");
	else if (f == HARDWARE) writeSerial("/f1\r");
}

//Set the address of the iPortAI
//Takes:
//	address: char * - character string, first two characters are the desired address
//Returns:
//	Nothing
void iPortAI_setAddress (char *address) {
	printf("Setting our own address...\n");
	if (!validateAddress(address)) {
		printf("Bad address.\n");
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
	printf("Selecting a slave...\n");
	if (!validateAddress(address)) {
		printf("Bad slave address.\n");
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
	printf("Opening the I2C bus...\n");
	writeSerial("/o\r");
	getReply(100);
	if (!strcmp(sig, "/OCC")) {
		identifySignal();
		return 1;
	}
	else {
		identifySignal();
		return 0;
	}
}

//Close the I2C bus
//Takes:
//	Nothing
//Returns:
//	1 on success, 0 on failure
int iPortAI_close () {
	printf("Closing the I2C bus...\n");
	writeSerial("/c\r");
	getReply(100);
	if (!strcmp(sig, "/CCC")) {
		identifySignal();
		return 1;
	}
	else {
		identifySignal();
		return 0;
	}
}

//Master Tx
//Takes:
//	s: char * - character string (null terminated)
//Returns:
//	1 on success, 0 on failure
int iPortAI_MTx (char *s) {
	printf("Sending data to a slave...\n");
	writeSerial("/t");
	writeSerial(s);
	writeSerial("\r");
	getReply(1000);
	if (!strcmp(sig, "/MTC")) {
		identifySignal();
		printf("Transmit succeeded.\n");
		return 1;
	}
	else {
		identifySignal();
		printf("Transmit failed.\n");
		return 0;
	}
}

//Master Rx
//Takes:
//	c: int - number of bytes to read
//Returns:
//	1 on success, 0 on failure
int iPortAI_MRx (int c) {
	printf("Recieving data from a slave...\n");

	//Validate the number of bytes we're supposed to read
	if (c < 0 || c > MAX_DATA_SIZE - 4) {
		printf("Number of bytes to read is out of range.\n");
		return 0;
	}

	//Send the message
	char temp[10];
	sprintf(temp, "%d", c);
	writeSerial("/r");
	writeSerial(temp);
	writeSerial("\r");
	getReply(1000);
	if (!strcmp(sig, "/MRC")) {
		identifySignal();
		printf("Recieve succeeded.\n");
		return 1;
	}
	else {
		identifySignal();
		printf("Recieve failed.\n");
		return 0;
	}
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
