#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h> 
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "mush.h"
#define DARRAY 2
#define READ_END 0
#define WRITE_END 1
#define PROMPT "8-P "

void handle_args(int argc, char *argv[], int *infile); 
void print_usage(void);
char *readLongString(FILE *infile); 
pipeline crack_pipeline(char* line); 
void print_pipeline(FILE *where, pipeline c1);
void free_pipeline(pipeline c1);
void handle_mush(FILE *fd, char *readstring, sigset_t *oldMask, sigset_t *newMask);
int create_pipes(int stages, int pipes[][2]);
void handler();
void setup_pipes(int pipe_array[][2], int numPipes, int index);
void closepipes(int pipe_array[][2], int numPipes); 
int in_redirect(char *inname); 
int out_redirect(char *outname); 
void sig_handle(struct sigaction *sa, sigset_t *newMask, sigset_t *oldMask);
void print_prompt(FILE *fd);
 
