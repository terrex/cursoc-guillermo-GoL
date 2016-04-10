CFLAGS += -std=c99 -g
LDFLAGS += -lssl -lcrypto

ALL: juego-vida

juego-vida: main.o world.o
	$(CC) $(CFLAGS) -o $@ main.o world.o  $(LDFLAGS)

world.o: world.c world.h
main.o: main.c world.h

check:
	checkpatch.pl -q --no-tree -f --fix-inplace *.c
