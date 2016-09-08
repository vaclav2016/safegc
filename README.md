# Safe Garbage Collector

This is minimalistic memory manager. I have two primary target for this project: detect memory leaks into my projects.

So, I had wrote simple memory allocator in MS-DOS style.

Also, this project could be used for embeded systems.

## How to use ?

Here is [example.c](example.c). This code have a problem into `test_bad()` and you will see memory dump. Memory dump `test_ok()` will be empty.

If you don't want have a debug info - just remove `GC_DUMP()` call and you will have small and sweet memory allocator into your project. :)

    #include "safegc.h"

    void test_ok() {
        char *str = GC_MALLOC(126);
        GC_FREE(str);
        GC_DUMP();
    }

    void test_bad() {
        char *str = GC_MALLOC(126);
        GC_DUMP();
    }

    int main() {
        GC_INIT(1024 * 64);
        test_ok();
        test_bad();
        GC_DONE();
        return 0;
    }

## Manual

To start use **safegc** you should include `safegc.h`. If you want use **safegc** (or switch to system calls) - you should pass complier option:

    $ gcc -DINTERNAL_GC ...

If you dont pass `INTERNAL_GC` option - your code will work with system's malloc / realloc / free.

Here is a few predefined "macros" from `safegc.h`:

`GC_INIT(size_t)` - Allocate system memory bytes for safegc.

`GC_MALLOC(size_t)` - Allocate internal memory. Return *void

`GC_REALLOC(void *, size_t)` - Realloc new size of internal memory. Return *void.

`GC_FREE(void *)` - Free internal memory.

`GC_DONE()` - Destroy allocated system memory. I shoud call only in end of your programm.

`GC_DUMP()` - Show internal memory state (if something has been allocated). You may not use it.

## Build

    $ make

or

    $ gcc -DINTERNAL_GC example.c safegc.c -o example_gc
    $ gcc example.c safegc.c -o example

## Dependencies

NONE

## Donate

I maintain this project in my free time, if it helped you please support my work via Bitcoins, thanks a lot!

Bitcoin-wallet is: `1QAfNs5Utygt2XQoV3YCykzHs63S3AfEJ3`

## Licensing

(c) 2016 Copyright Vaclav2016 https://github.com/vaclav2016, jabber id vaclav2016@jabber.cz

BOOST License, <http://www.boost.org/LICENSE_1_0.txt>
