# fastcounters
Fast, persistent, atomically incrementing, cross-process counters.

Copyright (C) 2011, 2015 Asher Blum; http://wildsparx.com/fastcounters/

* Can be used by multiple related or unrelated processes.
* Can be used by multiple threads. No need to lock.
* Persistent, but lazily - if you pull the power cord, data is lost.
* Command line tool for creating and inspecting the counter file.
* Counter file contains multiple named counters.


## Performance

Approx 385 million increments per second. Measured with:
* 4 processes
* 10M increments each
* 4 core i7 CPU
* 2.67GHz

## Platform

Linux, Intel, GCC.

## How It Works

Fastcounters uses a memory mapped file and increments the counters with
the atomic xaddl instruction; the code is taken from the linux kernel.

## License

GPL, since it uses code from the linux kernel.

## Creating the counter file

Create a file with four anonymous counters like this:

`fastcounters -n4 counters.dat`

(They will be named C0 thru C3)

Or create a file with three named counters like this:

`fastcounters -n cats,dogs,snakes counters.dat`

## Incrementing Counters

In a C program, you can increment a counter like this:

```
int res;
if(fcntrs_incr(&fc, 0, &res)) {
    // handle error
}
```

Note:
* fc is a fastcounters handle already opened.
* This increments the first (position 0) counter in the file.
* res now contains the new counter value.

## Ways of Addressing Counters

There are three ways of addressing counters:

### Literal Int

Shown in the example above.

### Defined Constant

Given a counters file, you can generate a C fragment with:

```fastcounters -poh counters.dat```

This will produce output like:

```
#define COUNTER_cats      0
#define COUNTER_dogs      1
#define COUNTER_snakes    2
```

You can use these defined constants to access the counters.

### Dynamic Name Lookup

You can dynamically resolve counter names to IDs:

```
int cnt_id;
if(fcntrs_resolve_name(fc, "dogs", &cnt_id)) {
    // handle error
}
```

   Since this lookup is signficantly slower than incrementing a counter,
   the result should be cached. One way to do this is with static
   variables:

```
static int cnt_id = -1;
if(cnt_id == -1) { // first time
    if(fcntrs_resolve_name(fc, "dogs", &cnt_id)) {
...
```
