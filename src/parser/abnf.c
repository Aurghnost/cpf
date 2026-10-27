#include "parser/abnf.h"
#include "parser/dfa.h"
#include <ctype.h>

cfg_t bnf_grammar() {
	cfg_t abnf = cfg_new(DEFAULT, _SIZEOF_BNF_SYMBOL);
	return abnf;
}

dfa_t bnf_lexer() {
	dfa_t bnf_reader = dfa_new(_SIZEOF_BNF_STATE, 1 << (8*sizeof(char)));
	bnf_reader.init = DEFAULT;

	// ignore comments
	dfa_add(bnf_reader, DEFAULT, ';', (transition_t){.q=COMMENT});
	for (unsigned c = 0; c < 256; c++) {
		dfa_add(bnf_reader, COMMENT, (char)c, (transition_t){.q=COMMENT});
	}
	dfa_add(bnf_reader, COMMENT, '\n', (transition_t){.q=DEFAULT, .symbol=ENDLINE, .flags=DFA_REGISTER});

	dfa_add(bnf_reader, DEFAULT, '\n', (transition_t){.q=DEFAULT, .symbol=ENDLINE, .flags=DFA_REGISTER});

	// rule name
	for (unsigned c = 0; c < 256; c++) {
		if (isalpha(c)) {
			dfa_add(bnf_reader, DEFAULT, (char)c, (transition_t){.q=SYMBOL,.flags=DFA_RECORD});
			dfa_add(bnf_reader, SYMBOL, (char)c, (transition_t){.q=SYMBOL,.flags=DFA_RECORD});
		} else if (isdigit(c) || c == '-') {
			dfa_add(bnf_reader, SYMBOL, (char)c, (transition_t){.q=SYMBOL,.flags=DFA_RECORD});
		} else {
			dfa_add(bnf_reader, SYMBOL, (char)c, (transition_t){.q=DEFAULT,.symbol=RULE_NAME,.flags=DFA_REGISTER|DFA_NO_FORWARD});
		}
	}

	// assignation et alternative
	dfa_add(bnf_reader, DEFAULT, '=', (transition_t){.q=ALTERN});
	dfa_add(bnf_reader, ALTERN, '/', (transition_t){.q=DEFAULT, .symbol=INCR_ALTERN,.flags=DFA_REGISTER});
	for (unsigned c = 0; c < 256; c++) {
		if (c != '/') {
			dfa_add(bnf_reader, ALTERN, (char)c, (transition_t){.q=DEFAULT, .symbol=ASSIGNATION, .flags=DFA_REGISTER|DFA_NO_FORWARD});
		}
	}
	dfa_add(bnf_reader, DEFAULT, '/', (transition_t){.q=DEFAULT,.symbol=ALTERNATIVE,.flags=DFA_REGISTER});

	// blank
	dfa_add(bnf_reader, DEFAULT, ' ', (transition_t){.q=DEFAULT});
	dfa_add(bnf_reader, DEFAULT, '\t', (transition_t){.q=DEFAULT});
	
	// terminal string
	dfa_add(bnf_reader, DEFAULT, '"', (transition_t){.q=CHARACTER0});
	for (unsigned c = 0; c < 256; c++) {
		if (c != '"') {
			dfa_add(bnf_reader, CHARACTER0, (char)c, (transition_t){.q=CHARACTER1,.flags=DFA_RECORD});
			dfa_add(bnf_reader, CHARACTER1, (char)c, (transition_t){.q=CHARACTER1,.flags=DFA_RECORD});
		}
	}
	dfa_add(bnf_reader, CHARACTER1, '"', (transition_t){.q=DEFAULT,.symbol=TERMINAL_LITTERAL_STRING,.flags=DFA_REGISTER}); 
	dfa_add(bnf_reader, DEFAULT, '\'', (transition_t){.q=CHARACTER2});
	for (unsigned c = 0; c < 256; c++) {
		if (c != '\'') {
			dfa_add(bnf_reader, CHARACTER2, (char)c, (transition_t){.q=CHARACTER3,.flags=DFA_RECORD});
			dfa_add(bnf_reader, CHARACTER3, (char)c, (transition_t){.q=CHARACTER3,.flags=DFA_RECORD});
		}
	}
	dfa_add(bnf_reader, CHARACTER3, '\'', (transition_t){.q=DEFAULT,.symbol=TERMINAL_LITTERAL_STRING,.flags=DFA_REGISTER});

	// grouping and optionnal
	dfa_add(bnf_reader, DEFAULT, '(', (transition_t){.q=DEFAULT, .symbol=GROUP_OPEN,.flags=DFA_REGISTER});
	dfa_add(bnf_reader, DEFAULT, ')', (transition_t){.q=DEFAULT, .symbol=GROUP_CLOSE,.flags=DFA_REGISTER});
	dfa_add(bnf_reader, DEFAULT, '[', (transition_t){.q=DEFAULT, .symbol=OPTION_OPEN,.flags=DFA_REGISTER});
	dfa_add(bnf_reader, DEFAULT, ']', (transition_t){.q=DEFAULT, .symbol=OPTION_CLOSE,.flags=DFA_REGISTER});

	// valeurs décimales et étoile 
	for (unsigned c = 0; c < 256; c++) {
		if (c >= '0' && c <= '9') {
			dfa_add(bnf_reader, DEFAULT, (char)c, (transition_t){.q=DVALUE, .flags=DFA_RECORD});
			dfa_add(bnf_reader, DVALUE, (char)c, (transition_t){.q=DVALUE, .flags=DFA_RECORD});
		} else {
			dfa_add(bnf_reader, DVALUE, (char)c, (transition_t){.q=DEFAULT, .symbol=COUNT,.flags=DFA_REGISTER|DFA_NO_FORWARD});
		}
	}
	dfa_add(bnf_reader, DEFAULT, '*', (transition_t){.q=DEFAULT, .symbol=STAR,.flags=DFA_REGISTER});

	// value range alternative
	dfa_add(bnf_reader, DEFAULT, '%', (transition_t){.q=VRANGE});
	
	dfa_add(bnf_reader, VRANGE, 'b', (transition_t){.q=BRANGE}); 
	for (unsigned c = 0; c < 256; c++) {
		if (c != '0' && c != '1') {
			dfa_add(bnf_reader, BRANGE, (char)c, (transition_t){.q=DEFAULT, .symbol=TERMINAL_BINARY,.flags=DFA_REGISTER|DFA_NO_FORWARD});
		} else {
			dfa_add(bnf_reader, BRANGE, (char)c, (transition_t){.q=BRANGE, .flags=DFA_RECORD});
		}
	}

	dfa_add(bnf_reader, VRANGE, 'd', (transition_t){.q=DRANGE});
	for (unsigned c = 0; c < 256; c++) {
		if (c < '0' || c > '9') {
			dfa_add(bnf_reader, DRANGE, (char)c, (transition_t){.q=DEFAULT, .symbol=TERMINAL_DECIMAL,.flags=DFA_REGISTER|DFA_NO_FORWARD});
		} else {
			dfa_add(bnf_reader, DRANGE, (char)c, (transition_t){.q=DRANGE, .flags=DFA_RECORD});
		}
	}

	dfa_add(bnf_reader, VRANGE, 'x', (transition_t){.q=XRANGE});
	for (unsigned c = 0; c < 256; c++) {
		if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
			dfa_add(bnf_reader, XRANGE, (char)c, (transition_t){.q=XRANGE, .flags=DFA_RECORD});
		} else {
			dfa_add(bnf_reader, XRANGE, (char)c, (transition_t){.q=DEFAULT, .symbol=TERMINAL_HEXA,.flags=DFA_REGISTER|DFA_NO_FORWARD}); 
		} 
	}
	
	dfa_add(bnf_reader, BRANGE, '.', (transition_t){.q=BRANGE_DOT, .symbol=TERMINAL_BINARY,.flags=DFA_REGISTER});
	dfa_add(bnf_reader, DRANGE, '.', (transition_t){.q=DRANGE_DOT, .symbol=TERMINAL_DECIMAL,.flags=DFA_REGISTER});
	dfa_add(bnf_reader, XRANGE, '.', (transition_t){.q=XRANGE_DOT, .symbol=TERMINAL_HEXA,.flags=DFA_REGISTER});
	dfa_add(bnf_reader, BRANGE, '-', (transition_t){.q=BRANGE_DASH, .symbol=TERMINAL_BINARY,.flags=DFA_REGISTER});
	dfa_add(bnf_reader, DRANGE, '-', (transition_t){.q=DRANGE_DASH, .symbol=TERMINAL_DECIMAL,.flags=DFA_REGISTER});
	dfa_add(bnf_reader, XRANGE, '-', (transition_t){.q=XRANGE_DASH, .symbol=TERMINAL_HEXA,.flags=DFA_REGISTER});
 
	dfa_add(bnf_reader, BRANGE_DOT, '0', (transition_t){.q=BRANGE, .symbol=PERIOD,.flags=DFA_REGISTER|DFA_NO_FORWARD});
	dfa_add(bnf_reader, BRANGE_DOT, '1', (transition_t){.q=BRANGE, .symbol=PERIOD,.flags=DFA_REGISTER|DFA_NO_FORWARD});
	for (char c = '0'; c <= '9'; c++) {
		dfa_add(bnf_reader, DRANGE_DOT, c, (transition_t){.q=DRANGE, .symbol=PERIOD,.flags=DFA_REGISTER|DFA_NO_FORWARD});
		dfa_add(bnf_reader, XRANGE_DOT, c, (transition_t){.q=XRANGE, .symbol=PERIOD,.flags=DFA_REGISTER|DFA_NO_FORWARD});
	}
	for (char c = 'A'; c <= 'F'; c++) {
		dfa_add(bnf_reader, XRANGE_DOT, c, (transition_t){.q=XRANGE, .symbol=PERIOD,.flags=DFA_REGISTER|DFA_NO_FORWARD});
		dfa_add(bnf_reader, XRANGE_DOT, (char)(c+32), (transition_t){.q=XRANGE, .symbol=PERIOD,.flags=DFA_REGISTER|DFA_NO_FORWARD});
	}

	dfa_add(bnf_reader, BRANGE_DASH, '0', (transition_t){.q=BRANGE, .symbol=DASH,.flags=DFA_REGISTER|DFA_NO_FORWARD});
	dfa_add(bnf_reader, BRANGE_DASH, '1', (transition_t){.q=BRANGE, .symbol=DASH,.flags=DFA_REGISTER|DFA_NO_FORWARD});
	for (char c = '0'; c <= '9'; c++) {
		dfa_add(bnf_reader, DRANGE_DASH, c, (transition_t){.q=DRANGE, .symbol=DASH,.flags=DFA_REGISTER|DFA_NO_FORWARD});
		dfa_add(bnf_reader, XRANGE_DASH, c, (transition_t){.q=XRANGE, .symbol=DASH,.flags=DFA_REGISTER|DFA_NO_FORWARD});
	}
	for (char c = 'A'; c <= 'F'; c++) {
		dfa_add(bnf_reader, XRANGE_DASH, c, (transition_t){.q=XRANGE, .symbol=DASH,.flags=DFA_REGISTER|DFA_NO_FORWARD});
		dfa_add(bnf_reader, XRANGE_DASH, (char)(c+32), (transition_t){.q=XRANGE, .symbol=DASH,.flags=DFA_REGISTER|DFA_NO_FORWARD});
	}

	return bnf_reader;
}
