#ifndef _HELP_GRAMMAR_H_INCLUDED
#define _HELP_GRAMMAR_H_INCLUDED

#include "parser/dfa.h"
#include "parser/cfg.h"

typedef enum {
	NONE,
	// Terminal symbols :
	TITLE,
	SET,
	SECTION,
	PLAIN,
	ARRAY,
	LP, //! Left parenthesis
	RP, //! Right parenthesis
	LB, //! Left Brace
	RB,//! Right Brace
	STRING,
	COMMA,
	COLUMN,
	MOD,
	FALSE,
	TRUE,
	// Non Terminal symbols :
	AXIOM,
	CMD,
	CMD_BLOCKS,
	DATA,
	LIST,
	LIST_P,
	DICT,
	DICT_L,
	DICT_LP,
	BOOL,
	_SIZEOF_SYMBOLS,
} help_grammar_symbols_t;

dfa_t help_grammar_lexer();
cfg_t help_grammar_grammar();

void help_grammar_symbols_printer(FILE *f, const lexem_t lex);

#endif // !_HELP_GRAMMAR_H_INCLUDED
