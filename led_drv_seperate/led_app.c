#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void showUsage(void)
{
	printf("app [dev_path] [on,off]\n");
}

int main(int argc, char *argv[])
{
	char status;
	if(argc < 3) {
		showUsage();
		return -1;
	}

	int fd = open(argv[1], O_RDWR);
	if(fd < 0) {
		printf("app open device failed path:%s", argv[1]);
		return -1;
	}

	if(0 == strcmp(argv[2], "on")) {
		status = 1;
		int ret = write(fd, &status, 1);
		if(ret <= 0) {
			printf("app write device fialed %s",argv[2]);
			return -1;
		} else {
			printf("app write device %x", status);
		}

	} else if(0 == strcmp(argv[2], "off")) {
		status = 0;
		int ret = write(fd, &status, 1);
		if(ret <= 0) {
			printf("app write device fialed %s",argv[2]);
			return -1;
		} else {
			printf("app write device %x", status);
		}
	}


	return 0;
}

