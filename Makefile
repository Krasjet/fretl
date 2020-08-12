.POSIX:

PREFIX = /usr/local

CFLAGS = -Wall -Wextra -std=c99 -g3 -pedantic -Os -D_POSIX_C_SOURCE
OBJ = fretl.o mnote/mint.o mnote/transpose.o mnote/mnote.o

fretl: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

mnote/mint.o: mnote/mint.h
mnote/mnote.o: mnote/mnote.h
mnote/transpose.o: mnote/mint.h mnote/transpose.h mnote/mnote.h

run: fretl
	@./fretl

install: fretl fretl.1
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1
	install -m 755 fretl $(DESTDIR)$(PREFIX)/bin
	gzip < fretl.1 > $(DESTDIR)$(PREFIX)/share/man/man1/fretl.1.gz

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/fretl
	rm -f $(DESTDIR)$(PREFIX)/share/man/man1/fretl.1.gz

clean:
	rm -f test *.o mnote/*.o

.PHONY: run clean
