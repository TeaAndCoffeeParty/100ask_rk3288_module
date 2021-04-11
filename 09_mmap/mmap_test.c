#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

/*
 * ./hello_drv_test -w abc
 * ./hello_drv_test -r
 */

int main(int argc, char **argv)
{
	int fd;
	char *buf;
	char str[1024];
	int len;

	fd = open("/dev/hello", O_RDWR);
	if(fd == -1) {
		printf("can not open file /dev/hello\n");
		return -1;
	}

	buf = mmap(NULL, 1024*8, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if(buf == MAP_FAILED) {
		printf("mmap failed\n");
		return -1;
	}

	printf("mmap address = 0x%x\n", buf);
	printf("buf origin data = %s\n", buf);

	strcpy(buf, "hello");

	read(fd, str, 1024);
	if(strcmp(buf, str) == 0) {
		printf("compare ok!\n");
	} else {
		printf("compare err!\n");
		printf("str = %s!\n", str);
		printf("buf = %s!\n", buf);
	}

	while(1) {
		sleep(10);
	}

	munmap(buf, 1024*8);
	close(fd);
	return 0;
}
