#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

int set_config(char *way_to_test, int *test_count, int *check_style) {
	char *path = malloc((strlen(way_to_test) + 20) * sizeof(char));
	sprintf(path, "%s/problem.cfg\0", way_to_test);
	int fconfig = open(path, O_RDONLY, S_IRUSR | S_IWUSR);
	char number[4], tmp;
	for (int i = 0; i < 6; i++) {
		read(fconfig, &tmp, 1);
	}
	for (int i = 0; i < 4; i++) {
		read(fconfig, number + i, 1);
	}
	*test_count = atoi(number);
	*check_style = 0;
	free(path);
	close(fconfig);
	return 0;
}

int test_user_problem(char *user_program, char *way_to_test, int test_count) {
	if (fork() > 0) {
		wait(NULL);
	}
	else {
		execlp("gcc", "gcc", user_program, "-o", "var/prog", NULL);
		exit(0);
	}
	if (fork() > 0) {
		wait(NULL);
	}
	else {
		execlp("gcc", "gcc", "checkers/checker_byte.c", "-o", "var/checker_byte", NULL);
		exit(0);
	}
	char *path = malloc((strlen(way_to_test) + 10) * sizeof(char));
	for (int i = 1; i <= test_count; i++) {
		int fd_1[2], fd_2[2];
		pipe(fd_1);
		if (fork() > 0) {
			wait(NULL);
		}
		else {
			sprintf(path, "%s/%02d.dat\0", way_to_test, i);
			int read_file = open(path, O_RDONLY, S_IRUSR | S_IWUSR);
			dup2(read_file, 0);
			dup2(fd_1[1], 1);
			close(fd_1[0]), close(fd_1[1]);
			execlp("./var/prog", "./var/prog", NULL);
		}
		pipe(fd_2);
		if (fork() > 0) {
			close(fd_1[0]), close(fd_1[1]);
			wait(NULL);
			close(fd_2[1]);
		}
		else {
			dup2(fd_1[0], 0);
			dup2(fd_2[1], 1);
			close(fd_1[0]), close(fd_1[1]), close(fd_2[0]), close(fd_2[1]);
			sprintf(path, "%s/%02d.ans\0", way_to_test, i);
			if (execlp("./var/checker_byte", "./var/checker_byte", path, NULL) < 0){
				exit(EXIT_FAILURE);
			}
		}
		char tmp;
		while (read(fd_2[0], &tmp, 1) > 0) {
			putchar(tmp);
		}
		close(fd_2[0]);
	}
	free(path);
	return 0;
}

int main(int argc, char **argv) {
	if (argc != 3) {
		perror("Not a 3 ");
		return 1;
	}
	int test_count = 3, check_style = 0;
	set_config(argv[2], &test_count, &check_style);
	test_user_problem(argv[1], argv[2], test_count);
		
	return 0;
}
