// Narkis Shallev Kremizi 205832447

#include <stdio.h>
#include <memory.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_SIZE 512

typedef struct {
    pid_t pid;
    char name[MAX_SIZE];
} Job;

void getLine(char *input);

void updateJobs(Job jobs[MAX_SIZE], int *indexInJobs);

void compactJobs(Job jobs[MAX_SIZE], int *indexInJobs);

void substring(char string[], char sub[], int position, int length);

int main() {
    char input[MAX_SIZE] = {};
    Job jobs[MAX_SIZE] = {};
    int indexInJobs = 0;
    int isAmpersand = 0;
    Job tempJob;
    char *token;
    char pwd[MAX_SIZE];
    char cwd[MAX_SIZE];
    int isPwdChanged = 0;

    // get the input from the user
    getLine(input);
    // while the command is not "exit" - decode and execute it
    while (strcmp(input, "exit") != 0) {

        // Handles case of no command
        if ((isspace(input[0])) || (strcmp(input, "") == 0)) {
            isAmpersand = 0;
            // get the input from the user
            getLine(input);
            continue;
        }

        // Handles case of "jobs" command
        if (strcmp(input, "jobs") == 0) {
            // update job's list by deleting the processes who exit
            updateJobs(jobs, &indexInJobs);
            // print the jobs in background
            int i;
            for (i = 0; i < MAX_SIZE; ++i) {
                if (jobs[i].pid != 0) {
                    printf("%ld %s\n", (long) jobs[i].pid, jobs[i].name);
                    fflush(stdout);
                }
            }
            isAmpersand = 0;
            // get the input from the user
            getLine(input);
            continue;
        }
        // Handles other commands
        char *parsed[MAX_SIZE] = {};
        char inputCopy[MAX_SIZE] = {};
        strcpy(inputCopy, input);
        int indexInParsed = 0;
        char withoutQuotes[MAX_SIZE] = {};

        // parse the input
        token = strtok(inputCopy, " ");
        parsed[0] = token;
        indexInParsed++;
        token = strtok(NULL, " ");
        while ((token != NULL) && (indexInParsed != MAX_SIZE)) {
            if (strcmp(token, "&") == 0) {
                isAmpersand = 1;
                break;
            }
            parsed[indexInParsed] = token;
            indexInParsed++;
            token = strtok(NULL, " ");
        }

        // Handles case of "cd" command
        if (strcmp(parsed[0], "cd") == 0) {
            // print the parent's pid
            printf("%ld\n", (long) getpid());
            // "cd" command without parameter of with "~"
            if ((parsed[1] == NULL) || (strcmp(parsed[1], "~") == 0)) {
                // save the current working directory to pwd
                char *getCwdCheck = getcwd(pwd, MAX_SIZE);
                if (getCwdCheck == NULL) {
                    fprintf(stderr, "Error in system call\n");
                }
                isPwdChanged = 1;
                // go to home
                int check = chdir(getenv("HOME"));
                if (check == -1) {
                    fprintf(stderr, "Error in system call\n");
                }
                isAmpersand = 0;
                // get the input from the user
                getLine(input);
                continue;
            }

            // "cd" command with "-"
            if (strcmp(parsed[1], "-") == 0) {
                // if the pwd is not set
                if (isPwdChanged == 0) {
                    printf("OLDPWD not set\n");
                    isAmpersand = 0;
                    // get the input from the user
                    getLine(input);
                    continue;
                }
                // move to pwd and save the current working directory to pwd
                char *getCwdCheck = getcwd(cwd, MAX_SIZE);
                if (getCwdCheck == NULL) {
                    fprintf(stderr, "Error in system call\n");
                }
                int check = chdir(pwd);
                if (check == -1) {
                    fprintf(stderr, "Error in system call\n");
                }
                strcpy(pwd, cwd);
                isAmpersand = 0;
                // get the input from the user
                getLine(input);
                continue;
            }

            // "cd" command with parameters
            // save the current working directory to pwd
            char *getCwdCheck = getcwd(pwd, MAX_SIZE);
            int check = 0;
            if (getCwdCheck == NULL) {
                fprintf(stderr, "Error in system call\n");
            }
            isPwdChanged = 1;
            // Handles cases where there are quotation marks
            char *withQuotes = strchr(input, '"');
            if (withQuotes != NULL) {
                int indexOfQuotes = (int) (withQuotes - input);
                // take off the quotation marks
                substring(input, withoutQuotes, indexOfQuotes + 2, (int) (strlen(input) - indexOfQuotes - 2));
                // go to path
                check = chdir(withoutQuotes);
            } else {
                // go to path
                check = chdir(parsed[1]);
            }
            if (check == -1) {
                fprintf(stderr, "Error in system call\n");
                isAmpersand = 0;
                // get the input from the user
                getLine(input);
                continue;
            }
            isAmpersand = 0;
            // get the input from the user
            getLine(input);
            continue;
        }

        int status;
        // create a child for running the commands with execvp
        pid_t pid = fork();
        // if you are the child
        if (pid == 0) {
            // print the child's pid
            printf("%ld\n", (long) getpid());
            fflush(stdout);
            // run the command in new shell
            int check = execvp(parsed[0], parsed);
            if (check == -1) {
                fprintf(stderr, "Error in system call\n");
                exit(3);
            }
        } else if (pid < 0) { // if has been an error
            fprintf(stderr, "Error in system call\n");
            isAmpersand = 0;
            // get the input from the user
            getLine(input);
            continue;
        } else { // if you are the parent
            // if the process doesn't run in background - wait
            if (isAmpersand == 0) {
                int check = wait(&status);
                if (check == -1) {
                    fprintf(stderr, "Error in system call\n");
                    isAmpersand = 0;
                    // get the input from the user
                    getLine(input);
                    continue;
                }
            }
                // if the process run in background add it to the jobs list
            else {
                tempJob.pid = pid;
                int i;
                for (i = 0; i < MAX_SIZE; ++i) {
                    if (input[i] == '&') {
                        input[i] = 0;
                    }
                }
                strcpy(tempJob.name, input);
                jobs[indexInJobs] = tempJob;
                indexInJobs++;
            }
        }
        isAmpersand = 0;
        // get the input from the user
        getLine(input);
    }
    // at exit the parent's pid is printed
    printf("%ld\n", (long) getpid());
    fflush(stdout);
    return 0;
}

// create substring
void substring(char string[], char sub[], int position, int length) {
    int c = 0;
    while (c < length) {
        sub[c] = string[position + c - 1];
        c++;
    }
    sub[c] = '\0';
}

// get line from the user
void getLine(char *input) {
    // Synchronize the ">"
    usleep(4000);
    printf("%s", "> ");
    fgets(input, MAX_SIZE, stdin);
    // delete the /n at the end
    input[strlen(input) - 1] = 0;
}

// update job's list by deleting the processes who exit
void updateJobs(Job jobs[MAX_SIZE], int *indexInJobs) {
    int status;
    int i;
    for (i = 0; i < MAX_SIZE; ++i) {
        if (jobs[i].pid != 0) {
            int retVal = waitpid(jobs[i].pid, &status, WNOHANG);
            if ((WIFEXITED(status)) && (retVal > 0)) {
                jobs[i].pid = 0;
                memset(jobs[i].name, 0, MAX_SIZE);
            }
        }
    }
    compactJobs(jobs, indexInJobs);
}

// compact the job's list
void compactJobs(Job jobs[MAX_SIZE], int *indexInJobs) {
    // make a copy of jobs
    Job jobsCopy[MAX_SIZE];
    int k;
    int l;
    int i;
    for (k = 0; k < MAX_SIZE; ++k) {
        jobsCopy[k] = jobs[k];
    }
    // init the jobs list;
    for (l = 0; l < MAX_SIZE; ++l) {
        jobs[l].pid = 0;
        memset(jobs[l].name, 0, MAX_SIZE);
    }
    int j = 0;
    for (i = 0; i < MAX_SIZE; ++i) {
        if ((jobsCopy[i].pid != 0) && (strcmp(jobsCopy[i].name, "") != 0)) {
            jobs[j] = jobsCopy[i];
            j++;
        }
    }
    *indexInJobs = j;
}
