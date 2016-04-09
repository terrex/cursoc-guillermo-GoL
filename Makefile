CFLAGS += -std=c99 -g

ALL: juego-vida

juego-vida: main.o world.o world.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ main.o world.o

world.o: world.c world.h
main.o: main.c world.h

check:
	checkpatch.pl -q --no-tree -f --fix-inplace *.c
