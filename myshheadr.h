#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <glob.h>

#define BUF 256

typedef struct myHistory {
    char h[20][BUF];
    int top;
    int size;
}myHistory;


//  Write shell prompt
void curr_path();


//  Checks whether "exit" has been inputted or not. Returns 0 for false, 1 for true.
int is_exit(char* input);


// Similar to strtok, but after dividing src the remaining string goes to dest. Returns the same as strtok.
char* strsplt(char* src, char* splitter, char* dest);


//  Checks for redirections ('<', '>', '>>') in command. Replaces '<' with ' '.
//  Returns 1 if '>' has been found, 2 for '>>' and -1 otherwise.
int redir_detect(char*);


//  Operates the "myHistory" command. For each input:
//  if "myHistory" has been called, it does its job how it's supposed to.
//  if input is not a "myHistory" command, current history array gets updated with said command (hist_update).

//  Returns 1 if "myHistory" has been called for printing purposes (no argument), and 0 otherwise.
int hist_call(char* command, myHistory* hist);


//  Updates the history array, which holds the 20 most recent input lines.
void hist_update(myHistory* hist);


// Get nth most recent input line from myHistory
char* hist_get(myHistory* hist, int pos);


//  Handles wildcard characters
void wildch_matc(char*);


//  Handles the execution of a command
void norm_exec(char* input, int arrow, int* pipe, int pipe_count, int read_or_write);