#ifndef _COMMANDS_RUN_H_INCLUDED
#define _COMMANDS_RUN_H_INCLUDED

#include "project.h"
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#define RUN_HELP_PAGE_PATH "resources/help/run.txt"

//! Type of project run
typedef enum : uint8_t { RUN_EXECUTABLE, RUN_TESTS, RUN_BENCHMARKS } run_t;

//! Configuration structure for the "run" command
typedef struct config_run_t {
  project_config_t project_config;
  bool quiet; //! true means no terminal output
  bool help;  //! true to display help message (ignore the command effect if so)
  bool build; //! true to rebuild the sources before running
  run_t type; //! The type of binaries to run : executable, tests or benchmarks
  char **argv; //! arguments passed to the executable
  int argc;    //! length of argv
} config_run_t;

//! Load a run command configuration from command line args
config_run_t config_run_of_args(int argc, char *argv[]);

//! Execute a run command with given configuration
int execute_config_run(config_run_t *config_run);

#endif
