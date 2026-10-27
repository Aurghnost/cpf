#include "help_page_renderer/grammar.h"

void help_grammar_symbols_printer(FILE *f, const lexem_t lex) {
	static const char* a[_SIZEOF_SYMBOLS+1] = {
		"<NONE",
		"<TITLE",
		"<SET",
		"<SECTION",
		"<PLAIN",
		"<ARRAY",
		"<LP",
		"<RP",
		"<LB",
		"<RB",
		"<STRING",
		"<COMMA",
		"<COLUMN",
		"<MOD",
		"<FALSE",
		"<TRUE",
	};

	fprintf(f, "%s", a[lex.symbol]);
	if (lex.repr.buf != NULL) {
		fprintf(f, "(%s)", lex.repr.buf);
	}
	fprintf(f, ">");
}
typedef enum {
	DEFAULT,
	COMMAND,
	S_STRING,
	S_ESCAPE,
	_SIZEOF_STATES,
} help_grammar_dfa_states_t;

dfa_t help_grammar_lexer() {
	dfa_t lexer= dfa_new(_SIZEOF_STATES, 1 << (8*sizeof(char)));
	lexer.init = DEFAULT;

	dfa_add(lexer, DEFAULT, '#', (transition_t){.q=COMMAND});
	dfa_add(lexer, DEFAULT, 't', (transition_t){.q=DEFAULT, .symbol=TRUE, .flags=DFA_REGISTER});
	dfa_add(lexer, DEFAULT, 'f', (transition_t){.q=DEFAULT, .symbol=FALSE, .flags=DFA_REGISTER});

	dfa_add(lexer, COMMAND, 'S', (transition_t){.q=DEFAULT, .symbol=SECTION, .flags=DFA_REGISTER});
	dfa_add(lexer, COMMAND, 'T', (transition_t){.q=DEFAULT, .symbol=TITLE, .flags=DFA_REGISTER});
	dfa_add(lexer, COMMAND, 'A', (transition_t){.q=DEFAULT, .symbol=ARRAY, .flags=DFA_REGISTER});
	dfa_add(lexer, COMMAND, 'P', (transition_t){.q=DEFAULT, .symbol=PLAIN, .flags=DFA_REGISTER});
	dfa_add(lexer, COMMAND, 's', (transition_t){.q=DEFAULT, .symbol=SET, .flags=DFA_REGISTER});

	dfa_add(lexer, DEFAULT, '(', (transition_t){.q=DEFAULT, .symbol=LP, .flags=DFA_REGISTER});
	dfa_add(lexer, DEFAULT, ')', (transition_t){.q=DEFAULT, .symbol=RP, .flags=DFA_REGISTER});
	dfa_add(lexer, DEFAULT, '{', (transition_t){.q=DEFAULT, .symbol=LB, .flags=DFA_REGISTER});
	dfa_add(lexer, DEFAULT, '}', (transition_t){.q=DEFAULT, .symbol=RB, .flags=DFA_REGISTER});
	dfa_add(lexer, DEFAULT, ',', (transition_t){.q=DEFAULT, .symbol=COMMA,.flags=DFA_REGISTER});
	dfa_add(lexer, DEFAULT, ':', (transition_t){.q=DEFAULT, .symbol=COLUMN, .flags=DFA_REGISTER});
	dfa_add(lexer, DEFAULT, '"', (transition_t){.q=S_STRING});
	for (unsigned short int c = 0; c < 256; c++) {
		dfa_add(lexer, S_STRING, (unsigned char)c, (transition_t){.q=S_STRING, .flags=DFA_RECORD});
	}
	dfa_add(lexer, S_STRING, '"', (transition_t){.q=DEFAULT, .symbol=STRING, .flags=DFA_REGISTER});
	dfa_add(lexer, S_STRING, '\\', (transition_t){.q=S_ESCAPE});
	dfa_add(lexer, S_ESCAPE, '\\', (transition_t){.q=S_STRING, .flags=DFA_RECORD});
	dfa_add(lexer, S_ESCAPE, '"', (transition_t){.q=S_STRING, .flags=DFA_RECORD});

	return lexer;
}

cfg_t help_grammar_grammar() {
	cfg_t g = cfg_new(AXIOM, _SIZEOF_SYMBOLS);

	cfg_add_terminal(&g, SECTION); 
	cfg_add_terminal(&g, TITLE); 
	cfg_add_terminal(&g, ARRAY); 
	cfg_add_terminal(&g, PLAIN); 
	cfg_add_terminal(&g, SET); 
	cfg_add_terminal(&g, LP); 
	cfg_add_terminal(&g, RP); 
	cfg_add_terminal(&g, LB); 
	cfg_add_terminal(&g, RB); 
	cfg_add_terminal(&g, COMMA); 
	cfg_add_terminal(&g, COLUMN); 
	cfg_add_terminal(&g, STRING); 
	cfg_add_terminal(&g, MOD); 
	cfg_add_terminal(&g, FALSE); 
	cfg_add_terminal(&g, TRUE); 
	
	cfg_add_token(&g, STRING);

	cfg_add_concrete(&g, LP);
	cfg_add_concrete(&g, RP);
	cfg_add_concrete(&g, LB);
	cfg_add_concrete(&g, RB);
	cfg_add_concrete(&g, COMMA);
	cfg_add_concrete(&g, COLUMN);

	cfg_add_prod(g, BOOL, prod_new(1, FALSE));
	cfg_add_prod(g, BOOL, prod_new(1, TRUE));

	cfg_add_prod(g, DATA, prod_new(1, LIST));
	cfg_add_prod(g, DATA, prod_new(1, DICT));
	cfg_add_prod(g, DATA, prod_new(1, STRING));
	cfg_add_prod(g, DATA, prod_new(1, MOD));
	// cfg_add_prod(g, DATA, prod_new(2, MOD, STRING));
	cfg_add_prod(g, DATA, prod_new(1, BOOL));
	cfg_add_prod(g, DATA, prod_new(0));

	cfg_add_prod(g, LIST, prod_new(3, LP, DATA, LIST_P));
	cfg_add_prod(g, LIST_P, prod_new(3, COMMA, DATA, LIST_P));
	cfg_add_prod(g, LIST_P, prod_new(1, RP));

	cfg_add_prod(g, DICT, prod_new(3, LB, DICT_L, RB));
	cfg_add_prod(g, DICT_L, prod_new(0));
	cfg_add_prod(g, DICT_L, prod_new(4, STRING, COLUMN, DATA, DICT_LP));
	cfg_add_prod(g, DICT_LP, prod_new(5, COMMA, STRING, COLUMN, DATA, DICT_LP));
	cfg_add_prod(g, DICT_LP, prod_new(0));

	cfg_add_prod(g, AXIOM, prod_new(1, CMD_BLOCKS));
	cfg_add_prod(g, CMD_BLOCKS, prod_new(2, CMD, CMD_BLOCKS));
	cfg_add_prod(g, CMD_BLOCKS, prod_new(0));

	cfg_add_prod(g, CMD, prod_new(2, TITLE, STRING));
	cfg_add_prod(g, CMD, prod_new(2, SECTION, STRING));
	cfg_add_prod(g, CMD, prod_new(2, PLAIN, STRING));
	cfg_add_prod(g, CMD, prod_new(2, ARRAY, DICT));
	cfg_add_prod(g, CMD, prod_new(2, SET, LIST));

	return g;
}
