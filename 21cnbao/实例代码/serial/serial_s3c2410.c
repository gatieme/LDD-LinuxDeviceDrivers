/*
 * device for S3C2410 Development Board
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2002/05/14 02:19:42 $ 
 *
 * $Revision: 1.1.2.4 $

   Mon May 20 2002 Janghoon Lyu <nandy@mizi.com>
   - initial, based on linux/drivers/serial/serial_s3c2400.c

   Wed Aug 14 2002 Yong-iL Joh <tolkien@mizi.com>
   - adopt new irq scheme

 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/console.h>
#include <linux/serial_core.h>

#include <asm/irq.h>
#include <asm/hardware.h>
#include <asm/arch/cpu_s3c2410.h>

#define CONFIG_USE_ERR_IRQ	1	    

#define SERIAL_S3C2410_MAJOR	204
#define CALLOUT_S3C2410_MAJOR	205
#define MINOR_START		5

#define UART_NR				3
#define UART_ULCON(port)		__REG((port)->iobase + 0x00)
#define UART_UCON(port)			__REG((port)->iobase + 0x04)
#define UART_UFCON(port)		__REG((port)->iobase + 0x08)
#define UART_UTRSTAT(port)		__REG((port)->iobase + 0x10)
#define UART_UERSTAT(port)		__REG((port)->iobase + 0x14)
#define UART_UTXH(port)			__REG((port)->iobase + 0x20)
#define UART_URXH(port)			__REG((port)->iobase + 0x24)
#define UART_UBRDIV(port)		__REG((port)->iobase + 0x28)

#define ERR_IRQ(port)		((port)->irq + 2)
#define TX_IRQ(port)		((port)->irq + 1)
#define RX_IRQ(port)		((port)->irq)

static struct tty_driver normal, callout;
static struct tty_struct *s3c2410_table[UART_NR];
static struct termios *s3c2410_termios[UART_NR], *s3c2410_termios_locked[UART_NR];

static void s3c2410uart_stop_tx(struct uart_port *port, u_int from_tty) {
	disable_irq(TX_IRQ(port));
}

static void s3c2410uart_start_tx(struct uart_port *port, u_int nonempty,
				 u_int from_tty) {
	enable_irq(TX_IRQ(port));
}

static void s3c2410uart_stop_rx(struct uart_port *port) {
	disable_irq(RX_IRQ(port));
}

static void s3c2410uart_enable_ms(struct uart_port *port)
{
}

static u_int s3c2410uart_tx_empty(struct uart_port *port)
{
    return (UART_UTRSTAT(port) & UTRSTAT_TR_EMP ? 0 : TIOCSER_TEMT);
}

static u_int s3c2410uart_get_mctrl(struct uart_port *port)
{
    return (TIOCM_CTS | TIOCM_DSR | TIOCM_CAR);
}

static void s3c2410uart_set_mctrl(struct uart_port *port, u_int mctrl)
{
}

static void s3c2410uart_break_ctl(struct uart_port *port, int break_state)
{
    u_int ucon;

    ucon = UART_UCON(port);

    if (break_state == -1)
	ucon |= UCON_BRK_SIG;
    else
	ucon &= ~UCON_BRK_SIG;

    UART_UCON(port) = ucon;
}

static void s3c2410uart_rx_interrupt(int irq, void *dev_id,
				     struct pt_regs *regs) {
    struct uart_info *info = dev_id;
    struct tty_struct *tty = info->tty;
    unsigned int status, ch, max_count = 256;
    struct uart_port *port = info->port;

    status = UART_UTRSTAT(port);
    while ((status & UTRSTAT_RX_RDY) && max_count--) {
      if (tty->flip.count >= TTY_FLIPBUF_SIZE) {
	tty->flip.tqueue.routine((void *) tty);
	if (tty->flip.count >= TTY_FLIPBUF_SIZE) {
	  printk(KERN_WARNING "TTY_DONT_FLIP set\n");
	  return;
	}
      }

      ch = UART_URXH(port);

      *tty->flip.char_buf_ptr = ch;
      *tty->flip.flag_buf_ptr = TTY_NORMAL;
      port->icount.rx++;
      tty->flip.flag_buf_ptr++;
      tty->flip.char_buf_ptr++;
      tty->flip.count++;
      /* No error handling just yet.
       * On the MX1 these are seperate
       * IRQs, so we need to deal with
       * the sanity of 5 IRQs for one
       * serial port before we deal
       * with the error path properly.
       */
      status = UART_UTRSTAT(port);
    }
    tty_flip_buffer_push(tty);
    return;
}

static void s3c2410uart_tx_interrupt(int irq, void *dev_id,
				     struct pt_regs *reg) {
    struct uart_info *info = dev_id;
    struct uart_port *port = info->port;
    int count;

    if (port->x_char) {
	UART_UTXH(port) = port->x_char;
	port->icount.tx++;
	port->x_char = 0;
	return;
    }

    if (info->xmit.head == info->xmit.tail
        || info->tty->stopped || info->tty->hw_stopped) {
      s3c2410uart_stop_tx(info->port, 0);
      return;
    }

    count = port->fifosize >> 1;
    do {
	UART_UTXH(port) = info->xmit.buf[info->xmit.tail];
	info->xmit.tail = (info->xmit.tail + 1) & (UART_XMIT_SIZE - 1);
	port->icount.tx++;
	if (info->xmit.head == info->xmit.tail)
	    break;
    } while (--count > 0);

    if (CIRC_CNT(info->xmit.head, info->xmit.tail,
		 UART_XMIT_SIZE) < WAKEUP_CHARS)
	uart_event(info, EVT_WRITE_WAKEUP);

    if (info->xmit.head == info->xmit.tail)
	s3c2410uart_stop_tx(info->port, 0);

}

#ifdef CONFIG_USE_ERR_IRQ
static void s3c2410uart_err_interrupt(int irq, void *dev_id,
				      struct pt_regs *reg) {
    struct uart_info *info = dev_id;
    struct uart_port *port = info->port;
    struct tty_struct *tty = info->tty;
    unsigned char err = UART_UERSTAT(port) & UART_ERR_MASK;
    unsigned int ch, flg;

    ch = UART_URXH(port);
    if (!(err & (UERSTAT_BRK | UERSTAT_FRAME |
		 UERSTAT_PARITY | UERSTAT_OVERRUN)))
      return;

    if (err & UERSTAT_BRK)
      port->icount.brk++;
    if (err & UERSTAT_FRAME)
      port->icount.frame++;
    if (err & UERSTAT_PARITY)
      port->icount.parity++;
    if (err & UERSTAT_OVERRUN)
      port->icount.overrun++;

    err &= port->read_status_mask;

    if (err & UERSTAT_PARITY)
      flg = TTY_PARITY;
    else if (err & UERSTAT_FRAME)
      flg = TTY_FRAME;
    else
      flg = TTY_NORMAL;

    if (err & UERSTAT_OVERRUN) {
      *tty->flip.char_buf_ptr = ch;
      *tty->flip.flag_buf_ptr = flg;
      tty->flip.flag_buf_ptr++;
      tty->flip.char_buf_ptr++;
      tty->flip.count++;
      if (tty->flip.count < TTY_FLIPBUF_SIZE) {
	ch = 0;
	flg = TTY_OVERRUN;
      }
    }
     
    *tty->flip.flag_buf_ptr++ = flg;
    *tty->flip.char_buf_ptr++ = ch;
    tty->flip.count++;
}
#endif

static int s3c2410uart_startup(struct uart_port *port, struct uart_info *info)
{
    int ret, flags;
    u_int ucon;

    ret = request_irq(RX_IRQ(port), s3c2410uart_rx_interrupt, SA_INTERRUPT,
		      "serial_s3c2410_rx", info);
    if (ret) goto rx_failed;
    ret = request_irq(TX_IRQ(port), s3c2410uart_tx_interrupt, SA_INTERRUPT,
		      "serial_s3c2410_tx", info);
    if (ret) goto tx_failed;
#ifdef CONFIG_USE_ERR_IRQ
    ret = request_irq(ERR_IRQ(port), s3c2410uart_err_interrupt, SA_INTERRUPT,
		      "serial_s3c2410_err", info);
    if (ret) goto err_failed;
#endif

    ucon = (UCON_TX_INT_LVL | UCON_RX_INT_LVL |
	    UCON_TX_INT | UCON_RX_INT | UCON_RX_TIMEOUT);

#if defined(CONFIG_IRDA) || defined(CONFIG_IRDA_MODULE)
      ULCON2 |= ULCON_IR | ULCON_PAR_NONE | ULCON_WL8 | ULCON_ONE_STOP;	
#endif	
    save_flags(flags);
    cli();
    
    UART_UCON(port) = ucon;
    
    sti();
    restore_flags(flags);

    return 0;

#ifdef CONFIG_USE_ERR_IRQ
 err_failed:
    free_irq(TX_IRQ(port), info);
#endif
 tx_failed:
    free_irq(RX_IRQ(port), info);
 rx_failed:
    return ret;
}

static void s3c2410uart_shutdown(struct uart_port *port, struct uart_info *info)
{
    free_irq(RX_IRQ(port), info);
    free_irq(TX_IRQ(port), info);
#ifdef CONFIG_USE_ERR_IRQ
    free_irq(ERR_IRQ(port), info);
#endif
    UART_UCON(port) = 0x0;
}

static void s3c2410uart_change_speed(struct uart_port *port, u_int cflag, u_int iflag, u_int quot)
{
    u_int ulcon, ufcon;
    int flags;

    ufcon = UART_UFCON(port);

    switch (cflag & CSIZE) {
	case CS5:	ulcon = ULCON_WL5; break;
	case CS6:	ulcon = ULCON_WL6; break;
	case CS7:	ulcon = ULCON_WL7; break;
	default:	ulcon = ULCON_WL8; break;
    }

    if (cflag & CSTOPB)
	ulcon |= ULCON_STOP;
    if (cflag & PARENB) {
	if (!(cflag & PARODD))
	    ulcon |= ULCON_PAR_EVEN;
    }
    
    if (port->fifosize > 1)
	ufcon |= UFCON_FIFO_EN;
    
    port->read_status_mask =  UERSTAT_OVERRUN;
    if (iflag & INPCK)
	port->read_status_mask |= UERSTAT_PARITY | UERSTAT_FRAME;

    port->ignore_status_mask = 0;
    if (iflag & IGNPAR)
	port->ignore_status_mask |= UERSTAT_FRAME | UERSTAT_PARITY;
    if (iflag & IGNBRK) {
	if (iflag & IGNPAR)
	    port->ignore_status_mask |= UERSTAT_OVERRUN;
    }

    quot -= 1;

    save_flags(flags);
    cli();

    UART_UFCON(port) = ufcon;
    UART_ULCON(port) = (UART_ULCON(port) & ~(ULCON_PAR | ULCON_WL)) | ulcon;
    UART_UBRDIV(port) = quot;

    sti();
    restore_flags(flags);
}

static const char *s3c2410uart_type(struct uart_port *port)
{
    return port->type == PORT_S3C2410 ? "S3C2410" : NULL;
}

static void s3c2410uart_config_port(struct uart_port *port, int flags)
{
    if (flags & UART_CONFIG_TYPE)
	port->type = PORT_S3C2410;
}

static void s3c2410uart_release_port(struct uart_port *port)
{
}

static int s3c2410uart_request_port(struct uart_port *port)
{
	return 0;
}

static struct uart_ops s3c2410_pops = {
        tx_empty:		s3c2410uart_tx_empty,
	set_mctrl:		s3c2410uart_set_mctrl,
	get_mctrl:		s3c2410uart_get_mctrl,
	stop_tx:		s3c2410uart_stop_tx,
	start_tx:		s3c2410uart_start_tx,
	stop_rx:		s3c2410uart_stop_rx,
	enable_ms:		s3c2410uart_enable_ms,
	break_ctl:		s3c2410uart_break_ctl,
	startup:		s3c2410uart_startup,
	shutdown:		s3c2410uart_shutdown,
	change_speed:		s3c2410uart_change_speed,
	type:			s3c2410uart_type,
	config_port:		s3c2410uart_config_port,
	release_port:		s3c2410uart_release_port,
	request_port:		s3c2410uart_request_port,
};

static struct uart_port	s3c2410_ports[UART_NR] = {
  {
	iobase:		(unsigned long)(UART0_CTL_BASE),
	iotype:		SERIAL_IO_PORT,
	irq:		IRQ_RXD0,
	uartclk:	130252800,
	fifosize:	16,
	ops:		&s3c2410_pops,
	type:		PORT_S3C2410,
	flags:		ASYNC_BOOT_AUTOCONF,
  }, {
	iobase:		(unsigned long)(UART1_CTL_BASE),
	iotype:		SERIAL_IO_PORT,
	irq:		IRQ_RXD1,
	uartclk:	130252800,
	fifosize:	16,
	ops:		&s3c2410_pops,
	type:		PORT_S3C2410,
	flags:		ASYNC_BOOT_AUTOCONF,
  }, {
	iobase:		(unsigned long)(UART2_CTL_BASE),
	iotype:		SERIAL_IO_PORT,
	irq:		IRQ_RXD2,
	uartclk:	130252800,
	fifosize:	16,
	ops:		&s3c2410_pops,
	type:		PORT_S3C2410,
	flags:		ASYNC_BOOT_AUTOCONF,
  }
};

void __init s3c2410_register_uart(int idx, int port)
{
        if (idx >= UART_NR) {
                printk(KERN_ERR "%s: bad index number %d\n", __FUNCTION__, idx);
                return;
        }
	s3c2410_ports[idx].uartclk = s3c2410_get_bus_clk(GET_PCLK);

        switch (port) {
        case 0:
	  s3c2410_ports[idx].iobase = (unsigned long)(UART0_CTL_BASE);
	  s3c2410_ports[idx].irq  = IRQ_RXD0;
	  CLKCON |= CLKCON_UART0;
	  break;
        case 1:
	  s3c2410_ports[idx].iobase = (unsigned long)(UART1_CTL_BASE);
	  s3c2410_ports[idx].irq  = IRQ_RXD1;
	  CLKCON |= CLKCON_UART1;
	  break;
        case 2:
	  s3c2410_ports[idx].iobase = (unsigned long)(UART2_CTL_BASE);
	  s3c2410_ports[idx].irq  = IRQ_RXD2;
	  CLKCON |= CLKCON_UART2;
	  break;
        default:
	  printk(KERN_ERR "%s : bad port number %d\n", __FUNCTION__, port);
        }
}


#ifdef CONFIG_SERIAL_S3C2410_CONSOLE

static void s3c2410_console_write(struct console *co, const char *s, u_int count)
{
    int i;
    struct uart_port *port = s3c2410_ports + co->index;

    for (i = 0; i < count; i++) {
	while (!(UART_UTRSTAT(port) & UTRSTAT_TX_EMP));
	UART_UTXH(port) = s[i];
	if (s[i] == '\n') {
	    while (!(UART_UTRSTAT(port) & UTRSTAT_TX_EMP));
	    	UART_UTXH(port) = '\r';
	}
    }
}

static int s3c2410_console_wait_key(struct console *co)
{
    int c;
    struct uart_port *port = s3c2410_ports + co->index;

    while (!(UART_UTRSTAT(port) & UTRSTAT_RX_RDY));
    	c = UART_URXH(port);
    return c;
}

static kdev_t s3c2410_console_device(struct console *co)
{
	return MKDEV(SERIAL_S3C2410_MAJOR, MINOR_START + co->index);
}

static int __init s3c2410_console_setup(struct console *co, char *options)
{
	struct uart_port *port;
	int baud = 115200;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';

	port = uart_get_console(s3c2410_ports, UART_NR, co);

	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);

	return uart_set_options(port, co, baud, parity, bits, flow);
}

static struct console s3c2410_cons = {
	name:		"ttyS",
	write:		s3c2410_console_write,
	device:		s3c2410_console_device,
	wait_key:	s3c2410_console_wait_key,
	setup:		s3c2410_console_setup,
	flags:		CON_PRINTBUFFER,
	index:		-1,
};

void __init s3c2410_console_init(void)
{
	register_console(&s3c2410_cons);
}
#define S3C2410_CONSOLE		&s3c2410_cons
#else	/* CONFIG_SERIAL_S3C2410_CONSOLE */
#define S3C2410_CONSOLE		NULL
#endif	/* CONFIG_SERIAL_S3C2410_CONSOLE */

static struct uart_driver s3c2410_reg = {
	owner:		THIS_MODULE,
	normal_major:	SERIAL_S3C2410_MAJOR,
#ifdef CONFIG_DEVFS_FS
	normal_name:	"ttyS%d",
	callout_name:	"cua%d",
#else
	normal_name:	"ttyS",
	callout_name:	"cua",
#endif
	normal_driver:	&normal,
	callout_major:	CALLOUT_S3C2410_MAJOR,
	callout_driver:	&callout,
	table:		s3c2410_table,
	termios:	s3c2410_termios,
	termios_locked:	s3c2410_termios_locked,
	minor:		MINOR_START,
	nr:		UART_NR,
	port:		s3c2410_ports,
	cons:		S3C2410_CONSOLE,
};

static int __init s3c2410uart_init(void)
{
	return uart_register_driver(&s3c2410_reg);
}

static void __exit s3c2410uart_exit(void)
{
	uart_unregister_driver(&s3c2410_reg);
}

module_init(s3c2410uart_init);
module_exit(s3c2410uart_exit);

EXPORT_NO_SYMBOLS;

MODULE_AUTHOR("MIZI Research Inc");
MODULE_DESCRIPTION("S3C2410 generic serial port driver");
MODULE_LICENSE("GPL");
