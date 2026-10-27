#ifndef _PROJECT_H_INCLUDED
#define _PROJECT_H_INCLUDED

#include "files.h"

typedef struct {
  filename_t ccompiler;
  filename_t cppcompiler;
  filename_t linker;
  filename_t project_name;
} project_config_t;

project_config_t project_config_from(pathname_t pathname);

#endif
