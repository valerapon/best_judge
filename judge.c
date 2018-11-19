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
    return;
}

int res_file_create (int tasks) {
    int res = open("var/results.csv", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (res < 0) {
        puts("Can't create results file");
        exit(0);
    }
    write(res, "users,", 6);
    char buf[2];
    for (int i = 0; i < tasks; i++) {
        sprintf(buf, "%c,", i + 'A');
        write(res, buf, 2);
    }
    write(res, "sum\n", 4);
    return res;
}

void write_to_res_0 (int res, int user, int *sum) {
    int tmp_res = open("var/result.txt", O_RDONLY, S_IRUSR | S_IWUSR);
    if (tmp_res < 0) {
        puts("I don't know how to get this error");
        return;
    }
    char c;
    read(tmp_res, &c, 1);
    while (1) {
        if (c == '-' || c == 'x' || c == 'C') {
            write(res, "-,", 2);
            close(tmp_res);
            return;
        }
        if (read(tmp_res, &c, 1) == 0) {
            break;
        }
    }
    write(res, "+,", 2);
    *sum += 1;
    close(tmp_res);
    return;
}

void write_to_res_1 (int res, int user, int *sum) {
    int tmp_res = open("var/result.txt", O_RDONLY, S_IRUSR | S_IWUSR);
    if (tmp_res < 0) {
        puts("No result.txt somehow");
        return;
    }
    char c;
    int tmp_sum = 0;
    read (tmp_res, &c, 1);
    while (1) {
        if (c == '+') {
            tmp_sum += 1;
            *sum += 1;
        }
        if (read(tmp_res, &c, 1) == 0) {
            break;
        }
    }
    char buf[4];
    buf[0] = 0; buf[1] = 0; buf[2] = 0;
    if (tmp_sum > 99) {
        buf[0] = (tmp_sum / 100) + '0';
    }
    if (tmp_sum > 9) {
        buf[1] = ((tmp_sum / 10) % 10) + '0';
    }
    buf[2] = (tmp_sum % 100) + '0';
    buf[3] = ',';
    write(res, buf, 4);
    close(tmp_res);
    return;
}

void write_sum (int res, int sum) {
    char buf[7];
    memset(buf, 0, 7);
    buf[6] = (sum % 10) + '0';
    if (sum > 9) {
        buf[5] = (sum / 10 % 10) + '0';
    }
    if (sum > 99) {
        buf[4] = (sum / 100 % 10) + '0';
    }
    if (sum > 999) {
        buf[3] = (sum / 1000 % 10 ) + '0';
    }
    if (sum > 9999) {
        buf[2] = (sum / 10000 % 10 ) + '0';
    }
    if (sum > 99999) {
        buf[1] = (sum / 100000 % 10 ) + '0';
    }
    if (sum > 999999) {
        buf[0] = (sum / 1000000 % 10 ) + '0';
    }
    write(res, buf, 7);
    write(res, "\n", 1);
}

void testing (int users, int tasks, int score_type, char *contest) {
    int res = res_file_create(tasks);
    for (int u = 1; u <= users; u++) {
        char *usr = malloc(7);
        sprintf(usr, "user%d,", u);
        if (u <= 9) {
            write(res, usr, 6);
        } else {
            write(res, usr, 7);
        }
        int sum = 0;
        char *user_dir = malloc(strlen(contest) + 13);
        sprintf(user_dir, "%s/code/user%d", contest, u);
        for (int t = 0; t < tasks; t++) {
            char *test_dir = malloc(strlen(contest) + 10);
            char *code_dir = malloc(strlen(user_dir) + 5);
            char cur_task = 'A' + t;
            sprintf(code_dir, "%s/%c.c", user_dir, cur_task);
            sprintf(test_dir, "%s/tests/%c", contest, cur_task);
            printf("%d(%c): \n", u, cur_task);
            if (fork() == 0) {
                execlp("./test", "./test", code_dir, test_dir, NULL);
            }
            int wstatus;
            wait(&wstatus);
            if (WEXITSTATUS(wstatus) != 0) {
                printf("\nuser = %d, task = %c failed\n", u, cur_task);
                if (score_type == 0) {
                    write(res, "-,", 2);
                } else {
                    write(res, "0,", 2);
                }
                free(test_dir);
                free(code_dir);
                continue;
            }
            printf("\n");
            if (score_type == 0) {
                write_to_res_0(res, u, &sum);
            } else {
                write_to_res_1(res, u, &sum);
            }
            free(test_dir);
            free(code_dir);
        }
        write_sum(res, sum);
        free(user_dir);
    }
    close(res);
    res = open("var/results.csv", O_RDONLY, S_IRUSR | S_IWUSR);
    char buf[10];
    int len;
    while((len = read(res, buf, 10)) != 0) {
        write(1, buf, len);
    }
    close(res);
}

int main (int argc, char **argv) {
    if (argc == 1) {
        puts("No contest folder specified");
        return 1;
    }
    int users, tasks, score_type;
    configuration(&users, &tasks, &score_type, argv[1]);
    testing(users, tasks, score_type, argv[1]);
    return 0;
}
