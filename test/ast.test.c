#include "unittest.h"
#include <string.h>
#include "parser/cfg.h"
#include "parser/dfa.h"
#include "parser/ast.h"

typedef enum {
	NONE,
	PLUS, // 0
	MULT, // 1
	LP, // 2
	RP, // 3
	INT, // 4
	E, // 5
	EP, // 6
	T, // 7
	TP, // 8
	F, // 9
	SUB, // 10
	_SIZEOF_SYMBOLS
} test_symbols_t;

void symbols_printer(FILE *f, const lexem_t lex) {
	static const char* a[_SIZEOF_SYMBOLS+1] = {
		"<NONE",
		"<PLUS",
		"<MULT",
		"<LP",
		"<RP",
		"<INT",
		"<EXPR",
		"<EXPR_p",
		"<TERM",
		"<TERM_p",
		"<FACTOR",
		"<SUB",
		"<EOF"
	};

	fprintf(f, "%s", a[lex.symbol]);
	if (lex.repr.buf != NULL) {
		fprintf(f, "(%s)", lex.repr.buf);
	}
	fprintf(f, ">");
}

void print(ast_t tree) {
	if (tree == NULL) {
		return;
	}
	symbols_printer(stderr, tree->label);
	fprintf(stderr, "(");
	for (unsigned i = 0; i < tree->length; i++)
		print(tree->children[i]);
	fprintf(stderr, ")\n");
}

int evaluate_ll1(ast_t tree) { 
	if (tree == NULL) {
		return 0;
	}
	int s;
	switch (tree->label.symbol) {
		case INT:
		return atoi(tree->label.repr.buf);
		case F:
		return evaluate_ll1(tree->children[0]);
		case E:
		case EP:
			s = 0;
			for (unsigned i = 0; i < tree->length; i++)
				s += evaluate_ll1(tree->children[i]);
			break;
		case T:
		case TP:
			s = 1;
			for (unsigned i = 0; i < tree->length; i++)
				s *= evaluate_ll1(tree->children[i]);
	}
	return s;
}

int evaluate_ultra_simple_arithmetic(ast_t tree) {
	if (tree == NULL) {
		return 0;
	}
	int s;
	switch (tree->label.symbol) {
		case INT:
		return atoi(tree->label.repr.buf);
		case E:
		case EP:
			s = 0;
			for (unsigned i = 0; i < tree->length; i++)
				s += evaluate_ultra_simple_arithmetic(tree->children[i]);
			break;
	}
	return s;
}

typedef enum {
	DEFAULT,
	NEG_INT,
	POS_INT,
	_SIZEOF_TEST_STATES
} test_dfa_states_t;

unittest_init();
unittest(
unittest_test("Table LL(1)", {

		dfa_t lexer = dfa_new(_SIZEOF_TEST_STATES, 1 << (8*sizeof(char)));
		lexer.init = DEFAULT;

/*
 * Regex :
 * PLUS : +
 * MULT : *
 * LP : '('
 * RP : ')'
 */

		dfa_add(lexer, DEFAULT, '+', (transition_t){.symbol=PLUS, .flags=DFA_REGISTER});
		dfa_add(lexer, DEFAULT, '*', (transition_t){.symbol=MULT, .flags=DFA_REGISTER});
		dfa_add(lexer, DEFAULT, '(', (transition_t){.symbol=LP, .flags=DFA_REGISTER});
		dfa_add(lexer, DEFAULT, ')', (transition_t){.symbol=RP, .flags=DFA_REGISTER});
		dfa_add(lexer, DEFAULT, '-', (transition_t){.q=NEG_INT, .flags=DFA_RECORD});
		dfa_add(lexer, DEFAULT, '0', (transition_t){ .q=DEFAULT, .symbol=POS_INT, .flags=DFA_RECORD|DFA_REGISTER});
		for (int c = 0; c < 256; c++) {
			if ('1' <= c && c <= '9') {
				dfa_add(lexer, NEG_INT, (char)c, (transition_t){.q=POS_INT, .flags=DFA_RECORD});
				dfa_add(lexer, DEFAULT, (char)c, (transition_t){.q=POS_INT, .flags=DFA_RECORD});
			} else {
				dfa_add_error(lexer, NEG_INT, (char)c);
			}
		}
		for (int c = 0; c < 256; c++) {
			if ('0' <= c && c <= '9')
				dfa_add(lexer, POS_INT, (char)c, (transition_t){.q=POS_INT, .flags=DFA_RECORD});
			else {
				dfa_add(lexer, POS_INT, (char)c, (transition_t){.symbol=INT, .flags=DFA_REGISTER|DFA_NO_FORWARD});
			}
		}


/* Grammaire définie par :
 * E -> T.Ep
 * Ep -> "+".T.Ep | 1
 * T -> F.Tp
 * Tp -> "*".F.Tp | 1
 * F -> "(".E.")" | INT
 */
		cfg_t g = cfg_new(E, _SIZEOF_SYMBOLS);
		
		cfg_add_terminal(&g, PLUS); 
		cfg_add_terminal(&g, MULT);
		cfg_add_terminal(&g, LP);
		cfg_add_terminal(&g, RP);
		cfg_add_terminal(&g, INT);

		cfg_add_prod(g, E, prod_new(2, T, EP));
		cfg_add_prod(g, EP, prod_new(3, PLUS, T, EP));
		cfg_add_prod(g, EP, prod_new(0));
		cfg_add_prod(g, T, prod_new(2, F, TP));
		cfg_add_prod(g, TP, prod_new(3, MULT, F, TP));
		cfg_add_prod(g, TP, prod_new(0));
		cfg_add_prod(g, F, prod_new(3, LP, E, RP));
		cfg_add_prod(g, F, prod_new(1, INT));

		cfg_add_token(&g, INT);
		cfg_add_concrete(&g, LP);
		cfg_add_concrete(&g, RP);
		cfg_add_concrete(&g, PLUS);
		cfg_add_concrete(&g, MULT);

		bitvector_t *first_g = cfg_first(g);
		bitvector_t *follow_g = cfg_follow(g, first_g);

		table_ll1_t table = cfg_table_ll1(g, first_g, follow_g);
		for (unsigned i = 0; i < g.unterminals.card; i++){
			bitvector_destroy(&first_g[i]); 
		}
		free(first_g);
		for (unsigned i = 0; i < g.unterminals.card; i++) {
			bitvector_destroy(&follow_g[i]);
		}
		free(follow_g);
	
		const char* text = " (95 * (4 + 2)) + 3 * 18"; 

		lexem_list_t ll = dfa_execute(lexer, text);
		lexem_list_reverse(&ll);

		unittest_assert(lexem_list_is_equal(ll, 13, LP, INT, MULT, LP, INT, PLUS, INT, RP, RP, PLUS, INT, MULT, INT), "");

		ast_t abstract = ast_from_ll1(g, table, ll, symbols_printer);

		if (abstract == NULL) {
			unittest_fail("LL(1) should have succeed...");
		} else {
			unittest_assert(evaluate_ll1(abstract) == 624, "Abstract Tree Semantic Evaluation is incorrect.\n");
			ast_destroy(abstract);
		}

		lexem_list_destroy(&ll);
		cfg_destroy(&g);
		free(table);
		dfa_destroy(&lexer);
	}, UNITTEST_DONE);

	unittest_test("Table LL(1) from file", {
		cfg_t g;
		table_ll1_t table;
		int fd = open("table.test.ll1", O_RDONLY);
		table_ll1_read(fd, &g, &table);
		close(fd);
		cfg_destroy(&g);
		free(table);
	}, UNITTEST_DONE);

	unittest_test("Table SLR(1) Dyck words", {
		cfg_t g = cfg_new(EP, _SIZEOF_SYMBOLS);
		cfg_add_terminal(&g, LP);
		cfg_add_terminal(&g, RP);

		cfg_add_prod(g, EP, prod_new(1, E));
		cfg_add_prod(g, E, prod_new(4, LP, E, RP, E));
		cfg_add_prod(g, E, prod_new(0));
		
		bitvector_t *first_g = cfg_first(g);
		bitvector_t *follow_g = cfg_follow(g, first_g);
		lr0_items_dfa_t automata = cfg_lr0_automata(g, first_g);

		lr_table_t table = slr1_table(g, automata, follow_g);

		if (table.action_table == NULL || table.goto_table == NULL) {
			fprintf(stderr, "Not SLR(1) !\n");
			exit(EXIT_FAILURE);
		}

		lexem_list_t ll = lexem_list_new();
		lexem_list_push(&ll, (lexem_t){.symbol=cfg_eof(g)});
		lexem_list_push(&ll, (lexem_t){.symbol=RP});
		lexem_list_push(&ll, (lexem_t){.symbol=LP});
		lexem_list_push(&ll, (lexem_t){.symbol=RP});
		lexem_list_push(&ll, (lexem_t){.symbol=LP});
		unittest_assert(lexem_list_is_equal(ll, 5, LP, RP, LP, RP, cfg_eof(g)), "");

		ast_t abstract = ast_from_lr_table(g, table, ll, symbols_printer);

		if (abstract == NULL) {
			unittest_fail("LR(1) should have succeed...");
		} else {
			ast_destroy(abstract);
		}
		for (unsigned i = 0; i < g.unterminals.card; i++){
			bitvector_destroy(&first_g[i]); 
		}
		free(first_g);
		for (unsigned i = 0; i < g.unterminals.card; i++) {
			bitvector_destroy(&follow_g[i]);
		}
		free(follow_g);
		lr_table_destroy(&table);
		cfg_lr0_automata_destroy(&automata);

		lexem_list_destroy(&ll);
		cfg_destroy(&g);
	}, UNITTEST_DONE);
	unittest_test("Table SLR(1) Ultra Simple Arithmetic", {
		cfg_t g = cfg_new(EP, _SIZEOF_SYMBOLS);
		cfg_add_terminal(&g, PLUS);
		cfg_add_terminal(&g, INT);
		cfg_add_concrete(&g, PLUS);

		cfg_add_prod(g, EP, prod_new(1, E));
		cfg_add_prod(g, E, prod_new(3, E, PLUS, INT));
		cfg_add_prod(g, E, prod_new(1, INT));
		
		bitvector_t *first_g = cfg_first(g);
		bitvector_t *follow_g = cfg_follow(g, first_g);
		lr0_items_dfa_t automata = cfg_lr0_automata(g, first_g);

		lr_table_t table = slr1_table(g, automata, follow_g);

		if (table.action_table == NULL || table.goto_table == NULL) {
			unittest_fail("Not SLR(1)");
			exit(EXIT_FAILURE);
		}

		lexem_list_t ll = lexem_list_new();
		lexem_list_push(&ll, (lexem_t){.symbol=cfg_eof(g)});
		lexem_list_push(&ll, (lexem_t){.symbol=INT, .repr=string_dup_cstring("42")});
		lexem_list_push(&ll, (lexem_t){.symbol=PLUS});
		lexem_list_push(&ll, (lexem_t){.symbol=INT, .repr=string_dup_cstring("64")});
		lexem_list_push(&ll, (lexem_t){.symbol=PLUS});
		lexem_list_push(&ll, (lexem_t){.symbol=INT, .repr=string_dup_cstring("18")});
		unittest_assert(lexem_list_is_equal(ll, 6, INT, PLUS, INT, PLUS, INT, cfg_eof(g)), "");

		ast_t abstract = ast_from_lr_table(g, table, ll, symbols_printer);

		if (abstract == NULL) {
			unittest_fail("LR(1) should have succeed...");
		} else {
			unittest_assert(evaluate_ultra_simple_arithmetic(abstract)==124, "");
			ast_destroy(abstract);
		}
		for (unsigned i = 0; i < g.unterminals.card; i++){
			bitvector_destroy(&first_g[i]); 
		}
		free(first_g);
		for (unsigned i = 0; i < g.unterminals.card; i++) {
			bitvector_destroy(&follow_g[i]);
		}
		free(follow_g);
		lr_table_destroy(&table);
		cfg_lr0_automata_destroy(&automata);

		lexem_list_destroy(&ll);
		cfg_destroy(&g);
	}, UNITTEST_DONE);
)
