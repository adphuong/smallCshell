/*****************************************************************************
 * File:           main.c
 * Assignment:     03 - smallsh
 * Author:         April Phuong
 * Date:           February 9, 2022
 * Description:    This program declares a struct to define a command object 
 *                 with args, inputFile, outputFile, bgFlag, and argsIndex
 * 				   data.
 *                 It provides a subset of features of well-known shells such
 *                 as built-in commands: cd, status, and exit. It allows
 *                 the user to run non-builtin commands by calling fork(), 
 * 				   and the execvp function and supports running processes in 
 * 				   the foreground and background. There are also two custom
 * 				   handlers implemented - SIGNINT and SIGTSTP.
 ****************************************************************************/

#include "smallsh.c"

int main(int argc, char *argv[]) {
	// Instantiate our command struct
	struct command *com = malloc(sizeof(struct command));;                 

	// Run our smallsh
	startSmallSh(com);

	destroyCommand(com);

	free(com);

	return EXIT_SUCCESS;
}