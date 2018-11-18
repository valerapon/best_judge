#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

int main(int argc, char **argv) {
	char *path = argv[1], tmp1 = 0, tmp2 = 0;
	int fans = open(path, O_RDONLY, S_IRUSR | S_IWUSR);
	if (fans < 0) {
		write(1, "x", 1);
		return 0;
	}
	while (1) {
		if (read(fans, &tmp1, 1) < 0) {
			write(1, "x", 1);
			return 0;
		}
		if (tmp1 >= '0' && tmp1 <= '9') {
			break;
		}
	}
	while (1) {
		if (read(0, &tmp2, 1) < 0) {
			write(1, "x", 1);
			return 0;
		}
		if (tmp2 >= '0' && tmp2 <= '9') {
			break;
		}
	}

	while (1) {
		if (tmp1 != tmp2) {
			write(1, "-", 1);
			return 0;
		}
		if (read(0, &tmp2, 1) <= 0) {
			break;
		}
		if (read(fans, &tmp1, 1) <= 0) {
			break;
		}
	}
	if (tmp1 >= '0' && tmp1 <= '9' && !(tmp2 >= '0' && tmp2 <= '9')) {
		write(1, "-", 1);
		return 0;
	}
	if (tmp2 >= '0' && tmp2 <= '9' && !(tmp1 >= '0' && tmp1 <= '9')) {
		write(1, "-", 1);
		return 0;
	}
	write(1, "+", 1);
	return 0;
}
