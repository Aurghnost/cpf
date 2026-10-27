#ifndef _BUCKET_H_INCLUDED
#define _BUCKET_H_INCLUDED

#include <stdint.h>

#define HASHSET_BUCKET_DEFAULT_CAPACITY 16

typedef void* htkey_t;

typedef uint64_t (*hkey_hash)(htkey_t);
typedef void (*hkey_destroy)(htkey_t);
typedef int (*hkey_cmp)(htkey_t, htkey_t);

typedef struct bucket_s bucket_t;
struct bucket_s {
    bucket_t* prev; //! pointer to previous non empty bucket of the bucket list
    bucket_t* next; //! pointer to next non empty bucket of the bucket list
    htkey_t *data; //! array list of keys
	unsigned int capacity; //! capacity of the data array
    unsigned int length; //! count of the list elements
};

//! Destroys nodes memory AND keys memory using the provided key destroyer
void bucket_deep_destroy(bucket_t * b, hkey_destroy);

//! Destroys only the bucket nodes memory but NOT the keys allocated memory
void bucket_shallow_destroy(bucket_t * b);

//! Adds a new element to the bucket
void bucket_push(bucket_t * b, htkey_t key);

#endif
