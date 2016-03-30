#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

extern void fb_display(unsigned char *, int, int, int, int, int, int);
extern int openFB(const char *name);
extern void closeFB(int fh);
extern char *dev;

#define X_BUFFER	(300)
#define Y_BUFFER	(200)

int main(void)
{
    float i,j;
    unsigned char *buffer;
    buffer = (unsigned char *) malloc(X_BUFFER * Y_BUFFER * 3 * sizeof(unsigned char));
    for(j=0.0; j<Y_BUFFER; j++) for(i=0.0; i<X_BUFFER; i++){
	buffer[(int)((i+j*X_BUFFER)*3)] = (unsigned char)((float)i * (float)256.0 / (float)X_BUFFER);
	buffer[(int)((i+j*X_BUFFER)*3+1)] = (unsigned char)((float)(X_BUFFER-i) * (float)256.0 / (float)X_BUFFER);
	buffer[(int)((i+j*X_BUFFER)*3+2)] = (unsigned char)((float)j * (float)256.0 / (float)Y_BUFFER);
    }
    fb_display(buffer, X_BUFFER, Y_BUFFER, 0, 0, 10, 10);
    free(buffer);
    return 0;
}
