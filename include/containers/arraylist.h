#ifndef _ARRAYLIST_H_INCLUDED
#define _ARRAYLIST_H_INCLUDED

#include <stddef.h>

typedef unsigned char byte_t;

typedef struct arraylist_s {
	byte_t * buffer;
	size_t block_size; //! size in byte of an element
	size_t capacity; //! unit is elements (with size block_size) count
	unsigned length; //! unit is elements (with size block_size) count
} arraylist_t;

arraylist_t arraylist_new(size_t init_capacity, size_t block_size);
void arraylist_destroy(arraylist_t * al);

void arraylist_push(arraylist_t * al, void * data);
void arraylist_pop(arraylist_t * al);
void arraylist_pop_mult(arraylist_t * al, unsigned n);

#endif // !_ARRAYLIST_H_INCLUDED
