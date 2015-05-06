/* Copyright (C) 2011, 2015 Asher Blum; http://wildsparx.com/fastcounters/ */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "fastcounters.h"
#include "atomic.h"


#define RETURN_OK return 0;
#define ERETURN(X) fc->err=X; return 1;
#define CHECK_INIT  if(!fc->hdr) { ERETURN(FCNTRS_EUNINIT); }
#define CHECK_INDEX if(index >= fc->hdr->ncntrs) { ERETURN(FCNTRS_EBADVAL) }

/** File = hdr + counters + toc
 **     counters = int x ncounters 
 **     toc = name1\0name2\0...
 **/

int fcntrs_errno(fcntrs_t *fc) {
    return fc->err;
}

BOOL fcntrs_create(fcntrs_t *fc, char *fn, uint ncounters, char **counter_names) {
    FILE *ofh;
    fcntrs_hdr_t hdr;
    int dum;
    int i;
    size_t len;

    ofh = fopen(fn, "wb");
    if(!ofh) {
        ERETURN(FCNTRS_EOPEN)
    }
    hdr.magic = FCNTRS_MAGIC;
    hdr.fmt_version = FCNTRS_FMT_VERSION;
    hdr.ncntrs = ncounters;

    if(fwrite(&hdr, sizeof(hdr), 1, ofh) != 1) {
        ERETURN(FCNTRS_EWRITE)
    }

    /* write counters */

    dum = 0;

    for(i=0; i<ncounters; i++) {
        if(fwrite(&dum, sizeof(dum), 1, ofh) != 1) {
            ERETURN(FCNTRS_EWRITE)
        }
    }

    /* write toc */

    if(counter_names) {
        for(i=0; i<ncounters; i++) {
            len = strlen(counter_names[i]) + 1; // include null term
            if(fwrite(counter_names[i], len, 1, ofh) != 1) {
                ERETURN(FCNTRS_EWRITE)
            }
        }
    }
    else { // none provided - make up our own
        for(i=0; i<ncounters; i++) {
            if(fprintf(ofh, "C%d%c", i, 0) < 0) {
                ERETURN(FCNTRS_EWRITE)
            }
        }
    }

    fclose(ofh);
    RETURN_OK
}
            
BOOL fcntrs_open(fcntrs_t *fc, char *fn) {
    struct stat st;
    uint fsize;
    void *v;
    off_t size;
    char *p;
    int i;

    fc->err = 0;
    if(stat(fn, &st)) {
        ERETURN(FCNTRS_ESTAT);
    }
    fc->fsize = st.st_size;
    fc->fd = open(fn, O_RDWR);
    if(fc->fd < 0) {
        ERETURN(FCNTRS_EOPEN);
    }

    v = mmap(0, fc->fsize, PROT_READ | PROT_WRITE, MAP_SHARED, fc->fd, 0);
    if(v == MAP_FAILED) {
        ERETURN(FCNTRS_EMAP);
    }

    fc->hdr = (fcntrs_hdr_t*)v;
    fc->cntrs = (int*)(fc->hdr + 1);
    fc->toc = (char*)(fc->cntrs + fc->hdr->ncntrs);
    fc->toc_ptrs = malloc(fc->hdr->ncntrs * sizeof(char*));
    if(fc->toc_ptrs == NULL) {
        ERETURN(FCNTRS_EMALLOC);
    }

    fc->toc_ptrs[0] = fc->toc;
    for(p = fc->toc, i=1; i < fc->hdr->ncntrs; p++) {
        if(!*p) {
            fc->toc_ptrs[i++] = p+1;
        }
    }
    RETURN_OK
}
    
BOOL fcntrs_close(fcntrs_t *fc) {
    if(munmap(fc->hdr, fc->fsize)) {
        ERETURN(FCNTRS_EUNMAP);
    }
    close(fc->fd);
    RETURN_OK
}

BOOL fcntrs_get_toc_size(fcntrs_t *fc, uint *size) {
    CHECK_INIT
    *size = fc->hdr->ncntrs;
    RETURN_OK
}

BOOL fcntrs_get_toc_entry(fcntrs_t *fc, uint index, char **label) {
    CHECK_INIT
    CHECK_INDEX
    *label = fc->toc_ptrs[index];
    RETURN_OK
}

BOOL fcntrs_set(fcntrs_t *fc, uint index, int value) {
    CHECK_INIT
    CHECK_INDEX
    fc->cntrs[index] = value;
    RETURN_OK
}

BOOL fcntrs_get(fcntrs_t *fc, uint index, int *out_val) {
    CHECK_INIT
    CHECK_INDEX
    *out_val = fc->cntrs[index];
    RETURN_OK
}

// The checks slow this from 302M/s to 111M/s; so disable

BOOL fcntrs_incr(fcntrs_t *fc, uint index, int *out_val) {
    //CHECK_INIT
    //CHECK_INDEX
    *out_val = atomic_add_return(1, (atomic_t*)&(fc->cntrs[index]));
    RETURN_OK
}

BOOL fcntrs_resolve_name(fcntrs_t *fc, char *name, int *out_val) {
    int i;
    for(i=0; i<fc->hdr->ncntrs; i++) {
        if(!strcmp(name, fc->toc_ptrs[i])) {
            *out_val = i;
            RETURN_OK
        }
    }
    ERETURN(FCNTRS_EUNMAP);
}
