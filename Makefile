# Copyright (C) 2011, 2015 Asher Blum; http://wildsparx.com/fastcounters/

VER=1
PREFIX=/usr/local
SO=libfastcounters.so.$(VER)
CFLAGS=-g
CFLAGS=-O3 -fPIC

all: cli bench $(SO)

fastcounters.o: fastcounters.c
	gcc $(CFLAGS) -c $^ -o $@

$(SO): fastcounters.o
	gcc $(CFLAGS) $^ -shared -Wl,-soname,$@ -o $@

cli.o: cli.c
	gcc $(CFLAGS) -c $^ -o $@

bench.o: bench.c
	gcc $(CFLAGS) -c $^ -o $@

example1.o: example1.c
	gcc $(CFLAGS) -c $^ -o $@

cli: cli.o fastcounters.o
	gcc $^ -o $@

bench: bench.o fastcounters.o
	gcc $^ -o $@

example1: example1.o fastcounters.o
	gcc $^ -o $@

test: bench
	./test.sh

install: cli $(SO)
	install -m0755 cli $(PREFIX)/bin/fastcounters
	install -m0644 fastcounters.h $(PREFIX)/include
	install -m0644 $(SO) $(PREFIX)/lib
	ln -sf $(SO) $(PREFIX)/lib/libfastcounters.so

clean:
	rm -f *.o *.tmp test.dat counters.dat cli bench example1 *.so $(SO)
