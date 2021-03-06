/*

(c) 2016 Copyright Vaclav2016 https://github.com/vaclav2016, jabber id vaclav2016@jabber.cz

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include"safegc.h"

#define SIGNATURE 0xF012

#pragma pack(push, 1)
typedef enum { BUSY, FREE } Type;

struct MemBlk {
	size_t size;
	uint16_t sign;
	Type type;
	struct MemBlk *next;
	struct MemBlk *prev;
};
#pragma pack(pop)

#define MEM_BLK_SIZE sizeof(struct MemBlk)
#define MIN_FREE_BLK (MEM_BLK_SIZE * 2)

struct MemBlk *first;
struct MemBlk *firstFree;
size_t gc_memsize;
size_t free_count;

void gc_init(size_t size) {
	first = malloc(size);
	first->type = FREE;
	first->sign = SIGNATURE;
	first->size = size - MEM_BLK_SIZE;
	first->next = NULL;
	first->prev = NULL;
	firstFree = first;
	gc_memsize = size;
	free_count = 0;
}

void gc_done() {
	free(first);
}

void _gc_merge(struct MemBlk *p) {
	p->size += p->next->size + MEM_BLK_SIZE;
	p->next = p->next->next;
	if(p->next != NULL) {
		p->next->prev = p;
	}
	firstFree = firstFree > p ? p : firstFree;
}

void *_gc_split(struct MemBlk *f, Type type1, Type type2, size_t rsize) {
	size_t fsize = rsize;
	size_t nsize = f->size - (MEM_BLK_SIZE + rsize);

	struct MemBlk *n = (void*)f + (MEM_BLK_SIZE + fsize);
	n->size = nsize;
	n->sign = SIGNATURE;
	n->type = type2;
	n->prev = f;
	n->next = f->next;
	if(f->next != NULL) {
		f->next->prev = n;
	}
	f->next = n;
	f->size = fsize;
	f->type = type1;
	return ((void*)f) + MEM_BLK_SIZE;
}

void *_gc_malloc(size_t size) {
	struct MemBlk *f = (firstFree==NULL) ? first : firstFree;
	if(size % MIN_FREE_BLK != 0) {
		size = (size / MIN_FREE_BLK + 1) * MIN_FREE_BLK;
	}
	size_t rsize = size + MIN_FREE_BLK;
	while(f != NULL) {
		if((f->type == FREE) && (f->size > rsize)) {
			return _gc_split(f, BUSY, FREE, size);
		}
		f = f->next;
	}
	printf("Not enought memory. Can't allocate %i byte(s)\n", size);
	return NULL;
}

void _gc_clear_all() {
	struct MemBlk *f = first;
	firstFree = NULL;
	while(f != NULL) {
		while(f->type == FREE && f->next != NULL && f->next->type == FREE) {
			_gc_merge(f);
		}
		firstFree = firstFree==NULL && f->type == FREE ? f : firstFree;
		f = f->next;
	}
}

void *gc_malloc(size_t size) {
	void *ptr;
	if( (ptr = _gc_malloc(size)) != NULL ) {
		return ptr;
	}
	_gc_clear_all();
	return _gc_malloc(size);
}

void gc_free(void *ptr) {
	ptr -= MEM_BLK_SIZE;
	if(((struct MemBlk *)ptr)->sign != SIGNATURE) {
		printf("free(%x) - invalid pointer.\n", ptr - ((void*)first));
	} else {
		((struct MemBlk *)ptr)->type = FREE;
	}
/*
	free_count++;
	if(free_count > 100) {
		_gc_clear_all();
		free_count = 0;
	}
*/
}

void *_gc_merge_nbhood(struct MemBlk *f, size_t rsize) {
	void *ptr =  ((void*)f) + MEM_BLK_SIZE;
	size_t osize = f->size;

	while(f->next != NULL && (f->next->type == FREE) && (f->next->next != NULL) && (f->next->next->type == FREE)) {
		_gc_merge(f->next);
	}
	size_t nsize = (f->next == NULL) || (f->next->type != FREE) ? 0 : f->next->size;

	while(f->prev!=NULL && (f->prev->type == FREE) && (f->prev->prev != NULL) && (f->prev->prev->type == FREE)) {
		_gc_merge(f->prev->prev);
	}
	size_t psize = f->prev==NULL || (f->prev->type != FREE) ? 0 : f->prev->size;

	if(nsize + psize + osize >rsize) {
		if(nsize > 0) {
			_gc_merge(f);
		}
		if(psize > 0) {
			f = f->prev;
			void *nptr = ((void*)f) + MEM_BLK_SIZE;
			_gc_merge(f);
			memmove(nptr, ptr, osize);
			ptr = nptr;
		}
		if(f->size > rsize + MIN_FREE_BLK) {
			_gc_split(f, BUSY, FREE, rsize);
		}
		return ptr;
	}
	return NULL;
}

void *gc_realloc(void *ptr, size_t rsize) {
	if(ptr==NULL) {
		return gc_malloc(rsize);
	}
	struct MemBlk *f = ptr - MEM_BLK_SIZE;

	if((f->size >= rsize) && (f->size * 7 / 10 < rsize)) {
		return ptr;
	}

	if(rsize % MIN_FREE_BLK != 0) {
		rsize = (rsize / MIN_FREE_BLK + 1) * MIN_FREE_BLK;
	}
	void *np = _gc_merge_nbhood(f, rsize + MIN_FREE_BLK * 2 );
	if(np != NULL) {
		return np;
	}
	np = gc_malloc( rsize + MIN_FREE_BLK * 2);
	if(ptr != NULL){
		memcpy(np, ptr, f->size);
		gc_free(ptr);
	}
	return np;
}

int gc_isempty() {
	firstFree = first;
	_gc_clear_all();
	return (first->next == NULL) && (first->type == FREE) && (first->size == (gc_memsize - MEM_BLK_SIZE));
}

void gc_dump() {
	struct MemBlk *p = first;
	if(gc_isempty()) {
		return;
	}
	printf("Memory leaks detected!\n");
	printf("- Memory dump ---------------------------\n");
	printf("MEM_BLK_SIZE = %x\tMIN_FREE_BLK = %x\n", MEM_BLK_SIZE, MIN_FREE_BLK);
	printf("| %10s | %5s | %8s |\n", "ADDR", "STATE", "SIZE");
	while(p != NULL) {
		printf("| %10x | %5s | %8x |\n", (size_t)( ((void *)p) - ((void *)first) ), (p->type == FREE ? "FREE" : "BUSY"), p->size);
		p = p->next;
	}
}
