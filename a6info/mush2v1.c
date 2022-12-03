#include <stdio.h>
#include <stdlib.h>
#include <mush.h>
#define PROMPT "8-P "
void display_prompt(void);

int main(int argc, char *argv[]) {
    char *longLine;
    pipeline  ptr_pipeline;
    printf("Start\n");
    while (1) {
        display_prompt();
        if ((longLine = readLongString(stdin)) == NULL) {
            break;
        }
        printf("longLine: ->%s<-\n", longLine);
        free(longLine);
    }
    printf("End\n");
    return 0;
}

void display_prompt(void) {
   printf(PROMPT);
}
