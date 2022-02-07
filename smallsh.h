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
int bgTracker = 1;
int status = 0;

/* Struct containing all the different elements included in a command */
struct command {
    char *args[MAXARGS];
    char *inputFile;
    char *outputFile;
    int bgFlag;
    int pid;
    int argsIndex;
    struct command *next;
};

// Function prototypes
void catchSIGINT(int signo);
void catchSIGTSTP(int signo);
void cdCommand(struct command *com);
struct command *createCommand(char *currLine);
struct command *destroyCommand();
void executeCommand(struct command *com, struct sigaction sigINT_action);
void exitCommand();
char *expOfPID(int PID, const char* argStr, const char* orig);
struct command *promptForCommand();
void startSmallSh();
void statusCommand(int childExitMethod);


#endif