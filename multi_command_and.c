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
        fprintf(stderr, "command not found\n");
        return 1;
    }


    char *input = strdup(argv[1]);
    char *command[64];
    int num_command = 0;


    char *cmd_tok = strtok(input, "&&");
    while (cmd_tok != NULL && num_command < 64) {

        while (*cmd_tok == ' ') {
            cmd_tok++;
        }
        command[num_command++] = cmd_tok;
        cmd_tok = strtok(NULL, "&&");
    }

    for (int i = 0; i < num_command; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork failed");
            free(input);
            return 1;
        } else if (pid == 0) {
            char *args[64];
            parse_args(command[i], args);
            execvp(args[0], args);
            perror("execvp failed");
            exit(1);
        } else {
            int status;
            waitpid(pid, &status, 0);
            if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                break;
            }
        }
    }


    free(input);
    return 0;
}

