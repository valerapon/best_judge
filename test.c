#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

int set_config(int *test_count, int *check_style) {
	int fconfig = open("problem.cfg", O_RDONLY, S_IRUSR | S_IWUSR);
	close(fconfig);
	*test_count = 1;
	*check_style = 0;
	return 0;
}

int test_user_problem(char *user_program, char *way_to_test, int test_count) {
	//int fd[2];
	//pipe(fd);
	//for (int i = 0; i < test_count; i++) {
		if (fork() > 0) {
			wait(NULL);
		}
		else {
			//puts("fuck");
			//execlp("ls", "ls", NULL);
			char *path = malloc(strlen(way_to_test) * sizeof(char) + 10);
			sprintf(path, "%s/0%d.dat\0", way_to_test, 1);
			if (fork() > 0) {
				wait(NULL);
			}
			else {
				//execlp("ls", "ls", NULL);
				execlp("gcc", "gcc", user_program, NULL);
			}
			int read_file = open(path, O_RDONLY, S_IRUSR | S_IWUSR);
			dup2(read_file, 0);
			execlp("./a.out", "./a.out", NULL);
		}
	//}
	return 0;
}

int main(int argc, char **argv) {
	if (argc != 3) {
		perror("Not a 3 ");
		return 1;
	}
	int test_count = 1, check_style = 0;
	//set_config(&test_count, &check_style);
	test_user_problem(argv[1], argv[2], test_count);

	return 0;
}
