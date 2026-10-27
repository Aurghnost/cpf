#ifndef _LEXEM_H_INCLUDED
#define _LEXEM_H_INCLUDED

#include "string.h"
#include <assert.h>
#include <stdbool.h>

typedef unsigned symbol_t;
typedef struct lexem_s {
  symbol_t symbol; //! abstract symbol
  string_t repr; //! string representation of the abstract symbol
} lexem_t;

//! Free memory a lexem's memory
void lexem_destroy(lexem_t);

typedef struct lexem_node_t lexem_node_t;
struct lexem_node_t {
  lexem_t lex;
  lexem_node_t *next;
};
typedef struct lexem_list_t {
  unsigned length; //! Taille de la liste
  lexem_node_t *head;  //! Pointeur vers le premier élément de la liste
} lexem_list_t;

typedef void (*lexem_function)(lexem_t);

/**
 * Renvoie la liste vide (de longueur 0)
 */
lexem_list_t lexem_list_new();

/**
 * Ajoute un lexème en tête d'une liste
 * @param[in, out] list Liste à laquelle ajouter le lexème.
 * @param[in] lex Lexème à ajouter
 */
void lexem_list_push(lexem_list_t *list, lexem_t lex);

/**
 * Supprime  et renvoie le lexème en tête d'une liste
 * @param[in, out] list Liste de laquelle supprimer la tête
 * @pre lexem_list_is_empty(list) == false
 */
lexem_t lexem_list_pop(lexem_list_t *list);

/**
 * Inverse la liste en entrée.
 * @param[in, out] list Liste à inverser
 */
void lexem_list_reverse(lexem_list_t *list);

/**
 * Libère la mémoire allouée pour une liste lexèmes.
 * Libère également les chaînes de caractères représentant chaque lexème.
 * @param[in, out] list Liste à libérer
 * @post lexem_list_is_empty(list) == true
 */
void lexem_list_destroy(lexem_list_t *list);

/**
 * @param list Liste d'entrée
 * @return true ssi list est vide
 */
bool lexem_list_is_empty(const lexem_list_t list);

bool lexem_list_is_equal(const lexem_list_t list, unsigned length, ...);
void lexem_list_iter(const lexem_list_t list, lexem_function f);
#endif
