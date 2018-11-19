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
	int fresult = open("var/result.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
	write(1, "Config error", 12);	
	write(fresult, "Config error", 12);
	close(fresult);
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
	return 0;
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
	char *path = malloc((strlen(way_to_test) + 20) * sizeof(char));
	if (path == NULL) {
		make_config_problem();
	}
	sprintf(path, "%s/problem.cfg", way_to_test);
	int fconfig = open(path, O_RDONLY, S_IRUSR | S_IWUSR);
	if (fconfig < 0) {
		make_config_problem();
	}
	*check_style = set_checker(fconfig);
	*test_count = set_tests(fconfig);
	free(path);
	close(fconfig);
	return 0;
}

void make_test_problem(int fresult) {
	write(1, "Compilation error", 17);
	write(fresult, "Compilation error", 17);
	close(fresult);
	exit(EXIT_FAILURE);
}

void compiler_user_program(char *user_program, int fresult) {
	int status;
	if (fork() > 0) {
		wait(&status);
		if (WEXITSTATUS(status)) {
			make_test_problem(fresult);
		}
	}
	else {
		close(2);
		if (execlp("gcc", "gcc", user_program, "-o", "var/prog", NULL) < 0) {
			exit(EXIT_FAILURE);
		}
	}

}

void compiler_checker(int check_style, int fresult) {
	int status;
	if (fork() > 0) {
		wait(&status);
		if (WEXITSTATUS(status)) {
			make_test_problem(fresult);
		}
	}
	else {
		close(2);
		if (check_style == 0){
			if (execlp("gcc", "gcc", "checkers/checker_byte.c", "-o", "var/checker_byte", NULL) < 0) {
				_exit(EXIT_FAILURE);
			}
		}
		else {
			if (execlp("gcc", "gcc", "checkers/checker_int.c", "-o", "var/checker_int", NULL) < 0) {
				_exit(EXIT_FAILURE);
			}
		}
	}
}

int do_user_prog(int fd_1[], int fd_2[], char *path, char *way_to_test, int i) {
	int status;
	if (fork() > 0) {
		pipe(fd_2);
		wait(&status);
		if (WEXITSTATUS(status)) {
			write(fd_2[1], "x", 1);
			close(fd_1[0]), close(fd_1[1]);
			return 0;
		}
	}
	else {
		sprintf(path, "%s/%02d.dat", way_to_test, i);
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
	return 1;
}

void do_check_ans(int fd_1[], int fd_2[], int fresult, char *path, char *way_to_test, int check_style, int i) {
	int status;
	if (fork() > 0) {
		close(fd_1[0]), close(fd_1[1]);
		wait(&status);
		close(fd_2[1]);
		if (WEXITSTATUS(status)) {
			make_test_problem(fresult);
		}
	}
	else {
		dup2(fd_1[0], 0);
		dup2(fd_2[1], 1);
		close(fd_1[0]), close(fd_1[1]), close(fd_2[0]), close(fd_2[1]);
		sprintf(path, "%s/%02d.ans", way_to_test, i);
		if (check_style == 0) {
			if (execlp("./var/checker_byte", "./var/checker_byte", path, NULL) < 0){
				_exit(EXIT_FAILURE);
			}
		}
		else {
			if (execlp("./var/checker_int", "./var/checker_int", path, NULL) < 0){
				_exit(EXIT_FAILURE);
			}
		}
	}
}

void make_ans(int fd_2[], char result_array[], int i, int *success_tests, int fresult) {
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
			(*success_tests)++;
		}
		putchar(tmp);
		write(fresult, &tmp, 1);
	}
}

char *get_user_name(char *user_program) {
	char *user = malloc(strlen(user_program) * sizeof(char));
	int i = 0, j = 0;
	while (user_program[i++] != '/');
	while (user_program[i++] != '/');
	while (user_program[i] != '/') {
		user[j] = user_program[i];
		j++, i++;
	}
	user[j] = '\0';	
	return user;
}

char *get_user_problem(char *way_to_test) {
	char *problem = malloc(strlen(way_to_test) * sizeof(char));
	int i = 0, j = 0;
	while (way_to_test[i++] != '/');
	while (way_to_test[i++] != '/');
	while (way_to_test[i] != '/') {
		problem[j] = way_to_test[i];
		j++, i++;
	}
	problem[j] = '\0';
	return problem;
}

void create_number(char array[], int n) {
	int i = 2;
	while (n) {
		array[i--] = (n % 10) + '0';
		n /= 10;
	}
}

void make_log_file(char *user_program, char *way_to_test, int test_count, int success_tests, char *result_array) {
	char *user = get_user_name(user_program);
	char *problem = get_user_problem(way_to_test);
	char *path = malloc((strlen(user) + strlen(problem) + 11) * sizeof(char));
	char number_1[4] = {'0', '0', '0', 0}, number_2[4] = {'0', '0', '0', 0};

	create_number(number_1, test_count);
	create_number(number_2, test_count - success_tests);
	sprintf(path, "logs/%s-%s.log", user, problem);
	int flog = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	char *string = malloc((strlen(user) + strlen(number_1) + strlen(number_2) + 70) * sizeof(char));
	sprintf(string, "start test\n user: %s\n tested: %s\n failed: %s\n", user, number_1, number_2);
	write(flog, string, strlen(string));
	char *test_info = malloc(32 * sizeof(char));
	for (int i = 1; i <= test_count; i++) {
		int len;
		if (result_array[i - 1] == 'O') {
			sprintf(test_info, "  Test %03d: OK\n", i);
			len = 15;
		}
		if (result_array[i - 1] == 'N') {
			sprintf(test_info, "  Test %03d: Wrong answer\n", i);
			len = 25;
		}
		if (result_array[i - 1] == 'X') {
			sprintf(test_info, "  Test %03d: Execution error\n", i);
			len = 28;
		}
		write(flog, test_info, len);
	}
	write(flog, "stop test", 10);
	close(flog);
	free(path), free(problem), free(user), free(string)/*, free(test_info)*/;
}

int test_user_problem(char *user_program, char *way_to_test, int test_count, int check_style) {
	int success_tests = 0;
	int fresult = open("var/result.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	char *path = malloc((strlen(way_to_test) + 10) * sizeof(char));
	char *result_array = malloc((test_count + 1) * sizeof(char));
	
	if (path == NULL || result_array == NULL) {
		make_test_problem(fresult);
	}
	compiler_user_program(user_program, fresult);
	compiler_checker(check_style, fresult);
	for (int i = 1; i <= test_count; i++) {
		int fd_1[2], fd_2[2];
		pipe(fd_1);
		if (do_user_prog(fd_1, fd_2, path, way_to_test, i)) {
			do_check_ans(fd_1, fd_2, fresult, path, way_to_test, check_style, i);
		}
		make_ans(fd_2, result_array, i, &success_tests, fresult);
		close(fd_2[0]);
	}
	make_log_file(user_program, way_to_test, test_count, success_tests, result_array);
	free(result_array), free(path);
	close(fresult);
	return 0;
}

int main(int argc, char **argv) {
	if (argc != 3) {
		perror("Need 3 arguments");
		return 1;
	}
	int test_count = 3, check_style = 0;
	set_config(argv[2], &test_count, &check_style);
	test_user_problem(argv[1], argv[2], test_count, check_style);
		
	return 0;
}
