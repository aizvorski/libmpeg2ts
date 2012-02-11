VERSION = 0.1.0

INCLUDES = 
LIBS = 

CC = gcc
CFLAGS += -std=c99 -Wall -W -Wshadow -Wwrite-strings -Wno-unused -g -O3 $(INCLUDES)

LD = gcc
LDFLAGS += $(LIBS)

AR = ar
ARFLAGS = rsc

BINARIES = 

all: libmpeg2ts.a $(BINARIES)


libmpeg2ts.a: mpeg2ts.c mpeg2ts.h
	$(CC) $(CFLAGS) -c -o mpeg2ts.o mpeg2ts.c
	$(AR) $(ARFLAGS) libmpeg2ts.a mpeg2ts.o


mpeg2ts.c: mpeg2ts.xml code_generator.pl mpeg2ts.c_prefix
	perl code_generator.pl

mpeg2ts.h: mpeg2ts.xml code_generator.pl mpeg2ts.h_prefix mpeg2ts.h_suffix
	perl code_generator.pl

clean:
	rm -f *.o libmpeg2ts.a $(BINARIES)

dox: mpeg2ts.c mpeg2ts.h
	doxygen Doxyfile

dist: clean
	mkdir libmpeg2ts-$(VERSION)
	tar c --files-from=MANIFEST -f tmp.tar ; cd libmpeg2ts-$(VERSION) ; tar xf ../tmp.tar ; rm -f ../tmp.tar
	tar czf ../libmpeg2ts-$(VERSION).tar.gz libmpeg2ts-$(VERSION)
	rm -rf libmpeg2ts-$(VERSION)

