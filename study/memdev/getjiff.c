
/*
* getjiff.c
*
* this toolkit shows how to get jiffies value from user space:
* 1. find jiffies's address from kernel image.
* 2. access virtual address space to get jiffies value.
* 3. access physical address sapce to get jiffies value.
*
* demostrate following techniques:
* o get ELF object symbol address by calling nlist()
* o access virtual memory space from /dev/kmem
* o access virtual memory space from /dev/mem
*/

#include <stdio.h>
#include <stdlib.h>         //exit
#include <linux/a.out.h>    //nlist
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <memory.h>

#define LONG *(volatile unsigned long*)

/* read from virtual memory */
int read_kmem(off_t offset, void* buf, size_t count)
{
    int fd;
    int n;

    fd = open("/dev/kmem", O_RDONLY);
    if (fd < 0)
    {
        perror("open /dev/kmem failed");
        return -1;
    }

    lseek(fd, offset, SEEK_SET);
    n = read(fd, buf, count);
    if (n != count)
        perror("/dev/kmem read failed");
    else
        printf("/dev/kmem read buf = %ld\n", *(unsigned long *)buf);

    close(fd);
    return n;
}

/* read from physical memory */
int read_mem(off_t offset, void* buf, size_t count)
{
    int fd;
    int n;
    int page_size;
    void *map_base;
    unsigned long value;

    printf("/dev/mem: the offset is %lx\n", offset);

    fd = open("/dev/mem", O_RDONLY);
    if (fd < 0)
    {
        perror("open /dev/mem failed");
        return -1;
    }

    if(1){
        page_size = getpagesize();
        printf("the page size = %d\n", page_size);
        map_base = mmap(0,page_size,PROT_READ,MAP_SHARED,fd,offset);
        if (map_base == MAP_FAILED){
            perror("mmap");
            exit(1);
        }
        value = LONG(map_base);
        printf("/dev/mem: the value is %ld\n", value);
        buf = (unsigned long *)map_base;
    }

    if(0){
        lseek(fd, offset, SEEK_SET);
        n = read(fd, buf, count);
        if (n != count)
            perror("/dev/mem read failed");
        else
            printf("/dev/mem read buf = %ld\n", *(unsigned long *)buf);
    }

    close(fd);
    return n;
}

int main(int argc, char **argv)
{
    FILE *fp;
    char addr_str[11]="0x";
    char var[51];
    unsigned long addr;
    unsigned long jiffies;
    char ch;
    int r;

    if (argc != 2) {
            fprintf(stderr,"usage: %s System.map\n",argv[0]);
            exit(-1);
    }

    if ((fp = fopen(argv[1],"r")) == NULL) {
            perror("fopen");
            exit(-1);
    }

    do {
            r = fscanf(fp,"%8s %c %50s\n",&addr_str[2],&ch,var);    // format of System.map
            if (strcmp(var,"jiffies")==0)
                    break;
    } while(r > 0);
    if (r < 0) {
            printf("could not find jiffies\n");
            exit(-1);
    }

    addr = strtoul(addr_str,NULL,16);                               //Convert string to unsigned long integer
    printf("found jiffies at (%s) %08lx\n",addr_str,addr);

    read_kmem(addr, &jiffies, sizeof(jiffies));
    printf("jiffies=%ld (read from virtual memory)\n\n", jiffies);

    jiffies = 0;                                                    //reinit for checking read_mem() below

    read_mem(addr-0xC0000000, &jiffies, sizeof(jiffies));
    printf("jiffies=%ld (read from physical memory)\n", jiffies);

    return 0;
}
