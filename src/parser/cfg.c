#include "parser/cfg.h"
#include "containers/sparseset.h"
#include "parser/abnf.h"

#include "allocators/arena_alloc.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h> // qsort, malloc, calloc
#include <stdarg.h>

prod_t prod_new(unsigned length, ...) {
	if (length == 0) {
		return (prod_t){
			.length = 0,
			.array = 0,
		};
	}
	prod_t p = {
		.array = SAFE_CALLOC(calloc, length, sizeof(unsigned)),
		.length = length,
	};

	va_list args;
	va_start(args, length);

	for (unsigned i = 0; i < length; i++) {
		p.array[i] = va_arg(args, unsigned);
	}

	va_end(args);

	return p;
}

rule_t rule_new(unsigned capacity) {
	prod_t * alternatives = SAFE_CALLOC(calloc, capacity, sizeof(prod_t));
	return (rule_t){
		.length = 0,
		.capacity = capacity,
		.alternatives = alternatives
	};
}

void rule_destroy(rule_t *r) {
	assert(r->alternatives != NULL);
	for (unsigned pindex = 0; pindex < r->length; pindex++) {
		prod_t p = r->alternatives[pindex];
		free(p.array);
	}
	SAFE_FREE(r->alternatives);
	r->length = 0;
}

unsigned rule_add_prod(rule_t *r, prod_t p) {
	assert(r->length < r->capacity);

	unsigned pindex = r->length++;
	r->alternatives[pindex] = p;
	return pindex;
}

cfg_t cfg_new(unsigned axiom, unsigned card_symbols) {
	assert(axiom < card_symbols);
	rule_t *rules = SAFE_ALLOC(malloc, card_symbols, sizeof(rule_t));	
	for (unsigned r = 0; r < card_symbols; r++) {
		rules[r] = rule_new(CFG_DEFAULT_RULE_CAPACITY);
	}
	sparseset_t non_terminal = sparseset_new(card_symbols, card_symbols);
	for (unsigned s = 0; s < card_symbols; s++) {
		sparseset_add(&non_terminal, s);
	}
	cfg_t new = {
		.terminals = sparseset_new(card_symbols, card_symbols),
		.unterminals = non_terminal,
		.axiom = axiom,
		.rules = rules,
		.sym_props = SAFE_CALLOC(calloc, card_symbols, sizeof(symbol_prop_t))
	};
	return new;
}

void cfg_destroy(cfg_t *cfg) {
	unsigned card_symbols = cfg->terminals.capacity;
	for (unsigned i = 0; i < card_symbols; i++) {
		rule_destroy(&cfg->rules[i]);
	}
	free(cfg->rules);
	free(cfg->sym_props);
	sparseset_destroy(&cfg->terminals);
	sparseset_destroy(&cfg->unterminals);
	memset(cfg, 0, sizeof(cfg_t));
}

unsigned cfg_add_prod(cfg_t cfg, unsigned symbol, prod_t p) {
	assert(!(cfg.sym_props[symbol] & IS_TERMINAL));
	return rule_add_prod(&cfg.rules[symbol], p);
}

unsigned cfg_eof(const cfg_t cfg) {
	return cfg.unterminals.card + cfg.terminals.card;
}

void cfg_add_terminal(cfg_t *cfg, unsigned symbol) {
	sparseset_del(&cfg->unterminals, symbol);
	sparseset_add(&cfg->terminals, symbol);
	cfg->sym_props[symbol] |= IS_TERMINAL;
}

void cfg_add_token(cfg_t *cfg, unsigned int symbol) {
	cfg->sym_props[symbol] |= IS_TOKEN;
}

void cfg_add_concrete(cfg_t *cfg, unsigned symbol) {
	cfg->sym_props[symbol] |= IS_CONCRETE;
}

void cfg_symbol_set_prop(cfg_t cfg, unsigned symbol, symbol_prop_t sp) {
	assert(symbol < cfg.unterminals.capacity || symbol < cfg.terminals.capacity);
	cfg.sym_props[symbol] = sp;
}

void cfg_set_axiom(cfg_t *cfg, unsigned axiom) {
	assert(sparseset_mem(&cfg->unterminals, axiom));
	cfg->axiom = axiom;
}

ssize_t prod_write(int fd, prod_t* p) {
	ssize_t wb = 0;
	wb += write(fd, &p->length, sizeof(p->length));
	wb += write(fd, p->array, p->length * sizeof(*p->array));
	return wb;
}

ssize_t prod_read(int fd, prod_t * p) {
	ssize_t rb = 0;
	rb += read(fd, &p->length, sizeof(p->length));
	p->array = SAFE_CALLOC(calloc, p->length, sizeof(*p->array));
	rb += read(fd, p->array, p->length * sizeof(*p->array));
	return rb;
}

ssize_t rule_write(int fd, rule_t *r) {
	ssize_t wb = 0;
	wb += write(fd, &r->length, sizeof(r->length));
	for (unsigned pindex = 0; pindex < r->length; pindex++) {
		wb += prod_write(fd, &r->alternatives[pindex]);
	}
	return wb;
}

ssize_t rule_read(int fd, rule_t * r) {
	assert(r->alternatives == NULL);

	ssize_t rb = 0;
	rb += read(fd, &r->length, sizeof(r->length));
	r->alternatives = SAFE_CALLOC(calloc, r->length, sizeof(prod_t));
	r->capacity = r->length;
	for (unsigned pindex = 0; pindex < r->capacity; pindex++) {
		rb += prod_read(fd, &r->alternatives[pindex]);
	}
	return rb;
}
ssize_t cfg_write(int fd, const cfg_t * cfg) {
	ssize_t wb = 0;

	wb += sparseset_write(fd, &cfg->terminals);
	wb += sparseset_write(fd, &cfg->unterminals);
	wb += write(fd, &cfg->axiom, sizeof(cfg->axiom));
	for (unsigned i = 0; i < cfg->terminals.capacity; i++) {
		wb += rule_write(fd, &cfg->rules[i]);
	}
	wb += write(fd, cfg->sym_props, cfg->unterminals.capacity * sizeof(*cfg->sym_props));

	return wb;
}

ssize_t cfg_read(int fd, cfg_t* cfg) {
	assert(cfg->rules == NULL);
	assert(cfg->sym_props == NULL);

	ssize_t rb = 0;

	rb += sparseset_read(fd, &cfg->terminals);
	rb += sparseset_read(fd, &cfg->unterminals);
	
	const unsigned card = cfg->terminals.capacity;

	rb += read(fd, &cfg->axiom, sizeof(cfg->axiom));

	cfg->rules = SAFE_CALLOC(calloc, card, sizeof(*cfg->rules));
	for (unsigned i = 0; i < card; i++) {
		rb += rule_read(fd, &cfg->rules[i]);
	}

	cfg->sym_props = SAFE_CALLOC(calloc, card, sizeof(*cfg->sym_props));
	rb += read(fd, cfg->sym_props, cfg->unterminals.capacity * sizeof(*cfg->sym_props));

	return rb;
}

bitvector_t* cfg_first(const cfg_t g) {
	unsigned card_unterm = g.unterminals.card;
	unsigned card_term = g.terminals.card;
	bitvector_t *firsts = SAFE_ALLOC(malloc, (card_unterm + 1), sizeof(bitvector_t));
	for (unsigned i = 0; i < card_unterm; i++) {
		firsts[i] = bitvector_new(card_term + 1); // 1 more for epsilon
	}

	bitvector_t rhs = bitvector_new(card_term + 1); // 1 more for epsilon
	bool not_stable = true;
	while (not_stable) {
		not_stable = false;
		for (unsigned a = 0; a < card_unterm; a++) {
			unsigned a_symbol = sparseset_dense_to_sparse(&g.unterminals, a);
			rule_t r = g.rules[a_symbol];
			for (unsigned pindex = 0; pindex < r.length; pindex++) {
				prod_t p = r.alternatives[pindex];
				if (p.array == NULL) {
					not_stable = not_stable || (!bitvector_mem(&firsts[a], 0));
					bitvector_add(&firsts[a], 0);
					continue;
				}
				// p a la forme : a -> B0.B1...B(k-1)
				unsigned b = p.array[0]; // abstrait
				bitvector_clear(&rhs);
				if (!(g.sym_props[p.array[0]] & IS_TERMINAL)) {
					b = sparseset_sparse_to_dense(&g.unterminals, b); // concret + 1
					bool has_eps = bitvector_mem(&rhs, 0);
					bitvector_union(&rhs, &firsts[b]);
					bitvector_set(&rhs, 0, has_eps);
				} else { // b est un symbole terminal
					b = sparseset_sparse_to_dense(&g.terminals, b) + 1;
					bitvector_add(&rhs, b);
				}
				unsigned i;
				for (i = 0; !(g.sym_props[p.array[i]] & IS_TERMINAL) && bitvector_mem(&firsts[b], 0) && i < p.length - 1; i++) {
					b = p.array[i+1];
					if (!(g.sym_props[b] & IS_TERMINAL)) {
						b = sparseset_sparse_to_dense(&g.unterminals, b);
						bool has_eps = bitvector_mem(&rhs, 0);
						bitvector_union(&rhs, &firsts[b]);
						bitvector_set(&rhs, 0, has_eps);
					} else { // b est un symbole terminal
						b = sparseset_sparse_to_dense(&g.terminals, b) + 1;
						bitvector_add(&rhs, b);
					}
				}
				if (!(g.sym_props[p.array[i]] & IS_TERMINAL) && i == p.length - 1 && bitvector_mem(&firsts[b], 0)) {
					bitvector_add(&rhs, 0);
				}
				not_stable = not_stable || bitvector_union(&firsts[a], &rhs);
			}
		}
	}
	bitvector_destroy(&rhs);

	return firsts;
}

bitvector_t* cfg_follow(const cfg_t g, const bitvector_t *first) {
	unsigned card_unterm = g.unterminals.card;
	unsigned card_term = g.terminals.card;
	bitvector_t *follow = SAFE_ALLOC(malloc, card_unterm, sizeof(bitvector_t));
	for (unsigned i = 0; i < card_unterm; i++) {
		follow[i] = bitvector_new(card_term + 2);
	}

	// Ajout d'un symbole EOF artificiel en fin de grammaire
	bitvector_add(&follow[sparseset_sparse_to_dense(&g.unterminals, g.axiom)], card_term + 1); // g.card_term represents EOF
	
	bitvector_t trailer = bitvector_new(card_term + 2);

	bool not_stable = true; // not_stable = true <=> une modification de follow a eu lieu durant dernière boucle
	while (not_stable) {
		not_stable = false;
		for (unsigned a = 0; a < card_unterm; a++) {
			unsigned a_symbol = sparseset_dense_to_sparse(&g.unterminals, a);
			rule_t r = g.rules[a_symbol];
			for (unsigned pindex = 0; pindex < r.length; pindex++) {
				prod_t p = r.alternatives[pindex];
				if (p.array == NULL) {
					continue;
				}
				// p a la forme i -> B0.B1...B(k-1)
				bitvector_clear(&trailer);
				bitvector_union(&trailer, &follow[a]);
				for (unsigned i = p.length - 1; i + 1 > 0; i--) {
					int b = p.array[i];
					if (!(g.sym_props[b] & IS_TERMINAL)) { // is non terminal
						b = sparseset_sparse_to_dense(&g.unterminals, b);
						not_stable = not_stable || bitvector_union(&follow[b], &trailer);
						// MAJ trailer :
						if (bitvector_mem(&first[b], 0)) {
							bitvector_union(&trailer, &follow[b]);
						}
						bool has_eps = bitvector_mem(&trailer, 0);
						bitvector_union(&trailer, &first[b]);
						bitvector_set(&trailer, 0, has_eps);
					} else { // is terminal
						// direct MAJ trailer : 
						b = sparseset_sparse_to_dense(&g.terminals, b) + 1;
						bitvector_clear(&trailer);
						bitvector_add(&trailer, b);
					}
				}
			}
		}
	}
	bitvector_destroy(&trailer);

	return follow;
}

table_ll1_t cfg_table_ll1(const cfg_t g, const bitvector_t *first, const bitvector_t *follow) {
	unsigned card_unterm = g.unterminals.card;
	unsigned card_term = g.terminals.card;
	table_ll1_t table = SAFE_ALLOC(malloc, card_term * card_unterm, sizeof(*table));
	for (unsigned i = 0; i < card_term * card_unterm; i++) {
		table[i] = (prod_coord_t){.left = (unsigned)-1, .pindex = (unsigned)-1};
	}
	bitvector_t first_p = bitvector_new(card_term);
	
	for (unsigned a = 0; a < card_unterm; a++) {
		unsigned a_symbol = sparseset_dense_to_sparse(&g.unterminals, a);
		rule_t r = g.rules[a_symbol];
		for (unsigned pindex = 0; pindex < r.length; pindex++) {
			prod_t p = r.alternatives[pindex];
			if (p.array == NULL) {
				continue;
			}
			bitvector_clear(&first_p);
			bool has_eps = true;
			for (unsigned i = 0; has_eps && i < p.length; i++) {
				int b = p.array[i]; 
				if (!(g.sym_props[b] & IS_TERMINAL)) { // is non terminal 
					b = sparseset_sparse_to_dense(&g.unterminals, b);
					has_eps = has_eps && !bitvector_mem(&first[b], 0);
					bitvector_union(&first_p, &first[b]);
				} else { // is terminal
					b = sparseset_sparse_to_dense(&g.terminals, b) + 1;
					has_eps = false;
					bitvector_add(&first_p, b);
				}
			}
			if (has_eps) {
				bitvector_union(&first_p, &follow[a]);
			}
			for (unsigned w = 1; w <= card_term; w++) { // start to 1 because of epsilon
				if (bitvector_mem(&first_p, w)) {
					// Si une transition a déjà été assignée, la grammaire n'est pas LL(1)
					if (table[a * card_term + (w - 1)].left != (unsigned)-1) {
						bitvector_destroy(&first_p);
						free(table);
						return NULL;
					}
					table[a * card_term + (w - 1)] = (prod_coord_t){
						.left = a_symbol,
						.pindex = pindex
					};
				}
			}
		}
	}
	bitvector_destroy(&first_p);
	return table;
}

ssize_t table_ll1_write(int fd, const cfg_t * g, const table_ll1_t table) {
	unsigned card_unterm = g->unterminals.card;
	unsigned card_term = g->terminals.card;
	ssize_t wb = 0;

	wb += cfg_write(fd, g);
	wb += write(fd, table, card_term * card_unterm * sizeof(prod_coord_t));

	return wb;
}

ssize_t table_ll1_read(int fd, cfg_t * g, table_ll1_t* table) {	
	ssize_t rb = 0;

	rb += cfg_read(fd, g);
	unsigned card_unterm = g->unterminals.card;
	unsigned card_term = g->terminals.card;
	*table = SAFE_ALLOC(malloc, card_term * card_unterm, sizeof(prod_coord_t)); 
	rb += read(fd, *table, card_term * card_unterm * sizeof(prod_coord_t));

	return rb;
}

uint64_t lr0_item_hash(lr0_item_t* s) {
	return 0;
}
void lr0_item_destroy(lr0_item_t* s) {
	free(s);
}

int lr0_item_cmp(lr0_item_t* s1, lr0_item_t* s2) {
	if (s1->prod != s2->prod) {
		return s2->prod > s1->prod ? -1 : 1;
	}
	if (s1->placeholder != s2->placeholder) {
		return s2->placeholder > s1->placeholder ? -1 : 1;
	}
	return 0;
}

uint64_t lr1_item_hash(lr1_item_t* s) {
	return 0; // TODO
}

void lr1_item_destroy(lr1_item_t* s) {
	free(s);
}

int lr1_item_cmp(lr1_item_t* s1, lr1_item_t* s2) {
	return memcmp(s1, s2, sizeof(lr1_item_t));
}


void lr0_item_pretty_print(lr0_item_t *item) {
	lr0_item_t it = *item;
	fprintf(stderr, "%u -> ", it.left);
	for (unsigned i = 0; i < it.placeholder; i++) {
		fprintf(stderr, "%u ", it.prod->array[i]);
	}
	fprintf(stderr, ". ");
	for (unsigned i = it.placeholder; i < it.prod->length; i++) {
		fprintf(stderr, "%u ", it.prod->array[i]);
	}
	fprintf(stderr, "\n");
}
void lr1_item_pretty_print(lr1_item_t *item) {
	lr1_item_t it = *item;
	fprintf(stderr, "%u -> ", it.left);
	for (unsigned i = 0; i < it.placeholder; i++) {
		fprintf(stderr, "%u ", it.prod->array[i]);
	}
	fprintf(stderr, ". ");
	for (unsigned i = it.placeholder; i < it.prod->length; i++) {
		fprintf(stderr, "%u ", it.prod->array[i]);
	}
	fprintf(stderr, "[%u]\n", it.lookahead);
}

void lr0_item_closure(const cfg_t g, lr0_item_set_t* s, const bitvector_t *firsts) {
	bool modified = true;
	while (modified) {
		modified = false;
		hashset_list_t items_l = hashset_list_yield(s);
		for (
			unsigned i = 0;
			i < items_l.length;
			i++
		) {
			const lr0_item_t* item = items_l.keys[i];
			if (item->placeholder >= item->prod->length) { // the item is an handle, i.e. a leaf of the items DFA
				continue;
			}
			rule_symbol_t C = item->prod->array[item->placeholder];
			if (g.sym_props[C] & IS_TERMINAL) {
				continue;
			}
			for (unsigned pindex = 0; pindex < g.rules[C].length; pindex++) {
				lr0_item_t item = {
					.left = C,
					.prod = &g.rules[C].alternatives[pindex],
					.placeholder = 0,
				};
				if (!hashset_mem(s, &item)) {
					lr0_item_t * n = SAFE_ALLOC(malloc, 1, sizeof(lr0_item_t));
					*n = item;
					hashset_add(s, n);
					modified = true;
				}
			}	
		}
		hashset_list_destroy(items_l);
	}
}

lr0_item_set_t lr0_item_goto(const cfg_t g, lr0_flat_item_set_t s, rule_symbol_t x, const bitvector_t *first) {
	lr0_item_set_t moved = hashset_new(
		(hkey_hash)lr0_item_hash,
		(hkey_cmp)lr0_item_cmp,
		(hkey_destroy)lr0_item_destroy,
	);
	for (
		unsigned i = 0;
		i < s.length;
		i++
	) {
		lr0_item_t item = s.data[i];
		if (item.placeholder < item.prod->length && item.prod->array[item.placeholder] == x) {
			lr0_item_t *n = SAFE_ALLOC(malloc, 1, sizeof(lr0_item_t));
			*n = (lr0_item_t) {
				.left = item.left,
				.prod = item.prod, 
				.placeholder = item.placeholder + 1,
			};
			hashset_add(&moved, n);
		}
	}
	lr0_item_closure(g, &moved, first);
	return moved;
}

lr0_flat_item_set_t lr0_item_set_flatten(lr0_item_set_t * s) {
	lr0_flat_item_set_t fs = {
		.data = SAFE_ALLOC(malloc, s->cardinal, sizeof(lr0_item_t)),
		.length = s->cardinal,
	};
	hashset_list_t items_l = hashset_list_yield(s);	
	for (
		unsigned i = 0;
		i < items_l.length;
		i++
	) {
		const lr0_item_t* item = items_l.keys[i];
		fs.data[i] = *item;
	}
	hashset_list_destroy(items_l);
	qsort(fs.data, fs.length, sizeof(lr0_item_t), (__compar_fn_t)lr0_item_cmp);
	return fs;
}

uint64_t lr0_flat_item_set_hash(lr0_flat_item_set_t * s) {
	return 0;
}

void lr0_flat_item_set_destroy(lr0_flat_item_set_t * s) {
	free(s->data);
	free(s);
}

int lr0_flat_item_set_cmp(lr0_flat_item_set_t * s1, lr0_flat_item_set_t * s2) {
	return !(s1->length == s2->length && !memcmp(s1->data, s2->data, s1->length * sizeof(lr0_item_t)));
}

uint64_t lr0_set_state_hash(lr0_set_state_t * fs) {
	return lr0_flat_item_set_hash(&fs->s);
}

void lr0_set_state_destroy(lr0_set_state_t * fs) {
	free(fs->s.data);
	free(fs);
}

int lr0_set_state_cmp(lr0_set_state_t * fs1, lr0_set_state_t * fs2) {
	return lr0_flat_item_set_cmp(&fs1->s, &fs2->s);
}

lr0_items_dfa_t cfg_lr0_automata(const cfg_t g, const bitvector_t * first_g) {
	const unsigned symbols_card = (g.unterminals.card + g.terminals.card);
	prod_t* axiom = &g.rules[g.axiom].alternatives[0];
	lr0_item_set_t s = hashset_new(
		(hkey_hash)lr0_item_hash, 
		(hkey_cmp)lr0_item_cmp, 
		(hkey_destroy)lr0_item_destroy
	);
	lr0_item_t *init = SAFE_ALLOC(malloc, 1, sizeof(lr0_item_t));
	init->left = g.axiom;
	init->prod = axiom;
	init->placeholder = 0;
	hashset_add(&s, init);
	lr0_item_closure(g, &s, first_g);
	
	lr0_items_dfa_t automata = {
		.initial = 0,
		.states = hashset_new(
			(hkey_hash)lr0_set_state_hash,
			(hkey_cmp)lr0_set_state_cmp,
			(hkey_destroy)lr0_set_state_destroy
		),
		.transitions = arraylist_new(16, symbols_card * sizeof(state_t)),
	};
	arraylist_push(&automata.transitions, NULL);

	lr0_set_state_t * fs = SAFE_ALLOC(malloc, 1, sizeof(lr0_set_state_t));
	*fs = (lr0_set_state_t){
		.s = lr0_item_set_flatten(&s),
		.state = 0,
	};
	hashset_destroy(&s);

	hashset_add(&automata.states, fs);

	unsigned max_visited = 0;

	bool modified = true;
	while (modified) {
		modified = false;
		hashset_list_t items_l = hashset_list_yield(&automata.states);
		for (unsigned i = 0; i < items_l.length; i++) {
			const lr0_set_state_t * fs = items_l.keys[i];
			if (fs->state + 1 <= max_visited) {
				continue;
			}
			for (unsigned j = 0; j < fs->s.length; j++) {
				lr0_item_t it = fs->s.data[j];
				if (it.placeholder >= it.prod->length) {
					continue;
				}
				rule_symbol_t x = it.prod->array[it.placeholder];

				lr0_item_set_t tmp = lr0_item_goto(g, fs->s, x, first_g);

				lr0_set_state_t * tmp_fs = SAFE_ALLOC(malloc, 1, sizeof(lr0_set_state_t));
				*tmp_fs = (lr0_set_state_t){
					.s = lr0_item_set_flatten(&tmp),
					.state = automata.transitions.length,
				};
				hashset_destroy(&tmp);

				state_t destination;

				lr0_set_state_t * old = hashset_find(&automata.states, tmp_fs);
				if (old == NULL) {
					hashset_add(&automata.states, tmp_fs);
					modified = true;
					arraylist_push(&automata.transitions, NULL);
					destination = tmp_fs->state;
				} else {
					destination = old->state;
					lr0_set_state_destroy(tmp_fs);
				}
				((state_t*)automata.transitions.buffer)[fs->state * symbols_card + x] = destination;
			}	
		}
		max_visited = items_l.length;
		hashset_list_destroy(items_l);
	}

	return automata;
}

void cfg_lr0_automata_destroy(lr0_items_dfa_t * automata) {
	hashset_destroy(&automata->states);
	arraylist_destroy(&automata->transitions);
	memset(automata, 0, sizeof(*automata));
}

void lr_table_destroy(lr_table_t * lrt) {
	free(lrt->action_table);
	free(lrt->goto_table);
	memset(lrt, 0, sizeof(*lrt));
}

lr_table_t slr1_table(const cfg_t g, const lr0_items_dfa_t automata, const bitvector_t * follow_g) {
	const unsigned symbols_card = (g.unterminals.card + g.terminals.card);
	lr_table_t lrt = {
		.nb_states = automata.transitions.length,
		.card_term = g.terminals.card + 1, // + 1 pour EOF
		.card_unterm = g.unterminals.card,
		.action_table = SAFE_CALLOC(calloc, automata.transitions.length * (g.terminals.card + 1), sizeof(lr_action_t)),
		.goto_table = SAFE_CALLOC(calloc, automata.transitions.length * g.unterminals.card, sizeof(state_t)),
	};
	rule_symbol_t eof = g.terminals.card;
	hashset_list_t items_l = hashset_list_yield(&automata.states);
	for (unsigned s = 0; s < items_l.length; s++) {
		// fill the goto table : OK
		for (unsigned x = 0; x < g.unterminals.card; x++) {
			rule_symbol_t term = sparseset_dense_to_sparse(&g.unterminals, x);
			if (lrt.goto_table[s * lrt.card_unterm + x] != 0) {
				goto error;
			}
			lrt.goto_table[s * lrt.card_unterm + x] = ((state_t*)automata.transitions.buffer)[s * symbols_card + term];
		}

		// fill the action table
		const lr0_set_state_t * lrss = items_l.keys[s];
		for (unsigned j = 0; j < lrss->s.length; j++) {	
			lr0_item_t it = lrss->s.data[j];
			if (it.placeholder >= it.prod->length) { // complete item <=> handle
				if (it.prod == g.rules[g.axiom].alternatives) {
					if (lrt.action_table[lrss->state * lrt.card_term + eof].t != LR_ACTION_NONE) {
						goto error;
					}
					lrt.action_table[lrss->state * lrt.card_term + eof] = (lr_action_t){
						.t = LR_ACTION_ACCEPT,
					};
					continue;
				}
				bitvector_iterator_t bv_iter = bitvector_iterator(&follow_g[sparseset_sparse_to_dense(&g.unterminals, it.left)]);
				for (
					unsigned x = bitvector_iter_next(&bv_iter);
					x != bitvector_iter_end();
					x = bitvector_iter_next(&bv_iter)
				) {
					if (lrt.action_table[lrss->state * lrt.card_term + x - 1].t != LR_ACTION_NONE) {
						goto error;
					}
					lrt.action_table[lrss->state * lrt.card_term + x - 1] = (lr_action_t){
						.t = LR_ACTION_REDUCE,
						.left = it.left,
						.reduce = it.prod,
					};
				}
			} else {
				rule_symbol_t x = it.prod->array[it.placeholder];
				if (g.sym_props[x] & IS_TERMINAL) {
					rule_symbol_t term = sparseset_sparse_to_dense(&g.terminals, x);
					if (lrt.action_table[lrss->state * lrt.card_term + term].t != LR_ACTION_NONE) {
						goto error;
					}
					lrt.action_table[lrss->state * lrt.card_term + term] = (lr_action_t){
						.t = LR_ACTION_SHIFT,
						.left = it.left,
						.shift = ((state_t*)automata.transitions.buffer)[lrss->state * symbols_card + x],
					};
				}
			}
		}
	}
	hashset_list_destroy(items_l);
	return lrt;
error:
	hashset_list_destroy(items_l);
	lr_table_destroy(&lrt);
	return lrt;
}

void lr_table_pretty_print(FILE * f, const cfg_t g, lr_table_t table) {
	for (unsigned s = 0; s < table.nb_states; s++) {
		fprintf(f, "state %u ", s);
		for (unsigned x = 0; x < table.card_term; x++) {
			rule_symbol_t term = x == g.terminals.card ? cfg_eof(g) : sparseset_dense_to_sparse(&g.terminals, x);
			lr_action_t action = table.action_table[s * table.card_term + x];
			switch (action.t) {
				case LR_ACTION_SHIFT:
				fprintf(f, "%u[S -> %u]", term, action.shift);
				break;
				case LR_ACTION_REDUCE:
				fprintf(f, "%u[R -> %u]", term, action.left);
				break;
				case LR_ACTION_ACCEPT:
				fprintf(f, "%u[Accept]", term);
				break;
				case LR_ACTION_NONE:
				fprintf(f, "%u[      ]", term);
				break;
			}
		}
		fprintf(f, "\n");
	}
}
/*
 * idée d'optimisation (mémoïsation) :
 * on peut facilement retomber sur un sous-ensemble d'une clôture déjà calculée.
 * Si c'est le cas, on doit obtenir la-dite clôture.
 *
 * La fonction dépend uniquement de lr1_item_set_t dont on doit calculer la clôture et de la grammaire G.
 * La clôture d'un LR(1) item set S est l'union itérée jusqu'à point fixe des clôtures de chaque LR(1) item de S.
 * Donc on peut conserver table qui à chaque LR(1) item associe les LR(1) items de clôture.
 * Si on tombe sur un LR(1) item déjà calculé on ajoute les LR(1) items de clôture 
 * (ce qui inclut la clôture de ces mêmes LR(1) items)
 */
void lr1_item_closure(const cfg_t g, lr1_item_set_t* s, int*c_lr1, const bitvector_t *firsts) {
	bool modified = true;
	while (modified) {
		modified = false;
		hashset_list_t items_l = hashset_list_yield(s);
		for (
			unsigned i = 0;
			i < items_l.length;
			i++
		) {
			const lr1_item_t* item = items_l.keys[i];
			if (item->placeholder < item->prod->length) { // else the item is an handle
				rule_symbol_t C = item->prod->array[item->placeholder];
				rule_symbol_t a = item->lookahead;
				bitvector_t first_delta = bitvector_new(g.terminals.card + 2); // + 2 for epsilon and EOF
				unsigned d = item->placeholder + 1;
				if (d < item->prod->length) {
					rule_symbol_t sym = item->prod->array[d];
					bool has_eps = false;
					if (!(g.sym_props[sym] & IS_TERMINAL)) {
						sym = sparseset_sparse_to_dense(&g.unterminals, sym);
						has_eps = bitvector_mem(&firsts[sym], 0);
						bitvector_union(&first_delta, &firsts[sym]);
					} else {
						sym = sparseset_sparse_to_dense(&g.terminals, sym) + 1;
						bitvector_add(&first_delta, sym);
					}
					d++;
					for (; has_eps && d < item->prod->length; d++) {
						rule_symbol_t sym = item->prod->array[d];
						has_eps = false;
						if (!(g.sym_props[sym] & IS_TERMINAL)) {
							sym = sparseset_sparse_to_dense(&g.unterminals, sym);
							has_eps = bitvector_mem(&firsts[sym], 0);
							bitvector_union(&first_delta, &firsts[sym]);
						} else { // b est un symbole terminal
							sym = sparseset_sparse_to_dense(&g.terminals, sym) + 1;
							bitvector_add(&first_delta, sym);
						}
					}
					if (bitvector_mem(&first_delta, 0)) {
						bitvector_add(&first_delta, sparseset_sparse_to_dense(&g.terminals, a) + 1);
					}
				} else { // delta = epsilon
					bitvector_add(&first_delta, sparseset_sparse_to_dense(&g.terminals, a) + 1);
				}
				if (!(g.sym_props[C] & IS_TERMINAL)) {
					for (unsigned pindex = 0; pindex < g.rules[C].length; pindex++) {
						unsigned max = first_delta.capacity << 5;
						for (unsigned i = 1; i < max; i++) {
							if (bitvector_mem(&first_delta, i)) {
								lr1_item_t item = {
									.left = C,
									.prod = &g.rules[C].alternatives[pindex],
									.placeholder = 0,
									.lookahead = sparseset_dense_to_sparse(&g.terminals, i - 1),
								};
								if (!hashset_mem(s, &item)) {
									lr1_item_t *n = SAFE_ALLOC(malloc, 1, sizeof(lr1_item_t));
									*n = item;
									hashset_add(s, n);
									modified = true;
								}
							}
						}
					}
				}
				bitvector_destroy(&first_delta);
			}
		}
		hashset_list_destroy(items_l);
	}
}

lr1_item_set_t lr1_item_goto(const cfg_t g, lr1_item_set_t* s, rule_symbol_t x, const bitvector_t *first) {
	lr1_item_set_t moved = hashset_new(
		(hkey_hash)lr1_item_hash, 
		(hkey_cmp)lr1_item_cmp,
		(hkey_destroy)lr1_item_destroy,
	);
	hashset_iterator_t iter = hashset_iterator(s); 
	for (
		lr1_item_t* item = hashset_iter_next(&iter);
		item;
		item = hashset_iter_next(&iter)
	) {
		if (item->placeholder < item->prod->length && item->prod->array[item->placeholder] == x) {
			lr1_item_t *n = malloc(sizeof(lr1_item_t));
			*n = (lr1_item_t) {
				.left = item->left,
				.prod = item->prod, 
				.placeholder = item->placeholder + 1,
				.lookahead = item->lookahead
			};
			hashset_add(&moved, n);
		}
	}
	lr1_item_closure(g, &moved, NULL, first);
	return moved;
}

