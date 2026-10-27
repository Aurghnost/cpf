#include "allocators/arena_alloc.h"
#include <err.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>

arena_allocator_t arena_alloc_create(size_t pool_size, size_t block_size, size_t alignment) {
	size_t block_padding = (alignment - (block_size % alignment)) % alignment;
	block_size += block_padding;
	assert(block_size >= sizeof(free_node_t));

    arena_allocator_t al_alloc;

    void* mempool = mmap(
		NULL, 
		pool_size, 
		PROT_READ | PROT_WRITE, 
		MAP_ANONYMOUS | MAP_PRIVATE, // | MAP_HUGETLB -> can't work if OS has not huge pages configured, must use madvise
		-1,
		0
    );

    if (mempool == MAP_FAILED) {
		err(EXIT_FAILURE, "mempool <- arena_alloc_create.mmap");
    } else {
		al_alloc = (arena_allocator_t){
			.mempool = mempool,
			.pool_size = pool_size,
			.hd = NULL,
			.limit = mempool,
			.block_size = block_size,
			.max_items = pool_size/block_size,
			.count = 0,
		};
    }

    return al_alloc;
}

void* arena_alloc(arena_allocator_t* ar_alloc) {
    void* alloc_addr = NULL;
    if (ar_alloc->hd == NULL) {
		if (ar_alloc->max_items <= ar_alloc->count) {
			return NULL;
		}
		alloc_addr = ar_alloc->limit;
		ar_alloc->limit += ar_alloc->block_size;
    } else {
		alloc_addr = ar_alloc->hd;
		ar_alloc->hd = ar_alloc->hd->next;
	}
	ar_alloc->count++;
    return alloc_addr;
}

void arena_free(arena_allocator_t *ar_alloc, void* addr) {
	assert(addr != NULL);
	free_node_t* new_hd = addr;
	new_hd->next = ar_alloc->hd;
	ar_alloc->hd = new_hd;
	ar_alloc->count--;
}

void arena_destroy(arena_allocator_t *ar_alloc) {
    int status = munmap(ar_alloc->mempool, ar_alloc->pool_size);
	if (status == -1) {
		err(EXIT_FAILURE, "arena_destroy.munmap");
	}
}
