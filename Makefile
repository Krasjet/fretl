.POSIX:

PREFIX = /usr/local

CC = cc
CFLAGS = -Wall -Wextra -std=c99 -g3 -pedantic -Os -D_POSIX_C_SOURCE -Imint
OBJ = fretl.o mint/mint.o mint/pitch.o mint/transpose.o

run: fretl
	@./fretl

fretl: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

mint/mint.o: mint/mint.h
mint/pitch.o: mint/pitch.h
mint/transpose.o: mint/mint.h mint/pitch.h mint/transpose.h

install: fretl fretl.1
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1
	install -m 755 fretl $(DESTDIR)$(PREFIX)/bin
	gzip < fretl.1 > $(DESTDIR)$(PREFIX)/share/man/man1/fretl.1.gz

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/fretl
	rm -f $(DESTDIR)$(PREFIX)/share/man/man1/fretl.1.gz

clean:
	rm -f fretl *.o mint/*.o

.PHONY: run clean
