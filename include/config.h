#ifndef _CONFIG_H_INCLUDED
#define _CONFIG_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "commands/build.h"
#include "commands/default.h"
#include "commands/new.h"
#include "commands/run.h"

//! Type of command line action to execute
typedef enum : uint8_t { A_DEFAULT, A_BUILD, A_NEW, A_RUN } action_t;

//! Configuration of the command line action to execute
typedef struct config_t {
  union {
    config_default_t d;
    config_new_t n;
    config_build_t b;
    config_run_t r;
  } data;
  action_t type;
} config_t;

//! Loads the configuration from command line args
config_t config_of_args(int argc, char *argv[]);

#endif
