#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char **argv) {
	char *path = argv[1], tmp1 = 0, tmp2 = 0;
	int fans = open(path, O_RDONLY, S_IRUSR | S_IWUSR);
	if (fans < 0) {
		write(1, "x", 1);
		return 0;
	}
	while (read(fans, &tmp1, 1) > 0) {
		if (tmp1 == 10) { // каретка, которая useless
			continue;
		}
		if (read(0, &tmp2, 1) <= 0) {
			write(1, "-", 1);
			return 0;
		}
		if (tmp1 != tmp2) {
			write(1, "-", 1);
			return 0;
		}
	}
	if (read(0, &tmp2, 1) > 0) {
		write(1, "-", 1);
		return 0;
	}
	write(1, "+", 1);
	return 0;
}
