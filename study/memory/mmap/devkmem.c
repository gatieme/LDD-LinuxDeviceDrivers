#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#define DEVKMEM         "/dev/kmem"

#define PAGE_SIZE       0x1000
#define PAGE_MASK       (~(PAGE_SIZE-1))


int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        printf("usage...");
        exit(-1);
    }
    int fd;
    char *mbase;
    char read_buf[10];
    unsigned int regAddr;
    unsigned int varAddr;

    varAddr = strtoul(argv[1], 0, 16);

    unsigned int ptr = varAddr & ~(PAGE_MASK);

    fd = open(DEVKMEM, O_RDONLY);
    if (fd == -1)
    {
        printf("%s %d", __func__, __LINE__);
        perror("open");
        exit(-1);
    }

    mbase = (char *)mmap(0, PAGE_SIZE, PROT_READ, MAP_SHARED,fd, (varAddr & PAGE_MASK));
    if (mbase == MAP_FAILED)
    {
        printf("map failed %s\n",strerror(errno));
    }

    printf("varAddr = 0x%X \n", varAddr);
    printf("mapbase = 0x%X \n", (unsigned int)mbase);
    printf("value   = 0x%X \n",*(unsigned int*)(mbase+ptr));
    printf("char    = %c%c%c%c \n",
    				*(char *)(mbase+ptr), *(char *)(mbase+ptr+1),
    				*(char *)(mbase+ptr+2), *(char *)(mbase+ptr+3));

    close(fd);
    munmap(mbase,PAGE_SIZE);

    return 0;
}
