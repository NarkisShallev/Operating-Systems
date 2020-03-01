// Narkis Shallev (Kremizi) 205832447
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_SIZE 151
#define ERROR_MSG write(2,"Error\n",strlen("Error\n"))
#define Num_OF_ARGS 3
#define IDENTICAL 1
#define SIMILAR 3
#define DIFFERENT 2

void checkIntegrityForOneMinus(int checked);

void removeEntersAndSpaces(char *source);

int isIdentical(char path1[MAX_SIZE], char path2[MAX_SIZE]);

int isSimilar(char path1[MAX_SIZE], char path2[MAX_SIZE]);

int main(int argc, char *argv[]) {
    // if two paths are not received
    if (argc < Num_OF_ARGS) {
        ERROR_MSG;
        exit(-1);
    }
    // saving paths in temporary variables
    char path1[MAX_SIZE] = {};
    char path2[MAX_SIZE] = {};
    strcpy(path1, argv[1]);
    strcpy(path2, argv[2]);
    // if identical - returns 1, if similar - returns 3, otherwise - returns 2
    int returnVal = 0;
    returnVal = isIdentical(path1, path2);
    if (returnVal) {
        return IDENTICAL;
    }
    returnVal = isSimilar(path1, path2);
    if (returnVal) {
        return SIMILAR;
    }
    return DIFFERENT;
}

void checkIntegrityForOneMinus(int checked) {
    if (checked == -1) {
        ERROR_MSG;
        exit(-1);
    }
}

void removeEntersAndSpaces(char *source) {
    // replace each enter to space
    char *i = source;
    while (*i != 0) {
        if (isspace(*i)) {
            *i = ' ';
        }
        i++;
    }
    // remove spaces
    char *j = source;
    char *k = source;
    while (*k != 0) {
        *j = *k++;
        if (*j != ' ') {
            j++;
        }
    }
    *j = 0;
}

int isIdentical(char path1[MAX_SIZE], char path2[MAX_SIZE]) {
    // open the files
    int openFile1 = open(path1, O_RDONLY);
    checkIntegrityForOneMinus(openFile1);
    int openFile2 = open(path2, O_RDONLY);
    checkIntegrityForOneMinus(openFile2);
    // create buffers to keep the line we read
    char buffer1[MAX_SIZE] = {};
    char buffer2[MAX_SIZE] = {};

    // read lines until one of the files is finished
    int r1 = read(openFile1, buffer1, MAX_SIZE);
    checkIntegrityForOneMinus(r1);
    int r2 = read(openFile2, buffer2, MAX_SIZE);
    checkIntegrityForOneMinus(r2);
    while ((r1 != 0) || (r2 != 0)) {
        if (strcmp(buffer1, buffer2) != 0) {
            int closeFile1 = close(openFile1);
            checkIntegrityForOneMinus(closeFile1);
            int closeFile2 = close(openFile2);
            checkIntegrityForOneMinus(closeFile2);
            return 0;
        }
        r1 = read(openFile1, buffer1, MAX_SIZE);
        checkIntegrityForOneMinus(r1);
        r2 = read(openFile2, buffer2, MAX_SIZE);
        checkIntegrityForOneMinus(r2);
    }
    int closeFile1 = close(openFile1);
    checkIntegrityForOneMinus(closeFile1);
    int closeFile2 = close(openFile2);
    checkIntegrityForOneMinus(closeFile2);
    return 1;
}

int isSimilar(char path1[MAX_SIZE], char path2[MAX_SIZE]) {
    // open the files
    int openFile1 = open(path1, O_RDONLY);
    checkIntegrityForOneMinus(openFile1);
    int openFile2 = open(path2, O_RDONLY);
    checkIntegrityForOneMinus(openFile2);
    // create buffers to keep the bytes we read
    char buffer1[2] = {};
    char buffer2[2] = {};

    // read lines while one of the files is not finished
    int r1 = read(openFile1, buffer1, 1);
    checkIntegrityForOneMinus(r1);
    int r2 = read(openFile2, buffer2, 1);
    checkIntegrityForOneMinus(r2);
    while ((r1 != 0) || (r2 != 0)) {
        removeEntersAndSpaces(buffer1);
        removeEntersAndSpaces(buffer2);
        // turn any up letter into a low letter
        int i;
        for (i = 0; buffer1[i]; i++) {
            buffer1[i] = tolower(buffer1[i]);
        }
        for (i = 0; buffer2[i]; i++) {
            buffer2[i] = tolower(buffer2[i]);
        }
        // read again if we read \n and space
        while (strcmp(buffer1, "") == 0){
            // read lines until one of the files is finished
            r1 = read(openFile1, buffer1, 1);
            checkIntegrityForOneMinus(r1);
            if (r1 == 0){
                break;
            }
            removeEntersAndSpaces(buffer1);
            // turn any up letter into a low letter
            for (i = 0; buffer1[i]; i++) {
                buffer1[i] = tolower(buffer1[i]);
            }
        }
        while (strcmp(buffer2,"") == 0){
            // read lines until one of the files is finished
            r2 = read(openFile2, buffer2, 1);
            checkIntegrityForOneMinus(r2);
            if (r2 == 0){
                break;
            }
            removeEntersAndSpaces(buffer2);
            // turn any up letter into a low letter
            for (i = 0; buffer2[i]; i++) {
                buffer2[i] = tolower(buffer2[i]);
            }
        }
        // compare the buffers
        if (strcmp(buffer1, buffer2) != 0) {
            int closeFile1 = close(openFile1);
            checkIntegrityForOneMinus(closeFile1);
            int closeFile2 = close(openFile2);
            checkIntegrityForOneMinus(closeFile2);
            return 0;
        }
        r1 = read(openFile1, buffer1, 1);
        checkIntegrityForOneMinus(r1);
        r2 = read(openFile2, buffer2, 1);
        checkIntegrityForOneMinus(r2);
    }
    int closeFile1 = close(openFile1);
    checkIntegrityForOneMinus(closeFile1);
    int closeFile2 = close(openFile2);
    checkIntegrityForOneMinus(closeFile2);
    return 1;
}
