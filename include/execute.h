#ifndef _EXECUTE_H_INCLUDED
#define _EXECUTE_H_INCLUDED

#include "config.h"

#include "commands/build.h"
#include "commands/default.h"
#include "commands/new.h"

//! Execute a command line configuration
int execute_config(config_t config);

#endif
