#include "containers/hashset/bucket.h"

#include <err.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define SAFE_FREE(p) do {free(p); p = NULL;} while(0)

#define SAFE_ALLOC(allocator, nmb, size) ({ void* x = allocator(nmb * size); if (x == NULL) err(EXIT_FAILURE, "%s:%u", __FILE__, __LINE__); x;})

#define SAFE_CALLOC(allocator, nmb, size) ({ void* x = allocator(nmb, size); if (x == NULL) err(EXIT_FAILURE, "%s:%u", __FILE__, __LINE__); x;})

#define SAFE_REALLOCARRAY(allocator, arrptr, nmb, size) ({ void* x = allocator(arrptr, nmb, size); if (x == NULL) err(EXIT_FAILURE, "%s:%u", __FILE__, __LINE__); x;})

void bucket_shallow_destroy(bucket_t * b) {
	free(b->data);
	memset(b, 0, sizeof(bucket_t));
}
void bucket_deep_destroy(bucket_t * b, hkey_destroy key_destroy) { 
	for (unsigned inode = 0; inode < b->length; inode++) {
		key_destroy(b->data[inode]);
	}
	free(b->data);
	memset(b, 0, sizeof(bucket_t));
}

void bucket_push(bucket_t * b, htkey_t key) {
	if (b->data == NULL) {
		b->data = SAFE_CALLOC(calloc, HASHSET_BUCKET_DEFAULT_CAPACITY, sizeof(htkey_t));
		b->capacity = HASHSET_BUCKET_DEFAULT_CAPACITY;
	}
	if (b->length == b->capacity) {
		b->capacity = b->capacity << 1;
		b->data = SAFE_REALLOCARRAY(reallocarray, b->data, b->capacity, sizeof(htkey_t));
	}
	b->data[b->length++] = key;
}

