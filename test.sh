#!/bin/sh

# Copyright (C) 2011, 2015 Asher Blum; http://wildsparx.com/fastcounters/

die() {
    echo "ERROR: $1"
    exit 1
}

test -e cli || die "cli does not exist"
test -e bench || die "bench does not exist"
rm -f test.dat
./cli -n 2 test.dat || die "creating test.dat"
./bench 4 100 > bench.tmp || die "bench"
./cli -p test.dat > res.tmp || die "dumping test.dat"
grep 'C0         400$' res.tmp > /dev/null || die "unexpected value in res.tmp"
