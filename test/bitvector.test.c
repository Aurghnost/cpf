#include "unittest.h"

#include "containers/bitvector.h"

unittest_init();
unittest(
	unittest_test("bitvector_t capacity", {
		bitvector_t bv = bitvector_new(144);
		unittest_assert(bv.capacity == 5, "Capacity should be %u instead of %u", 5, bv.capacity);
		bitvector_resize(&bv, 193);
		unittest_assert(bv.capacity == 7, "Capacity should be %u instead of %u", 7, bv.capacity);
		bitvector_destroy(&bv);
		unittest_assert(bv.capacity == 0 && bv.array == NULL, "Data structure should be nullified");
	}, UNITTEST_DONE);
	unittest_test("bitvector_t usage", {
		bitvector_t a = bitvector_new(150);
		bitvector_t b = bitvector_new(150);

		unittest_assert(!bitvector_mem(&a, 200), "200 shouldn't belong to [a]");
		unittest_assert(!bitvector_mem(&a, 42), "42 shouldn't belong to [a]");
		bitvector_add(&a, 42);
		unittest_assert(bitvector_mem(&a, 42), "42 should belong to [a]");
		bitvector_add(&b, 98);
		unittest_assert(bitvector_mem(&b, 98), "98 should belong to [b]");
		bitvector_union(&a, &b);
		unittest_assert(bitvector_mem(&a, 98), "98 should belong to [a] after a <- a U b");
		unittest_assert(!bitvector_mem(&b, 42), "[b] shouldn't be modified by a <- a U b");
		bitvector_add(&b, 123);
		unittest_assert(bitvector_mem(&b, 123), "123 should belong to [b]");
		bitvector_inter(&b, &a);
		unittest_assert(!bitvector_mem(&b, 123), "123 shouldn't belong to [b] after b <- a n b");
		unittest_assert(bitvector_mem(&b, 98), "98 should belong to [b] after b <- a n b");

		bitvector_del(&b, 98);
		unittest_assert(!bitvector_mem(&b, 98), "[b] shoud be empty");

		bitvector_destroy(&a);
		bitvector_destroy(&b);
	}, UNITTEST_DONE);
	unittest_test("bitvector cardinal", {
		unittest_assert(__builtin_popcount(2922) == 7, "");
		unittest_assert(__builtin_popcount(4095) == 12, "");
		bitvector_t a = bitvector_new(12);
		bitvector_add(&a, 4);
		bitvector_add(&a, 11);
		bitvector_add(&a, 8);
		unittest_assert(bitvector_cardinal(a) == 3, "");
		bitvector_destroy(&a);
	}, UNITTEST_DONE);
	unittest_test("bitvector iterator", {
		bitvector_t a = bitvector_new(12);
		bitvector_add(&a, 4);
		bitvector_add(&a, 11);
		bitvector_add(&a, 8);
		bitvector_iterator_t it = bitvector_iterator(&a);
		for (
			unsigned v = bitvector_iter_next(&it);
			v != bitvector_iter_end();
			v = bitvector_iter_next(&it)
		) {
			fprintf(stderr, "%u\n", v);
		}
		bitvector_destroy(&a);
	}, UNITTEST_DONE);
)
