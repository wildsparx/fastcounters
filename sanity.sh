#!/bin/sh
# Copyright (C) 2011, 2015 Asher Blum; http://wildsparx.com/fastcounters/
# Test sanity - are counters incrementing atomically?

die() {
    echo $1
    exit -1
}

rm -f test.dat
./cli -n4 test.dat
taskset 1 ./bench 10 100000000 > /dev/null 2>&1
./cli -p test.dat | grep 'C0 *1000000000$' > /dev/null || die "FAILED"
exit 0
