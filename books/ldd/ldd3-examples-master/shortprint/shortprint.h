/*
 * Useful info describing the parallel port device.
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 */

/*
 * Register offsets
 */
#define SP_DATA    0x00
#define SP_STATUS  0x01
#define SP_CONTROL 0x02
#define SP_NPORTS     3

/*
 * Status register bits.
 */
#define SP_SR_BUSY 	0x80
#define SP_SR_ACK	0x40
#define SP_SR_PAPER	0x20
#define SP_SR_ONLINE	0x10
#define SP_SR_ERR	0x08

/*
 * Control register.
 */
#define SP_CR_IRQ	0x10
#define SP_CR_SELECT	0x08
#define SP_CR_INIT	0x04
#define SP_CR_AUTOLF	0x02
#define SP_CR_STROBE	0x01

/*
 * Minimum space before waking up a writer.
 */
#define SP_MIN_SPACE	PAGE_SIZE/2
