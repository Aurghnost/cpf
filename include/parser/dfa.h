#ifndef _LEXER_H_INCLUDED
#define _LEXER_H_INCLUDED

#include "parser/lexem.h"
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "parser/types.h"

typedef enum {
	DFA_RECORD = 1 << 0, //! record the read character in a lexem representation string
	DFA_NO_FORWARD = 1 << 1, //! reading the transition must not go forward in the read characters stream
	DFA_REGISTER = 1 << 2, //! the generated symbol is registered as a lexem with it's representation 
} transition_flags_t;

typedef struct transition_t{
  state_t q; //! destination state of the transition
  symbol_t symbol; //! symbol for registration, used only if flags.DFA_REGISTER is set
  transition_flags_t flags;
} transition_t;

typedef struct dfa_t {
  transition_t *trans; //! transitions matrix indexed by (state, character)
  unsigned states; //! automaton size
  state_t init; //! initial automaton state
  unsigned alpha; //! alphabet size
} dfa_t;

//! Initialize an empty automaton
/**
 * Alphabet is {0, ..., nb_alpha-1}
 * @param nb_states Number of states of the automaton
 * @param nb_alpha size of the alphabet
 */
dfa_t dfa_new(unsigned nb_states, unsigned nb_alpha);

/**
 * Adds a transition to the automaton from state p to state t.q reading l
 * @param p original state of the transition
 * @param l label of the transition 
 * @param t transition data (destination state, generated symbol and flags)
 */
void dfa_add(dfa_t a, state_t p, unsigned char l, transition_t t);

void dfa_del(dfa_t a, state_t p, unsigned char l);

void dfa_add_error(dfa_t a, state_t p, unsigned char l);

transition_t dfa_trans(const dfa_t a, state_t p, unsigned char l);

/**
 * Execute a transductor automaton on a text and compute the accepted lexem list
 * if the text is not fully accepted, only lexem list of the partially accepted text is returned
 * @param a transductor automaton
 * @param text input text
 * @return lexem list transduced by the automaton
 */
lexem_list_t dfa_execute(const dfa_t a, const char *text);

/**
 * Free the automaton allocated memory
 * @param[in, out] a pointer to an automaton
 */
void dfa_destroy(dfa_t *a);

//! Read from a file an automaton description with written format of the dfa_write
ssize_t dfa_read(int fd, dfa_t * a);

//! Write an automaton description in a file
ssize_t dfa_write(int fd, const dfa_t * a);

#endif
