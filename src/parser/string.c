/**
 * Structure de chaîne de caractères réalisée par Tableau Dynamique
 */

#include "parser/string.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_GAP_SIZE 5

string_t string_new() {
    return (string_t){
	.buf = calloc(DEFAULT_GAP_SIZE + 1, sizeof(char)),
	.capacity = DEFAULT_GAP_SIZE,
	.length = 0
    };
}

void string_write_c(string_t *s, char c) {
    if (s->length == s->capacity) {
	unsigned new_capa = s->capacity << 1;
	s->buf = realloc(s->buf, 1 + new_capa);
	memset(s->buf + s->capacity, 0, s->capacity);
	s->capacity = new_capa;
    }
    s->buf[s->length++] = c;
}

char string_get(const string_t *s, unsigned i) {
    assert(i < s->length);
    return s->buf[i];
}

void string_destroy(string_t *s) {
    free(s->buf);
    s->buf = NULL;
}

string_t string_dup_cstring(const char *s) {
	unsigned n = (unsigned)strlen(s);
	return (string_t){
		.buf = strdup(s),
		.length = n,
		.capacity = n,
	};
}
