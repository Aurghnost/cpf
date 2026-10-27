#include "unittest.h"

#include "containers/sparseset.h"

unittest_init();
unittest(
	unittest_test("Sparse Set creation", {
		sparseset_t s = sparseset_new(10, 20);

		unittest_assert(!sparseset_mem(&s, 8), "");
		sparseset_add(&s, 8);
		sparseset_add(&s, 2);
		sparseset_add(&s, 3);
		unittest_assert(!sparseset_mem(&s, 5), "");
		unittest_assert(sparseset_mem(&s, 8), "");
		unittest_assert(sparseset_mem(&s, 3), "");
		unittest_assert(!sparseset_del(&s, 5),);
		unittest_assert(sparseset_del(&s, 8), "");
		unittest_assert(sparseset_mem(&s, 3), "");
		unittest_assert(!sparseset_mem(&s, 8), "");

		sparseset_destroy(&s);
	}, UNITTEST_DONE);
	unittest_test("Sparse Set file writing/reading", {
		sparseset_t s1 = sparseset_new(10, 20);

		sparseset_add(&s1, 8);
		sparseset_add(&s1, 2);
		sparseset_add(&s1, 3);

		int test_fd = open("sparse.set", O_WRONLY | O_CREAT | O_TRUNC, 0666);

		sparseset_write(test_fd, &s1);

		close(test_fd);


		sparseset_t s2;

		test_fd = open("sparse.set", O_RDONLY);
		sparseset_read(test_fd, &s2);
		close(test_fd);

		unittest_assert(sparseset_eq(&s1, &s2), "");

		sparseset_destroy(&s1);
		sparseset_destroy(&s2);

	}, UNITTEST_DONE);
)
