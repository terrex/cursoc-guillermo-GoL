CFLAGS += -std=c99 -g
LDFLAGS += -lssl -lcrypto

ALL: juego-vida

juego-vida: main.o world.o
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) main.o world.o

world.o: world.c world.h
main.o: main.c world.h

check:
	checkpatch.pl -q --no-tree -f --fix-inplace *.c
