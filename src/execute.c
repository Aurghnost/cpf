#include "execute.h"
#include "commands/new.h"
#include "config.h"

int execute_config(config_t config) {
	switch (config.type) {
	case A_DEFAULT:
		return execute_config_default(config.data.d);
	case A_BUILD:
		return execute_config_build(&config.data.b);
	case A_NEW:
		return execute_config_new(&config.data.n);
	case A_RUN:
		return execute_config_run(&config.data.r);
	default:
		return -1;
	}
}
