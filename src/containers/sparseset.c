#include "containers/sparseset.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <err.h>
#include <stdarg.h>

#define eprintf(msg, ...) fprintf (stderr, msg "\n", __VA_ARGS__)

#define SAFE_FREE(p) do {free(p); p = NULL;} while(0)

#define SAFE_ALLOC(allocator, nmb, size) ({ void* x = allocator(nmb * size); if (x == NULL) err(EXIT_FAILURE, "%s:%u", __FILE__, __LINE__); x;})

#define SAFE_CALLOC(allocator, nmb, size) ({ void* x = allocator(nmb, size); if (x == NULL) err(EXIT_FAILURE, "%s:%u", __FILE__, __LINE__); x;})

#define SAFE_REALLOCARRAY(allocator, arrptr, nmb, size) ({ void* x = allocator(arrptr, nmb, size); if (x == NULL) err(EXIT_FAILURE, "%s:%u", __FILE__, __LINE__); x;})

sparseset_t sparseset_new(const uint32_t capacity, const eid_t max_id) {
	void* buffer = SAFE_ALLOC(malloc, capacity * sizeof(eid_t) + max_id * sizeof(uint32_t), 1);
	eid_t *dense = buffer;
	uint32_t *sparse = &dense[capacity];
	sparseset_t s = {
		.dense = dense,
		.sparse = sparse,
		.max_id = max_id,
		.capacity = capacity,
		.card = 0
	};
	return s;
}

void sparseset_destroy(sparseset_t *s) {
	free(s->dense);
	memset(s, 0, sizeof(sparseset_t));
}

bool sparseset_mem(const sparseset_t *s, const eid_t id) {
	uint32_t a = s->sparse[id];
	return a < s->card && id == s->dense[a];
}

eid_t sparseset_sparse_to_dense(const sparseset_t *s, const eid_t id) {
	assert(id < s->max_id);
	assert(sparseset_mem(s, id));
	return s->sparse[id];
}

eid_t sparseset_dense_to_sparse(const sparseset_t *s, const eid_t id) {
	assert(id < s->card);
	return s->dense[id];
}

void sparseset_add(sparseset_t *s, const eid_t id) {
	assert(id < s->max_id);
	assert(s->card < s->capacity);

	if (sparseset_mem(s, id)) {
		#ifndef NDEBUG 
		warn("sparseset_add(%p, %u)\n", s, id);
		#endif
		return;
	}

	s->dense[s->card] = id;
	s->sparse[id] = s->card;
	s->card++;
}

bool sparseset_del(sparseset_t *s, const eid_t id) {
	if (!sparseset_mem(s, id)) {
		return false;
	}
	s->card--;
	uint32_t place = s->sparse[id];
	key_t last = s->dense[s->card];
	s->sparse[last] = place;
	s->dense[place] = last;
	s->sparse[id] = s->card;
	return true;
}

void sparseset_prettyprint(FILE* fd, const sparseset_t *s) {
	fprintf(fd, "{");
	for (unsigned i = 0; i < s->card; i++) {
		fprintf(fd, "%u,", s->dense[i]);
	}
	fprintf(fd, "}");
}

ssize_t sparseset_write(int fd, const sparseset_t *s) {
	assert(s->sparse != NULL && s->dense != NULL);

	ssize_t wb = 0;
	ssize_t status;

	status = write(fd, &s->capacity, sizeof(s->capacity));
	if (status == -1) {
		err(EXIT_FAILURE, "sparseset_write(capacity)");
	}
	wb += status;

	status = write(fd, &s->max_id, sizeof(s->max_id));
	if (status == -1) {
		err(EXIT_FAILURE, "sparseset_write(max_id)");
	}
	wb += status;

	status = write(fd, &s->card, sizeof(s->card));
	if (status == -1) {
		err(EXIT_FAILURE, "sparseset_write(card)");
	}
	wb += status;

	status = write(fd, s->dense, s->card * sizeof(*s->dense));
	if (status == -1) {
		err(EXIT_FAILURE, "sparseset_write(dense)");
	}
	wb += status;

	const ssize_t write_need = sizeof(s->capacity) + sizeof(s->max_id) + sizeof(s->card) + s->card * sizeof(*s->dense);
	if (wb != write_need) {
		eprintf("sparseset_write write %lu bytes instead of %lu", wb, write_need);
	}

	return wb;
}

ssize_t sparseset_read(int fd, sparseset_t *s) {
	assert(s->dense == NULL && s->sparse == NULL);

	ssize_t rb = 0;
	ssize_t status;

	status = read(fd, &s->capacity, sizeof(s->capacity));
	if (status == -1) {
		err(EXIT_FAILURE, "sparseset_read(capacity)");
	}
	rb += status;

	status = read(fd, &s->max_id, sizeof(s->max_id));
	if (status == -1) {
		err(EXIT_FAILURE, "sparseset_read(max_id)");
	}
	rb += status;

	status = read(fd, &s->card, sizeof(s->card));
	if (status == -1) {
		err(EXIT_FAILURE, "sparseset_read(card)");
	}
	rb += status;

	const ssize_t read_need = sizeof(s->capacity) + sizeof(s->max_id) + sizeof(s->card);
	if (rb != read_need) {
		eprintf("sparseset_read reads %lu bytes instead of %lu", rb, read_need);
	}

	void * buffer = SAFE_ALLOC(malloc, s->capacity * sizeof(eid_t) + s->max_id * sizeof(uint32_t), 1);
	s->dense = buffer;
	status = read(fd, s->dense, s->card * sizeof(*s->dense));
	if (status == -1) {
		err(EXIT_FAILURE, "sparseset_read(dense)");
	}
	if ((unsigned long)status != s->card * sizeof(*s->dense)) {
		eprintf("sparseset_read(dense) reads %lu bytes instead of %lu", status, s->card * sizeof(*s->dense));
	}
	rb += status;

	s->sparse = &s->dense[s->capacity];
	for (unsigned i = 0; i  < s->card; i++) {
		s->sparse[s->dense[i]] = i;
	}

	return rb;
}

bool sparseset_is_equal_set(const sparseset_t * s, unsigned cardinal, ...) {
	va_list args;
	va_start(args, cardinal);

	bool equals = cardinal == s->card;
	for (unsigned i = 0; equals && i < cardinal; i++) {
		unsigned x = va_arg(args, unsigned);
		equals = equals && sparseset_mem(s, x);
	}

	va_end(args);

	return equals;
}

bool sparseset_eq(const sparseset_t * s1, const sparseset_t * s2) {
	if (s1->card != s2->card) {
		return false;
	}
	bool equals = true;
	for (unsigned i = 0; equals && i < s1->card; i++) {
		equals = equals && sparseset_mem(s2, s1->dense[i]);
	}
	return equals;
}
