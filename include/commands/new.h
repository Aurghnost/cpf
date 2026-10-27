#ifndef _COMMANDS_NEW_INCLUDED
#define _COMMANDS_NEW_INCLUDED

#include <stdbool.h>
#include <sys/stat.h>

#include "files.h"
#include "types.h"

#define NEW_DEFAULT_RIGHTS S_IRWXU
#define NEW_HELP_PAGE_PATH "resources/help/new.txt"

//! Configuration structure for the new project command
typedef struct config_new_t {
  filename_t project_name;
  filename_t include_dir;
  filename_t src_dir;
  filename_t tests_dir;
  filename_t ld_dir;
  filename_t lib_dir;
  filename_t template_dir;
  filename_t docs_dir;
  filename_t res_dir;
  lang_t lang;
  bool help;
} config_new_t;

#define CONFIG_NEW_DEFAULT_PNAME "default"
#define CONFIG_NEW_DEFAULT_INC_DIR "include"
#define CONFIG_NEW_DEFAULT_SRC_DIR "src"
#define CONFIG_NEW_DEFAULT_TESTS_DIR "test"
#define CONFIG_NEW_DEFAULT_LD_DIR "ldscripts"
#define CONFIG_NEW_DEFAULT_LIB_DIR "lib"
#define CONFIG_NEW_DEFAULT_TP_DIR "templates"
#define CONFIG_NEW_DEFAULT_DOCS_DIR "docs"
#define CONFIG_NEW_DEFAULT_RES_DIR "resources"

//! Load a new project command configuration from command line args
config_new_t config_new_of_args(int argc, char *argv[]);

//! Execute a new project command with given configuration
int execute_config_new(config_new_t *config_new);

#endif // !_COMMANDS_NEW_INCLUDED
