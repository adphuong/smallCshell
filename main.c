/*****************************************************************************
 * File:           main.c
 * Assignment:     03 - smallsh
 * Author:         April Phuong
 * Date:           February 9, 2022
 * Description:    This program is a small command line application that 
 *                 provides a subset of features of well-known shells such
 *                 as the following built-in commands: cd, status, and exit. 
 *		   Other commands are also implemented and support input and
 *		   output redirection. Commands can execute either in the 
 * 	           foreground or background. Variable expansion is provided for 
 *                 instances of "$$", which will give the current program's PID.
 *		   There are also two custom handlers implemented - SIGNINT 
 *                 and SIGTSTP.
 ****************************************************************************/

#include "smallCshell.c"


/*****************************************************************************
* This is the main driver of the program. Functions are found in smallsh.c
****************************************************************************/
int main(int argc, char *argv[]) {
	// Instantiate our command struct
	struct command *com = malloc(sizeof(struct command));;                 

	// Run our smallsh with command
	startSmallSh(com);

	// Destroy dynamic allocation
	destroyCommand(com);

	free(com);

	return EXIT_SUCCESS;
}
