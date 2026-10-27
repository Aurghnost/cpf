#include "containers/bitvector.h"

#include <err.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

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

#define CAPA_FROM_N(n) ({unsigned m = (n >> 3) + ((n & 7) != 0); (m>> 2) + ((m&3) != 0);})

bitvector_t bitvector_new(unsigned n) {
	assert(n > 0);
	unsigned capacity = CAPA_FROM_N(n);
	return (bitvector_t){
		.capacity = capacity,
		.array = (uint32_t*)SAFE_CALLOC(calloc, capacity, sizeof(uint32_t)),
	};
}

void bitvector_resize(bitvector_t *bv, unsigned n) {
	assert(n > 0);

	unsigned capacity = CAPA_FROM_N(n);
	bv->array = SAFE_REALLOCARRAY(reallocarray, bv->array, capacity, sizeof(uint32_t));
	bv->capacity = capacity;
}

void bitvector_destroy(bitvector_t *bv) {
	assert(bv != NULL && bv->array != NULL);

	free(bv->array);
	memset(bv, 0, sizeof(bitvector_t));
}

bool bitvector_mem(const bitvector_t *bv, unsigned x) {
	assert(bv != NULL);
	assert(bv->array != NULL);
	if (x >= (bv->capacity << 5)) {
		return false;
	}
	unsigned bloc = x >> 5;
	uint64_t mask = 1 << (x & 31);
	return (bv->array[bloc] & mask) != 0;
}

void bitvector_set(bitvector_t *bv, unsigned x, bool v) {
	assert(bv != NULL);
	if (v) {
		bitvector_add(bv, x);
	} else {
		bitvector_del(bv, x);
	}
}

void bitvector_add(bitvector_t *bv, unsigned x) {
	assert(bv != NULL);
	assert(x < (bv->capacity << 5));

	unsigned bloc = x >> 5;
	uint32_t mask = 1 << (x & 31);
	bv->array[bloc] |= mask;
}

void bitvector_del(bitvector_t *bv, unsigned x) {
	assert(bv != NULL);
	assert(x < (bv->capacity << 5));

	unsigned bloc = x >> 5;
	uint32_t mask = 1 << (x & 31);
	bv->array[bloc] &= ~mask;
}

bool bitvector_union(bitvector_t *dest, const bitvector_t *op) {
	assert(dest->capacity >= op->capacity);
	bool mod = false;
	for (unsigned i = 0; i < op->capacity; i++) {
		uint32_t x = dest->array[i];
		dest->array[i] |= op->array[i];
		mod = mod || x != dest->array[i];
	}
	return mod;
}

bool bitvector_inter(bitvector_t *dest, const bitvector_t *op) {
	assert(dest->capacity >= op->capacity);
	bool mod = false;
	for (unsigned i = 0; i < op->capacity; i++) {
		unsigned long x = dest->array[i];
		dest->array[i] &= op->array[i];
		mod = mod || x != dest->array[i];
	}
	return mod;
}

void bitvector_clear(bitvector_t *bv) {
	memset(bv->array, 0, bv->capacity << 2);
}

void bitvector_prettyprint(FILE *fd, const bitvector_t bv) {
	unsigned max = bv.capacity << 5;
	fprintf(fd, "{");
	for (unsigned i = 0; i < max; i++) {
		if (bitvector_mem(&bv, i)) {
			fprintf(fd, "%u,", i);
		}
	}
	fprintf(fd, "}");
}

unsigned bitvector_cardinal(const bitvector_t bv) {
	unsigned card = 0;
	for (unsigned i = 0; i < bv.capacity; i++) {
		card += __builtin_popcount(bv.array[i]);
	}
	return card;
}

bool bitvector_is_equal_set(const bitvector_t bv, unsigned cardinal, ...) {
	va_list args;
	va_start(args, cardinal);

	bool ok = cardinal == bitvector_cardinal(bv);
	for (unsigned i = 0; i < cardinal; i++) {
		unsigned x = va_arg(args, unsigned);
		ok = ok && bitvector_mem(&bv, x);
		if (!ok) {
			va_end(args);
			return false;
		}
	}

	va_end(args);

	return true;
}

bitvector_iterator_t bitvector_iterator(const bitvector_t * bv) {
	return (bitvector_iterator_t){
		.bv = bv,
		.index = 0,
		.x = bv->array[0],
	};
}

unsigned bitvector_iter_next(bitvector_iterator_t * it) {
	while (it->index < it->bv->capacity && it->x == 0) {
		it->x = it->bv->array[it->index];
		it->index++;
	}
	if (it->index == it->bv->capacity) {
		return bitvector_iter_end();
	}
	unsigned v = __builtin_ffs(it->x) - 1;
	it->x -= it->x & (-it->x);
	return v;
}
