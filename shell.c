#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64
#define MAX_HISTORY 200

char *history[MAX_HISTORY];
int h_count = 0;
int add_history(char *cmd) {
	if (h_count < MAX_HISTORY) {
		history[h_count++] = strdup(cmd);
	} else {
		free(history[0]);
		for (int i=1; i < MAX_HISTORY; i++){
			history[i-1] = history[i];
		
		}
		history[MAX_HISTORY-1]= strdup(cmd);
	
	}	
	return 0;
	
}

int print_history() {
	for (int i=0; i<h_count ; i++) {
		fprintf(stdout,"%d: %s\n", i+1, history[i]);
	
	}
	return 0;

}


pid_t child_pid = -1; // Global to track child

void sigint_handler(int signum) {
    if (child_pid > 0) {
        kill(child_pid, SIGINT); // Send SIGINT to child
    }
    printf("\n");
}

int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];

    // Handle Ctrl+C
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);

    // Compile external files
    if (system("gcc -o pipe pipe.c") != 0) {
        fprintf(stderr, "Compilation of pipe.c failed!\n");
        exit(1);
    }
    if (system("gcc -o mcmd multi_command_and.c") != 0) {
        fprintf(stderr, "Compilation of multi_command_and.c failed!\n");
        exit(1);
    }
    if (system("gcc -o mcmd1 multi_command_semi.c") != 0) {
        fprintf(stderr, "Compilation of multi_command_semi.c failed!\n");
        exit(1);
    }
    fprintf(stdout, "****WELCOME TO UBUNTU SHELL****\n");
    fprintf(stdout,"***CREATED BY SHAJEDUL AREFIN & MAZHARUL ISLAM***\n");
    while (1) {
        printf("sh> ");


        if (fgets(input, sizeof(input), stdin) == NULL) 
        	continue;

        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) 
        	continue;
        if (strcmp(input, "exit") == 0) 
        	break;
        if (strcmp(input, "history")==0) {
        	print_history();
        	continue;
        } 
        add_history(input);

	


        if (strchr(input, '|')) {
        	pid_t pid = fork();
        	if (pid == 0){
            		char *args[] = {"./pipe", input, NULL};
            		execvp(args[0], args);
            		perror("Execution failed");
            		exit(1);
            	} else {
            		waitpid(pid,NULL,0);
            		continue;
            	}

        } else if (strstr(input, "&&")) {
        	pid_t pid = fork();
        	if (pid == 0) {
            		char *args[] = {"./mcmd", input, NULL};
            		execvp(args[0], args);
            		perror("Execution failed");
            		exit(1);
            	} else {
            		waitpid(pid,NULL,0);
            		continue;
            	}

        } else if (strchr(input, ';')) {
        	pid_t pid = fork();
        	if (pid == 0) {
            		char *args[] = {"./mcmd1", input, NULL};
            		execvp(args[0], args);
            		perror("Execution failed");
            		exit(1);
            	} else {
            		waitpid(pid,NULL,0);
            		continue;
            	}

        } else {
            int i = 0;
            char *token = strtok(input, " ");
            while (token != NULL && i < MAX_ARGS - 1) {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;

            if (args[0] == NULL) 
            	continue;

            // Handle cd
            if (strcmp(args[0], "cd") == 0) {
                if (args[1] == NULL) {
                    fprintf(stderr, "cd: missing argument\n");
                } else if (chdir(args[1]) != 0) {
                    perror("cd failed");
                }
                continue;
            }

            // Redirection flags
            int input_redirect = 0, output_redirect = 0, append_redirect = 0;
            char *input_file = NULL, *output_file = NULL;

            for (int j = 0; args[j] != NULL; j++) {
                if (strcmp(args[j], "<") == 0) {
                    input_redirect = 1;
                    input_file = args[j+1];
                    args[j] = NULL;
                    break;
                } else if (strcmp(args[j], ">") == 0) {
                    output_redirect = 1;
                    output_file = args[j+1];
                    args[j] = NULL;
                    break;
                } else if (strcmp(args[j], ">>") == 0) {
                    append_redirect = 1;
                    output_file = args[j+1];
                    args[j] = NULL;
                    break;
                }
            }

            child_pid = fork();
            if (child_pid < 0) {
                perror("fork failed");
                exit(1);
            } else if (child_pid == 0) {
                signal(SIGINT, SIG_DFL);

                if (input_redirect) {
                    int fd = open(input_file, O_RDONLY);
                    if (fd < 0) {
                        perror("open input file failed");
                        exit(1);
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }

                if (output_redirect || append_redirect) {
                    int flags = O_WRONLY | O_CREAT;
                    flags |= append_redirect ? O_APPEND : O_TRUNC;

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
                waitpid(child_pid, NULL, 0);
                child_pid = -1;
            }
        }
    }

    return 0;
}

