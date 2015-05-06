/* Copyright (C) 2011, 2015 Asher Blum; http://wildsparx.com/fastcounters/ */

/* Command line tool to manage counter files */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include "fastcounters.h"

#define OPTIONS "n:pho:c:s:"
#define OP_NEW      1
#define OP_PRINT    2
#define OP_SET      3

#define SEP ','

#define XNZ(X) if(X) { fprintf(stderr, "Error %d : %d at line %d\n", fcntrs_errno(&fc), errno, __LINE__); exit(-1); }

static char *my_name;

typedef struct {
    int op;
    char *filename;
    char *new_arg;
    char out_fmt;
    int new_val;
    char *key;
} global_t;


void usage() {
    fprintf(stderr, "Usage: %s CMD [OPTS] FILENAME\n\n", my_name);
    fprintf(stderr, "Commands:\n\n");
    fprintf(stderr, "    -n N                    Create a new file with N counters.\n");
    fprintf(stderr, "                            named C0, C1 ...\n\n");
    fprintf(stderr, "    -n name1,name2,...      Create a new file with the\n");
    fprintf(stderr, "                            specified counter names\n\n");
    fprintf(stderr, "    -p                      Print file.\n\n");
    fprintf(stderr, "    -s N [-c C]             Set counter N to value C (or 0)\n\n");
    fprintf(stderr, "Options:\n\n");
    fprintf(stderr, "    -oh                     Output a C header file\n\n");

    exit(-1);
}

void init(global_t *g) {
    bzero(g, sizeof(global_t));
}

void do_new(global_t *g) {
    int nc; // num counters;
    char *p;
    char **keys;
    int rc;
    fcntrs_t fc;
    int i;

    nc = atoi(g->new_arg);
    if(nc < 0) {
        usage();
    }

    // If name-list provided:

    if(nc==0) { 
        nc = 1;
        for(p=g->new_arg; *p; p++) {
            if(*p == SEP) {
                nc ++;
            }
        }
        keys = malloc(nc * sizeof(char*));
        keys[0] = g->new_arg; // first key
        i = 1;
        for(p=g->new_arg; *p; p++) {
            if(*p == SEP) {
                *p = 0; // null term
                keys[i++] = p+1;
            }
        }
    }
    else { // qty provided
        keys = NULL;
    }

    XNZ( fcntrs_create(&fc, g->filename, nc, keys) )
    exit(0);
}

void do_print(global_t *g) {
    fcntrs_t fc;
    uint nc;
    char *label;
    int i;
    int cnt;
    int max_label_len;
    int llen;
    int lw; // label width

    XNZ( fcntrs_open(&fc, g->filename) )
    XNZ( fcntrs_get_toc_size(&fc, &nc) )

    /* find widest label for fmt: */

    max_label_len = 0;

    for(i=0; i<nc; i++) {
        XNZ( fcntrs_get_toc_entry(&fc, i, &label) )
        llen = strlen(label);
        if(llen > max_label_len) {
            max_label_len = llen;
        }
    }
    lw = (max_label_len < 50) ? max_label_len : 50;

    /* print output */

    for(i=0; i<nc; i++) {
        XNZ( fcntrs_get_toc_entry(&fc, i, &label) )
        XNZ( fcntrs_get(&fc, i, &cnt) )

        switch(g->out_fmt) {
        case 0:
            printf("%-*.*s %11d\n", lw, lw, label, cnt);
            break;
        case 'h':
            printf("#define COUNTER_%-*.*s %4d\n", lw, lw, label, i);
            break;
        default:
            usage();
            break;
        }

    }
    exit(0);
}

void do_set(global_t *g) {
    fcntrs_t fc;
    int keyno;
    int i;
    char *label;
    uint nc;

    XNZ( fcntrs_open(&fc, g->filename) )
    XNZ( fcntrs_get_toc_size(&fc, &nc) )

    if(isdigit(g->key[0])) {
        keyno = atoi(g->key);
    }
    else { // string
        keyno = -1;
        for(i=0; i<nc; i++) {
            XNZ( fcntrs_get_toc_entry(&fc, i, &label) )
            if(!strcmp(g->key, label)) {
                keyno = i;
            }
        }
        if(keyno < 0) {
            fprintf(stderr, "Key not found\n");
            exit(-1);
        }
    }

    XNZ( fcntrs_set(&fc, keyno, g->new_val) )
    printf("set %d = %d\n", keyno, g->new_val);
    exit(0);
}
    



void get_opts(global_t *g, int argc, char **argv) {
    int opt;

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch(opt) {
        case 'n':
            g->op = OP_NEW;
            g->new_arg = optarg;
            break;
        case 'p':
            g->op = OP_PRINT;
            break;
        case 'o':
            g->out_fmt = optarg[0];
            break;
        case 'h':
            usage();
            break;
        case 's':
            g->key = optarg;
            g->op = OP_SET;
            break;
        case 'c':
            g->new_val = atoi(optarg);
            break;
        default:
            usage();
            break;
        }
    }

    if (optind >= argc) {
        usage();
    }
    g->filename = argv[optind];
}

int main(int argc, char **argv) {
    global_t g;
    my_name = argv[0];
    init(&g);
    get_opts(&g, argc, argv);

    switch(g.op) {
    case OP_NEW:
        do_new(&g);
        break;
    case OP_PRINT:
        do_print(&g);
        break;
    case OP_SET:
        do_set(&g);
        break;
    }
    exit(0);
}
        
    
