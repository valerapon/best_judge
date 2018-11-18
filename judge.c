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

int score_set (int cfg) {   // 0 = sum, 1 = all
    char tmparr[7], type[4];
    int score;
    if (read(cfg, tmparr, 6) < 0) {
        cfg_error();
    }
    tmparr[6] = '\0';
    if (strcmp(tmparr, "score=") != 0) {
        cfg_error();
    }
    if (read(cfg, type, 3) < 0) {
        cfg_error();
    }
    type[3] = '\0';
    if (strcmp(type, "sum") == 0) {
        score = 0;
    } else if (strcmp(type, "all") == 0) {
        score = 1;
    } else {
        cfg_error();
    }
    return score;
}
void configuration (int *users, int *tasks, int *score_type, char *contest) {
    int cfg;
    char *path = malloc(strlen(contest) + 12);
    sprintf(path, "%s/global.cfg", contest);
    cfg = open(path, O_RDONLY, S_IRUSR | S_IWUSR);
    if (cfg < 0) {
        puts("No configuration file found.");
        free(path);
        exit(0);
    }
    int buf[10];
    *users = user_set(cfg);
    *tasks = task_set(cfg);
    *score_type = score_set(cfg);
    close(cfg);
    free(path);
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
            int wstatus;
            wait(&wstatus);
            if (WEXITSTATUS(wstatus) != 0){
                printf("user = %d, task = %c failed\n", u, cur_task);
            }
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
    int users, tasks, score_type;
    configuration(&users, &tasks, &score_type, argv[1]);
    testing(users, tasks, argv[1]);
    return 0;
}
