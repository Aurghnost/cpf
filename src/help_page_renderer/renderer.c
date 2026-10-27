#include "help_page_renderer/renderer.h"

#include <err.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

void ast_render(int fdout, ast_t tree) {
	if (tree == NULL) {
		return;
	}
	switch (tree->label.symbol) {
		case LP:
		case RP:
		case LB:
		case RB:
		case COMMA:
		case COLUMN:
		default:
			fprintf(stderr, "GROS BUG !\n");
			return;
	}
}

void renderer_render(int fdout, const char * filename) {
	dfa_t lexer = help_grammar_lexer();
	cfg_t g = help_grammar_grammar();
	bitvector_t *first_g = cfg_first(g);
	bitvector_t *follow_g = cfg_follow(g, first_g);
	lr0_items_dfa_t automata = cfg_lr0_automata(g, first_g);

	lr_table_t table = slr1_table(g, automata, follow_g);

	int fdin;
	struct stat statut;
	if (-1 == (fdin = open(filename, O_RDONLY))) {
		err(EXIT_FAILURE, "%s:%u", __FILE__, __LINE__);
	}
	int fs = fstat(fdin, &statut);
	if (fs == -1) {
		err(EXIT_FAILURE, "%s:%u", __FILE__, __LINE__);
	}
	size_t filesize = statut.st_size;

	const char * text = (char *)mmap(NULL, filesize, PROT_READ, MAP_SHARED, fdin, 0);

	lexem_list_t ll = dfa_execute(lexer, text);
	lexem_list_reverse(&ll);
	lexem_list_push(&ll, (lexem_t){.symbol=cfg_eof(g)});

	ast_t abstract = ast_from_lr_table(g, table, ll, help_grammar_symbols_printer);

	lexem_list_destroy(&ll);

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
	
	if (abstract == NULL) {
		fprintf(stderr, "Erreur de parsing.\n");
	} else {
		ast_destroy(abstract);
	}
	cfg_destroy(&g);
}

