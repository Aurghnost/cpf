#ifndef _BITVECTOR_H_INCLUDED
#define _BITVECTOR_H_INCLUDED

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct bitvector_s {
  unsigned capacity; //! Number of bits in the vector
  uint32_t *array; //! Array of bits
} bitvector_t;

/**
 * Allocate memory for a bitvector of at least capacity bits
 * @param capacity Lower bound of bitvector size
 */
bitvector_t bitvector_new(unsigned capacity);

/**
 * Free memory of the allocated bitvector.
 * Nullify the structure.
 *
 * @param[in, out] bv Bitvector to free
 */
void bitvector_destroy(bitvector_t *bv);

/**
 * @brief Change the capacity of the bitvector.
 * @details If the new capacity is less than the older on, elements not fitting are deleted of the set.
 *
 * @param[in, out] bv Bitvector to resize
 * @param capacity New capacity
 */
void bitvector_resize(bitvector_t *bv, unsigned capacity);

/**
 * Check if x belong to the bitvector
 * @param[in] bv Bitvector
 * @param x Element whose presence is checked
 * @return true iif x fit withing the bitvector capacity AND the bit x of bv->array is 1
 */
bool bitvector_mem(const bitvector_t *bv, unsigned x);

/**
 * Add x to the bitvector
 * @param[in, out] bv Bitvector
 * @param x Element to add
 * @pre x fit within the bitvector capacity
 */
void bitvector_add(bitvector_t *bv, unsigned x);

/**
 * Delete x from the bitvector
 * @param[in, out] bv Bitvector
 * @param x Element to delete
 * @pre x fit within the bitvector capacity
 * @note x may not belong to the bitvector before "deletion"
 */
void bitvector_del(bitvector_t *bv, unsigned x);

/**
 * Make the bitvector a null vector <=> empty set
 */
void bitvector_clear(bitvector_t *bv);

void bitvector_set(bitvector_t *bv, unsigned x, bool v);

/**
 * Dest <- dest U op
 * @return true if dest is modified
 */
bool bitvector_union(bitvector_t *dest, const bitvector_t *op);

/**
 * Dest <- Dest n op
 * @return true if dest is modified
 */
bool bitvector_inter(bitvector_t *dest, const bitvector_t *op);

//! Print the bitvector in a file as a integer set
void bitvector_prettyprint(FILE *fd, const bitvector_t bv);

/**
 * @return the number of elements in the set represented by the bitvector
 */
unsigned bitvector_cardinal(const bitvector_t bv);

/**
 * @return true if the bitvector set is exactly the set given as variadic args integers and false otherwise
 */
bool bitvector_is_equal_set(const bitvector_t bv, unsigned cardinal, ...);

typedef struct bitvector_iterator_s {
	const bitvector_t * bv;
	unsigned index;
	unsigned x;
} bitvector_iterator_t;

bitvector_iterator_t bitvector_iterator(const bitvector_t * bv);
unsigned bitvector_iter_next(bitvector_iterator_t * it);
static inline unsigned bitvector_iter_end() {
	return (unsigned)(-1);
}

#endif
