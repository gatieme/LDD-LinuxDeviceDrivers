#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#define LEN (10*4096)



int main(void)
{
  int fd;
  unsigned int *vadr;

  if ((fd=open("mapfile", O_RDWR))<0)
    {
      perror("open");
      exit(-1);
    }
  (char *)vadr = (char*)mmap(0, LEN, PROT_READ, MAP_SHARED, fd, 0);

  if (vadr == MAP_FAILED)
  {
          perror("mmap");
          exit(-1);
  }

  printf("%s\n",vadr);
  close(fd);
  return(0);
}


