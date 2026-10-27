#ifndef _CFG_H_INCLUDED
#define _CFG_H_INCLUDED

#include <stdbool.h>
#include "containers/hashset.h"
#include "containers/sparseset.h"
#include "containers/bitvector.h"
#include "containers/arraylist.h"
#include "parser/types.h"

#define SAFE_FREE(p) do {free(p); p = NULL;} while(0)

#define SAFE_ALLOC(allocator, nmb, size) ({ void* x = allocator(nmb * size); if (x == NULL) err(EXIT_FAILURE, "%s:%u", __FILE__, __LINE__); x;})

#define SAFE_CALLOC(allocator, nmb, size) ({ void* x = allocator(nmb, size); if (x == NULL) err(EXIT_FAILURE, "%s:%u", __FILE__, __LINE__); x;})

typedef struct prod_s prod_t;
typedef unsigned rule_symbol_t;
struct prod_s {
	rule_symbol_t *array; //! List of concatenation rule symbols where array == NULL <==> \varepsilon
	unsigned length; //! Number of concatenation rule symbols
};

prod_t prod_new(unsigned length, ...);
ssize_t prod_write(int fd, prod_t* p);

#define CFG_DEFAULT_RULE_CAPACITY 8
typedef struct prod_list_s prod_list_t;
struct prod_list_s {
	prod_t *alternatives; //! List array of alternatives concatenation production rules
	unsigned capacity; //! Capacity of the alternatives array
	unsigned length; //! Number of alternatives concatenation rules
};
typedef prod_list_t rule_t;

rule_t rule_new(unsigned capacity);
void rule_destroy(rule_t *r);
unsigned rule_add_prod(rule_t *r, prod_t p);
ssize_t rule_write(int fd, rule_t *r);

typedef enum {
	IS_TOKEN = 1 << 0, //! Symbol string derivation is recorded
	IS_TERMINAL = 1 << 1, //! Symbol is a leaf
	IS_CONCRETE = 1 << 2, //! Symbol is ignored in the abstract tree
} symbol_prop_t;

typedef struct cfg_s cfg_t;
struct cfg_s {
	symbol_prop_t *sym_props; //! Properties of symbols
	sparseset_t unterminals; //! Terminal symbols
	sparseset_t terminals; //! Non Terminal symbols
	rule_symbol_t axiom; //! Index of the axiomatic rule in rules array
	rule_t *rules; //! Array of grammar rules of length unterminals.capacity == terminals.capacity
};

/**
 * Allocate and initialize memory for an empty grammar
 * @param card_symbols Number of symbols of the grammar
 * @param axiom Axiom symbol of the grammar
 * @pre axiom < card_symbols
 */
cfg_t cfg_new(unsigned axiom, unsigned card_symbols);

//! Free memory allocated for a grammar data entity
void cfg_destroy(cfg_t *cfg);

ssize_t cfg_write(int fd, const cfg_t * cfg);
ssize_t cfg_read(int fd, cfg_t* cfg);

void cfg_set_axiom(cfg_t *cfg, unsigned axiom);

unsigned cfg_add_prod(cfg_t cfg, unsigned symbol, prod_t p);

/**
 * Set a symbol as a token of the grammar (i.e. a terminal symbol)
 */
void cfg_add_token(cfg_t *cfg, unsigned symbol);
void cfg_add_terminal(cfg_t *cfg, unsigned symbol);
void cfg_add_concrete(cfg_t *cfg, unsigned symbol);

unsigned cfg_eof(const cfg_t cfg);

cfg_t cfg_from_file(const char *filename);

/**
 * Compute the First sets of a grammar
 * If T is non terminal, First(T) is the set of the first terminal symbols which can produce T by left production
 * represented by a vector V of MAX_SYMBOL bits where MAX_SYMBOL is the greatest inteeger representing a lexem symbol
 *
 */
bitvector_t* cfg_first(const cfg_t g);

/**
 * Compute the Follow sets of a grammar with the same representation as First sets
 */
bitvector_t* cfg_follow(const cfg_t g, const bitvector_t *first);

typedef struct {
	rule_symbol_t left; //! Index/Symbol of the rule left non terminal
	unsigned pindex; //! Index of the production
} prod_coord_t;

typedef prod_coord_t *table_ll1_t;
/**
 * Compute the LL(1) table of a grammar
 * A LL(1) table is a matrix of productions indexed by the rule left symbol and the alternative indice in the rule
 * To be usable, the LL(1) table needs the grammar.
 * @return LL(1) table of g if g is LL(1), NULL otherwise
 *
 * @warning the table use g for its transitions. g must not be freed before finishing using the table
 */
table_ll1_t cfg_table_ll1(const cfg_t g, const bitvector_t *first, const bitvector_t *follow);

//! Save a LL(1) table and its associated grammar in a file
/**
 * @note To be usable as a standalone, the table needs the grammar productions, 
 * so is written in the file both the LL(1) grammar and the LL(1) table
 */
ssize_t table_ll1_write(int fd, const cfg_t * g, const table_ll1_t table);

//! Load a LL(1) table and its associated grammar from a file
/** 
 * @note To be usable as a standalone, the table needs the grammar productions, 
 * so is written in the file both the LL(1) grammar and the LL(1) table
 */
ssize_t table_ll1_read(int fd, cfg_t* g, table_ll1_t* table);

/**
 * LR(0) items are sorted by lexicographic order (prod, placeholder).
 * It guarantee the equivalence (memory equality <=> logical equality). Order is needed to ensure <==.
 */
typedef struct lr0_item_s {
	rule_symbol_t left; //! non terminal left symbol of the production
	prod_t *prod; //! production
	unsigned placeholder; //! cut the production in two parts : the presumed top of the stack and the end of the production to read
} lr0_item_t;

//! Set of LR(0) items
typedef hashset_t lr0_item_set_t;
uint64_t lr0_item_hash(lr0_item_t* i);
void lr0_item_destroy(lr0_item_t* i);
int lr0_item_cmp(lr0_item_t* i1, lr0_item_t* i2);

typedef struct lr0_flat_item_set_s {
	lr0_item_t * data;
	unsigned length;
} lr0_flat_item_set_t;

/**
 * Flatten the LR(0) hashset into a sorted LR(0) array 
 * @post the arrray is sorted
 * @note memcmp may be used correctly to check flatten LR(0) items sets equality
 */
lr0_flat_item_set_t lr0_item_set_flatten(lr0_item_set_t * s);

uint64_t lr0_flat_item_set_hash(lr0_flat_item_set_t * s);
void lr0_flat_item_set_destroy(lr0_flat_item_set_t * s);
int lr0_flat_item_set_cmp(lr0_flat_item_set_t * s1, lr0_flat_item_set_t * s2);

// Compute the epsilon-closure of valids productions of a LR(0) items set on the abstract items NFA
void lr0_item_closure(const cfg_t g, lr0_item_set_t* s, const bitvector_t *first);

lr0_item_set_t lr0_item_goto(const cfg_t g, lr0_flat_item_set_t s, rule_symbol_t x, const bitvector_t *first);

typedef hashset_t lr0_set_state_map_t;
typedef struct lr0_set_state_s {
	lr0_flat_item_set_t s;
	unsigned state;
} lr0_set_state_t; 
uint64_t lr0_set_state_hash(lr0_set_state_t * fs);
void lr0_set_state_destroy(lr0_set_state_t * fs);
int lr0_set_state_cmp(lr0_set_state_t * fs1, lr0_set_state_t * fs2);

typedef struct lr0_items_dfa_s {
	state_t initial; //! État initial de l'automate
	lr0_set_state_map_t states;
	arraylist_t transitions; //! Matrice de transitions indicé (état, alphabet) // TODO : à voir si on réalise principalement un parcours, il faut un graphe par liste d'adjacence
} lr0_items_dfa_t;

//! Construit l'automate des items LR0 de la grammaire
// TODO :
lr0_items_dfa_t cfg_lr0_automata(const cfg_t g, const bitvector_t * first_g);
void cfg_lr0_automata_destroy(lr0_items_dfa_t * automata);

typedef enum {
	LR_ACTION_NONE,
	LR_ACTION_SHIFT,
	LR_ACTION_REDUCE,
	LR_ACTION_ACCEPT,
} lr_action_type_t;

typedef struct {
	lr_action_type_t t;
	rule_symbol_t left;
	union {
		state_t shift;
		prod_t* reduce;
	};
} lr_action_t;

typedef struct lr_table_s {
	lr_action_t * action_table; //! Table des actions de parsing LR, indexé par état de l'automate et par terminal d'entrée lu
	state_t * goto_table; //! Table de saut de transitions non terminals de l'automate
	unsigned nb_states; //! Nombre d'états de l'automate
	unsigned card_term; //! Nombre de symboles terminaux
	unsigned card_unterm; //! Nombre de symboles non terminaux
} lr_table_t;

// Save a LR(1) table in a file
void table_lr_save(const lr_table_t table, FILE* f); // TODO:

// Load a LR(1) table from a file
lr_table_t lr_table_load(FILE* f); // TODO: 

void lr_table_destroy(lr_table_t * lrt);

lr_table_t slr1_table(const cfg_t g, const lr0_items_dfa_t automata, const bitvector_t * follow_g);

typedef struct lr1_item_s {
	rule_symbol_t left; //! Symbole de gauche de production
	prod_t *prod; // Production
	unsigned placeholder; 
	rule_symbol_t lookahead; // Terminal symbol
} lr1_item_t;


//! Set of LR(1) items
typedef hashset_t lr1_item_set_t;
uint64_t lr1_item_hash(lr1_item_t* s);
void lr1_item_destroy(lr1_item_t* s);
int lr1_item_cmp(lr1_item_t* s1, lr1_item_t* s2);

// //! Associe à chaque lr1 item un ensemble de LR(1) items de clôture
// /**
//	* Permet de calculer les clôtures efficacement par mémoïsation.
//	*/
// typedef struct lr1_item_to_closure_s {
// 	lr1_item_t it;
// 	lr1_item_set_t *closure;
// } lr1_item_to_closure_t;
// static inline uint64_t lr1_item_to_closure_hash(lr1_item_to_closure_t* ic) {
// 	return lr1_item_hash(&ic->it);
// }
// static inline void lr1_item_to_closure_destroy(lr1_item_to_closure_t* ic) {
// 	hashset_destroy(ic->closure);
// 	free(ic);
// }
// static inline int lr1_item_to_closure_cmp(lr1_item_to_closure_t* ic1, lr1_item_to_closure_t* ic2) {
// 	return lr1_item_cmp(ic1->it, ic2->it);
// }
// typedef hashset_t lr1_item_closure_t; // *lr1_item_t -> *lr1_item_set_t

typedef struct lr1_items_dfa_s {
	unsigned states; //! Nombre d'états de l'automate
	state_t initial; //! État initial de l'automate
	lr1_item_t *items; //! LR0 items indexés : l'alphabet est assimilable aux indices
	struct {
		state_t q;
		rule_symbol_t s;
	}* transitions; //! Matrice de transitions indicé (état, alphabet)
} lr1_items_dfa_t;

//! Construit l'automate des items LR1 de la grammaire
/**
 * Chaque état contient un ensemble de LR1 items.
 * On associe à chaque LR1 item un entier unique, donc à chaque état un ensemble d'entiers
 * Pour comparer deux états, on vérifie que l'ensemble d'entiers associés à chaque état est différent d'au moins un élément
 *
 */
lr1_items_dfa_t cfg_lr1_automata(const cfg_t g);

//! Set of LR(1) items sets
typedef hashset_t lr1_cc_t;
uint64_t lr1_cc_hash(lr1_cc_t* s);
void lr1_cc_destroy(lr1_cc_t* s);
int lr1_cc_cmp(lr1_cc_t* s1, lr1_cc_t* s2);

// Calcule la clôture des productions valides d'un ensemble d'items LR(1)
// void lr1_item_closure(const cfg_t g, lr1_item_set_t* s, lr1_item_closure_t *c_lr1, const bitvector_t *firsts);

/** Calcule l'ensemble des items accessibles après lecture d'un symbole depuis une liste d'items
 * @param g grammaire
 * @param first ensemble résultant de cfg_first(g)
 * @param s ensemble d'items LR(1) résultant d'une clôture par lr1_item_closure
 * @param x Symbol terminal lu
 * @return ensemble clôt par lr1_item_closure des items résultant de la lecture de x dans un des items de s
 */
lr1_item_set_t lr1_item_goto(const cfg_t g, lr1_item_set_t* s, rule_symbol_t x, const bitvector_t *first);

#endif

