
  fb_display()

Simple, cruel library function to display pictures on a framebuffer device.


SYNOPSIS:

extern void fb_display(unsigned char *rgbbuff,
	int x_size, int y_size,
	int x_pan, int y_pan,
	int x_offs, int y_offs);

extern void getCurrentRes(int *x,int *y);

#define DEFAULT_FRAMEBUFFER	"/dev/..."
or set FRAMEBUFFER="/dev/..." environment variable (takes precedense)


fb_display recives:

rgbbuff	- (Red Green Blue) triples in an unsigned char[] table,
x_size	- x picture size,
y_size	- y picture size,
x_pan	- x panning in picture,
y_pan	- y panning in picture,
x_offs	- x offset on screen,
y_offs	- y offset on screen.


getCurrentRes sets:

*x	- x screen size,
*y	- y screen size.



It uses some code from fbset utility, which I think is GPL'ed,
so this code has to be GPL'ed too - and it is ;)
And one more - if You use it, You have to mention You used it. ;)

                    smoku / Stellar Technologies '2000
                    <smoku@linux-pl.com>, http://s-tech.linux-pl.com