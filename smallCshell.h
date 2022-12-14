/*****************************************************************************
 * File:            smallCshell.h
 * Assignment:      03 - smallsh
 * Author:          April Phuong
 * Date:            February 9, 2022
 * Description:     This is the header file for our smallsh program -
 *                  it contains all the function prototypes and struct.
 *                  Documentation of functions found in smallsh.c.
 ****************************************************************************/

#ifndef SMALLCSHELL_H
#define SMALLCSHELL_H
#define _GNU_SOURCE

// Libraries used
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h> 
#include <errno.h>
#include <dirent.h>
#include <pthread.h>
#include <assert.h>
#include <sys/wait.h>
#include <signal.h>


#define MAXLENGTH 2048
#define MAXARGS 512
#define ERR -1

// Global Vars
int bgEnabled = 1;
int status = 0;

/* Struct containing all the different elements included in a command */
struct command {
    char *args[MAXARGS];
    char *inputFile;
    char *outputFile;
    int bgFlag;
    int argsIndex;
    struct command *next;
};

// Function prototypes
void catchSIGINT(int signo);
void catchSIGTSTP(int signo);
void cdCommand(struct command *com);
char *expandDollarSigns(int PID, const char* argStr);
struct command *createCommand(char *currLine);
void destroyCommand(struct command *com);
void executeCommand(struct command *com, struct sigaction sigINT_action, struct sigaction sigTSTP_action);
void exitCommand();
struct command *promptForCommand();
void startSmallSh(struct command *com);
void statusCommand(int childExitMethod);


#endif
