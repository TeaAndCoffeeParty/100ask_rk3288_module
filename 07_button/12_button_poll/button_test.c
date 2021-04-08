#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>


int main(int argc, char *argv[])
{
	int fd;
	int val;
	struct pollfd fds[1];
	int timeout_ms = 1000;

	if(argc != 2) {
		printf("Usage: %d <dev>\n", argv[0]);
		return -1;
	}

	fd = open(argv[1], O_RDWR);
	if(fd<0) {
		printf("can not open file %s\n", argv[1]);
		return -1;
	}

	fds[0].fd = fd;
	fds[0].events = POLLIN;

	while(1) {
		if((poll(fds, 1, timeout_ms) == 1) && (fds[0].revents & POLLIN)) {
			read(fd, &val, 4);
			printf("get button : %x \n", val);
		} else {
			printf("timeout\n");
		}
	}

	close(fd);

	return 0;
}
