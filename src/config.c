#include "config.h"

config_t config_of_args(int argc, char *argv[]) {
	// assure qu'une commande est bien donnée :
	if (argc <= 1) {
		return (config_t){
			.type = A_DEFAULT,
			.data.d = {.version = false, .help = true}
		};
	}
	// on considère que le programme est la commande :
	argc--;
	argv++;

	// choix du parser et chargement de la configuration d'exécution
	config_t config;
	if (!strcmp(argv[0], "new")) {
		config.type = A_NEW;
		config.data.n = config_new_of_args(argc, argv);
	} else if (!strcmp(argv[0], "build")) {
		config.type = A_BUILD;
		config.data.b = config_build_of_args(argc, argv);
	} else if (!strcmp(argv[0], "run")) {
		config.type = A_RUN;
		config.data.r = config_run_of_args(argc, argv);
	} else {
		config.type = A_DEFAULT;
		config.data.d = config_default_of_args(++argc, --argv);
	}
	return config;
}
