/* Copyright (C) 2011, 2015 Asher Blum; http://wildsparx.com/fastcounters/ */

#ifndef FAST_COUNTERS_H
#define FAST_COUNTERS_H

#ifdef  __cplusplus
extern "C" {
#endif

#define FCNTRS_MAGIC 1621
#define FCNTRS_FMT_VERSION 1

#ifndef BOOL
typedef char BOOL;
#endif

#ifndef uint
typedef unsigned int uint;
#endif

/* header structure at start of counters file
 * client programmers - treat as opaque!
 */

typedef struct {
    int magic;
    int fmt_version;
    int ncntrs;
} fcntrs_hdr_t;

/* handle used by most functions
 * client programmers - treat as opaque!
 */

typedef struct {
    int fd;
    uint fsize;
    int err;
    fcntrs_hdr_t *hdr;
    int *cntrs;
    char *toc;  // start of toc as serialized string
    char **toc_ptrs; // array of pointers to toc members
} fcntrs_t;

// api_start
/**********************************************************************/

#define FCNTRS_EOPEN    100
#define FCNTRS_EWRITE   101
#define FCNTRS_EMAP     102
#define FCNTRS_ESTAT    103
#define FCNTRS_EUNMAP   104
#define FCNTRS_EMALLOC  105
#define FCNTRS_EUNINIT  200
#define FCNTRS_EBADVAL  201

/** Fast Counters API:
 **
 ** Most functions return BOOL; 0=good, 1=error
 ** If error, fcntrs_errno() will specify which one
 ** 
 **/

/** Create a new counters file
 **     ncounters = number of counters to create
 **     counter_names (optional) - names to assign
 **         if NULL, names will be 'C0', 'C1', ...
 **     Does NOT initialize fc; fc is used only for
 **         error reporting.  fc does not need to
 **         initialized before calling.
 **/

BOOL fcntrs_create(fcntrs_t *fc, char *fn, uint ncounters, char **counter_names);


/** Open an existing counters file and initialize fc
 **       
 **     Try to keep the file open rather than opening it
 **     for each increment.
 **     Client must allocate fcntrs before calling
 **         (heap or stack)
 **/

BOOL fcntrs_open(fcntrs_t *fc, char *fn);


/** Close an open counters file
 **/

BOOL fcntrs_close(fcntrs_t *fc);


/** Get the size of the table of contents;
 **     this is also the number of counters.
 ** Mainly for use by scripting language wrappers
 **/

BOOL fcntrs_get_toc_size(fcntrs_t *fc, uint *size); 


/** Get nth label from table of contents
 **     0 <= ind < toc_size
 **     Do not modify the returned string!
 **     label must point to a char*; 
 **/

BOOL fcntrs_get_toc_entry(fcntrs_t *fc, uint ind, char **label);


/** Set a counter to a value.
 **/

BOOL fcntrs_set(fcntrs_t *fc, uint index, int value);


/** Get the value of a counter.
 **/

BOOL fcntrs_get(fcntrs_t *fc, uint index, int *out_val);


/** Increment a counter and return the new value
 **     0 <= index < toc_size
 **     if out_val is NULL, no return
 **     if out_val points to an int, return new value
 **/

BOOL fcntrs_incr(fcntrs_t *fc, uint index, int *out_val);

/** Given a counter name, return the counter id.
 ** This is relatively slow and should only be called once.
 **/

BOOL fcntrs_resolve_name(fcntrs_t *fc, char *name, int *out_val);

/** Get last error number; numbers are internal to this
 **     library.
 **/

int fcntrs_errno(fcntrs_t *fc);



// api_end

#ifdef  __cplusplus
}
#endif
#endif // FAST_COUNTERS_H
