
#include "unittest.h"
#include <string.h>
#include <stdio.h>
#include "parser/cfg.h"
#include "parser/ast.h"

#include "help_page_renderer/grammar.h"

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

void symbols_printer(FILE *f, const lexem_t lex) {
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
		"<AXIOM",
		"<CMD",
		"<CMD_BLOCKS",
		"<DATA",
		"<LIST",
		"<LIST_P",
		"<DICT",
		"<DICT_L",
		"<DICT_LP",
		"<BOOL",
		"<EOF"
	};

	fprintf(f, "%s", a[lex.symbol]);
	if (lex.repr.buf != NULL) {
		fprintf(f, "(%s)", lex.repr.buf);
	}
	fprintf(f, ">");
}

unittest_init();
unittest(
	unittest_test("Test lexer", {
		dfa_t lexer = help_grammar_lexer();
		cfg_t g = help_grammar_grammar();

		const char* text = 
			"#T \"CPF's build help page\"\n"
			"#s\n"
			"( \"section\", {\"mod\":t}\n"
			")\n\n" 
			"#S \"Usage\"\n"
			"#P \"des trucs\"\n\n"
			"#S \"Options\""

"#A {\n"
"	\"cols\":(, \"Description\"),\n"
"	\"data\":(\n"
"		((\"-a\", \"all\"), \"Build all\"),"
"		((\"-r\", \"release\"), \"Build executable target in release mode\"),"
"	)\n"
"}\n";
		lexem_list_t ll = dfa_execute(lexer, text);
		lexem_list_reverse(&ll);

		unittest_assert(lexem_list_is_equal(ll, 52, 
			TITLE, STRING, 
			SET, LP, 
				STRING, COMMA, 
				LB, STRING, COLUMN, TRUE, RB, 
			RP, 
			SECTION, STRING,
			PLAIN, STRING,
			SECTION, STRING,
			ARRAY, LB, 
				STRING, COLUMN, LP, COMMA, STRING, RP, COMMA,
				STRING, COLUMN, LP,
					LP,LP,STRING, COMMA, STRING, RP, COMMA, STRING, RP, COMMA,
					LP, LP, STRING, COMMA, STRING, RP, COMMA, STRING, RP, COMMA, 
				RP, 
			RB), "");

		lexem_list_destroy(&ll);

		ll = dfa_execute(lexer, "");
		unittest_assert(lexem_list_is_equal(ll, 0), "");
		lexem_list_destroy(&ll);

		dfa_destroy(&lexer);
		cfg_destroy(&g);
	}, UNITTEST_DONE);
	unittest_test("Test grammar SLR(1)", {
		cfg_t g = help_grammar_grammar();
		bitvector_t *first_g = cfg_first(g);
		bitvector_t *follow_g = cfg_follow(g, first_g);
		lr0_items_dfa_t automata = cfg_lr0_automata(g, first_g);

		lr_table_t table = slr1_table(g, automata, follow_g);

		unittest_assert((table.action_table != NULL && table.goto_table != NULL), "grammar is not SLR(1)...")

		lr_table_destroy(&table);

		cfg_lr0_automata_destroy(&automata);

		for (unsigned i = 0; i < g.unterminals.card; i++){
			bitvector_destroy(&first_g[i]); 
		}
		free(first_g);
		for (unsigned i = 0; i < g.unterminals.card; i++) {
			bitvector_destroy(&follow_g[i]);
		}
		free(follow_g);
		cfg_destroy(&g);
	}, UNITTEST_DONE);
	unittest_test("LR PARSING", {
		
		dfa_t lexer = help_grammar_lexer();
		cfg_t g = help_grammar_grammar();
		bitvector_t *first_g = cfg_first(g);
		bitvector_t *follow_g = cfg_follow(g, first_g);
		lr0_items_dfa_t automata = cfg_lr0_automata(g, first_g);

		lr_table_t table = slr1_table(g, automata, follow_g);

		const char* text = 
			"#T \"CPF's build help page\"\n"
			"#s\n"
			"( \"section\", {\"mod\":t}\n"
			")\n\n" 
			"#S \"Usage\"\n"
			"#P \"des trucs\"\n\n"
			"#S \"Options\""

"#A {\n"
"	\"cols\":(, \"Description\"),\n"
"	\"data\":(\n"
"		((\"-a\", \"all\"), \"Build all\"),"
"		((\"-r\", \"release\"), \"Build executable target in release mode\"),"
"	)\n"
"}\n";
		lexem_list_t ll = dfa_execute(lexer, text);
		lexem_list_reverse(&ll);
		lexem_list_push(&ll, (lexem_t){.symbol=cfg_eof(g)});
		
		ast_t abstract = ast_from_lr_table(g, table, ll, symbols_printer);

		lexem_list_destroy(&ll);

		if (abstract == NULL) {
			unittest_fail("LR parsing should have succeed...");
		} else {
			ast_destroy(abstract);
		}

		lr_table_destroy(&table);

		cfg_lr0_automata_destroy(&automata);

		for (unsigned i = 0; i < g.unterminals.card; i++){
			bitvector_destroy(&first_g[i]); 
		}
		free(first_g);
		for (unsigned i = 0; i < g.unterminals.card; i++) {
			bitvector_destroy(&follow_g[i]);
		}
		free(follow_g);
		cfg_destroy(&g);
		dfa_destroy(&lexer);
	}, UNITTEST_DONE);
)
