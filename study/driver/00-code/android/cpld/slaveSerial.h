/*
create 2013.1.31byTX

*/
#define CPLD_SIZE 256
#define CPLD_BASE 0x80000000

#define REG_TEST_OFFSET 0

#define REG_CCLK_OFFSET 0x2
#define REG_PROG_B_OFFSET 0x4
#define REG_DIN_OFFSET 0x6
#define REG_STAT_OFFSET 0x8
#define STAT_DONE_BIT 0x2
#define STAT_INIT_BIT 0x1
#define REG_LED_OFFSET 0xa
#define REG_CONTROL_OFFSET 0xc
#define REG_DATA_OFFSET 0x20

#define REG_FPGA_BURST_OFFSET 0x20
#define FPGA_BURST_SIZE 0x10 //words

#define INFOR_DONE_SUCCEED 0
#define INFOR_DONE_FAILED 1
#define INFOR_INIT_SUCCEED 1	//new,diff as done
#define INFOR_INIT_FAILED 0	//new

#define DEBUG_PRINT 0

#define CMD_START_PROG 0x10
#define CMD_START_SHIFT 0x11
#define CMD_CHECK_DONE 0x12
#define CMD_SET_LED 0x13
#define CMD_SEND_HIGH_BYTE 0x14 //new
#define CMD_SEND_LOW_BYTE 0x15	//new
#define CMD_CHECK_INIT 0x16
#define CMD_SET_CONTROL 0x17
#define CMD_SET_DATA 0x18
#define SET_LED_ON 0
#define SET_LED_OFF 1
void set_cclk(void *io_base,int value);
void set_din(void *io_base,int value);
void set_prog_b(void *io_base,int value);
unsigned short int get_stat(void *io_base);
int get_done_bit(void *io_base);
int get_init_bit(void *io_base);
int check_done(void *io_base);
void shiftWordOut(unsigned short int data, void *io_base);
void shiftByteOut(char data, void *io_base);

