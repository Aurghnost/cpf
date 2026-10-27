#include "containers/arraylist.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <err.h>

#define SAFE_FREE(p) do {free(p); p = NULL;} while(0)

#define SAFE_ALLOC(allocator, nmb, size) ({ \
	void* x = allocator(nmb * size); \
	if (x == NULL) \
		err(EXIT_FAILURE, "%s:%u", __FILE__, __LINE__); \
	x;\
})

#define SAFE_CALLOC(allocator, nmb, size) ({ \
	void* x = allocator(nmb, size); \
	if (x == NULL) \
		err(EXIT_FAILURE, "%s:%u", __FILE__, __LINE__); \
	x;\
})

#define SAFE_REALLOCARRAY(allocator, arrptr, nmb, size) ({ \
	void* x = allocator(arrptr, nmb, size); \
	if (x == NULL) \
		err(EXIT_FAILURE, "%s:%u", __FILE__, __LINE__); \
	x;\
})

arraylist_t arraylist_new(size_t init_capacity, size_t block_size) {
	return (arraylist_t){
		.buffer = SAFE_CALLOC(calloc, init_capacity, block_size),
		.length = 0,
		.capacity = init_capacity,
		.block_size = block_size,
	};
}
void arraylist_destroy(arraylist_t * al) {
	free(al->buffer);
	memset(al, 0, sizeof(*al));
}

void arraylist_push(arraylist_t * al, void * data) {
	if (al->length >= al->capacity) {
		al->capacity *= 2;
		al->buffer = SAFE_REALLOCARRAY(reallocarray, al->buffer, al->capacity, al->block_size);
	}
	if (data != NULL) {
		memcpy(al->buffer + al->length * al->block_size, data, al->block_size);
	}
	al->length++;
}
void arraylist_pop(arraylist_t * al) {
	assert(al->length > 0);
	al->length--;
}

void arraylist_pop_mult(arraylist_t * al, unsigned n) {
	assert(al->length >= n);
	al->length -= n;
}
