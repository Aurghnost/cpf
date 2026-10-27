#include "commands/new.h"
#include "files.h"

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int gen_makefile(char *resources_dir, config_new_t *config_new) {
	char origin_pathname[PATH_MAX];
	file_path_concat(origin_pathname, resources_dir, "Makefile");
	printf("%s\n", origin_pathname);
	FILE *origin = fopen(origin_pathname, "r");
	if (origin == NULL) {
		perror("gen_makefile.fopen[origin]");
		return -1;
	}
	FILE *dest = fopen("Makefile", "w");

	fprintf(dest, "$(VERBOSE).SILENT:\n");
	fprintf(dest, "SRC_DIR=%s\n", config_new->src_dir);
	fprintf(dest, "INC_DIR=%s\n", config_new->include_dir);
	fprintf(dest, "TEST_DIR=%s\n", config_new->tests_dir);
	fprintf(dest, "LD_DIR=%s\n", config_new->ld_dir);
	fprintf(dest, "LIB_DIR=%s\n", config_new->lib_dir);
	fprintf(dest, "ELF_NAME=%s\n", config_new->project_name);
	fprintf(dest, "TEMPLATE_DIR=%s\n", config_new->template_dir);
	fprintf(dest, "DOCS_DIR=%s\n", config_new->docs_dir);
	fprintf(dest, "LIB_NAME=lib%s\n", config_new->project_name);

	file_text_copy(dest, origin);
	fclose(origin);
	fclose(dest);
	return 0;
}

void gen_base_sources(char *resources_dir, config_new_t *config_new) {
	char origin[PATH_MAX];
	char dest[PATH_MAX];
	// unittest.h
	file_path_concat(origin, resources_dir, "unittest.h");
	file_path_concat(dest, config_new->include_dir, "unittest.h");
	file_text_copy_from_pathnames(dest, origin);
	// main.c
	file_path_concat(origin, resources_dir, "main.c");
	file_path_concat(dest, config_new->src_dir, "main.c");
	file_text_copy_from_pathnames(dest, origin);
	// link.template.ld
	file_path_concat(origin, resources_dir, "link.template.ld");
	file_path_concat(dest, config_new->ld_dir, "link.template.ld");
	file_text_copy_from_pathnames(dest, origin);
	// example.test.c
	file_path_concat(origin, resources_dir, "example.test.c");
	file_path_concat(dest, config_new->tests_dir, "example.test.c");
	file_text_copy_from_pathnames(dest, origin);
}

int execute_config_new(config_new_t *config_new) {
	// Finding default project files in CPF resources
	if (config_new->help) {
		char filename[PATH_MAX];
		FILE *help_file = fopen(
				file_path_concat(filename, get_executable_dir(), NEW_HELP_PAGE_PATH),
				"r");
		if (help_file == NULL) {
			perror("execute_config_default.fopen");
			return -1;
		}
		file_text_copy(stdout, help_file);
		fclose(help_file);
		return 0;
	}
	// Making project directory
	if (mkdir(config_new->project_name, NEW_DEFAULT_RIGHTS) && errno != EEXIST) {
		perror("execute_config_new.mkdir");
		return -1;
	}

	chdir(config_new->project_name);

	// Making project subdirectories
	mkdir(config_new->res_dir, NEW_DEFAULT_RIGHTS);
	mkdir(config_new->include_dir, NEW_DEFAULT_RIGHTS);
	mkdir(config_new->template_dir, NEW_DEFAULT_RIGHTS);
	mkdir(config_new->docs_dir, NEW_DEFAULT_RIGHTS);
	mkdir(config_new->src_dir, NEW_DEFAULT_RIGHTS);
	mkdir(config_new->lib_dir, NEW_DEFAULT_RIGHTS);
	mkdir(config_new->ld_dir, NEW_DEFAULT_RIGHTS);
	mkdir(config_new->tests_dir, NEW_DEFAULT_RIGHTS);

	// Finding default project files in CPF resources
	char resources_dir[PATH_MAX];
	file_path_concat(resources_dir, get_executable_dir(), "resources");

	// Copy of default project files in directory :
	gen_makefile(resources_dir, config_new);
	gen_base_sources(resources_dir, config_new);
	return 0;
}

typedef struct option Option;

typedef enum {
	OPT_PNAME,
	OPT_INC_DIR,
	OPT_SRC_DIR,
	OPT_TESTS_DIR,
	OPT_LD_DIR,
	OPT_TP_DIR,
	OPT_DOCS_DIR,
	OPT_RES_DIR,
	OPT_LIB_DIR
} option_id;

config_new_t config_new_of_args(int argc, char *argv[]) {
	// default values :
	config_new_t config_new = {.project_name = CONFIG_NEW_DEFAULT_PNAME,
														 .include_dir = CONFIG_NEW_DEFAULT_INC_DIR,
														 .src_dir = CONFIG_NEW_DEFAULT_SRC_DIR,
														 .res_dir = CONFIG_NEW_DEFAULT_RES_DIR,
														 .tests_dir = CONFIG_NEW_DEFAULT_TESTS_DIR,
														 .ld_dir = CONFIG_NEW_DEFAULT_LD_DIR,
														 .lib_dir = CONFIG_NEW_DEFAULT_LIB_DIR,
														 .template_dir = CONFIG_NEW_DEFAULT_TP_DIR,
														 .docs_dir = CONFIG_NEW_DEFAULT_DOCS_DIR,
														 .lang = C,
														 .help = false};
	static Option long_options[] = {
			{"project-name", required_argument, NULL, OPT_PNAME},
			{"include-dir", required_argument, NULL, OPT_INC_DIR},
			{"sources-dir", required_argument, NULL, OPT_SRC_DIR},
			{"tests-dir", required_argument, NULL, OPT_TESTS_DIR},
			{"linker-scripts-dir", required_argument, NULL, OPT_LD_DIR},
			{"templates-dir", required_argument, NULL, OPT_TP_DIR},
			{"docs-dir", required_argument, NULL, OPT_DOCS_DIR},
			{"resources-dir", required_argument, NULL, OPT_RES_DIR},
			{"libraries-dir", required_argument, NULL, OPT_LIB_DIR},
			{"lang", required_argument, NULL, 'l'},
			{"help", no_argument, NULL, 'h'},
			{NULL, 0, NULL, 0}};
	int c;
	int option_index = 0;
	while ((c = getopt_long(argc, argv, "hl:", long_options, &option_index)) !=
				 -1) {
		switch (c) {
		case OPT_PNAME:
			strncpy(config_new.project_name, optarg, sizeof(filename_t));
			break;
		case OPT_LIB_DIR:
			strncpy(config_new.lib_dir, optarg, sizeof(filename_t));
			break;
		case OPT_INC_DIR:
			strncpy(config_new.include_dir, optarg, sizeof(filename_t));
			break;
		case OPT_SRC_DIR:
			strncpy(config_new.src_dir, optarg, sizeof(filename_t));
			break;
		case OPT_TESTS_DIR:
			strncpy(config_new.tests_dir, optarg, sizeof(filename_t));
			break;
		case OPT_LD_DIR:
			strncpy(config_new.ld_dir, optarg, sizeof(filename_t));
			break;
		case OPT_TP_DIR:
			strncpy(config_new.template_dir, optarg, sizeof(filename_t));
			break;
		case OPT_DOCS_DIR:
			strncpy(config_new.docs_dir, optarg, sizeof(filename_t));
			break;
		case OPT_RES_DIR:
			strncpy(config_new.res_dir, optarg, sizeof(filename_t));
			break;
		case 'h':
			config_new.help = true;
			break;
		// case 'l':
		//	 // TODO
		//	 break;
		case '?': // Unknown option
			break;
		default: // parsing error
			break;
		}
	}
	return config_new;
}
