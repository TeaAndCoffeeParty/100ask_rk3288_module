#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int fd;
	char val;

	if(argc != 2) {
		printf("Usage: %d <dev>\n", argv[0]);
		return -1;
	}

	fd = open(argv[1], O_RDWR);
	if(fd<0) {
		printf("can not open file %s\n", argv[1]);
		return -1;
	}

	read(fd, &val, 1);
	printf("get button : %d \n", val);

	close(fd);

	return 0;
}
