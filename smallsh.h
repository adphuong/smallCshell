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

struct command *createCommand(char *currLine);
struct command *destroyCommand();
void startSmallSh();
struct command *promptForCommand();
void statusCommand(int childExitMethod);
void exitCommand();
void cdCommand(struct command *com);
void cd(char * path);
char* expOfPID(int PID, const char* argStr, const char* orig);
void executeCommand(struct command *com, struct sigaction sigINT_action);
void catchSIGINT(int signo);
void catchSIGTSTP(int signo);

#endif