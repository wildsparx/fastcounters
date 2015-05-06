/* Copyright (C) 2011, 2015 Asher Blum; http://wildsparx.com/fastcounters/ */

/** Demonstrates incrementing counters by name and by position
 **
 ** First create a counters file:
 ** fastcounters -n cats,dogs,snakes test.dat
 **
 ** run this program, then:
 ** fastcounters -p test.dat
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>
#include "fastcounters.h"

#define TESTFILE "test.dat"

// Increment a counter by position --
// in this case, position 0

void inc_first_counter(fcntrs_t *fc) {
    int res;
    if(fcntrs_incr(fc, 0, &res)) {
        fprintf(stderr, "Failed to increment counter\n");
        exit(-1);
    }
}

// Increment a named counter
// use the TOC to look it up ONLY the first time

void inc_named_counter(fcntrs_t *fc) {
    int res;
    static int cnt_id = -1;
    if(cnt_id == -1) { // first time
        if(fcntrs_resolve_name(fc, "dogs", &cnt_id)) {
            fprintf(stderr, "Failed to resolve counter 'dogs'\n");
            exit(-1);
        }
        printf("resolved cnt_id=%d\n", cnt_id);
    }
    if(fcntrs_incr(fc, cnt_id, &res)) {
        fprintf(stderr, "Failed to increment counter\n");
        exit(-1);
    }
}

void main_loop() {
    int j, nreps = 100;
    fcntrs_t fc;

    if(fcntrs_open(&fc, TESTFILE)) {
        fprintf(stderr, "Failed to open counter file\n");
        exit(-1);
    }
    for(j=0; j<nreps; j++) {
        sleep(1);
        printf("xx %d\n", j);
        inc_first_counter(&fc);
        if(!(j%3)) {
            inc_named_counter(&fc);
        }
    }
    if(fcntrs_close(&fc)) {
        fprintf(stderr, "Failed to close counter file\n");
        exit(-1);
    }
}

int main(int argc, char **argv) {
    main_loop();
    return 0;
}
