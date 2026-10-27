#ifndef _HASHSET_H_INCLUDED
#define _HASHSET_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <err.h> // err, warn
#include <assert.h> // assert
#include <unistd.h> // exit

#define _HASHSET_LOG2_DEFAULT_BASE_SIZE 5 // default base size is 2^5 = 32
#define _HASHSET_DEFAULT_BASE_SIZE (1 << _HASHSET_LOG2_DEFAULT_BASE_SIZE)

#include "hashset/bucket.h"
#include "hashset/hashfunction.h"

// TODO : optimiser en implémentant par flat map + dynamic allocation with mmap and arena allocator
// on peut aussi utiliser SIMD pour fast check mem si full flat map de la table (voir C++ 2017 conf.)

/**
 * Defines a set data structure using hash table.
 * This can define sets on any type providing :
 * - a hash method 
 * - a object destroy method allowing to free allocated memory
 * - a comparator method allowing to check semantic equality
 *
 * @note the data structure may be used to represent hash maps from type A to type B 
 * by storing (*A, *B) in the hashset instead of just single pointer to data.
 *
 * @warning NOT thread safe
 */
typedef struct hashset_s {
    bucket_t *table; //! array indexed by hash values containing lists of elements
    bucket_t *bhead; //! list head of non empty buckets, NULL if the list is empty, allowing to iterate fast on the set elements
    hashfunction_t hf; //! currently used hashing parameters
    hkey_hash hash; //! key hash function pointer
    hkey_cmp cmp; //! keys comparator function pointer
    hkey_destroy destroy; //! key memory free function pointer
    unsigned int log_capacity; //! log2 of the number of buckets in the table
    unsigned int cardinal; //! count of elements in the table
} hashset_t;

struct _hashset_new_opt {
	bool local;
};

hashset_t _hashset_new(hkey_hash hash, hkey_cmp cmp, hkey_destroy destroy, struct _hashset_new_opt opt);

#define hashset_new(hash, cmp, destroy, ...) _hashset_new(hash, cmp, destroy, (struct _hashset_new_opt){.local=false,__VA_ARGS__})

void hashset_destroy(hashset_t *ht);
void hashset_shallow_destroy(hashset_t *ht);

void hashset_add(hashset_t *ht, htkey_t key);
bool hashset_del(hashset_t *ht, htkey_t key);
bool hashset_mem(hashset_t *ht, htkey_t key);
htkey_t hashset_find(hashset_t *ht, htkey_t key);

static inline unsigned hashset_card(hashset_t *s) {
    return s->cardinal;
}

/** Mutable iterator on a set data structure 
 * @warning the set can't be written during an iteration.
 * one must first compute the list of pointers to items 
 * using hashset_list_yield and then iterating on it
 */
typedef struct hashset_iterator_s {
    bucket_t* bucket; //! Current bucket
    unsigned inode; //! Current node index in the bucket
} hashset_iterator_t;

//! Initialize a iterator on a hashset
hashset_iterator_t hashset_iterator(const hashset_t * s); 

/** Go to the next element of the set and return it
 * @example 
	hashset_iterator_t iter = hashset_iterator(s); 
	for (
		lr1_item_t* item = hashset_iter_next(&iter);
		item;
		item = hashset_iter_next(&iter)
	) {
		...
	}
 */
htkey_t hashset_iter_next(hashset_iterator_t* iter);

static inline htkey_t hashset_iter_end() {
	return NULL;
}

typedef void (*hkey_func)(htkey_t);
void hashset_iter(hashset_t *s, hkey_func f);

/** Array based list of the set elements 
 * @note shallow destroy of the hashset do not affect the list elements validity,
 * but hashset_destroy invalidate the all elements !
 */
typedef struct hashset_list_s {
	htkey_t* keys;
	unsigned length;
} hashset_list_t;

//! Compute the list of all of the set elements
/**
 * @warning the list isn't affected by the addition of new elements after the call
 */
hashset_list_t hashset_list_yield(const hashset_t * s);

//! Free the list allocated memory
static inline void hashset_list_destroy(hashset_list_t l) {
	free(l.keys);
}
#endif
