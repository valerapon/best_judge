#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

void testing(int users, int tasks, char *contest) {
    for (int u = 1; u <= users; u++) {
        char *user_dir = malloc(strlen(contest) + 11);
        sprintf(user_dir, "%s/code/user%d", contest, u);
        for (int t = 0; t < tasks; t++) {
            char *test_dir = malloc(strlen(contest) + 9);
            char *code_dir = malloc(strlen(user_dir) + 4);
            char cur_task = 'A' + t;
            sprintf(code_dir, "%s/%c.c", user_dir, cur_task);
            sprintf(test_dir, "%s/tests/%c", contest, cur_task);
            if (fork() == 0) {
                execlp("./test", "./test", code_dir, test_dir, NULL);
            }
            wait(NULL);
            free(test_dir);
            free(code_dir);
        }
        free(user_dir);
    }
}

int main(int argc, char **argv) {
    if (argc == 1) {
        puts("No contest folder specified");
        return 1;
    }
    //config
    int users = 2;
    int tasks = 2;
    testing(users, tasks, argv[1]);
    return 0;
}
