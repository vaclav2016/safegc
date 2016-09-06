all: example example_gc

example: example.c safegc.c safegc.h
	gcc -Wall -O2 example.c safegc.c -o example

example_gc: example.c safegc.c safegc.h
	gcc -Wall -O2 -DINTERNAL_GC example.c safegc.c -o example_gc

clean:
	rm -f example
	rm -f example_gc
