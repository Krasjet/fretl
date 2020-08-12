.POSIX:

CFLAGS = -Wall -Wextra -std=c99 -g3 -pedantic -Os -D_POSIX_C_SOURCE
OBJ = fretl.o mnote/mint.o mnote/transpose.o mnote/mnote.o

run: fretl
	./fretl

fretl: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

mnote/mint.o: mnote/mint.h
mnote/mnote.o: mnote/mnote.h
mnote/transpose.o: mnote/mint.h mnote/transpose.h mnote/mnote.h

clean:
	rm -f test *.o mnote/*.o

.PHONY: run clean
