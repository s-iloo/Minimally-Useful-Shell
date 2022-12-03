CC = gcc
CFLAGS = -Wall -Werror -g -pedantic


all: mush2

mush2: compile
	$(CC) $(CFLAGS) -L ~pn-cs357/Given/Mush/lib64 -o mush mush2.o -lmush 
compile: mush2.c mush2.h 
	$(CC) $(CFLAGS) -c -I ~pn-cs357/Given/Mush/include mush2.c
clean: 
	rm -f mush mush2.o a.out
