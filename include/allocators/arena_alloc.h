#ifndef _ARENA_ALLOC_H_INCLUDED
#define _ARENA_ALLOC_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>

typedef struct free_node_s free_node_t;
struct free_node_s {
	free_node_t* next;
};

typedef struct arena_allocator_s {
    void* mempool; //! pointer to allocated pages
    size_t pool_size; //! size of allocated pages
    free_node_t *hd; //! First free block to allocate, NULL if no space
    free_node_t *limit; //! Last free block out of the allocated items
    size_t block_size; //! Size of a single allocated elemen
	uint64_t count; //! Count of used blocks 
	uint64_t max_items; //! max usable blocks
} arena_allocator_t;

//! Create an arena allocator and allocate a memory pool
arena_allocator_t arena_alloc_create(size_t pool_size, size_t block_size, size_t alignment);
#define arena_alloc_create_bt(pool_size, T) arena_alloc_create((pool_size), sizeof(T), alignof(T))

//! Allocate a new object in the arena allocator 
void* arena_alloc(arena_allocator_t* ar_alloc);

//! Desallocate a object from an arena allocator
void arena_free(arena_allocator_t *ar_alloc, void* addr);

//! Desallocate the memory pool of the arena allocator
void arena_destroy(arena_allocator_t* ar_alloc);

#endif
