#ifndef _BUILD_H_INCLUDED
#define _BUILD_H_INCLUDED

#include <getopt.h>
#include <stdbool.h>
#include <sys/types.h>

#include "project.h"
#define BUILD_HELP_PAGE_PATH "resources/help/build.txt"

typedef struct build_log_t {
  bool release;
} build_log_t;

build_log_t build_log_from(const char *pathname);

//! Configuration structure for the "build" command
typedef struct config_build_t {
  project_config_t project_config;
  bool quiet;     //! No display on stdout
  bool all;       //! Compile every C file
  bool help;      //! Display help page and quit
  bool run;       //! Run the executable after build
  bool release;   //! Compile in release mode
  bool staticlib; //! Compile the static library
  bool dynlib;    //! Compile the dynamic library
  bool docs;      //! Compile the documentation
  bool tests;     //! Compile the tests
} config_build_t;

//! Load build command configuration from command line args
config_build_t config_build_of_args(int argc, char *argv[]);

//! Execute a build command with given configuration
int execute_config_build(config_build_t *config_build);

#endif // !_BUILD_H_INCLUDED
