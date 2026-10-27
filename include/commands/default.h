#ifndef _COMMANDS_DEFAULT_H_INCLUDED
#define _COMMANDS_DEFAULT_H_INCLUDED

#include <getopt.h>
#include <stdbool.h>

typedef struct config_default_t {
  bool version;
  bool help;
} config_default_t;

#define DEFAULT_HELP_PAGE_PATH "resources/help/default.txt"

//! Load default command configuration from command line args
config_default_t config_default_of_args(int argc, char *argv[]);

//! Execute a default command with given configuration
int execute_config_default(config_default_t config_default);

#endif
