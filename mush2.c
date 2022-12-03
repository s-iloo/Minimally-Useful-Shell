#include "mush2.h"

int main(int argc, char *argv[]){
  int infile = 0; 
  FILE* fd = stdin;
  char *readstring = NULL; 
  struct sigaction sa; 
  sigset_t newMask, oldMask; 
  /*handle the arguments */
  handle_args(argc, argv, &infile);
  /*open the file for reading if specified in command line */
  if(infile == 1){
    if(!(fd = fopen(argv[infile], "r"))){
      perror("fopen");
      exit(EXIT_FAILURE); 
    }
  }
  /*handle the signals*/
  sig_handle(&sa, &newMask, &oldMask);
  /*print prompt if not file specified*/ 
  print_prompt(fd); 
  /*while loop that reads input and parses it*/
  while((readstring = readLongString(fd)) != NULL){ 
    print_prompt(fd); 
    /*execute command line arguments*/
    handle_mush(fd, readstring, &oldMask, &newMask);
    /*fflush stdout */
    if(fflush(stdout)==EOF){
      perror("fflush"); 
      exit(EXIT_FAILURE); 
    }
    /*free the read string */
    free(readstring);
    yylex_destroy(); 
  }
  return 0; 
}

void handle_mush(FILE *fd, char *readstring, sigset_t *oldMask, sigset_t *newMask){
  pipeline pipeinfo;
  clstage currstage;
  int (*pipes)[DARRAY];  
  pid_t child;
  int status, i, badform = 0, cd = 0;     
  memset(&currstage, 0, sizeof(currstage)); 
  memset(&pipeinfo, 0, sizeof(pipeinfo)); 
  /*parse the read string */
  pipeinfo = crack_pipeline(readstring);
  
  if(pipeinfo == NULL)
    badform = 1;  
  /*make sure pipeinfo is valid and check if we need pipes */
  if(!badform){
    if(pipeinfo -> length > 1){
      pipes = calloc(((pipeinfo -> length)-1) * DARRAY, sizeof(int)); 
      if(-1 == create_pipes(pipeinfo -> length, pipes)){
        perror("pipes"); 
        exit(EXIT_FAILURE); 
      }  
    }
    /*get the first stage */
    currstage = pipeinfo -> stage; 
    /*loop through all stages */
    for(i = 0; i < pipeinfo -> length; i++){
      if(currstage == NULL){
        perror("currstage"); 
        exit(EXIT_FAILURE); 
      }
      /*check for cd special case */
      if(!strcmp(currstage -> argv[0], "cd")){
        if(currstage -> argc != 2){
          if(-1 == chdir(getenv("HOME"))){
            perror("chdir"); 
            badform = 1; 
          }
          cd = 1;  
        }
        else if(-1 == chdir(currstage -> argv[1])){
          perror("chdir");
          badform = 1;   
        }
        cd = 1;
      }
      else{
        /*start children processes */
        if((child = fork()) == 0){
          /*unblock signal handling in case stop program is needed*/
          if(sigprocmask(SIG_SETMASK, oldMask, NULL) == -1){
            perror("sigprocmask");
            exit(EXIT_FAILURE);
          }
          /*set up the pipes if needed*/
          if(pipeinfo -> length > 1){     
            setup_pipes(pipes, (pipeinfo -> length) -1, i); 
          }
          /*check if we need to use file redirection */
          if(currstage -> inname){
            if(0==in_redirect(currstage -> inname)){
              perror("bad redirect"); 
              badform = 1; 
            }
          }if(currstage -> outname){
            if(0==out_redirect(currstage -> outname)){
              perror("bad redirect");  
              badform = 1; 
            } 
          }
          /*execute the command specified*/
          execvp(currstage -> argv[0], currstage -> argv); 
          perror("bad exec");
          badform = 1;
          _exit(2);
        }
       /*deal with parent*/
        else{
          if(child == -1){
            perror("fork"); 
            exit(EXIT_FAILURE); 
          }
        }
      /*increment to the next stage */
      currstage++;   
      }
    }
    /*close pipes and free pipe memory if needed*/
    if(pipeinfo -> length > 1){
      closepipes(pipes, (pipeinfo -> length)-1);
      free(pipes); 
    }
    /*wait for child- parent only*/
    if(!badform && !cd){
      for(i = 0; i < pipeinfo -> length; i++){
        if((child = wait(&status)) == -1){
          if(errno != EINTR){
            perror("wait"); 
            exit(EXIT_FAILURE); 
          } 
        }
      }
    }
    free_pipeline(pipeinfo);
  }
}

int out_redirect(char *outname){
  int outfd = 0; 
  /*handle file redirection for output */
  outfd = creat(outname, S_IRUSR|S_IWUSR); 
  if(outfd == -1){
    perror("out redirect"); 
    return 0;
  }
  if(dup2(outfd, 1) < 0){
    perror("dup out\n"); 
    return 0; 
  }
  close(outfd); 
  return 1; 

}

int in_redirect(char *inname){
  int infd = 0;
  /*handle file redirection for input*/ 
  infd = open(inname, O_RDONLY); 
  if(infd == -1){
    perror("in redirect"); 
    return 0; 
  }
  if(dup2(infd, 0) < 0){
    perror("dup2");
    return 0; 
  }
  close(infd); 
  return 1; 
}

void setup_pipes(int pipe_array[][2], int numPipes, int index){
  /*connect the read end write end for all the pipes */
  int in, out; 
  if(index == 0){
    in = STDIN_FILENO; 
  }else{
    in = pipe_array[index - 1][READ_END];
  }if(index < numPipes){
    out = pipe_array[index][WRITE_END];
  }else{
    out = STDOUT_FILENO; 
  }if(in != STDIN_FILENO){
    if(dup2(in, STDIN_FILENO) == -1){
      perror("dup2-in"); 
      exit(EXIT_FAILURE); 
    }
  }if(out != STDOUT_FILENO){
    if(dup2(out, STDOUT_FILENO) == -1){
      perror("dup2-out"); 
      exit(EXIT_FAILURE); 
    }    
  }
  closepipes(pipe_array, numPipes);
}

void closepipes(int pipe_array[][2], int numPipes){
  int j;
  /*loop through all pipes and close the read/write end */
  for(j = 0; j < numPipes; ++j){
    if(close(pipe_array[j][READ_END])){
      perror("close read");
      exit(EXIT_FAILURE); 
    }if(close(pipe_array[j][WRITE_END])){
      perror("close write");  
      exit(EXIT_FAILURE); 
    }
  }
}

int create_pipes(int stages, int pipes[][2]){
  int i; 
  /*create pipes for each stage - 1 */
  for(i = 0; i < stages - 1; i++){
    if(pipe(pipes[i])){
      return -1; 
    }
  }
  return 0; 
}

void handle_args(int argc, char *argv[], int *infile){
  if(argc > 2){
    print_usage();
    exit(1);
  }else if(argc == 2){
    *infile = 1; 
  }
}

void handler(int sig){
  /*handler incase encountering SIGINT */
  printf("\n");
  fflush(stdout); 
}

void print_usage(void){
  fprintf(stderr, "usage: mush [input file]\n"); 
  exit(1); 
}

void sig_handle(struct sigaction *sa, sigset_t *newMask, sigset_t *oldMask){
  /*setting up signal handler*/
  sa -> sa_handler = handler;
  sigemptyset(&sa -> sa_mask);
  sa -> sa_flags = 0;
  if(sigemptyset(newMask) == -1){
    perror("sigemptyset");
    exit(EXIT_FAILURE);
  }
  if(sigemptyset(oldMask) == -1){
    perror("sigemptyset");
    exit(EXIT_FAILURE);
  }
  if(sigaddset(newMask, SIGINT) == -1){
    perror("sigaddset");
    exit(EXIT_FAILURE);
  }
  if(sigprocmask(SIG_BLOCK, newMask, oldMask) == -1){
    perror("sigprocmask");
    exit(EXIT_FAILURE);
  }
  if(sigaction(SIGINT, sa, NULL) < 0){
    perror("sigaction");
    exit(EXIT_FAILURE);
  }
}

void print_prompt(FILE *fd){
  if(isatty(fileno(fd)) && isatty(fileno(stdout)))
    printf(PROMPT);
}
