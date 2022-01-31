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
// struct command *createCommand(char *currLine) {
//     // Initialize our values
//     struct command *com = malloc(sizeof(struct command));
//     char *saveptr = NULL;
//     char *token = NULL;

//     // The first token is the title
//     token = strtok_r(currLine, ",", &saveptr);
//     currMovie->title = calloc(strlen(token) + 2, sizeof(char));
//     strcpy(currMovie->title, token);

//     // The next token is the year
//     token = strtok_r(NULL, ",", &saveptr);
//     currMovie->year = atoi(token);

//     // Set the next node to NULL in the newly created student entry
//     currMovie->next = NULL;

//     return currMovie;
// }


void startSmallSh() {
    struct command *com;                 // Instantiate our command struct
    // char *argsArr[512];

    com = promptForCommand();
    printf("userInput = %s\n", com->args);
}

struct command *promptForCommand() {
    char userInput[2048];
    size_t len = 0;
    size_t nread = 0;
    int commandIndex = 0;

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
		nread = fgets(userInput, MAXLENGTH, stdin);
	}

    // Read the file line by line
    if (nread != -1) {

        // Get a new movie node corresponding to the current line
        struct command *newNode = createCommand(userInput);
        commandIndex++;

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

    token = strtok(currInput, " \n");

	while (token != NULL) {
        if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " \n");
            currCom->outputFile = calloc(strlen(token) + 2, sizeof(char));
            strcpy(currCom->outputFile, token);
        }
        else if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " \n");
            currCom->inputFile = calloc(strlen(token) + 2, sizeof(char));
            strcpy(currCom->inputFile, token);
        }
        else if (strcmp(token, "&") == 0) {
            currCom->bgFlag = 1;
        }
        else if(strcmp(token, "$$") == 0) {
            currCom->pid = getpid();
        }
        else {
            currCom->args = calloc(strlen(token) + 2, sizeof(char));
            strcpy(currCom->args, token);
        }


        token = strtok(NULL, " \n");
    }

    // Set the next node to NULL in the newly created student entry
    currCom->next = NULL;

    return currCom;
}