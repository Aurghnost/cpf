#ifndef _SPARSE_SET_H_INCLUDED
#define _SPARSE_SET_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef uint32_t eid_t; //! Identifiant des entités

struct sparseset_s {
	eid_t *dense; //! Set of elements ids (primary key)
	uint32_t *sparse; //! Array of keys index
	uint32_t card; //! Number of elements in the set
	eid_t max_id; //! Length of sparse Array
	uint32_t capacity; //! Length of dense array
};
typedef struct sparseset_s sparseset_t;

//! Alloue et initialise un ensemble creux
/*!
 * @param capacity Le nombre d'éléments maximum de l'ensemble
 * @param max_value La valeur maximum exclusive des éléments de l'ensemble (max_value > 0)
 * @return Un ensemble vide
 */
sparseset_t sparseset_new(const uint32_t capacity, const eid_t max_value);

//! Détruit l'ensemble s
/*!
 * La mémoire allouée est libérée. Les champs de la structure sont remis à 0.
 */
void sparseset_destroy(sparseset_t *s);

//! Renvoie le cardinal de l'ensemble passé en argument
static inline uint32_t sparseset_cardinal(const sparseset_t *s) {
	return s->card;
}

//! Renvoie vrai ssi x appartient à s
bool sparseset_mem(const sparseset_t *s, const eid_t id);

eid_t sparseset_sparse_to_dense(const sparseset_t *s, const eid_t id);
eid_t sparseset_dense_to_sparse(const sparseset_t *s, const eid_t id);

//! Ajoute un élément dans l'ensemble
void sparseset_add(sparseset_t *s, const eid_t id);

//! Supprime un élément de l'ensemble
bool sparseset_del(sparseset_t *s, const eid_t id);

//! Enregistre les informations d'un l'ensemble creux dans un fichier
ssize_t sparseset_write(int fd, const sparseset_t *s);

//! Lit les informations d'un ensemble creux depuis un fichier
/**
 * la lecture suit exactement le format d'écriture de sparseset_write
 */
ssize_t sparseset_read(int fd, sparseset_t *s);

//! Affiche l'ensemble formaté
void sparseset_prettyprint(FILE* fd, const sparseset_t *s);

/**
 * @return true if the sparse set is exactly the set given as variadic args integers and false otherwise
 */
bool sparseset_is_equal_set(const sparseset_t * s, unsigned cardinal, ...);

/** 
 * @return true iff both sparsesets are equals.
 */
bool sparseset_eq(const sparseset_t * s1, const sparseset_t * s2);
#endif
