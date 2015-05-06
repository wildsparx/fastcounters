/* Copyright (C) 2011, 2015 Asher Blum; http://wildsparx.com/fastcounters/ */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>

// from /usr/src/kernels/2.6.9-55.EL-smp-i686/include/asm-i386/atomic.h


#ifdef CONFIG_SMP
#define LOCK "lock ; "
#else
#define LOCK ""
#endif

/*
 * Make sure gcc doesn't try to be clever and move things around
 * on us. We need to use _exactly_ the address the user gave us,
 * not some alias that contains the same information.
 */
typedef struct { volatile int counter; } atomic_t;

/**
 * atomic_add_return - add and return
 * @v: pointer of type atomic_t
 * @i: integer value to add
 *
 * Atomically adds @i to @v and returns @i + @v
 */
/* stripped out 386 stuff - apb */
static __inline__ int atomic_add_return(int i, atomic_t *v)
{
    int __i;
    __i = i;
    __asm__ __volatile__(
        LOCK "xaddl %0, %1;"
        :"=r"(i)
        :"m"(v->counter), "0"(i));
    return i + __i;
}

