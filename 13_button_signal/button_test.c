#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>

static int fd;
static void sig_func(int sig)
{
	int val;
	read(fd, &val, 4);
	printf("get button: 0x%x\n", val);
}


int main(int argc, char *argv[])
{
	int val;
	int flags;

	if(argc != 2) {
		printf("Usage: %d <dev>\n", argv[0]);
		return -1;
	}
	signal(SIGIO, sig_func);

	fd = open(argv[1], O_RDWR);
	if(fd<0) {
		printf("can not open file %s\n", argv[1]);
		return -1;
	}


	fcntl(fd, F_SETOWN, getpid());
	flags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, flags | FASYNC);

	while(1) {
		printf("hello world\n");
		sleep(2);
	}

	close(fd);

	return 0;
}
