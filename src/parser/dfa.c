#include "parser/dfa.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

#define SAFE_FREE(p) do {free(p); p = NULL;} while(0)

#define SAFE_ALLOC(allocator, nmb, size) ({ \
	void* x = allocator(nmb * size); \
	if (x == NULL) \
		err(EXIT_FAILURE, "%s:%u", __FILE__, __LINE__); \
	x;\
})

#define SAFE_CALLOC(allocator, nmb, size) ({ \
	void* x = allocator(nmb, size); \
	if (x == NULL) \
		err(EXIT_FAILURE, "%s:%u", __FILE__, __LINE__); \
	x;\
})

#define SAFE_REALLOCARRAY(allocator, arrptr, nmb, size) ({ \
	void* x = allocator(arrptr, nmb, size); \
	if (x == NULL) \
		err(EXIT_FAILURE, "%s:%u", __FILE__, __LINE__); \
	x;\
})

dfa_t dfa_new(unsigned nb_states, unsigned nb_alpha) {
	transition_t* trans = SAFE_CALLOC(calloc, nb_states * nb_alpha, sizeof(transition_t)); 
	dfa_t a = {
		.states = nb_states, 
		.alpha = nb_alpha,
		.trans = trans,
	};	
	return a;
}

void dfa_add(dfa_t a, state_t p, unsigned char l, transition_t t) {
	assert(p*a.alpha + l < a.states * a.alpha);
	a.trans[p * a.alpha + l] = t;
}

void dfa_del(dfa_t a, state_t p, unsigned char l) {
	memset(&a.trans[p * a.alpha + l], 0, sizeof(*a.trans));
}

void dfa_add_error(dfa_t a, state_t p, unsigned char l) {
	dfa_add(a, p, l, (transition_t){.q = p, .flags=DFA_NO_FORWARD});
}

transition_t dfa_trans(const dfa_t a, state_t p, unsigned char l) {
	assert(p*a.alpha + l < a.alpha * a.states);
	return a.trans[p * a.alpha + l];
}

lexem_list_t dfa_execute(const dfa_t a, const char *text) {
	lexem_list_t l = lexem_list_new();

	string_t repr = (string_t){
		.buf = NULL,
		.length=0,
		.capacity=0,
	};

	const char *original = text;

	state_t p = a.init;
	while (text == original || *(text-1)) {
		transition_t t = dfa_trans(a, p, *text);
		if (t.flags & DFA_NO_FORWARD && p == t.q) {
			lexem_list_destroy(&l);
			goto end;
		}
		p = t.q;
		if (t.flags & DFA_RECORD) {
			if (repr.buf == NULL) {
				repr = string_new();
			}
			string_write_c(&repr, *text);
		}
		if (t.flags & DFA_REGISTER) {
			lexem_list_push(&l, (lexem_t){.symbol = t.symbol, .repr = repr});
			repr = (string_t){.buf=NULL,.length=0,.capacity=0};
		}
		text += !(t.flags & DFA_NO_FORWARD);
	}
end:
	if (repr.buf != NULL) string_destroy(&repr);

	return l;
}

void dfa_destroy(dfa_t *a) {
	free(a->trans);
	memset(a, 0, sizeof(*a));
}

ssize_t dfa_read(int fd, dfa_t * a) {
	ssize_t rb = 0;

	rb += read(fd, &a->states, sizeof(a->states));
	rb += read(fd, &a->init, sizeof(a->init));
	rb += read(fd, &a->alpha, sizeof(a->alpha));
	a->trans = SAFE_CALLOC(calloc, a->states * a->alpha, sizeof(*a->trans));
	rb += read(fd, a->trans, a->states * a->alpha * sizeof(*a->trans));

	return rb;
}

ssize_t dfa_write(int fd, const dfa_t * a) {
	assert(a->trans != NULL);
	ssize_t wb = 0;

	wb += write(fd, &a->states, sizeof(a->states));
	wb += write(fd, &a->init, sizeof(a->init));
	wb += write(fd, &a->alpha, sizeof(a->alpha));
	wb += write(fd, a->trans, a->states * a->alpha * sizeof(*a->trans));

	return wb;
}

