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
	while (read(fans, &tmp1, 1) > 0) {
		if (tmp1 == 10) { // каретка, которая useless
			continue;
		}
		if (read(0, &tmp2, 1) <= 0) {
			write(1, "-", 1);
			return 1;
		}
		if (tmp1 != tmp2) {
			write(1, "-", 1);
			return 2;
		}
	}
	if (read(0, &tmp2, 1) > 0) {
		write(1, "-", 1);
		return 3;
	}
	write(1, "+", 1);
	return 0;
}
