#ifndef _STRING_H_INCLUDED
#define _STRING_H_INCLUDED

typedef struct string_s string_t;

struct string_s {
  char *buf;
  unsigned length;
  unsigned capacity;
};


/**
 * Renvoie une chaîne de caractères vide.
 */
string_t string_new();

/**
 * 
 */
string_t string_dup_cstring(const char *s);

/**
 * Écrit un caractère 
 */
void string_write_c(string_t *s, char c);

/**
 * Renvoie le caractère d'indice i de la chaîne
 */
char string_get(const string_t *s, unsigned i);

/**
 * Libère la mémoire allouée à la chaîne *s
 * @param[in, out] s Pointeur vers une chaîne de caractères
 */
void string_destroy(string_t *s);

#endif // !_STRING_H_INCLUDED
