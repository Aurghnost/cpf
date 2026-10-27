#include "parser/ast.h"
#include "containers/bitvector.h"
#include "parser/cfg.h"
#include "parser/lexem.h"

#include "containers/arraylist.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct ll1_context_s {
	table_ll1_t table;
	cfg_t g;
	lexem_node_t *current;
	unsigned rule;
	struct error_state_s {
		bool error;
		int expected;
		lexem_t got;
	} state;
} ll1_context_t;

typedef struct lr_context_s {
	lr_table_t table;
	cfg_t g;
	lexem_node_t *current;
	symbol_printer_t sp;
	struct error_state_s {
		bool error;
		int expected;
		lexem_t got;
	} state;

} lr_context_t;

ast_node_t *ast_ll1_parsing(ll1_context_t* c) {
	// Symbole Terminal
	if (c->g.sym_props[c->rule] & IS_TERMINAL) {
		if (c->current == NULL) { // EOF reached
			c->state = (struct error_state_s) {
				.error = true,
				.expected=c->rule,
				.got={.symbol=cfg_eof(c->g)}
			};
			return NULL;
		}

		if (c->rule != c->current->lex.symbol) {
			c->state = (struct error_state_s) {
				.error = true,
				.expected=c->rule,
				.got=c->current->lex
			};
			return NULL;
		}

		ast_node_t *leaf = NULL;
		if (!(c->g.sym_props[c->rule] & IS_CONCRETE)) {
			leaf = SAFE_ALLOC(malloc, 1, sizeof(ast_node_t));
			*leaf = (ast_node_t) {
				.length = 0,
				.children = NULL,
				.label = c->current->lex
			};
		}
		c->current = c->current->next;
		return leaf;
	}
	
	// Symbole non terminal : règle à appliquer
	lexem_node_t *nd = c->current;
	if (nd == NULL) { // TODO : il faut gérer la fin de fichier proprement
		// unsigned eof = c->g.terminals.card + c->g.unterminals.card;
		// c->state = (struct error_state_s){.error =true, .expected = c->rule, .got=(lexem_t){.symbol=eof}};
		return NULL;
	}
	unsigned term = nd->lex.symbol;
 
	unsigned concrete_rule = sparseset_sparse_to_dense(&c->g.unterminals, c->rule);
	prod_coord_t pc = c->table[concrete_rule * c->g.terminals.card + term];
	if (pc.left == (unsigned)-1) {
		// c->state = (struct error_state_s){.error =true};
		return NULL;
	}
	prod_t p = c->g.rules[pc.left].alternatives[pc.pindex];
	if (p.array == NULL) { // is epsilon
		return NULL;
	}

	ast_node_t *new = SAFE_ALLOC(malloc, 1, sizeof(ast_node_t));
	new->label = (lexem_t){.symbol=c->rule};
	new->length = 0;
	new->children = SAFE_ALLOC(malloc, p.length, sizeof(*new->children));

	for (unsigned i = 0; i < p.length; i++) { 
		c->rule = p.array[i];
		new->children[new->length] = ast_ll1_parsing(c);
		if (c->state.error) {
			ast_destroy(new);
			return NULL;
		}
		if (new->children[new->length] != NULL) {
			new->length++;
		}
	}
	return new;
}

typedef struct lr_parse_stack_elt_s {
	rule_symbol_t term;
	state_t s;
	ast_node_t * node;
} lr_parse_stack_elt_t;

void stack_elt_prettyprint(lr_parse_stack_elt_t e, symbol_printer_t sp) {
	fprintf(stderr, "[");
	sp(stderr, (lexem_t){.symbol=e.term});
	fprintf(stderr, "|%u", e.s);
	fprintf(stderr, "]");
}

void stack_prettyprint(arraylist_t stack, symbol_printer_t sp) {
	for (unsigned i = 0; i < stack.length; i++) {
		lr_parse_stack_elt_t e = ((lr_parse_stack_elt_t*)stack.buffer)[i];
		stack_elt_prettyprint(e, sp);
	}
	fprintf(stderr, "\n");
}

ast_node_t *ast_lr_parsing(lr_context_t * c) {
	arraylist_t stack = arraylist_new(32, sizeof(lr_parse_stack_elt_t));
	lr_parse_stack_elt_t elt = {.term = cfg_eof(c->g), .s = 0};
	arraylist_push(&stack, &elt);

	const unsigned eof = cfg_eof(c->g);

	while (true) {
		lr_parse_stack_elt_t top = ((lr_parse_stack_elt_t*)stack.buffer)[stack.length-1];
		rule_symbol_t term = c->current->lex.symbol == eof 
			? c->g.terminals.card 
			: sparseset_sparse_to_dense(&c->g.terminals, c->current->lex.symbol);
		// stack_prettyprint(stack, c->sp);
		lr_action_t action = c->table.action_table[top.s * c->table.card_term + term];
		switch (action.t) {
			case LR_ACTION_SHIFT:
				// fprintf(stderr, "SHIFT ");
				// c->sp(stderr, (lexem_t){.symbol=action.left});
				// fprintf(stderr, " -> %u\n", action.shift);
				ast_node_t * leaf = NULL;
				if (!(c->g.sym_props[c->current->lex.symbol] & IS_CONCRETE)) {
					leaf = SAFE_ALLOC(malloc, 1, sizeof(ast_node_t));
					*leaf = (ast_node_t) {
						.length = 0,
						.children = NULL,
						.label = c->current->lex
					};
				}
				elt = (lr_parse_stack_elt_t){
					.term = c->current->lex.symbol,
					.s = action.shift,
					.node = leaf,
				};
				arraylist_push(&stack, &elt);
				c->current = c->current->next;
			break;
			case LR_ACTION_REDUCE:
				// fprintf(stderr, "REDUCE");
				// c->sp(stderr, (lexem_t){.symbol=action.left});
				// fprintf(stderr, " <- ");
				prod_t * p = action.reduce;
				rule_symbol_t left = action.left;
				ast_node_t *new = SAFE_ALLOC(malloc, 1, sizeof(ast_node_t));
				*new = (ast_node_t){
					.label=(lexem_t){.symbol=left},
					.length = 0,
					.children = SAFE_ALLOC(malloc, p->length, sizeof(ast_node_t*)),
				};
				for (unsigned i = 0; i < p->length; i++) {
					rule_symbol_t x = p->array[p->length - i - 1];
					// c->sp(stderr, (lexem_t){.symbol=p->array[i]});
					// fprintf(stderr, " ");
					top = ((lr_parse_stack_elt_t*)stack.buffer)[stack.length-1];
					if (!(c->g.sym_props[x] & IS_CONCRETE)) {
						new->children[new->length++] = top.node;
					}
					arraylist_pop(&stack);
				}
				// fprintf(stderr, "\n");
				top = ((lr_parse_stack_elt_t*)stack.buffer)[stack.length-1];
				// stack_elt_prettyprint(top, c->sp);
				rule_symbol_t term_left = sparseset_sparse_to_dense(&c->g.unterminals, left);
				elt = (lr_parse_stack_elt_t) {
					.term = left,
					.s = c->table.goto_table[top.s * c->table.card_unterm + term_left],
					.node = new,
				};
				arraylist_push(&stack, &elt);
			break;
			case LR_ACTION_ACCEPT:
				arraylist_destroy(&stack);
				return top.node;
			break;
			case LR_ACTION_NONE:
			default:
				fprintf(stderr, "Locally available inputs are : ");
				for (unsigned x = 0; x < c->table.card_term; x++) {
					lr_action_t action = c->table.action_table[top.s * c->table.card_term + x];
					if (action.t != LR_ACTION_NONE){
						c->sp(stderr, (lexem_t){
							.symbol=(x == c->g.terminals.card) ? eof : sparseset_dense_to_sparse(&c->g.terminals, x)
						});
					}
				}
				fprintf(stderr, "\n");
				c->state = (struct error_state_s) {
					.error = true,
					.expected=0,
					.got=c->current->lex,
				};
				for (unsigned i = 0; i < stack.length; i++) {
					lr_parse_stack_elt_t n = ((lr_parse_stack_elt_t*)stack.buffer)[i];
					if (n.node != NULL) {
						ast_destroy(n.node);
					}
				}
				arraylist_destroy(&stack);
				return NULL;
			break;
		}
	}
	return NULL;
}

ast_t ast_from_lr_table(const cfg_t g, lr_table_t table, lexem_list_t src, symbol_printer_t sp) {
	lr_context_t c = {
		.table = table,
		.g = g,
		.current = src.head,
		.sp = sp,
		.state = (struct error_state_s) {
			.error = false,
			.expected = 0,
			.got = {.symbol = 0, .repr={.buf=NULL,.length=0,.capacity=0}},
		},
	};

	ast_t tree = ast_lr_parsing(&c);
	
	if (c.state.error) {
		if (sp == NULL)
			fprintf(stderr, "Expecting %u, got : %u\n", c.state.expected, c.state.got.symbol);
		else {
			fprintf(stderr, "Expecting ");
			sp(stderr, (lexem_t){.symbol = c.state.expected});
			fprintf(stderr, ", got : ");
			sp(stderr, c.state.got);
			fprintf(stderr, "\n");
		}
	}

	return tree;
}

ast_t ast_from_ll1(const cfg_t g, table_ll1_t table, lexem_list_t src, symbol_printer_t sp) {
	ll1_context_t c = {
		.table = table,
		.rule = g.axiom,
		.current = src.head,
		.g = g,
		.state = (struct error_state_s) {
			.error = false,
			.expected = 0,
			.got = {.symbol = 0, .repr={.buf=NULL,.length=0,.capacity=0}},
		},
	};

	ast_t tree = ast_ll1_parsing(&c);

	if (c.state.error) {
		if (sp == NULL)
			fprintf(stderr, "Expecting %u, got : %u\n", c.state.expected, c.state.got.symbol);
		else {
			fprintf(stderr, "Expecting ");
			sp(stderr, (lexem_t){.symbol = c.state.expected});
			fprintf(stderr, ", got : ");
			sp(stderr, c.state.got);
			fprintf(stderr, "\n");
		}
	}
	return tree;
}

void ast_destroy(ast_t tree) {
	for (unsigned i = 0; i < tree->length; i++) {
		ast_destroy(tree->children[i]);
	}
	free(tree->children);
	free(tree);
}

ast_t ast_from_lr_cfg(const cfg_t g, lexem_list_t src, symbol_printer_t sp) {
	return NULL;
}
