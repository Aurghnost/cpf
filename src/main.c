#include <stdlib.h>

#include "config.h"
#include "execute.h"

int main(int argc, char *argv[]) {
	config_t config = config_of_args(argc, argv);
	execute_config(config);
	return EXIT_SUCCESS;
}

