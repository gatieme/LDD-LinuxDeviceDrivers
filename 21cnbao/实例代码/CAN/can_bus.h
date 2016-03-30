#ifndef	_CAN_BUS_H_
#define	_CAN_BUS_H_

#define CAN_BUS_Intel_MODE      0x400000
#define CAN_BUS_Motorola_MODE   0xffbf
#define CAN_BUS_MODE CAN_BUS_Intel_MODE

#define CAN_BUS_INT_ENABLE 0x08

//#define	PeliCANMode

extern	int ADDR_SJA1000_ADDR;
extern  int ADDR_SJA1000_DATA;

//********************************************
#define ControlReg	0

//********************************************

#define ModeControlReg	10

#define RM_RR_Bit	0x01
#ifdef	PeliCANMode
#define	LOM_Bit		0x02
#define	STM_Bit		0x04
#define	AFM_Bit		0x08
#define	SM_Bit		0x10
#endif

//*********************************************

#ifdef	PeliCANMode

#define	InterruptEnReg	4

#define	RIE_Bit		0x01
#define	TIE_Bit		0x02
#define	EIE_Bit		0x04
#define	DOIE_Bit	0x08
#define	WUIE_Bit	0x10
#define	EPIE_Bit	0x20
#define	ALIE_Bit	0x40
#define	BEIE_Bit	0x80

#else	/* BasicCAN Mode */


	
#define	InterruptEnReg	0

#define	RIE_Bit		0x02
#define TIE_Bit		0x04
#define	EIE_Bit		0x08
#define DOIE_Bit	0x10

#endif

//**********************************************

#define	CommandReg	1

#define	TR_Bit		0x01
#define	AT_Bit		0x02
#define	RRB_Bit		0x04
#define	CDO_Bit		0x08
#ifdef	PeliCANMode
#define SRR_Bit		0x10
#else	/* BasicCAN Mode */
#define	GTS_Bit 	0x10
#endif

//**********************************************

#define	StatusReg	2

#define	RBS_Bit		0x01
#define	DOS_Bit		0x02
#define	TBS_Bit		0x04
#define	TCS_Bit		0x08
#define	RS_Bit		0x10
#define	TS_Bit		0x20
#define	ES_Bit		0x40
#define	BS_Bit		0x80

//**********************************************

#define	InterruptReg	3

#define	RI_Bit		0x01
#define	TI_Bit		0x02
#define	EI_Bit		0x04
#define	DOI_Bit		0x08
#define	WUI_Bit		0x10
#ifdef	PeliCANMode
#define	EPI_Bit		0x20
#define	ALI_Bit		0x40
#define	BEI_Bit		0x80
#endif

//**********************************************

#define	BusTiming0Reg	6
#define	BusTiming1Reg	7

#define	SAM_Bit		0x80

//**********************************************

#define	OutControlReg	8

#define	BiPhaseMode	0x00
#define	NormalMode	0x02
#define	ClkOutMode	0x03
	
#define	OCPOL1_Bit	0x20
#define	Tx1Float	0x00
#define	Tx1PullDn	0x40
#define	Tx1PullUp	0x80
#define	Tx1PshPull	0xc0

#define	OCPOL0_Bit	0x04
#define	Tx0Float	0x00
#define	Tx0PullDn	0x08
#define	Tx0PullUp	0x10
#define	Tx0PshPull	0x18

//**********************************************

#ifdef	PeliCANMode
#define	AcceptCode0Reg	16
#define	AcceptCode1Reg	17
#define AcceptCode2Reg	18
#define AcceptCode3Reg	19

#define AccepMask0Reg	20
#define AccepMask1Reg	21
#define AccepMask2Reg	22
#define AccepMask3Reg	23
#else	/* BasicCAN Mode */
#define	AcceptCodeReg	4
#define	AcceptMaskReg	5
#endif

//**********************************************

#ifdef	PeliCANMode
#define	RxFramlnFo	16
#define	RxBuffer1	17
#define	RxBuffer2	18
#define RxBuffer3       19
#define RxBuffer4       20
#define RxBuffer5       21
#define RxBuffer6       22
#define RxBuffer7       23
#define RxBuffer8       24
#define RxBuffer9       25
#define RxBuffer10      26
#define RxBuffer11      27
#define RxBuffer12      28
#else   /* BasicCAN Mode */
#define RxBuffer1       20
#define RxBuffer2       21
#define RxBuffer3       22
#define RxBuffer4       23
#define RxBuffer5       24
#define RxBuffer6       25
#define RxBuffer7       26
#define RxBuffer8       27
#define RxBuffer9       28
#define RxBuffer10      29
#endif

//*******************************************

#ifdef	PeliCANMode
#define TxFramlnFo      16
#define TxBuffer1       17
#define TxBuffer2       18
#define TxBuffer3       19
#define TxBuffer4       20
#define TxBuffer5       21
#define TxBuffer6       22
#define TxBuffer7       23
#define TxBuffer8       24
#define TxBuffer9       25
#define TxBuffer10      26
#define TxBuffer11      27
#define TxBuffer12      28

#define	TxFraminFoRd	96
#define	TxBufferRd1	97
#define	TxBufferRd2     98
#define TxBufferRd3     99
#define TxBufferRd4     100
#define TxBufferRd5	101
#define TxBufferRd6	102
#define TxBufferRd7	103
#define TxBufferRd8     104
#define TxBufferRd9     105
#define TxBufferRd10    106
#define TxBufferRd11    107
#define TxBufferRd12    108
#else   /* BasicCAN Mode */
#define TxBuffer1       10
#define TxBuffer2       11
#define TxBuffer3       12
#define TxBuffer4       13
#define TxBuffer5       14
#define TxBuffer6       15
#define TxBuffer7       16
#define TxBuffer8       17
#define TxBuffer9       18
#define TxBuffer10      19
#endif

//********************************************

#ifdef	PeliCANMode
#define	ArbLostCapReg	11
#define	ErrCodeCapReg	12
#define	ErrWamLimitReg	13
#define	RxErrCountReg	14
#define	TxErrCountReg	15
#define	RxMsgCountReg	29
#define	RxBufstartAdr	30
#endif
//********************************************

#define	ClockDivideReg	31
#define	DivBy1		0x07
#define	DivBy2		0x00

#define	CLKOff_Bit	0x08
#define	RXINTEN_Bit	0x20
#define CBP_Bit		0x40
#define CANMode_Bit	0x80

#define	ClrByte		0x00
#define	DontCare	0xff
#define	ClrIntEnSJA	ClrByte

//********************************************

#define SJW_MB_24	0x00
#define	Prec_MB_24	0x00

#define	TSEG2_MB_24	0x10
#define	TSEG1_MB_24	0x08

void write_can_reg(char value, int reg)
{
	outb(reg, ADDR_SJA1000_ADDR);
	udelay(5);	
	outb(value, ADDR_SJA1000_DATA);	
}

char  read_can_reg(int reg)
{
	outb(reg, ADDR_SJA1000_ADDR);
	udelay(5);
	return inb(ADDR_SJA1000_DATA);
}


#endif





















































