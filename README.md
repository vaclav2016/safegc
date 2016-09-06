# Safe Garbage Collector

I have two primary target for this project:

1. Detect memory leaks into my program
2. Reduce call to system malloc / free (some time it is have a big cost) with lightweight replacement
 
So, I had wrote simple memory manager in CP/M or MS-DOS style.

Also this project could be used for embeded systems.

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

## Build

    $ gcc example.c safegc.c -o example

## Dependencies

None

## Licensing

(c) 2016 Copyright Vaclav2016 https://github.com/vaclav2016, jabber id vaclav2016@jabber.cz

BOOST License, <http://www.boost.org/LICENSE_1_0.txt>
