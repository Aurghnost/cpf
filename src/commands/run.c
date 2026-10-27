#include "commands/run.h"
#include "commands/build.h"
#include "files.h"
#include <getopt.h>
#include <limits.h>
#include <string.h>

int execute_config_run(config_run_t *config_run) {
	// Finding default project files in CPF resources
	if (config_run->help) {
		char filename[PATH_MAX];
		FILE *help_file = fopen(
				file_path_concat(filename, get_executable_dir(), RUN_HELP_PAGE_PATH),
				"r");
		if (help_file == NULL) {
			perror("execute_config_run.fopen");
			return -1;
		}
		file_text_copy(stdout, help_file);
		fclose(help_file);
		return 0;
	}
	if (config_run->build) {
		config_build_t config_build = {
			.project_config = config_run->project_config,
			.quiet = config_run->quiet,
			.run = false,
			.help = false,
			.release = false,
			.docs = false,
			.dynlib = false,
			.staticlib = false,
			.tests = false
		};
		execute_config_build(&config_build);
	}
	switch (config_run->type) {
	case RUN_EXECUTABLE:
		break;
	case RUN_BENCHMARKS:
		break;
	case RUN_TESTS:
		break;
	default:
		break;
	}
	return 0;
}

typedef struct option Option;

config_run_t config_run_of_args(int argc, char *argv[]) {
	config_run_t run_config;
	static Option long_options[] = {{"build", no_argument, NULL, 'b'},
									{"quiet", no_argument, NULL, 'q'},
									{"help", no_argument, NULL, 'h'},
									{"tests", no_argument, NULL, 'T'},
									{"executable", no_argument, NULL, 'X'},
									{"benchmarks", no_argument, NULL, 'B'},
									{NULL, 0, NULL, 0}};
	int c;
	int option_index = 0;
	while ((c = getopt_long(argc, argv, "qhbTX", long_options, &option_index)) !=
				 -1) {
		switch (c) {
		case 'q':
			run_config.quiet = true;
			break;
		case 'h':
			run_config.help = true;
			break;
		case 'B':
			run_config.type = RUN_BENCHMARKS;
			break;
		case 'T':
			run_config.type = RUN_TESTS;
			break;
		case 'X':
			run_config.type = RUN_EXECUTABLE;
			break;
		case '?':
			break;
		default:
			break;
		}
	}
	run_config.argv = argv + optind;
	run_config.argc = argc - optind;
	if (run_config.argc <= 0) {
		run_config.argc = 0;
		run_config.argv = NULL;
	}
	return run_config;
}
