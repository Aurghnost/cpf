#include "unittest.h"

#include <stdint.h>

#include "allocators/arena_alloc.h"

#define PAGE_SIZE 0x1000

typedef struct foo_s foo_t;
struct foo_s {
	float bar;
	uint32_t toto;
};

unittest_init();
unittest(
	unittest_test("Creation/Destruction", {
		arena_allocator_t aa = arena_alloc_create_bt(PAGE_SIZE, foo_t);

		unittest_assert(aa.count == 0, "");
		unittest_assert(aa.block_size == 8, "");
		unittest_assert(aa.max_items == 512, "");

		arena_destroy(&aa);
	}, UNITTEST_DONE)
	unittest_test("Basic Usage", {
		arena_allocator_t foo_aa = arena_alloc_create_bt(PAGE_SIZE, foo_t);
		
		foo_t* ptr = arena_alloc(&foo_aa);
		unittest_assert(ptr != NULL, "");
		arena_free(&foo_aa, ptr);

		arena_destroy(&foo_aa);
	}, UNITTEST_DONE)
)
