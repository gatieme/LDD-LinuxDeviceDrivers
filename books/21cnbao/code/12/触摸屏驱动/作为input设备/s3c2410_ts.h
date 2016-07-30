/* linux/include/asm/arch-s3c2410/s3c2410_ts.h
*
* Copyright (c) 2005 Arnaud Patard <arnaud.patard@rtp-net.org>
*
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
*
*  Changelog:
*     24-Mar-2005     RTP     Created file
*/

#ifndef __ASM_ARM_S3C2410_TS_H
#define __ASM_ARM_S3C2410_TS_H

struct s3c2410_ts_mach_info {
	int             delay;
	int             presc;
	int             oversampling_shift;
};

void __init set_s3c2410ts_info(struct s3c2410_ts_mach_info *hard_s3c2410ts_info);

#endif /* __ASM_ARM_S3C2410_TS_H */

