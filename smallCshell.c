/*****************************************************************************
 *  File:           smallCshell.c
 *  Assignment:     03 - smallsh
 *  Author:         April Phuong
 *  Date:           February 9, 2022
 *  Description:    This file contains the following functions used for the
 *                  program (in alphabetical order):
 *                      catchSIGINT();
 *                      catchSIGTSTP();
 *                      cdCommand();
 *                      createCommand();
 *                      destroyCommand();
 *                      executeCommand();
 *                      exitCommand();
 *                      expandDollarSigns();
 *                      promptForCommand();
 *                      startSmallSh();
 *                      statusCommand();
 ****************************************************************************/

#include "smallCshell.h"


/*****************************************************************************
 * Custom handler for the CTRL-C command (SIGINT). Only the child running
 * as a foreground process will terminate itself when this SIGINT is 
 * received. The parent process and any children running in the background
 * will ignore this signal. 
 *
 * @params:   int signo - signal number
 * @returns:  none
 ****************************************************************************/
void catchSIGINT(int signo) {
    char *msg = "\n";
    write(1, msg, 1);
}


/*****************************************************************************
 * Custom handler for the CTRL-Z command (SIGTSTP). Only the parent process 
 * running the shell will receive this signal. Shell will display an
 * informative message and enters a state where commands will no longer be 
 * run in the background ('&' is ignored). All commands will run as 
 * foreground processes.
 *
 * @params:   int signo - signal number
 * @returns:  none
 ****************************************************************************/
void catchSIGTSTP(int signo) {
    if (bgEnabled == 0) {
        // Background process is allowed to run - set to true
        bgEnabled = 1;
        char *exitFgMsg = "Exiting foreground-only mode\n";
        write(1, exitFgMsg, 29);
    }
    else {
        // Background process is not allowed - set to false
        bgEnabled = 0;
        char *enterFgMsg = "Entering foreground-only mode (& is now ignored)\n";
        write(1, enterFgMsg, 49);  
    }
}

/*****************************************************************************
 * This is a built-in command. It changes the working directory of smallsh.
 *
 * @params:   struct command
 * @returns:  none
 ****************************************************************************/
void cdCommand(struct command *com) {
    char **argsPtr = com->args;

    // If there is an arg after 'cd', change directory to that arg
    if (com->argsIndex == 2) {
        chdir(argsPtr[1]);
    }
    // Change to HOME directory
    else {
        chdir(getenv("HOME"));
    }
}


/*****************************************************************************
 * Create our new struct command that will hold all the elements entered
 * by the user - arguments (up to 512), inputFile, outputFile, bgFlag, and
 * argsIndex (keeps track of num of args entered).
 *
 * @params:   c-string pointer of input (command) entered by user
 * @returns:  struct command
 ****************************************************************************/
struct command *createCommand(char *currInput) {
    // Initialize our values
    struct command *currCom = malloc(sizeof(struct command));
    char *token = NULL;
    int argsIndex = 0;

    token = strtok(currInput, " \n");

    while (token != NULL) {
        // Checking for output file redirection
        if (strcmp(token, ">") == 0) {
            // Store input file without the '>' into our struct
            token = strtok(NULL, " \n");
            currCom->outputFile = calloc(strlen(token) + 1, sizeof(char));
            strcpy(currCom->outputFile, token);
        }
        // Checking for input file redirection
        else if (strcmp(token, "<") == 0) {
            // Store input file without the '<' into our struct
            token = strtok(NULL, " \n");
            currCom->inputFile = calloc(strlen(token) + 1, sizeof(char));
            strcpy(currCom->inputFile, token);
        }
        // Replace the '$$' with the pid and store in struct
        else if (strcmp(token, "$$") == 0) {
            // Holds the string with expanded PID     
            
            char strPID[20];

            // Get PID and convert it to a string
            sprintf(strPID, "%d", getpid()); 

            currCom->args[argsIndex] = calloc(strlen(token) + 1, sizeof(char));
            strcpy(currCom->args[argsIndex], strPID);

            // Increment count for num of args and save this in our command struct
            argsIndex++;
            currCom->argsIndex = argsIndex;
        }
        // This is an arg, might also contain the '$$'
        else {
            // There is an occurrence of '$$' in the string token
            if (strstr(token, "$$") != NULL) {

                // Expand the '$$' to the PID into the string, allocate memory
                // and store into our struct
                char *expandedString = expandDollarSigns(getpid(), token);
                currCom->args[argsIndex] = calloc(strlen(token) + 1, sizeof(char));
                strcpy(currCom->args[argsIndex], expandedString);

                // Increment count for num of args and save this in our command struct
                argsIndex++;
                currCom->argsIndex = argsIndex;
            }
            // No expansion of '$$' needed, just store the arg
            else {
                // Allocate memory and copy token into struct for args
                currCom->args[argsIndex] = calloc(strlen(token) + 1, sizeof(char));
                strcpy(currCom->args[argsIndex], token);

                // Increment count for num of args and save this in our command struct
                argsIndex++;
                currCom->argsIndex = argsIndex;
            }
            
        }        
        token = strtok(NULL, " \n");
    }

    // Check to see if last arg is '&' so we can set our bgFlag
    if (strcmp(currCom->args[argsIndex - 1], "&") == 0) {
        // We don't need to store this in our args array, so set to NULL
        // and update argsIndex counter
        currCom->args[argsIndex - 1] = NULL;
        currCom->argsIndex--;

        // Background processes are enabled
        currCom->bgFlag = 1;
    }

    // Set the next node to NULL in the newly created command
    currCom->next = NULL;

    return currCom;
}


/*****************************************************************************
 * Free up memory that was created to create our struct commmand
 *
 * @params:   none
 * @returns:  none
 ****************************************************************************/
void destroyCommand(struct command *com) {
    char **argsPtr = com->args;

    // Traverse through command struct and free up memory
    if (com->outputFile) {
        free(com->outputFile);
    }
    if (com->inputFile) {
        free(com->inputFile);
    }
    if (argsPtr) {
        int i = 0;
        while (argsPtr[i]) {
            free(argsPtr[i]);

            i++;
        }
    }
}


/*****************************************************************************
 * When a non-built in command is received, smallsh will fork off a child,
 * and the child will use execvp() to run the command
 *
 * @params:   command struct and signal handler for SIGINT and SIGTSTP
 * @returns:  none
 * @source:   https://youtu.be/1R9h-H2UnLs (3.1 Processes Lecture)
 *            https://youtu.be/9Gsp-wucTNw (3.4 More UNIX IO)
 ****************************************************************************/
void executeCommand(struct command *com, struct sigaction sigINT_action, 
                    struct sigaction sigTSTP_action) {
    int execStatus = 0;
    int redirection = 0;
    pid_t spawnPID = fork();
    char **argsPtr = com->args;

    // Determines if fork was successful and acts accordingly
    switch(spawnPID) {
        // Fork failed, print error and set status to 1
        case -1:
            perror("Error! Failed to fork.\n");
            fflush(stdout);
            status = 1;

            break;
        // Fork Successful, child will execvp() command
        case 0:
            // The ignore_action struct as SIG_TSTP as its signal handler
            sigTSTP_action.sa_handler = SIG_IGN;

            // If there background processes are not enabled,
            // child will get default SIGINT
            if (com->bgFlag == 0 || bgEnabled == 0) {
                sigINT_action.sa_handler = SIG_DFL;
                sigaction(SIGINT, &sigINT_action, NULL);
            }

            // Redirect to '/dev/null' if background processes are allowed
            if (com->bgFlag == 1 && bgEnabled == 1) {
                // Redirect to /dev/null if no input file entered by user
                if (com->inputFile == NULL) {

                    // Open file for read only
                    int tempIn = open("/dev/null", O_RDONLY);

                    // Error handling
                    if (tempIn == -1) {
                        perror("Error, cannot set /dev/null to input\n");
                        fflush(stdout);
                        _exit(1);
                    }
                    if (dup2(tempIn, STDIN_FILENO) == -1) {
                        perror("Error, dup2 failed to redirect\n");
                        fflush(stdout);
                        _exit(1);
                    }
                    // Close on exec
                    fcntl(tempIn, F_SETFD, FD_CLOEXEC);
                }
                // Redirect to /dev/null if no output file entered by user
                if (com->outputFile == NULL) {

                    // Open file for read and write
                    int tempOut = open("/dev/null", O_RDWR, 0644);

                    // Error handling
                    if (tempOut == -1) {
                        perror("Error, cannot set /dev/null to output\n");
                        fflush(stdout);
                        _exit(1);
                    }
                    if (dup2(tempOut, STDOUT_FILENO) == -1) {
                        perror("Error, dup2 failed to redirect\n");
                        fflush(stdout);
                        _exit(1);
                    }
                    // Close on exec
                    fcntl(tempOut, F_SETFD, FD_CLOEXEC);
                }
            }

            // Set redirection for stdout before calling exec
            if (com->outputFile != NULL) {

                // Open our target file
                int targetFD = open(com->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                fflush(stdout);

                // Error in opening file
                if (targetFD == -1) {
                    printf("%s: no such file or directory\n", argsPtr[0]);
                    fflush(stdout);

                    _exit(1);
                }

                // Redirect our stdout (1) to targetFD
                redirection = dup2(targetFD, 1);

                // Error with redirecting stdout
                if (redirection == -1) {
                    perror("Error! dup2 failed to redirect\n");
                    fflush(stdout);
                    _exit(1);
                }

                // Close on exec
                fcntl(targetFD, F_SETFD, FD_CLOEXEC);
            }

            // Set redirection for stdin before calling exec
            if (com->inputFile != NULL) {

                // Open our source file
                int sourceFD = open(com->inputFile, O_RDONLY);

                // Error in opening source file
                if (sourceFD == -1) {
                    printf("cannot open %s for input\n", com->inputFile);
                    fflush(stdout);

                    _exit(1);
                }

                // Redirect our stdin (0) to sourceFD
                redirection = dup2(sourceFD, 0);

                // dup2 error, set status to 1 and exit
                if (redirection == -1) {
                    perror("Error! dup2 failed to redirect\n");
                    fflush(stdout);

                    _exit(1);
                }

                // Close on exec
                fcntl(sourceFD, F_SETFD, FD_CLOEXEC);
            } 

            // Execute non-built in command
            execStatus = execvp(argsPtr[0], argsPtr);

            // Check for error
            if (execStatus == -1) {
                printf("%s: no such file or directory\n", argsPtr[0]);
                fflush(stdout);

                _exit(2);
            }
            break;
            
        // Parent process is here
        default:

            // Background processes are enabled - we want to set it up
            // so that parent process can continue with other tasks while
            // child process is still running
            if (com->bgFlag == 1 && bgEnabled == 1) {

                // If child dies, its PID will be returned
                waitpid(spawnPID, &status, WNOHANG);
                printf("background pid is %d\n", spawnPID);
                fflush(stdout);
            }
            else {
                // Otherwise, we launch a foreground process - we do
                // nothing until child process is done running
                waitpid(spawnPID, &status, 0);
            }
            break;
    }

    // Check for any child processes (in bg) that have terminated
    while ((spawnPID = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("background pid %d is done: ", spawnPID);
        fflush(stdout);

        // Get exit status and print
        statusCommand(status);
    }
}


/*****************************************************************************
 * This is a built-in command. If user enters 'exit', this will exit our
 * smallsh program.
 *
 * @params:   none
 * @returns:  none
 ****************************************************************************/
void exitCommand() {

    // Exit the shell
    exit(0);
}


/*****************************************************************************
 * Expands the '$$' in a command into the process ID of the smallsh itself.
 *
 * @params:   int PID - PID of smallsh
 *            pointer to cstring argStr - input entered by user with the '$$' 
 * @returns:  pointer to cstring with the '$$' expanded
 * @source:   https://stackoverflow.com/questions/12784766/check-substring-
 *            exists-in-a-string-in-c/12784812 (checking for substring '$$')
 ****************************************************************************/
char *expandDollarSigns(int PID, const char* argStr) {
    char strPID[20];
    char *strFound;
    int unexpandedVarLength = 2;        // Length of '$$' is 2

    // Convert PID int to string
    sprintf(strPID, "%d", PID);                                                    

    // Get the length of converted PID
    int pidLen = strlen(strPID);

    // Setup counters
    int arrInd = 0;
    int count = 0;

    // Traverse the string that is passed in and count the number 
    // of occurrences for our substring '$$'
    int i = 0;
    while (argStr[i] != '\0') {
        // Check to see if substring '$$' is in argStr
        strFound = strstr(&argStr[i], "$$");

        // Check for the '$$'. If found, update counters that will be used
        // to allocate memory for our expanded string
        if(strFound == &argStr[i]) {
            i = i + unexpandedVarLength - 1;

            // Counter for '$$' found in the string
            count++;
        }
        i++;
    }

    // Allocate memory to store expanded string in var
    char *stringPIDexpanded = (char*) malloc(i + count * (pidLen - unexpandedVarLength) + 1);

    // Store the new string with the PID (the '$$' is expanded)
    while(*argStr != '\0') {
        // If substring is found, strstr will return a pointer to where
        // this substring begins
        strFound = strstr(argStr, "$$");

        // Substring '$$' not found, continue traversing through argStr
        if(strFound != argStr) {
            // Store whatever argStr holds into stringPIDexpanded
            stringPIDexpanded[arrInd] = *argStr;

            // Update counters
            arrInd++;
            argStr++;            
        }
        // Substring '$$' found, add PID to the string, without the '$$'
        else {
            // Copy the PID (of string type) into new variable
            strcpy(&stringPIDexpanded[arrInd], strPID);

            // Update arrInd counter to accommodate the newly 
            // added PID string
            arrInd = arrInd + pidLen;

            // Move our argStr pointer forward by 2, since
            // we need to continue traversing argStr after
            // running into '$$' occurrence
            argStr = argStr + unexpandedVarLength;
        }
    }

    // Set last index to newline
    stringPIDexpanded[arrInd] = '\0';

    return stringPIDexpanded;
}


/*****************************************************************************
 * Prompts user for command, creates a new command struct, and returns 
 * this new struct
 *
 * @params:   none
 * @returns:  struct command with elements entered by user
 ****************************************************************************/
struct command *promptForCommand() {
    char userInput[2048];
    size_t nread = 0;

    struct command *head = NULL;      // The head of the linked list
    struct command *tail = NULL;      // The tail of the linked list

    // Prompt user for input and store in variable
    printf(": ");
    fflush(stdout);

    // Get user command
    fgets(userInput, MAXLENGTH, stdin);

    // Keep prompting user for command if blank lines or '#' entered,
    // these be ignored by the shell
    while (strcmp(userInput, "\n") == 0 || userInput[0] == '#'){
        //print out the prompt for the user
        printf("\n: ");
        fflush(stdout);

        // Read input from
        nread = (size_t) fgets(userInput, MAXLENGTH, stdin);
    }

    // If read was successful, we create new struct command
    if (nread != -1) {

        // Get a new command node corresponding to the command entered
        struct command *newNode = createCommand(userInput);

        // If head is the first node of the list
        if (head == NULL) {
            // This is the first node in the linked list
            // Set the head and the tail to this node
            head = newNode;
            tail = newNode;
        }
        else {
            // This is not the first node.
            // Add this node to the list and advance the tail
            tail->next = newNode;
            tail = newNode;
        }
    }

    return head;
}


/*****************************************************************************
 * Sets up our struct command and signal handlers, then runs our smallsh with
 * the command entered by the user.
 *
 * @params:   none
 * @returns:  none
 * @source:   https://youtu.be/VwS3dx3uyiQ (3.3 Signals)
 ****************************************************************************/
void startSmallSh(struct command *com) {
    int runShell = 1;

    // Set up sigaction - this was taken from SIGNALS module
    struct sigaction sigINT_action = {{0}}, sigTSTP_action = {{0}};

    // Set up handler for SIGINT signal
    sigINT_action.sa_handler = catchSIGINT;
    sigfillset(&sigINT_action.sa_mask);
    sigINT_action.sa_flags = SA_RESTART;

    // Set up handler for SIGTSTP signal
    sigTSTP_action.sa_handler = catchSIGTSTP;
    sigfillset(&sigTSTP_action.sa_mask);
    sigTSTP_action.sa_flags = SA_RESTART;

    sigaction(SIGINT, &sigINT_action, NULL);
    sigaction(SIGTSTP, &sigTSTP_action, NULL);

    while (runShell == 1) { 
        // Prompt for command and store in our command struct
        com = promptForCommand();

        // Used to get args data from our struct
        char **argsPtr = com->args;

        // Check for builtin commands - 'exit', 'status', and 'cd'
        // Check for 'exit' built-in command
        if ((strcmp(argsPtr[0], "exit") == 0)){
            runShell = 0;
            exitCommand();
        }
        // Check for 'status' built-in command
        else if ((strcmp(argsPtr[0], "status") == 0)){
            statusCommand(status);
        }
        // Check for 'cd' built-in command
        else if ((strcmp(argsPtr[0], "cd") == 0)){
            cdCommand(com);
        }
        // Execute a non-built in command
        else {
            executeCommand(com, sigINT_action, sigTSTP_action);
        }
    }
}


/*****************************************************************************
 * This is a built-in command.
 * Prints out either the exit status or the terminating signal of the last
 * foreground process ran by our shell.
 *
 * @params:   int indicating the status
 * @returns:  none
 ****************************************************************************/
void statusCommand(int status) {
    int exitStatus = 0;

    // Process was terminated by a signal
    if (!WIFEXITED(status)){
        exitStatus = WTERMSIG(status);

        printf("terminated by signal %i\n", exitStatus);
        fflush(stdout);
    }
    // Process exited normally
    else {
        exitStatus = WEXITSTATUS(status);

        printf("exit value %i\n", exitStatus);
        fflush(stdout);
    }
}


