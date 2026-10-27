#include "parser/lexem.h"
#include "parser/string.h"

#include <stdlib.h>
#include <stdarg.h>

void lexem_destroy(lexem_t lex) {
    if (lex.repr.buf != NULL) {
	string_destroy(&lex.repr);
    }
}

lexem_list_t lexem_list_new() {
    return (lexem_list_t){
	.head = NULL,
	.length = 0
    };
}

void lexem_list_push(lexem_list_t *list, lexem_t lex) {
    lexem_node_t *new_node = malloc(sizeof(lexem_node_t));
    new_node->lex = lex;
    new_node->next = list->head;
    list->head = new_node;
    list->length++;
}

lexem_t lexem_list_pop(lexem_list_t *list) {
    assert(!lexem_list_is_empty(*list));
    lexem_node_t *to_del = list->head;
    list->head = to_del->next;
    lexem_t lex = to_del->lex;
    free(to_del);
    list->length--;
    return lex;
}

void lexem_list_reverse(lexem_list_t *list) {
    lexem_list_t reversed = lexem_list_new();

    while (!lexem_list_is_empty(*list)) {
	lexem_list_push(&reversed, lexem_list_pop(list));
    }
    *list = reversed;
}

void lexem_list_destroy(lexem_list_t *list) {
    while (!lexem_list_is_empty(*list)) {
	lexem_t lex = lexem_list_pop(list);
      	lexem_destroy(lex);
    }
    assert(list->head == NULL && list->length == 0);
}

bool lexem_list_is_empty(const lexem_list_t list) {
    return list.length == 0;
}

bool lexem_list_is_equal(const lexem_list_t list, unsigned length, ...) {
	bool is_eq = list.length == length;
	va_list ap;
	va_start(ap, length);
	for (lexem_node_t * node = list.head; is_eq && node != NULL; node = node->next) {
		is_eq = is_eq && node->lex.symbol == va_arg(ap, symbol_t);
	}
	va_end(ap);

	return is_eq;
}

void lexem_list_iter(const lexem_list_t list, lexem_function f) {
	for (lexem_node_t * node = list.head; node != NULL; node = node->next) {
		f(node->lex);
	}

}
