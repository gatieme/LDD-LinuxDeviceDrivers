#include <linux/fb.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <string.h>
#include <errno.h>

#define DEFAULT_FRAMEBUFFER     "/dev/fb0"
