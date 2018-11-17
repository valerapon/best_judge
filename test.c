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
		execlp("gcc", "gcc", user_program, NULL);
	}
	char *path = malloc((strlen(way_to_test) + 10) * sizeof(char));
	for (int i = 1; i <= test_count; i++) {
		if (fork() > 0) {
			wait(NULL);
		}
		else {
			sprintf(path, "%s/%02d.dat\0", way_to_test, i);
			int read_file = open(path, O_RDONLY, S_IRUSR | S_IWUSR);
			dup2(read_file, 0);
			execlp("./a.out", "./a.out", NULL);
		}
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
