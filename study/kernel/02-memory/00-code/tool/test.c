#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
	int fd = open("1.bat", O_RDWR|O_CREAT|O_TRUNC, 0777);
	ftruncate(fd, 5000);

	int pagesize = sysconf(_SC_PAGE_SIZE);
	printf("pagesize = %d\n", pagesize);

	char *addr_mmap_p = mmap(NULL, pagesize, PROT_WRITE, MAP_PRIVATE, fd, 0);
	char *addr_mmap_s = mmap(NULL, pagesize, PROT_WRITE, MAP_SHARED, fd, 0);
	unsigned long *addr_malloc = malloc(sizeof(unsigned long));

	printf("addr_mmap_p: %p, addr_mmap_s: %p, addr_malloc: %p\n",
			addr_mmap_p, addr_mmap_s, addr_malloc);

	addr_mmap_p[pagesize - 1] = 0;
	addr_mmap_s[pagesize - 1] = 0;
	*addr_malloc = 0;
	char c = getchar();

	return 0;
}
