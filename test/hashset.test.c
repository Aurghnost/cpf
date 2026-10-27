#include "unittest.h"

#include <string.h>
#include "containers/hashset.h"

// naif juste pour tester
uint64_t string_hash(const char * str) {  
    uint64_t sum = 0;
    while (*str != '\0') {
	sum = sum << 8; // * 256
	sum += *(str++);
    }
    return sum;
}

unittest_init();
unittest(
    unittest_test("hashtable basic usage", {
		hashset_t s = hashset_new((hkey_hash)string_hash, (hkey_cmp)strcmp, (hkey_destroy)free);

		hashset_add(&s, strdup("Salut"));
		hashset_add(&s, strdup("Bonjour"));
		unittest_assert(s.cardinal == 2, "");
		unittest_assert(hashset_mem(&s, "Salut"), "");
		unittest_assert(!hashset_mem(&s, "Au revoir !"), "");
		unittest_assert(hashset_mem(&s, "Bonjour"), "");

		hashset_del(&s, "Salut");
		unittest_assert(!hashset_mem(&s, "Salut"), "");
		unittest_assert(s.cardinal == 1, "");

		hashset_destroy(&s);
    }, UNITTEST_DONE);

    unittest_test("hashtable iterator on full add", {
		hashset_t s = hashset_new((hkey_hash)string_hash, (hkey_cmp)strcmp, (hkey_destroy)free);

		hashset_add(&s, strdup("Salut"));
		hashset_add(&s, strdup("Coucou"));
		hashset_add(&s, strdup("Bonjour"));
		hashset_add(&s, strdup("Bonsoir"));

		unittest_assert(s.cardinal == 4, "");

		hashset_iterator_t iter = hashset_iterator(&s); 
		unittest_assert(iter.bucket != NULL, "");
		unittest_assert(iter.inode < iter.bucket->length, "");
		const char* item = hashset_iter_next(&iter);
		unittest_assert(!strcmp(item, "Bonsoir"), "");
		item = hashset_iter_next(&iter);
		unittest_assert(!strcmp(item, "Bonjour"), "");
		item = hashset_iter_next(&iter);
		unittest_assert(!strcmp(item, "Salut"), "");
		item = hashset_iter_next(&iter);
		unittest_assert(!strcmp(item, "Coucou"), "");
		item = hashset_iter_next(&iter);
		unittest_assert(item == NULL, "");

		hashset_destroy(&s);
    }, UNITTEST_DONE);

    unittest_test("hashtable iterator with add and del (1)", {
		hashset_t s = hashset_new((hkey_hash)string_hash, (hkey_cmp)strcmp, (hkey_destroy)free);

		hashset_add(&s, strdup("Salut"));
		hashset_add(&s, strdup("Coucou"));
		hashset_add(&s, strdup("Bonjour"));
		hashset_add(&s, strdup("Bonsoir"));
		hashset_del(&s, "Bonjour");

		unittest_assert(s.cardinal == 3, "");

		hashset_iterator_t iter = hashset_iterator(&s); 
		unittest_assert(iter.bucket != NULL, "");
		unittest_assert(iter.inode < iter.bucket->length, "");

		const char* item = hashset_iter_next(&iter);
		unittest_assert(!strcmp(item, "Bonsoir"), "");
		item = hashset_iter_next(&iter);
		unittest_assert(!strcmp(item, "Salut"), "");
		item = hashset_iter_next(&iter);
		unittest_assert(!strcmp(item, "Coucou"), "");
		item = hashset_iter_next(&iter);
		unittest_assert(item == NULL, "");

		hashset_destroy(&s);
    }, UNITTEST_DONE);

    unittest_test("hashtable iterator with add and del (2)", {
		hashset_t s = hashset_new((hkey_hash)string_hash, (hkey_cmp)strcmp, (hkey_destroy)free);

		hashset_add(&s, strdup("Salut"));
		hashset_add(&s, strdup("Coucou"));
		hashset_add(&s, strdup("Bonjour"));
		hashset_add(&s, strdup("Bonsoir"));
		hashset_del(&s, "Bonjour");
		hashset_del(&s, "Bonsoir");
		hashset_del(&s, "Coucou");
		hashset_del(&s, "Salut");

		unittest_assert(s.cardinal == 0, "");

		hashset_iterator_t iter = hashset_iterator(&s); 
		unittest_assert(iter.bucket == NULL, "");

		const char* item = hashset_iter_next(&iter);
		unittest_assert(item == NULL, "");

		hashset_destroy(&s);
    }, UNITTEST_DONE);
)
