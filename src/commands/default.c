#include "commands/default.h"
#include "files.h"
#include <getopt.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

typedef struct option Option;

int execute_config_default(config_default_t config_default) {
	if (config_default.version) {
		printf("cpf (C Programmer's Friend) version 0.1\n");
	}
	// Finding default project files in CPF resources
	if (config_default.help) {
		char filename[PATH_MAX];
		FILE *help_file = fopen(file_path_concat(filename, get_executable_dir(),
												 DEFAULT_HELP_PAGE_PATH),
								"r");
		if (help_file == NULL) {
			perror("execute_config_default.fopen");
			return -1;
		}
		file_text_copy(stdout, help_file);
		fclose(help_file);
	}
	return 0;
}

config_default_t config_default_of_args(int argc, char *argv[]) {
	config_default_t config_default = {.version = false, .help = false};
	static Option long_options[] = {{"version", no_argument, NULL, 'v'},
									{"help", no_argument, NULL, 'h'},
									{NULL, 0, NULL, 0}};
	int c;
	int option_index = 0;
	while ((c = getopt_long(argc, argv, "vh", long_options, &option_index)) != -1) {
		switch (c) {
		case 'v':
			config_default.version = true;
			break;
		case 'h':
			config_default.help = true;
			break;
		case '?': // unknown option
			break;
		default: // parsing error
			break;
		}
	}
	return config_default;
}
