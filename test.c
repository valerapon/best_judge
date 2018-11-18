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
	write(1, "Compilation error", 17);
	exit(EXIT_FAILURE);
}

void make_log_file_if_all_ok(char *, char *, int, int, char *);

int test_user_problem(char *user_program, char *way_to_test, int test_count) {
	int status, success_tests = 0;
	char *result_array = malloc((test_count + 1) * sizeof(char));
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
				_exit(EXIT_FAILURE);
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
			if (tmp == '+') {
				result_array[i - 1] = 'O';
			}
			if (tmp == '-') {
				result_array[i - 1] = 'N';
			}
			if (tmp == 'x') {
				result_array[i - 1] = 'X';
			}
			if (tmp != 'x') {
				success_tests++;
			}
			putchar(tmp);
		}
		close(fd_2[0]);
	}
	make_log_file_if_all_ok(user_program, way_to_test, test_count, success_tests, result_array);
	free(result_array);
	free(path);
	return 0;
}

void create_number(char array[], int n) {
	int i = 0;
	if (n == 0) {
		array[2] = '0';
	}
	while (n) {
		array[i++] = (n % 10) + '0';
		n /= 10;
	}
}

void make_log_file_if_all_ok(char *user_program, char *way_to_test, int test_count, int success_tests, char *result_array) {
	char *user = malloc(strlen(user_program) * sizeof(char));
	int i = 0, j = 0;
	while (user_program[i] && user_program[i] != '/') i++;
	i++;
	while (user_program[i] && user_program[i] != '/') i++;
	i++;
	while (user_program[i] && user_program[i] != '/') {
		user[j] = user_program[i];
		j++, i++;
	}
	user[j] = '\0';
	
	char *problem = malloc(strlen(way_to_test) * sizeof(char));
	i = 0, j = 0;
	while (way_to_test[i] && way_to_test[i] != '/') i++;
	i++;
	while (way_to_test[i] && way_to_test[i] != '/') i++;
	i++;
	while (way_to_test[i] && way_to_test[i] != '/') {
		problem[j] = way_to_test[i];
		j++, i++;
	}
	problem[j] = '\0';
	

	char *path = malloc((strlen(user) + strlen(problem) + 11) * sizeof(char));
	sprintf(path, "logs/%s-%s.log", user, problem);
	int flog = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	write(flog, "start test\n", 11);
	write(flog, " user: ", 7);
	write(flog, user, strlen(user));
	write(flog, "\n", 1);
	write(flog, " problem: ", 10);
	write(flog, problem, strlen(problem));
	write(flog, "\n", 1);
	write(flog, " tested: ", 9);
	int tests = test_count, fail_tests = test_count - success_tests;
	char number[3] = {0, 0, 0};
	create_number(number, test_count);
	write(flog, number, 3);
	write(flog, "\n", 1);
	write(flog, " failed: ", 9);
	number[0] = number[1] = number[2] = 0;
	create_number(number, test_count - success_tests);
	write(flog, number, 3);
	write(flog, "\n", 1);
	char *test_info = malloc(32 * sizeof(char));
	for (int i = 1; i <= test_count; i++) {
		int len;
		if (result_array[i - 1] == 'O') {
			sprintf(test_info, "  Test %03d: OK\n", i);
			len = 15;
		}
		if (result_array[i - 1] == 'N') {
			sprintf(test_info, "  Test %03d: Wrong answer\n\0", i);
			len = 25;
		}
		if (result_array[i - 1] == 'X') {
			sprintf(test_info, "  Test %03d: Execution error\n\0", i);
			len = 28;
		}
		write(flog, test_info, len);
	}
	write(flog, "stop test", 10);
	close(flog);
	free(path), free(problem), free(user);
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
