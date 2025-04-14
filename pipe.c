#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int parse_args(char *cmd, char **args) {
    int i = 0;
    args[i] = strtok(cmd, " ");
    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, " ");
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr,"command not found!\n");
        return 1;
    }

    char *input = strdup(argv[1]);

    char *commands[64];
    int num_cmds = 0;

    char *token = strtok(input, "|");
    while (token != NULL && num_cmds < 64) {
        while (*token == ' ') token++; 
        commands[num_cmds++] = token;
        token = strtok(NULL, "|");
    }

    int fd[2 * (num_cmds - 1)];

    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(fd + i*2) < 0) {
            perror("pipe error");
            exit(1);
        }
    }

    for (int i = 0; i < num_cmds; i++) {
        pid_t pid = fork();
        if (pid == 0) {

            if (i > 0) {
                dup2(fd[(i - 1) * 2], STDIN_FILENO);
            }


            if (i < num_cmds - 1) {
                dup2(fd[i * 2 + 1], STDOUT_FILENO);
            }

            for (int j = 0; j < 2 * (num_cmds - 1); j++) {
                close(fd[j]);
            }

            char *args[64];
            parse_args(commands[i], args);
            execvp(args[0], args);
            perror("execvp failed");
            exit(1); 
        } else if (pid < 0) {
            perror("fork failed");
            exit(1);
        }
    }

    
    for (int i = 0; i < 2 * (num_cmds - 1); i++) {
        close(fd[i]);
    }

    for (int i = 0; i < num_cmds; i++) {
        wait(NULL);
    }


    free(input);
    return 0;
}

