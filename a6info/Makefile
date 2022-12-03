CCUNIXFLAGS = -g -Wall -pedantic

all: mush2 mush2v1 mush2v2

mush2.o: mush2.c
	gcc -c $(CUNIXFLAGS) -I ~pn-cs357/Given/Mush/include mush2.c -o mush2.o
mush2: mush2.o
	gcc $(CCUNIXFLAGS) -L ~pn-cs357/Given/Mush/lib64 -o mush2 mush2.o -lmush

mush2v1.o: mush2v1.c
	gcc -c $(CUNIXFLAGS) -I ~pn-cs357/Given/Mush/include mush2v1.c -o mush2v1.o
mush2v1: mush2v1.o
	gcc $(CCUNIXFLAGS) -L ~pn-cs357/Given/Mush/lib64 -o mush2v1 mush2v1.o -lmush

mush2v2.o: mush2v2.c
	gcc -c $(CUNIXFLAGS) -I ~pn-cs357/Given/Mush/include mush2v2.c -o mush2v2.o
mush2v2: mush2v2.o
	gcc $(CCUNIXFLAGS) -L ~pn-cs357/Given/Mush/lib64 -o mush2v2 mush2v2.o -lmush

clean:
	rm -f mush2 mush2.o  mush2v1 mush2v1.o  mush2v2 mush2v2.o
