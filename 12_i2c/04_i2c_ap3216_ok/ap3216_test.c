#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char *argv[])
{
	int fd;
	char buf[6];
	int len;
	int i;

	fd = open("/dev/ap3216", O_RDWR);
	if(fd < 0) {
		printf("can not open file /dev/ap3216\n");
		return -1;
	}

	len = read(fd, buf, sizeof(buf));
	printf("App read :");
	for(i=0;i<len;i++) {
		printf("%02x ", buf[i]);
	}
	printf("\n");

	close(fd);

	return 0;
}
