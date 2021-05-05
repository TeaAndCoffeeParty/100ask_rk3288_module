#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
	int fd;

	fd = open("/dev/pca9685", O_RDWR);
	if(fd < 0) {
		printf("can not open file\n");
		return -1;
	}


	close(fd);
	return 0;
}
