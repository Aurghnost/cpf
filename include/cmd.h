#ifndef _CMD_H_INCLUDED
#define _CMD_H_INCLUDED

#include <stdbool.h>

// argv must contains :
// - at index 0 : pathname
// - at last index : NULL
int cmd_run(char *const commandline[], bool quiet);

#endif
