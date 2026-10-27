#ifndef _HASHFUNCTION_H_INCLUDED
#define _HASHFUNCTION_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>

#define _HASH_P 4200122639 // why not, some big primary number

typedef struct hashfunction_s {
	uint64_t a; //! @inv a < P and a != 0
	uint64_t b; //! @inv b < P
} hashfunction_t;

static inline hashfunction_t hashfunction_rand_new() {
  return (hashfunction_t){.a = (rand() % (_HASH_P - 1)) + 1, .b = rand() % _HASH_P};
}
#endif
