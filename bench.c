/* Copyright (C) 2011, 2015 Asher Blum; http://wildsparx.com/fastcounters/ */

/* Program to benchmark the library */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>
#include "fastcounters.h"

#define TESTFILE "test.dat"

#define XNZ(X) if(X) { fprintf(stderr, "Error %d : %d\n", fcntrs_errno(&fc), errno); exit(-1); }


void usage() {
    fprintf(stderr, "Invoke with NUM_PROCS NUM_REPS\n");
    exit(-1);
}

// get dur in usec

inline int get_dur(struct timeval *start_time, struct timeval *end_time) {
    int res;
    res = (end_time->tv_sec-start_time->tv_sec)*1000*1000 +
           (end_time ->tv_usec-start_time->tv_usec);
    //printf("res=%d\n", res); exit(0);
    return res;
}

void t1(int nprocs, int nreps) {
    struct timeval st, et;
    int us;
    float per_sec;
    fcntrs_t fc;
    int res;
    int i, j;
    pid_t pid;
    int kid_status;
    int nkids_done;

    for(i=0; i<nprocs; i++) {
        pid = fork();
        if(pid < 0) {
            perror("fork");
            exit(-1);
        }
        if(pid == 0) { // child
            XNZ( fcntrs_open(&fc, TESTFILE) )
            gettimeofday(&st, 0);

            for(j=0; j<nreps; j++) {
                XNZ( fcntrs_incr(&fc, 0, &res) )
            }

            gettimeofday(&et, 0);
            us = get_dur(&st, &et);
            per_sec = (float)nreps * 1000000.0 / (float)us;
            printf("usec=%d   nreps=%d   =%.1f/s  =%.3fM/s\n", us , nreps, per_sec, per_sec/1000000);
            exit(0);
        }
        else {
            //printf("launched %u\n", pid);
        }
    }
    while(nkids_done < nprocs) {
        wait(&kid_status);
        nkids_done ++;
    }
    //sleep(1);
}

int main(int argc, char **argv) {
    int nprocs, nreps;
    if(argc < 3) {
        usage();
    }
    nprocs = atoi(argv[1]);
    nreps = atoi(argv[2]);
    t1(nprocs, nreps);
    return 0;
}
