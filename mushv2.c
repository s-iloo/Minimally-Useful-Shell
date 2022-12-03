#include "mush2.h"


int main(int argc, char *argv[]){
  int infile = 0; 
  FILE* fd = stdin;
  char *readstring = NULL; 
  struct sigaction sa; 
  
  sa.sa_handler = handler;
  sigemptyset(&sa.sa_mask); 
  sa.sa_flags = 0; 
 
  if(sigaction(SIGINT, &sa, NULL) < 0){
    perror("sigaction");
    exit(EXIT_FAILURE); 
  }

  handle_args(argc, argv, &infile);
  
  if(infile == 1){
    if(!(fd = fopen(argv[infile], "r"))){
      perror("fopen");
      exit(EXIT_FAILURE); 
    }
  }
  /*check if the fd and stdout is associated with the terminal if we are
   *to use the prompt*/
  if(isatty(fileno(fd)) && isatty(fileno(stdout))){
    printf(PROMPT); 
  }
 
  while((readstring = readLongString(fd)) != NULL){ 
    if(isatty(fileno(fd)) && isatty(fileno(stdout))){
      printf(PROMPT);
    }
    handle_mush(fd, readstring);
    free(readstring);
  }
  return 0; 
}

void handle_mush(FILE *fd, char *readstring){
  pipeline pipeinfo;
  clstage currstage;
  int (*pipes)[DARRAY];  
  pid_t child;
  int status, i, badform = 0;  
  
  memset(&currstage, 0, sizeof(currstage)); 
  memset(&pipeinfo, 0, sizeof(pipeinfo)); 
  
  pipeinfo = crack_pipeline(readstring);
  print_pipeline(stdout, pipeinfo); 

  if(pipeinfo == NULL)
    badform = 1;  
  
  if(!badform){
    if(pipeinfo -> length > 1){
      pipes = calloc(((pipeinfo -> length)-1) * DARRAY, sizeof(int)); 
      if(-1 == create_pipes(pipeinfo -> length, pipes)){
        perror("pipes"); 
        exit(EXIT_FAILURE); 
      }  
    }else{
     
    }
 
    currstage = pipeinfo -> stage; 
  
  /*IF I HAVE ONE STAGE CHECK FOR CD OTHERWISE DON'T AND CALL ANOTHER FUNCTION
   *TO EXECUTE THE COMMAND*/

  /*for every stage we must redirect the input and the output*/
  
    for(i = 0; i < pipeinfo -> length; i++){
      if(currstage == NULL){
        perror("currstage"); 
        exit(EXIT_FAILURE); 
      } 
      if(!strcmp(currstage -> argv[0], "cd")){
        if(currstage -> argc != 2){
          fprintf(stderr, "cd: missing argument\n"); 
        }
        else if(-1 == chdir(currstage -> argv[1])){
          perror("chdir");  
        }
      }
      else{ 
        if((child = fork()) == 0){ 
          if(pipeinfo -> length > 1){     
            setup_pipes(pipes, (pipeinfo -> length) -1, i); 
          }
          execvp(currstage -> argv[0], currstage -> argv); 
          perror("bad exec"); 
          badform = 1;        
        }
       /*deal with parent*/
        else{
          if(child == -1){
            perror("fork"); 
            exit(EXIT_FAILURE); 
          } 
        }
      }
      currstage++; 
    }
    if(pipeinfo -> length > 1)
      closepipes(pipes, (pipeinfo -> length)-1);
    
    /*wait for child- parent only*/
    if(!badform){
      for(i = 0; i < pipeinfo -> length; i++){
        if((child = wait(&status)) == -1){
          perror("wait");
          exit(EXIT_FAILURE); 
        }
      }
    }
    free_pipeline(pipeinfo);
    yylex_destroy(); 
  } 
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
  
  /*printf("\n8-P ");*/ 


}

void print_usage(void){
  fprintf(stderr, "usage: mush [input file]\n"); 
  exit(1); 
}
