// Narkis Shallev (Kremizi) 205832447

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAX_SIZE 151
#define ERROR_MSG write(2,"Error\n",strlen("Error\n"))
#define Num_OF_ARGS 2
#define SIMILAR 3
#define DIFFERENT 2

void checkIntegrityForOneMinus(int checked);

void passTheSubdirectories(char lines[3][MAX_SIZE]);

void handleSpecificSubdirectory(char *pathOfSubdirectory, char *userName, char lines[3][MAX_SIZE], int *isCFileExist,
                                int openResultFile);

int checkIfCFile(struct dirent *ptrToItemInSubdirectory);

int compile(char *pathToCompile);

int execute(char line2[MAX_SIZE]);

int compare(char line3[MAX_SIZE]);

void writeToTheResultFile(char *userName, char *grade, char *reason, int openResultFile);

int main(int argc, char *argv[]) {
    // if the path of conf file is not received
    if (argc < Num_OF_ARGS) {
        ERROR_MSG;
        exit(-1);
    }

    // saving the path of conf file in temporary variable
    char confFilePath[MAX_SIZE] = {};
    strcpy(confFilePath, argv[1]);

    // open the files
    int openConfFile = open(confFilePath, O_RDONLY);
    checkIntegrityForOneMinus(openConfFile);

    // create buffer to keep the 3 lines we read
    char buffer[MAX_SIZE * 3] = {};
    int r = read(openConfFile, buffer, MAX_SIZE * 3);
    checkIntegrityForOneMinus(r);

    // parse the buffer
    char lines[3][MAX_SIZE] = {};
    char *token;
    token = strtok(buffer, "\n");
    strcpy(lines[0], token);
    token = strtok(NULL, "\n");
    strcpy(lines[1], token);
    token = strtok(NULL, "\n");
    strcpy(lines[2], token);

    // if one of the lines is misses
    if ((lines[0] == NULL) || (lines[1] == NULL) || (lines[2] == NULL)) {
        ERROR_MSG;
        exit(-1);
    }

    passTheSubdirectories(lines);

    // close the file
    int closeFile = close(openConfFile);
    checkIntegrityForOneMinus(closeFile);
    return 0;
}

void passTheSubdirectories(char lines[3][MAX_SIZE]) {
    char *pathOfMainDirectory = lines[0];
    // create a pointer to the main directory
    DIR *ptrToMainDirectory = opendir(pathOfMainDirectory);
    if (ptrToMainDirectory == NULL) {
        ERROR_MSG;
        exit(-1);
    }

    // open results.csv file
    int openResultFile = open("results.csv", O_WRONLY | O_CREAT, 0777 | O_APPEND);
    checkIntegrityForOneMinus(openResultFile);

    // looping through the main directory
    struct dirent *ptrToItemInMainDirectory = readdir(ptrToMainDirectory);
    while (ptrToItemInMainDirectory != NULL) {
        // create path to the subdirectory
        char *nameOfSubdirectory = ptrToItemInMainDirectory->d_name;
        if (strcmp(nameOfSubdirectory, ".") == 0 || strcmp(nameOfSubdirectory, "..") == 0){
            ptrToItemInMainDirectory = readdir(ptrToMainDirectory);
            continue;
        }
        char pathOfSubDirectory[MAX_SIZE] = {};
        strcpy(pathOfSubDirectory, pathOfMainDirectory);
        strcat(pathOfSubDirectory, "/");
        strcat(pathOfSubDirectory, nameOfSubdirectory);

        int isCFileExist = 0;
        handleSpecificSubdirectory(pathOfSubDirectory, nameOfSubdirectory, lines, &isCFileExist, openResultFile);
        if (isCFileExist == 0) {
            writeToTheResultFile(nameOfSubdirectory, "0", "NO_C_FILE", openResultFile);
        }

        ptrToItemInMainDirectory = readdir(ptrToMainDirectory);
    }
    // close the file
    int closeFile = close(openResultFile);
    checkIntegrityForOneMinus(closeFile);
    int closeDir = closedir(ptrToMainDirectory);
    checkIntegrityForOneMinus(closeDir);
}

void handleSpecificSubdirectory(char *pathOfSubdirectory, char *userName, char lines[3][MAX_SIZE], int *isCFileExist,
                                int openResultFile) {
    // create a pointer to the subdirectory
    DIR *ptrToSubdirectory = opendir(pathOfSubdirectory);
    if (ptrToSubdirectory == NULL) {
        ERROR_MSG;
        exit(-1);
    }

    // looping through the sub directory
    struct dirent *ptrToItemInSubdirectory = readdir(ptrToSubdirectory);
    while (ptrToItemInSubdirectory != NULL) {
        // create path to the itemInSubdirectory
        char *nameOfItemInSubdirectory = ptrToItemInSubdirectory->d_name;
        char pathOfItemInSubDirectory[MAX_SIZE] = {};
        strcpy(pathOfItemInSubDirectory, pathOfSubdirectory);
        strcat(pathOfItemInSubDirectory, "/");
        strcat(pathOfItemInSubDirectory, nameOfItemInSubdirectory);

        struct stat stat_p;
        int statRetVal = stat(pathOfItemInSubDirectory, &stat_p);
        checkIntegrityForOneMinus(statRetVal);

        // if the pointer points to a file
        if (S_ISREG(stat_p.st_mode)) {
            if (checkIfCFile(ptrToItemInSubdirectory)) {
                // create path to compile
                char *nameOfFile = ptrToItemInSubdirectory->d_name;
                char pathToCompile[MAX_SIZE] = {};
                strcpy(pathToCompile, pathOfSubdirectory);
                strcat(pathToCompile, "/");
                strcat(pathToCompile, nameOfFile);
                // compile
                int compileRetVal = compile(pathToCompile);
                if (compileRetVal == 0) {
                    writeToTheResultFile(userName, "20", "COMPILATION_ERROR", openResultFile);
                } else {
                    int executeRetVal = execute(lines[1]);
                    if (executeRetVal == 0) {
                        writeToTheResultFile(userName, "40", "TIMEOUT", openResultFile);
                    } else {
                        int compareRetVal = compare(lines[2]);
                        if (compareRetVal == DIFFERENT) {
                            writeToTheResultFile(userName, "60", "BAD_OUTPUT", openResultFile);
                        } else if (compareRetVal == SIMILAR) {
                            writeToTheResultFile(userName, "80", "SIMILAR_OUTPUT", openResultFile);
                        } else {
                            writeToTheResultFile(userName, "100", "GREAT_JOB", openResultFile);
                        }
                    }
                    // delete the output file
                    int removeRetVal = unlink("output.txt");
                    checkIntegrityForOneMinus(removeRetVal);
                }
                *isCFileExist = 1;
            }
        } else if (S_ISDIR(stat_p.st_mode)) { // if the pointer points to a directory
            // create path to the sub-subdirectory
            char *nameOfSubSubdirectory = ptrToItemInSubdirectory->d_name;
            if (strcmp(nameOfSubSubdirectory, ".") == 0 || strcmp(nameOfSubSubdirectory, "..") == 0){
                ptrToItemInSubdirectory = readdir(ptrToSubdirectory);
                continue;
            }
            char pathOfSubSubDirectory[MAX_SIZE] = {};
            strcpy(pathOfSubSubDirectory, pathOfSubdirectory);
            strcat(pathOfSubSubDirectory, "/");
            strcat(pathOfSubSubDirectory, nameOfSubSubdirectory);

            handleSpecificSubdirectory(pathOfSubSubDirectory, userName, lines, isCFileExist, openResultFile);
        }
        ptrToItemInSubdirectory = readdir(ptrToSubdirectory);
    }
    int closeDir = closedir(ptrToSubdirectory);
    checkIntegrityForOneMinus(closeDir);
}

void writeToTheResultFile(char *userName, char *grade, char *reason, int openResultFile) {
    // create buffer to keep the line we want to write
    char buffer[MAX_SIZE] = {};
    strcpy(buffer, userName);
    strcat(buffer, ",");
    strcat(buffer, grade);
    strcat(buffer, ",");
    strcat(buffer, reason);
    strcat(buffer, "\n");

    write(openResultFile, buffer, MAX_SIZE);
}

int compare(char line3[MAX_SIZE]) {
    // create an array with arguments for execvp
    char *args[MAX_SIZE] = {};
    args[0] = "./comp.out";
    args[1] = line3;
    args[2] = "output.txt";

    // create a child for the comparing
    pid_t pid = fork();
    if (pid == 0) { // if you are the child
        // compare
        int execvpRetVal = execvp("./comp.out", args);
        checkIntegrityForOneMinus(execvpRetVal);
    } else if (pid < 0) { // if has been an error
        ERROR_MSG;
        exit(-1);
    } else { // if you are the parent
        int status;
        // wait for any child of the same id group to change status
        int waitpidRetVal = waitpid(pid, &status, 0);
        checkIntegrityForOneMinus(waitpidRetVal);
        // return what the compare function return
        return WEXITSTATUS(status);
    }
}

int execute(char line2[MAX_SIZE]) {
    // create a child for the executing
    pid_t pid = fork();
    if (pid == 0) { // if you are the child
        // open output file
        int openOutputFile = open("output.txt", O_WRONLY | O_CREAT, 0777);
        checkIntegrityForOneMinus(openOutputFile);
        // open input file
        int openInputFile = open(line2, O_RDONLY);
        checkIntegrityForOneMinus(openInputFile);

        // change input file descriptor
        int dup2RetVal1 = dup2(openInputFile, 0);
        checkIntegrityForOneMinus(dup2RetVal1);
        // close the file
        int closeInputFile = close(openInputFile);
        checkIntegrityForOneMinus(closeInputFile);

        // change output file descriptor
        int dup2RetVal2 = dup2(openOutputFile, 1);
        checkIntegrityForOneMinus(dup2RetVal2);
        // close the output file
        int closeOutputFile = close(openOutputFile);
        checkIntegrityForOneMinus(closeOutputFile);

        // create an array with arguments for execvp
        char *args[MAX_SIZE] = {};
        args[0] = "./a.out";

        // execute
        int execvpRetVal = execvp("./a.out", args);
        checkIntegrityForOneMinus(execvpRetVal);
    } else if (pid < 0) { // if has been an error
        ERROR_MSG;
        exit(-1);
    } else { // if you are the parent
        int status;
        // wait for child to finish for 5 seconds
        sleep(5);
        int waitpidRetVal = waitpid(pid, &status, WNOHANG);
        // if the child finished return 1
        if (waitpidRetVal > 0) {
            return 1;
        }
        checkIntegrityForOneMinus(waitpidRetVal);
        return 0;
    }
}

int compile(char *pathToCompile) {
    // create an array with arguments for execvp
    char *args[MAX_SIZE] = {};
    args[0] = "gcc";
    args[1] = pathToCompile;
    // create a child for the compiling
    pid_t pid = fork();
    if (pid == 0) { // if you are the child
        // compile
        int execvpRetVal = execvp("gcc", args);
        checkIntegrityForOneMinus(execvpRetVal);
    } else if (pid < 0) { // if has been an error
        ERROR_MSG;
        exit(-1);
    } else { // if you are the parent
        int status;
        // wait for any child of the same id group to change status
        int waitpidRetVal = waitpid(pid, &status, 0);
        checkIntegrityForOneMinus(waitpidRetVal);
        // if the compilation succeeded return 1
        if (WEXITSTATUS(status) == 0) {
            return 1;
        }
        return 0;
    }
}

int checkIfCFile(struct dirent *ptrToItemInSubdirectory) {
    char *fileName = ptrToItemInSubdirectory->d_name;
    if ((fileName[strlen(fileName) - 2] == '.') && (fileName[strlen(fileName) - 1] == 'c') &&
        (fileName[strlen(fileName)] == '\0')) {
        return 1;
    }
    return 0;
}

void checkIntegrityForOneMinus(int checked) {
    if (checked == -1) {
        ERROR_MSG;
        exit(-1);
    }
}