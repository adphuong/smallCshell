/*****************************************************************************
 * File:            smallsh.h
 * Assignment:      03 - smallsh
 * Author:          April Phuong
 * Date:            January 26, 2022
 * Description:     This is the header file for our smallsh program,
 *                  it contains all the function prototypes and struct.
 ****************************************************************************/

#ifndef SMALLSH_H
#define SMALLSH_H
#define _GNU_SOURCE

#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAXLENGTH 2048
#define MAXARGS 512

/* Struct containing all the different elements included in a command */
struct command {
    char *args;
    char *inputFile;
    char *outputFile;
    int bgFlag;
    int pid;
    struct command *next;
};

struct command *createCommand(char *currLine);
struct command *destroyCommand();
void startSmallSh();
struct command *promptForCommand();
#endif