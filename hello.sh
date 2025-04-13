#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>        // Needed for open() flags

#define MAX_INPUT 1024
#define MAX_ARGS 64

int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];

    while (1) {
        printf("sh> ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }

        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "exit") == 0) {
            break;
        }

        int i = 0;
        char *token = strtok(input, " ");
        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        // Handle cd command specially
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                fprintf(stderr, "cd: missing argument\n");
            } else {
                if (chdir(args[1]) != 0) {
                    perror("cd failed");
                }
            }
            continue;
        }

        // Redirection variables
        int input_redirect = 0;
        int output_redirect = 0;
        int append_redirect = 0;
        char *input_file = NULL;
        char *output_file = NULL;

        for (int j = 0; args[j] != NULL; j++) {
            if (strcmp(args[j], "<") == 0) {
                input_redirect = 1;
                input_file = args[j+1];
                args[j] = NULL;  // Remove the '<' from args
                break;
            } else if (strcmp(args[j], ">") == 0) {
                output_redirect = 1;
                output_file = args[j+1];
                args[j] = NULL;  // Remove the '>' from args
                break;
            } else if (strcmp(args[j], ">>") == 0) {
                append_redirect = 1;
                output_file = args[j+1];
                args[j] = NULL;  // Remove the '>>' from args
                break;
            }
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            exit(1);
        } else if (pid == 0) {
            // Input redirection
            if (input_redirect) {
                int fd = open(input_file, O_RDONLY);
                if (fd < 0) {
                    perror("open input file failed");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            // Output redirection
            if (output_redirect || append_redirect) {
                int flags = O_WRONLY | O_CREAT;
                if (append_redirect) {
                    flags |= O_APPEND;
                } else {
                    flags |= O_TRUNC;
                }
                int fd = open(output_file, flags, 0644);
                if (fd < 0) {
                    perror("open output file failed");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            execvp(args[0], args);
            perror("exec failed");
            exit(1);
        } else {
            wait(NULL);
        }
    }

    return 0;
}