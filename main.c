/*****************************************************************************
 * File:           main.c
 * Assignment:     03 - smallsh
 * Author:         April Phuong
 * Date:           January 26, 2022
 * Description:    This program declares a struct to define a Movie object with
 *                 title and year data.
 *                 It prompts the user with a main menu that allows them to
 *                 either select a file to process or exit the program.
 *                 If option 1 is chosen then a submenu will be displayed:
 *                     Enter 1 to pick the largest file
 *                     Enter 2 to pick the smalest file
 *                     Enter 3 to specify the name of a file
 ****************************************************************************/

#include "smallsh.c"

int main(int argc, char *argv[]) {
    struct command *com = malloc(sizeof(struct command));;                 // Instantiate our command struct

    startSmallSh(com);

    destroyCommand(com);

    free(com);

    return EXIT_SUCCESS;
}