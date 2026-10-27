#include "containers/hashset.h"
#include "containers/hashset/bucket.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h> // rand, malloc, EXIT_FAILURE, EXIT_SUCCESS

#define SAFE_FREE(p) do {free(p); p = NULL;} while(0)

#define SAFE_ALLOC(allocator, nmb, size) ({ void* x = allocator(nmb * size); if (x == NULL) err(EXIT_FAILURE, "%s:%u", __FILE__, __LINE__); x;})

#define SAFE_CALLOC(allocator, nmb, size) ({ void* x = allocator(nmb, size); if (x == NULL) err(EXIT_FAILURE, "%s:%u", __FILE__, __LINE__); x;})

#define SAFE_REALLOCARRAY(allocator, arrptr, nmb, size) ({ void* x = allocator(arrptr, nmb, size); if (x == NULL) err(EXIT_FAILURE, "%s:%u", __FILE__, __LINE__); x;})

static inline uint64_t key_hash(hashfunction_t hf, uint64_t key) {
	return (hf.a * key + hf.b) % _HASH_P;
}
static inline uint64_t key_reduction(uint64_t f_k, uint64_t log_capacity) {
	return f_k & (~(((uint64_t)-1) << log_capacity));
}

hashset_t _hashset_new(hkey_hash hash, hkey_cmp cmp, hkey_destroy destroy, struct _hashset_new_opt opt) {
	hashset_t ht = (hashset_t){
		.hf = hashfunction_rand_new(),
		.log_capacity = _HASHSET_LOG2_DEFAULT_BASE_SIZE,
		.cardinal = 0,
		.table = calloc(1 << _HASHSET_LOG2_DEFAULT_BASE_SIZE, sizeof(bucket_t)),
		.bhead = NULL, // tous les buckets sont vides initialement
		.hash = hash,
		.cmp = cmp,
		.destroy = destroy,
	};
	if (ht.table == NULL) {
		err(EXIT_FAILURE, "ht.table <- hashset_new.calloc");
	}
	return ht;
}

void hashset_destroy(hashset_t *ht) {
	assert(ht != NULL);
	// TODO : peut être long
	// On peut donc avoir envie de programmer une allocation dynamique personnalisée avec mmap
	// pour accélérer la désallocation en désallouant simplement les pages
	unsigned long capacity = 1 << ht->log_capacity;
	for (unsigned long i = 0; i < capacity; i++) {
		bucket_deep_destroy(&ht->table[i], ht->destroy);
	}
	free(ht->table);
	memset(ht, 0, sizeof(*ht));
}

void hashset_shallow_destroy(hashset_t *ht) {
	assert(ht != NULL);
	// TODO : peut être long
	// On peut donc avoir envie de programmer une allocation dynamique personnalisée avec mmap
	// pour accélérer la désallocation en désallouant simplement les pages
	unsigned long capacity = 1 << ht->log_capacity;
	for (unsigned long i = 0; i < capacity; i++) {
		bucket_shallow_destroy(&ht->table[i]);
	}
	free(ht->table);
	memset(ht, 0, sizeof(*ht));
}

void hashset_expand(hashset_t *ht) {
	unsigned long new_capacity = 1 << (ht->log_capacity + 1);
	bucket_t *table = calloc(new_capacity , sizeof(bucket_t));
	for (unsigned long b = 0; b < (long unsigned)(1 << ht->log_capacity); b++) {
		for (unsigned inode = 0; inode < ht->table[b].length; inode++) {
			htkey_t key = ht->table[b].data[inode];
			unsigned long new_bucket = key_reduction(key_hash(ht->hf, ht->hash(key)), ht->log_capacity + 1); 
			bucket_push(&table[new_bucket], key);
		}
		bucket_shallow_destroy(&ht->table[b]);
	}
	free(ht->table);
	ht->table = table;
	ht->log_capacity++;
}

// C'est fait naïvement : move complet avec nouveaux hashs.
void hashset_rehash(hashset_t *ht) { 
	hashfunction_t new_hf = hashfunction_rand_new(); 
	ht->hf = new_hf;
	ht->bhead = NULL;
	bucket_t *table = calloc(1 << ht->log_capacity , sizeof(bucket_t));
	for (unsigned long b = 0; b < (long unsigned)(1 << ht->log_capacity); b++) {
		for (unsigned inode = 0; inode < ht->table[b].length; inode++) {
			htkey_t key = ht->table[b].data[inode];
			unsigned long new_bucket = key_reduction(key_hash(ht->hf, ht->hash(key)), ht->log_capacity); 
			bucket_push(&table[new_bucket], key);
			if (table[new_bucket].length == 1) {
				if (ht->bhead)
					ht->bhead->prev = &table[new_bucket];
				table[new_bucket].next = ht->bhead;
				ht->bhead = &table[new_bucket];
			}
		}
		bucket_shallow_destroy(&ht->table[b]);
	}
	free(ht->table);
	ht->table = table;
}

void hashset_add(hashset_t *ht, htkey_t key) {
	unsigned long capacity = 1 << ht->log_capacity;
	ht->cardinal++;
	// si alpha >= 1, on double la table
	if (ht->cardinal >= capacity) {
		hashset_expand(ht);
	}
	unsigned long bucket = key_reduction(key_hash(ht->hf, ht->hash(key)), ht->log_capacity);
	bool empty = ht->table[bucket].length == 0 || ht->bhead == NULL;

	bucket_push(&ht->table[bucket], key);
	
	if (empty) {
		if (ht->bhead)
			ht->bhead->prev = &ht->table[bucket];
		ht->table[bucket].next = ht->bhead;
		ht->bhead = &ht->table[bucket];
	}

	// si |bucket| >= 2*alpha = (n/|T|) * 2, on rehash en jouant sur les excellentes propriétés de l'espérance
	if (ht->table[bucket].length >= (ht->cardinal / capacity) << 1) {
		hashset_rehash(ht);
	}
}

bool hashset_del(hashset_t *ht, htkey_t key) {
	unsigned long bucket = key_reduction(key_hash(ht->hf, ht->hash(key)), ht->log_capacity);
	htkey_t* data = ht->table[bucket].data;
	if (data == NULL) {
		return false;
	}
	bool successful = false;
	for (unsigned inode = 0; inode < ht->table[bucket].length; inode++) {
		htkey_t ikey = data[inode];
		if (!ht->cmp(ikey, key)) {
			ht->destroy(ikey);
			ht->table[bucket].length--;
			data[inode] = data[ht->table[bucket].length];
			successful = true;
			break;
		}
	}
	if (ht->table[bucket].length == 0) {
		if (ht->table[bucket].prev) {
			ht->table[bucket].prev->next = ht->table[bucket].next;
		} else {
			ht->bhead = ht->table[bucket].next;
		}
		if (ht->table[bucket].next) {
			ht->table[bucket].next->prev = ht->table[bucket].prev;
		}
	}
	ht->cardinal -= successful ? 1 : 0;
	return successful;
}

bool hashset_mem(hashset_t *ht, htkey_t key) {
	return hashset_find(ht, key) != NULL;
}

htkey_t hashset_find(hashset_t *ht, htkey_t key) {
	unsigned long bucket = key_reduction(key_hash(ht->hf, ht->hash(key)), ht->log_capacity);
	for (unsigned inode = 0; inode < ht->table[bucket].length; inode++) {
		htkey_t ikey = ht->table[bucket].data[inode];
		if (!ht->cmp(ikey, key)) {
			return ikey;
		}
	}
	return NULL;

}

hashset_iterator_t hashset_iterator(const hashset_t * s) {
	hashset_iterator_t iter;
	iter.bucket = s->bhead;
	iter.inode = 0;
	return iter;
}

htkey_t hashset_iter_next(hashset_iterator_t* iter) {
	if (iter->bucket == NULL || iter->inode >= iter->bucket->length) {
		return NULL;
	}

	htkey_t key = iter->bucket->data[iter->inode];

	iter->inode++;
	if (iter->inode >= iter->bucket->length) {
		iter->bucket = iter->bucket->next;
		iter->inode = 0;
	}
	return key;
}

void hashset_iter(hashset_t *s, void(*f)(htkey_t)) {
	hashset_iterator_t iter = hashset_iterator(s);
	for (
		htkey_t item = hashset_iter_next(&iter);
		item != NULL;
		item = hashset_iter_next(&iter)
	) {
		f(item);
	}
}

hashset_list_t hashset_list_yield(const hashset_t * s) {
	htkey_t* keys = calloc(s->cardinal, sizeof(htkey_t));
	if (keys == NULL) {
		err(EXIT_FAILURE, "keys <- hashset_yield_list.calloc");
	}

	hashset_list_t l = {
		.keys = keys,
		.length = s->cardinal,
	};
	unsigned i = 0;
	hashset_iterator_t iter = hashset_iterator(s);
	for (
		htkey_t item = hashset_iter_next(&iter);
		item != NULL;
		(item = hashset_iter_next(&iter), i++)
	) {
		l.keys[i] = item;
	}
	return l;
}

