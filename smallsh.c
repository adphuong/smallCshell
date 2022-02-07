/*****************************************************************************
 *  File:           smallsh.c
 *  Assignment:     03 - smallsh
 *  Author:         April Phuong
 *  Date:           January 26, 2022
 *  Description:    This file contains the following functions used for the
 *                  program (in alphabetical order):
 *                      createMovie()
 *                      createDir()
 *                      executeMainMenu()
 *                      executeSubMenu()
 *                      freeMemory()
 *                      findFile()
 *                      findSmallestBiggestFile()
 *                      getFileName()
 *                      getMainMenuChoice()
 *                      getSubMenuChoice()
 *                      printMainMenu()
 *                      printSubMenu()
 *                      processFile()
 ****************************************************************************/

#include "smallsh.h"

/*****************************************************************************
 * Parse the current line (which is space delimited) and create a
 * movie struct with the data processed from this line
 *
 * @params:   pointer to address of current line of file
 * @returns:  struct containing the data of the movie that was just parsed
 * source:    https://replit.com/@cs344/studentsc#main.c
 ****************************************************************************/

void startSmallSh() {
    struct command *com;                 // Instantiate our command struct
    int runShell = 1;

    // Set up sigaction
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

struct command *promptForCommand() {
    char userInput[2048];
    size_t nread = 0;

    struct command *head = NULL;      // The head of the linked list
    struct command *tail = NULL;      // The tail of the linked list

    // Prompt user for input and store in variable
    printf(": ");
    fflush(stdout);

    fgets(userInput, MAXLENGTH, stdin);
    //if there's nothing, repeat until there is *something*
	while (strcmp(userInput, "\n") == 0 || userInput[0] == '#'){
		//print out the prompt for the user
		printf(": ");
		fflush(stdout);

		//read input from
		nread = (size_t) fgets(userInput, MAXLENGTH, stdin);
	}

    // Read the file line by line
    if (nread != -1) {

        // Get a new movie node corresponding to the current line
        struct command *newNode = createCommand(userInput);
        // commandIndex++;

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

    // printf("input is: %s\n", userInput);

    return head;
}

struct command *createCommand(char *currInput) {
    // Initialize our values
    struct command *currCom = malloc(sizeof(struct command));
    // char *saveptr = NULL;
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
            // char **argsPtr = currCom->args;

            // for (int i = 0 ; i < commandIndex; i++) {
            //     if ((currCom->args[i] = malloc(sizeof(char) * MAXLENGTH)) == NULL) {
            //         printf("unable to allocate memory \n");
            //         fflush(stdout);
            //     }
            // }
            currCom->args[argsIndex] = calloc(strlen(token) + 1, sizeof(char));
            strcpy(currCom->args[argsIndex], token);

            // printf("args[%d] = %s\n", argsIndex, argsPtr[argsIndex]);
            // fflush(stdout);

            argsIndex++;
            currCom->argsIndex = argsIndex;
        }


        token = strtok(NULL, " \n");
    }

    // Set the next node to NULL in the newly created student entry
    currCom->next = NULL;

    return currCom;
}

/******************************** Built-in Commands *********************************/
void exitCommand() {

    // Exit the shell
    exit (0);
}

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

void cdCommand(struct command *com) {
    char **argsPtr = com->args;

    if (com->argsIndex == 2) {
        chdir(argsPtr[1]);
    }
    else {
        chdir(getenv("HOME"));
    }
}

void cd(char * path) {

    // printf("===== [cd] = Called with path %s.\n", path);
    // fflush(stdout);

    if (chdir(path) != 0){

        
        return;
    }

    else{

        chdir(path);
        return;
    }
}

char* expOfPID(int PID, const char* argStr, const char* orig) {
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

    // Traverse the string that's passed in and delete the '$$'
    for(i = 0; argStr[i] != '\0'; i++) {
        // Check for the '$$' and remove it
        if(strstr(&argStr[i], orig) == &argStr[i]) {
            i = i + origLen - 1;

            count++;
        }
    }

    stringPID = (char*)malloc(i + count * (pidLen - origLen) + 1);

    // This is where we store the new string with the PID nums
    while(*argStr) {
        // Add PID to the end of the string, without the '$$'
        if(strstr(argStr, orig) == argStr) {
            strcpy(&stringPID[indCount], spid);

            // Take into account the newly added strings
            indCount = indCount + pidLen;
            argStr = argStr + origLen;
        }
      // If here then just keep moving through the string.*/
        else {
            stringPID[indCount] = *argStr;

            indCount++;
            argStr++;
        }
    }

    stringPID[indCount] = '\0';

    return stringPID;
}

void executeCommand(struct command *com, struct sigaction sigINT_action) {
    int execStatus = 0;
    int result = 0;
    pid_t spawnPID = fork();
    char **argsPtr = com->args;

    switch(spawnPID) {
        case -1:
            perror("Error! Failed to fork.\n");
            fflush(stdout);
            status = 1;

            break;
        case 0:
            // Fork successful

            // If there is no bg flag, child will get default sigint
            if (com->bgFlag == 0) {
                sigINT_action.sa_handler = SIG_DFL;
                sigaction(SIGINT, &sigINT_action, NULL);
            }

            // Need to set redirection for stdout before calling exec
            if (com->outputFile != NULL) {

                // Open our target file
                int targetFD = open(com->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                // Error in opening file
                if (targetFD == -1) {
                    printf("%s: no such file or directory\n", argsPtr[0]);
                    fflush(stdout);
                    status = 1;

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

            // Need to set redirection for stdin before calling exec
            if (com->inputFile != NULL) {
                // Open our source file
                int sourceFD = open(com->inputFile, O_RDONLY);

                // Error in opening source file
                if (sourceFD == -1) {
                    printf("cannot open %s for input\n", argsPtr[0]);
                    fflush(stdout);
                    status = 1;

                    _exit(1);
                }

                // Redirect our stdin (0) to sourceFD
                result = dup2(sourceFD, 0);
                if (result == -1) {
                    perror("Error! dup2 failed to redirect\n");
                    fflush(stdout);
                    status = 1;
                    _exit(1);
                }

                // Close on exec
                fcntl(sourceFD, F_SETFD, FD_CLOEXEC);
            } 

            execStatus = 0;
            int aIndex = com->argsIndex;
            argsPtr[aIndex] = NULL;
            // printf("Righbe before exec. args[0]: %s\n", argsPtr[0]);        //Delete
            execStatus = execvp(argsPtr[0], argsPtr);

            // Check for error
            if (execStatus == -1) {
                printf("%s: no such file or directory\n", argsPtr[0]);
                fflush(stdout);
                status = 1;

                _exit(1);
            }

            exit(0);
            break;
        default:
            // Parent process here

            // This will launch background process
            if (com->bgFlag && bgTracker) {
                waitpid(spawnPID, &status, WNOHANG);
                printf("background pid is %d\n", spawnPID);
                fflush(stdout);
            }
            else {
                // Otherwise, we launch a foreground process
                waitpid(spawnPID, &status, 0);
            }
            break;
    } 
    pid_t childPID_exit = waitpid(-1, &status, WNOHANG);

    while (childPID_exit > 0) {
        printf("background pid %d is done: ", childPID_exit);
        fflush(stdout);
        statusCommand(status);

        childPID_exit = waitpid(-1, &status, WNOHANG);
    }
    
}

void catchSIGINT(int signo) {
    char *message = "terminated by signal 2\n";
    write(1, message, 23);
}


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