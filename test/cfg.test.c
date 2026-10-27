#include "unittest.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include "parser/cfg.h"

typedef enum {
	PLUS, // 0
	MULT, // 1
	E, // 5
	EP, // 6
	T, // 7
	TP, // 8
	F, // 9
	LP, // 2
	RP, // 3
	INT, // 4
	_SIZEOF_SYMBOLS // 10
} test_symbols_t;

void lr1_item_pretty_print(lr1_item_t *item);
void lr0_item_pretty_print(lr0_item_t *item);

unittest_init();
unittest(
	unittest_test("Grammar creation", {
		cfg_t g = cfg_new(0, _SIZEOF_SYMBOLS);
		
		cfg_add_terminal(&g, PLUS); 
		cfg_add_terminal(&g, MULT);
		cfg_add_terminal(&g, LP);
		cfg_add_terminal(&g, RP);
		cfg_add_terminal(&g, INT);

		unittest_assert((g.sym_props[PLUS] & IS_TERMINAL), "should be a terminal");
		unittest_assert((g.sym_props[MULT] & IS_TERMINAL), "should be a terminal");
		unittest_assert((g.sym_props[LP] & IS_TERMINAL), "should be a terminal");
		unittest_assert((g.sym_props[RP] & IS_TERMINAL), "should be a terminal");
		unittest_assert((g.sym_props[INT] & IS_TERMINAL), "should be a terminal");

		unittest_assert(!(g.sym_props[E] & IS_TERMINAL), "shouldn't be a terminal");
		unittest_assert(!(g.sym_props[EP] & IS_TERMINAL), "shouldn't be a terminal");
		unittest_assert(!(g.sym_props[T] & IS_TERMINAL), "shouldn't be a terminal");
		unittest_assert(!(g.sym_props[TP] & IS_TERMINAL), "shouldn't be a terminal");
		unittest_assert(!(g.sym_props[F] & IS_TERMINAL), "shouldn't be a terminal");

		cfg_add_prod(g, E, prod_new(2, T, EP));
		cfg_add_prod(g, EP, prod_new(3, PLUS, T, EP));
		cfg_add_prod(g, EP, prod_new(0));
		cfg_add_prod(g, T, prod_new(2, F, TP));
		cfg_add_prod(g, TP, prod_new(3, MULT, F, TP));
		cfg_add_prod(g, TP, prod_new(0));
		cfg_add_prod(g, F, prod_new(3, LP, E, RP));
		cfg_add_prod(g, F, prod_new(1, INT));

		cfg_destroy(&g);
	}, UNITTEST_DONE);
	unittest_test("Grammar First and Follow", {
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

		/*
		 * E -> T EP
		 * EP -> + T EP | eps
		 * T -> F TP
		 * TP -> * F TP | eps
		 * F -> ( E ) | int
		 */
		
		unsigned plus_term = sparseset_sparse_to_dense(&g.terminals, PLUS);
		unsigned mult_term = sparseset_sparse_to_dense(&g.terminals, MULT);
		unsigned lp_term = sparseset_sparse_to_dense(&g.terminals, LP);
		unsigned int_term = sparseset_sparse_to_dense(&g.terminals, INT);
		unsigned rp_term = sparseset_sparse_to_dense(&g.terminals, RP);

		bitvector_t *first_g = cfg_first(g);
		unittest_assert(bitvector_is_equal_set(first_g[sparseset_sparse_to_dense(&g.unterminals, F)], 2, lp_term + 1, int_term + 1), "");
		unittest_assert(bitvector_is_equal_set(first_g[sparseset_sparse_to_dense(&g.unterminals, TP)], 2, 0, mult_term + 1), "");
		unittest_assert(bitvector_is_equal_set(first_g[sparseset_sparse_to_dense(&g.unterminals, T)], 2, lp_term + 1, int_term + 1), "");
		unittest_assert(bitvector_is_equal_set(first_g[sparseset_sparse_to_dense(&g.unterminals, EP)], 2, 0, plus_term + 1), "");
		unittest_assert(bitvector_is_equal_set(first_g[sparseset_sparse_to_dense(&g.unterminals, E)], 2, lp_term + 1, int_term + 1), "");

		bitvector_t *follow_g = cfg_follow(g, first_g);
		const unsigned eof = g.terminals.card;
		unittest_assert(bitvector_is_equal_set(follow_g[sparseset_sparse_to_dense(&g.unterminals, F)], 4, plus_term + 1, mult_term + 1, rp_term + 1, eof + 1), "");
		unittest_assert(bitvector_is_equal_set(follow_g[sparseset_sparse_to_dense(&g.unterminals, TP)], 3, plus_term + 1, rp_term + 1, eof + 1), "");
		unittest_assert(bitvector_is_equal_set(follow_g[sparseset_sparse_to_dense(&g.unterminals, T)], 3, plus_term + 1, rp_term + 1, eof + 1), "");
		unittest_assert(bitvector_is_equal_set(follow_g[sparseset_sparse_to_dense(&g.unterminals, EP)], 2, rp_term + 1, eof + 1), "");
		unittest_assert(bitvector_is_equal_set(follow_g[sparseset_sparse_to_dense(&g.unterminals, E)], 2, rp_term + 1, eof + 1), "");

		for (unsigned i = 0; i < g.unterminals.card; i++){
			bitvector_destroy(&first_g[i]); 
		}
		free(first_g);
		for (unsigned i = 0; i < g.unterminals.card; i++) {
			bitvector_destroy(&follow_g[i]);
		}
		free(follow_g);
		cfg_destroy(&g);
	}, UNITTEST_DONE)
	unittest_test("Table LL(1)", {
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

		bitvector_t *first_g = cfg_first(g);
		bitvector_t *follow_g = cfg_follow(g, first_g);

		table_ll1_t table = cfg_table_ll1(g, first_g, follow_g);

		int fd = open("table.test.ll1", O_RDWR | O_CREAT | O_TRUNC, 0666);
		table_ll1_write(fd, &g, table);
		close(fd);

		cfg_t g2;
		table_ll1_t table2;
		fd = open("table.test.ll1", O_RDONLY);
		table_ll1_read(fd, &g2, &table2);
		close(fd);

		unsigned card_unterm = g.unterminals.card;
		unsigned card_term = g.terminals.card;

		unittest_assert(!memcmp(table, table2, card_term * card_unterm * sizeof(prod_coord_t)), "");

		cfg_destroy(&g2);
		free(table2);

		free(table);

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
	unittest_test("LR(0) Items Closure", {
		cfg_t g = cfg_new(EP, _SIZEOF_SYMBOLS + 1);
		cfg_add_terminal(&g, LP);
		cfg_add_terminal(&g, RP);

		unsigned pindex = cfg_add_prod(g, EP, prod_new(1, E));
		prod_t* axiom = &g.rules[EP].alternatives[pindex];
		cfg_add_prod(g, E, prod_new(4, LP, E, RP, E));
		cfg_add_prod(g, E, prod_new(0));
		bitvector_t *first_g = cfg_first(g);
		
		lr0_item_set_t s = hashset_new(
			(hkey_hash)lr0_item_hash, 
			(hkey_cmp)lr0_item_cmp, 
			(hkey_destroy)lr0_item_destroy
		);
		lr0_item_t *n = malloc(sizeof(lr0_item_t));
		n->left = EP;
		n->prod = axiom;
		n->placeholder = 0;
		hashset_add(&s, n);

		lr0_item_closure(g, &s, first_g);	

		// TODO : tests

		hashset_destroy(&s);
		for (unsigned i = 0; i < g.unterminals.card; i++){
			bitvector_destroy(&first_g[i]); 
		}
		free(first_g);

		cfg_destroy(&g);

	}, UNITTEST_DONE);
	unittest_test("LR(0) Items Automata", {	
		cfg_t g = cfg_new(EP, _SIZEOF_SYMBOLS);
		cfg_add_terminal(&g, LP);
		cfg_add_terminal(&g, RP);

		cfg_add_prod(g, EP, prod_new(1, E));
		cfg_add_prod(g, E, prod_new(4, LP, E, RP, E));
		cfg_add_prod(g, E, prod_new(0));

		bitvector_t *first_g = cfg_first(g);
		lr0_items_dfa_t automata = cfg_lr0_automata(g, first_g);

		unittest_assert(automata.transitions.length == 6, "");
		// fprintf(stderr, "A(0, LP) = %u\n", (automata.transitions.buffer[(0 * g.unterminals.card + LP) * sizeof(state_t)]));
		// fprintf(stderr, "A(0, E) = %u\n", (automata.transitions.buffer[(0 * g.unterminals.card + E) * sizeof(state_t)]));
		unittest_assert((automata.transitions.buffer[(1 * _SIZEOF_SYMBOLS + LP) * sizeof(state_t)] == 1), "");

		for (unsigned i = 0; i < g.unterminals.card; i++){
			bitvector_destroy(&first_g[i]); 
		}
		free(first_g);
		cfg_destroy(&g);

		cfg_lr0_automata_destroy(&automata);
	}, UNITTEST_DONE);
	unittest_test("SLR(1) Table Dyck words", {	
		cfg_t g = cfg_new(EP, _SIZEOF_SYMBOLS);
		cfg_add_terminal(&g, LP);
		cfg_add_terminal(&g, RP);
		unsigned lp_term = sparseset_sparse_to_dense(&g.terminals, LP);
		unsigned rp_term = sparseset_sparse_to_dense(&g.terminals, RP);
		unsigned eof = g.terminals.card;

		cfg_add_prod(g, EP, prod_new(1, E));
		cfg_add_prod(g, E, prod_new(4, LP, E, RP, E));
		cfg_add_prod(g, E, prod_new(0));

		bitvector_t *first_g = cfg_first(g);
		bitvector_t *follow_g = cfg_follow(g, first_g);
		lr0_items_dfa_t automata = cfg_lr0_automata(g, first_g);

		lr_table_t table = slr1_table(g, automata, follow_g);

		unittest_assert(table.action_table != NULL && table.goto_table != NULL, "Grammar IS SLR(1) but construction failed");
		if (table.action_table == NULL || table.goto_table == NULL) {
			exit(EXIT_FAILURE);
		}

		unittest_assert((table.action_table[0 * table.card_term + lp_term].t == LR_ACTION_SHIFT), "");
		unittest_assert((table.action_table[0 * table.card_term + lp_term].shift == 1), "");
		unittest_assert((table.action_table[1 * table.card_term + lp_term].t == LR_ACTION_SHIFT), "");
		unittest_assert((table.action_table[1 * table.card_term + lp_term].shift == 1), "");
		unittest_assert((table.action_table[3 * table.card_term + rp_term].t == LR_ACTION_SHIFT), "");
		unittest_assert((table.action_table[3 * table.card_term + rp_term].shift == 4), "");
		unittest_assert((table.action_table[4 * table.card_term + lp_term].t == LR_ACTION_SHIFT), "");
		unittest_assert((table.action_table[4 * table.card_term + lp_term].shift == 1), "");
		unittest_assert((table.action_table[1 * table.card_term + rp_term].t == LR_ACTION_REDUCE), "");
		unittest_assert((table.action_table[1 * table.card_term + eof].t == LR_ACTION_REDUCE), "");

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

	unittest_test("SLR(1) Table Ultra Simple Arithmetic", {	
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

		unittest_assert(table.action_table != NULL && table.goto_table != NULL, "Grammar IS SLR(1) but construction failed");
		if (table.action_table == NULL || table.goto_table == NULL) {
			exit(EXIT_FAILURE);
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
	}, UNITTEST_DONE);

	unittest_test("LR(1) Items Closure", {
		cfg_t g = cfg_new(E, _SIZEOF_SYMBOLS + 1);

		cfg_add_terminal(&g, PLUS); 
		cfg_add_terminal(&g, MULT);
		cfg_add_terminal(&g, LP);
		cfg_add_terminal(&g, RP);
		cfg_add_terminal(&g, INT);
		cfg_add_terminal(&g, _SIZEOF_SYMBOLS);

		/*
		 * E -> T EP
		 * EP -> + T EP | eps
		 * T -> F TP
		 * TP -> * F TP | eps
		 * F -> ( E ) | int
		 * 
		 * E -> E + T | T
		 * T -> T * F | F
		 * F -> int | ( E )
		 * 5 * (4 + 2) : E -> T -> T * F -> F * F -> F * (E) -> F * (E + T) ->3 F * (F * F) ->3 5 * (4 + 2)
		*/
		unsigned pindex = cfg_add_prod(g, E, prod_new(1, EP));
		prod_t* axiom = &g.rules[E].alternatives[pindex];
		cfg_add_prod(g, EP, prod_new(3, LP, EP, RP));
		cfg_add_prod(g, EP, prod_new(1, INT));

		bitvector_t *first_g = cfg_first(g);
		bitvector_t *follow_g = cfg_follow(g, first_g);

		lr1_item_set_t s = hashset_new(
			(hkey_hash)lr1_item_hash, 
			(hkey_cmp)lr1_item_cmp, 
			(hkey_destroy)lr1_item_destroy
		);
		lr1_item_t *n = malloc(sizeof(lr1_item_t));
		n->left = E;
		n->prod = axiom;
		n->placeholder = 0;
		n->lookahead = _SIZEOF_SYMBOLS;
		hashset_add(&s, n);

		// lr1_item_closure(g, &s, first_g);	

		hashset_iter(&s, (hkey_func)lr1_item_pretty_print);

		hashset_destroy(&s);

		for (unsigned i = 0; i < g.unterminals.card; i++){
			bitvector_destroy(&first_g[i]); 
		}
		free(first_g);
		for (unsigned i = 0; i < g.unterminals.card; i++) {
			bitvector_destroy(&follow_g[i]);
		}
		free(follow_g);
		cfg_destroy(&g);
	}, UNITTEST_TODO);
	unittest_test("Grammar file saving", {	
		cfg_t g = cfg_new(0, _SIZEOF_SYMBOLS);
		
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

		
		int fd = open("grammar.test.cfg", O_WRONLY | O_CREAT | O_TRUNC, 0666);
		ssize_t wb = cfg_write(fd, &g);
		close(fd);	

		cfg_t g2;
		fd = open("grammar.test.cfg", O_RDONLY);
		ssize_t rb = cfg_read(fd, &g2);
		close(fd);

		unittest_assert(wb == rb, "");
		
		cfg_destroy(&g);
		cfg_destroy(&g2);
	}, UNITTEST_DONE);
)
