/*****************************************************************************
 *  File:           smallsh.c
 *  Assignment:     03 - smallsh
 *  Author:         April Phuong
 *  Date:           February 7, 2022
 *  Description:    This file contains the following functions used for the
 *                  program (in alphabetical order):
 *                      catchSIGINT();
 *                      catchSIGTSTP();
 *                      cdCommand();
 *                      createCommand();
 *                      destroyCommand();
 *                      executeCommand();
 *                      exitCommand();
 *                      expOfPID();
 *                      promptForCommand();
 *                      startSmallSh();
 *                      statusCommand();
 ****************************************************************************/

#include "smallsh.h"


/*****************************************************************************
 * Sets up our struct command and signal handlers, then runs our smallsh with
 * the command entered by the user.
 *
 * @params:   none
 * @returns:  none
 ****************************************************************************/
void startSmallSh() {
    struct command *com;                 // Instantiate our command struct
    int runShell = 1;

    // Set up sigaction - this was taken from SIGNALS module
    struct sigaction sigINT_action = {{0}}, sigTSTP_action = {{0}};

    sigINT_action.sa_handler = catchSIGINT;
    sigfillset(&sigINT_action.sa_mask);
    sigINT_action.sa_flags = 0;

    sigTSTP_action.sa_handler = catchSIGTSTP;
    sigfillset(&sigTSTP_action.sa_mask);
    sigTSTP_action.sa_flags = 0;

    sigaction(SIGINT, &sigINT_action, NULL);
    sigaction(SIGTSTP, &sigTSTP_action, NULL);

    while (runShell == 1) { 
        // Prompt for command and store in our command struct
        com = promptForCommand();
        char **argsPtr = com->args;

        //check if we need to use a builtin
        if ( (strcmp(argsPtr[0], "exit") == 0)   || 
             (strcmp(argsPtr[0], "status") == 0) || 
             (strcmp(argsPtr[0], "cd") == 0)){

            if ((strcmp(argsPtr[0], "exit") == 0)){
                fflush(stdout);
                runShell = 0;
                exitCommand();

            }
            else if ((strcmp(argsPtr[0], "status") == 0)){
                statusCommand(status);
            }
            else if ((strcmp(argsPtr[0], "cd") == 0)){
                cdCommand(com);
            }
        }
        else {
            executeCommand(com, sigINT_action);
        }
    }    
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
		printf(": ");
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
        if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " \n");
            currCom->outputFile = calloc(strlen(token) + 1, sizeof(char));
            strcpy(currCom->outputFile, token);
        }
        else if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " \n");
            currCom->inputFile = calloc(strlen(token) + 1, sizeof(char));
            strcpy(currCom->inputFile, token);
        }
        else if (strcmp(token, "&") == 0) {
            currCom->bgFlag = 1;
            bgTracker = 1;
        }
        else if(strstr(token, "$$") != NULL) {
            char stringPID[512];
            char **argsPtr = currCom->args;

            strcpy(stringPID, token);
            currCom->args[argsIndex] = strdup(expOfPID(getpid(), stringPID, "$$")); 

            printf("%s\n", argsPtr[argsIndex]);  
            fflush(stdout);

            argsIndex++;
        }   
        else {
            // This is an argument, so we allocate memory and store it in our 
            // args array in our struct command
            currCom->args[argsIndex] = calloc(strlen(token) + 1, sizeof(char));
            strcpy(currCom->args[argsIndex], token);

            // Increment count for num of args and save this in our command struct
            argsIndex++;
            currCom->argsIndex = argsIndex;
        }
        token = strtok(NULL, " \n");
    }

    // Set the next node to NULL in the newly created command
    currCom->next = NULL;

    return currCom;
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
    exit (0);
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


/*****************************************************************************
 * This is a built-in command. It changes the working directory of smallsh.
 *
 * @params:   struct command
 * @returns:  none
 ****************************************************************************/
void cdCommand(struct command *com) {
    char **argsPtr = com->args;

    if (com->argsIndex == 2) {
        chdir(argsPtr[1]);
    }
    else {
        chdir(getenv("HOME"));
    }
}


/*****************************************************************************
 * Expands the '$$' in a command into the process ID of the smallsh itself.
 *
 * @params:   int PID - PID of smallsh
 *            pointer to cstring argStr - input entered by user with the '$$' 
 *            pointer to cstring orig - original input
 * @returns:  pointer to cstring with the '$$' expanded
 ****************************************************************************/
char *expOfPID(int PID, const char* argStr, const char* orig) {
    int pidLen;
    int origLen;
    char spid[100];
    char *stringPID;

    // Convert PID to string
    sprintf(spid, "%d", PID);                                                    

    // Get the length of converted PID and the '$$' (orig)
    origLen = strlen(orig);
    pidLen = strlen(spid);

    int i;
    int indCount = 0;
    int count = 0;

    // Traverse the string that is passed in and remove the '$$'
    for(i = 0; argStr[i] != '\0'; i++) {
        // Check for the '$$' and remove it by updating counters
        if(strstr(&argStr[i], orig) == &argStr[i]) {
            i = i + origLen - 1;

            count++;
        }
    }

    stringPID = (char*)malloc(i + count * (pidLen - origLen) + 1);

    // Store the new string with the PID (without the '$$')
    while(*argStr) {
        // Add PID to the end of the string, without the '$$'
        if(strstr(argStr, orig) == argStr) {
            strcpy(&stringPID[indCount], spid);

            // Take into account the newly added strings
            indCount = indCount + pidLen;
            argStr = argStr + origLen;
        }
        // Continue traversing through the string
        else {
            stringPID[indCount] = *argStr;

            indCount++;
            argStr++;
        }
    }

    // Add NULL to end of array
    stringPID[indCount] = '\0';

    return stringPID;
}


/*****************************************************************************
 * When a non-built in command is received, smallsh will fork off a child,
 * and the child will use execvp() to run the command
 *
 * @params:   command struct and signal handler for SIGINT
 * @returns:  none
 ****************************************************************************/
void executeCommand(struct command *com, struct sigaction sigINT_action) {
    int execStatus = 0;
    int result = 0;
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
            // If there is no bg flag, child will get default SIGINT
            if (com->bgFlag == 0) {
                sigINT_action.sa_handler = SIG_DFL;
                sigaction(SIGINT, &sigINT_action, NULL);
            }

            // Set redirection for stdout before calling exec
            if (com->outputFile != NULL) {

                // Open our target file
                int targetFD = open(com->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                // Error in opening file
                if (targetFD == -1) {
                    printf("%s: no such file or directory\n", argsPtr[0]);
                    fflush(stdout);
                    // status = 1;

                    _exit(1);
                }

                // Redirect our stdout (1) to targetFD
                result = dup2(targetFD, 1);

                // Error with redirecting stdout
                if (result == -1) {
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
                    // status = 1;

                    _exit(1);
                }

                // Redirect our stdin (0) to sourceFD
                result = dup2(sourceFD, 0);

                // dup2 error, set status to 1 and exit
                if (result == -1) {
                    perror("Error! dup2 failed to redirect\n");
                    fflush(stdout);
                    // status = 1;
                    _exit(1);
                }

                // Close on exec
                fcntl(sourceFD, F_SETFD, FD_CLOEXEC);
            } 

            execStatus = 0;
            int aIndex = com->argsIndex;
            argsPtr[aIndex] = NULL;

            // Execute non-built in command
            execStatus = execvp(argsPtr[0], argsPtr);

            // Check for error
            if (execStatus == -1) {
                printf("%s: no such file or directory\n", argsPtr[0]);
                fflush(stdout);
                // status = 1;

                _exit(1);
            }

            exit(0);
            break;
        // Parent process is here
        default:
            // This will launch background process if both vars are true
            if (com->bgFlag == 1 && bgTracker == 1) {
                // Prevents from blocking so that parent process can go on 
                // with other tasks while child process is still running. 
                // If child dies, its PID will be returned
                waitpid(spawnPID, &status, WNOHANG);
                printf("background pid is %d\n", spawnPID);
                fflush(stdout);
            }
            else {
                // Otherwise, we launch a foreground process - we do
                // nothing while child process is running
                waitpid(spawnPID, &status, 0);
            }
            break;
    } 
    pid_t childPID_exit = waitpid(-1, &status, WNOHANG);

    // Continue until all background processes have completed
    while (childPID_exit > 0) {
        printf("background pid %d is done: ", childPID_exit);
        fflush(stdout);

        // Get exit status and print
        statusCommand(status);

        // Get next child process
        childPID_exit = waitpid(-1, &status, WNOHANG);
    }
    
}


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

    char *message = "terminated by signal 2";
    write(1, message, 22);
    
    printf("\n");
    fflush(stdout);
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
    switch(bgTracker) {
        case 0:
            // Background process is allowed to run
            bgTracker = 1;
            char *exitFgMsg = "Exiting foreground-only mode.\n";
            write(1, exitFgMsg, 30);

            break;
        case 1:
            // Background process is not allowed
            bgTracker = 0;
            char *enterFgMsg = "Entering foreground-only mode (& is no ignored)\n";
            write(1, enterFgMsg, 48);
            break;
    }
}