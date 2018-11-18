#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

void cfg_error () {
    puts("Configuration file corrupted.");
    exit(0);
}

int user_set (int cfg) {
    char tmparr[7], c;
    int users = 0;
    if (read(cfg, tmparr, 6) < 0) {
        cfg_error();
    }
    tmparr[6] = '\0';
    if (strcmp(tmparr, "users=") != 0) {
        cfg_error();
    }
    while (1) {
        if (read(cfg, &c, 1) < 0) {
            cfg_error();
        }
        if (c < '0' || c > '9') {
            break;
        }
        if ((users = users * 10 + (c - '0')) > 99) {
            puts("Please, reduce number of users");
            exit(0);
        }
    }
    if (c != '\n') {
        cfg_error();
    }
    return users;
}

int task_set (int cfg) {
    char tmparr[10], c;
    int tasks = 0;
    if (read(cfg, tmparr, 9) < 0) {
        cfg_error();
    }
    tmparr[9] = '\0';
    if (strcmp(tmparr, "problems=") != 0) {
        cfg_error();
    }
    while(1) {
        if (read(cfg, &c, 1) < 0) {
            cfg_error();
        }
        if (c < '0' || c > '9') {
            break;
        }
        if ((tasks = tasks * 10 + (c - '0')) > 26) {
            puts("Please, reduce number of tasks");
            exit(0);
        }
    }
    if (c != '\n') {
        cfg_error();
    }
    return tasks;
}

void configuration (int *users, int *tasks) {
    int cfg;
    cfg = open("contest/global.cfg", O_RDONLY, S_IRUSR | S_IWUSR);
    if (cfg < 0) {
        puts("No configuration file found.");
        exit(0);
    }
    int buf[10];
    *users = user_set(cfg);
    *tasks = task_set(cfg);
    close(cfg);
    return ;
}

void testing (int users, int tasks, char *contest) {
    for (int u = 1; u <= users; u++) {
        char *user_dir = malloc(strlen(contest) + 12);
        sprintf(user_dir, "%s/code/user%d", contest, u);
        for (int t = 0; t < tasks; t++) {
            char *test_dir = malloc(strlen(contest) + 10);
            char *code_dir = malloc(strlen(user_dir) + 5);
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

int main (int argc, char **argv) {
    if (argc == 1) {
        puts("No contest folder specified");
        return 1;
    }
    int users, tasks;
    configuration(&users, &tasks);
    testing(users, tasks, argv[1]);
    return 0;
}
