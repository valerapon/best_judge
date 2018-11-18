#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

void make_config_problem() {
	write(1, "Config error", 12);
	exit(EXIT_FAILURE);
}

int set_checker(int fconfig) {
	char array[15], tmp;
	if (read(fconfig, array, 8) < 0) {
			make_config_problem();
	}
	array[8] = '\0';
	if (strcmp(array, "checker=")) {
		make_config_problem();
	}
	if (read(fconfig, array, 11) < 0) {
		make_config_problem();
	}
	array[11] = '\0';
	if (!strcmp(array, "checker_int")) {
		if (read(fconfig, &tmp, 1) < 0) {
			make_config_problem();
		}
		if (tmp != 10 && tmp != '\n') {
			make_config_problem();
		}
		return 1;
	}
	if (!strcmp(array, "checker_byt")) {
		if (read(fconfig, &tmp, 1) < 0) {
			make_config_problem();
		}
		if (tmp != 'e') {
			make_config_problem();
		}
		if (read(fconfig, &tmp, 1) < 0) {
			make_config_problem();
		}
		if (tmp != 10 && tmp != '\n') {
			make_config_problem();
		}
		return 0;
	}
}

int set_tests(int fconfig) {
	char array[15], tmp;
	int count = 0;
	if (read(fconfig, array, 6) < 0) {
		make_config_problem();
	}
	array[6] = '\0';
	if (strcmp(array, "tests=")) {
		make_config_problem();
	}
	while (1) {
		if (read(fconfig, &tmp, 1) < 0) {
			make_config_problem();
		}
		if (tmp > '9' || tmp < '0') {
			break;
		}
		count = 10 * count + (tmp - '0');
		if (count > 999) {
			make_config_problem();
		}
	}
	return count;
}

int set_config(char *way_to_test, int *test_count, int *check_style) {
	char *path = malloc((strlen(way_to_test) + 20) * sizeof(char)), array[14], tmp;
	if (path == NULL) {
		make_config_problem();
	}
	sprintf(path, "%s/problem.cfg\0", way_to_test);
	int fconfig = open(path, O_RDONLY, S_IRUSR | S_IWUSR), count = 0;
	if (fconfig < 0) {
		make_config_problem();
	}
	*check_style = set_checker(fconfig);
	*test_count = set_tests(fconfig);
	free(path);
	close(fconfig);
	return 0;
}

void make_test_problem() {
	//int file = open("file.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	//dup2(file, 1);
	write(1, "Compilation error", 17);
	exit(EXIT_FAILURE);
}

int test_user_problem(char *user_program, char *way_to_test, int test_count) {
	int status;
	if (fork() > 0) {
		wait(&status);
		if (WEXITSTATUS(status)) {
			make_test_problem();
		}
	}
	else {
		close(2);
		if (execlp("gcc", "gcc", user_program, "-o", "var/prog", NULL) < 0) {
			exit(EXIT_FAILURE);
		}
	}
	if (fork() > 0) {
		wait(&status);
		if (WEXITSTATUS(status)) {
			make_test_problem();
		}
	}
	else {
		close(2);
		if (execlp("gcc", "gcc", "checkers/checker_byte.c", "-o", "var/checker_byte", NULL) < 0) {
			_exit(EXIT_FAILURE);
		}
	}
	char *path = malloc((strlen(way_to_test) + 10) * sizeof(char));
	if (path == NULL) {
		make_test_problem();
	}
	for (int i = 1; i <= test_count; i++) {
		int fd_1[2], fd_2[2], flag = 1;
		pipe(fd_1);
		if (fork() > 0) {
			pipe(fd_2);
			wait(&status);
			if (WEXITSTATUS(status)) {
				write(fd_2[1], "x", 1);
				close(fd_1[0]), close(fd_1[1]);
				flag = 0;
			}
		}
		else {
			sprintf(path, "%s/%02d.dat\0", way_to_test, i);
			int read_file = open(path, O_RDONLY, S_IRUSR | S_IWUSR);
			if (read_file < 0) {
				make_test_problem();
			}
			dup2(read_file, 0);
			dup2(fd_1[1], 1);
			close(fd_1[0]), close(fd_1[1]);
			if (execlp("./var/prog", "./var/prog", NULL) < 0) {
				_exit(EXIT_FAILURE);
			}
		}
		if (flag)
		if (fork() > 0) {
			close(fd_1[0]), close(fd_1[1]);
			wait(&status);
			close(fd_2[1]);
			if (WEXITSTATUS(status)) {
				make_test_problem();
			}
		}
		else {
			dup2(fd_1[0], 0);
			dup2(fd_2[1], 1);
			close(fd_1[0]), close(fd_1[1]), close(fd_2[0]), close(fd_2[1]);
			sprintf(path, "%s/%02d.ans\0", way_to_test, i);
			if (execlp("./var/checker_byte", "./var/checker_byte", path, NULL) < 0){
				_exit(EXIT_FAILURE);
			}
		}
		char tmp;
		if (read(fd_2[0], &tmp, 1) > 0) {
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
