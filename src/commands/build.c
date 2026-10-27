#include "commands/build.h"
#include "cmd.h"
#include "files.h"
#include "project.h"
#include <dirent.h>
#include <getopt.h>
#include <limits.h>
#include <linux/limits.h>
#include <string.h>

int execute_config_build(config_build_t *config_build) {
    // Finding default project files in CPF resources
    if (config_build->help) {
		char filename[PATH_MAX];
		FILE *help_file = fopen(
			file_path_concat(filename, get_executable_dir(), BUILD_HELP_PAGE_PATH),
			"r");
		if (help_file == NULL) {
			perror("execute_config_default.fopen");
			return -1;
		}
		file_text_copy(stdout, help_file);
		fclose(help_file);
		return 0;
    }

    // build_log_t build_log = build_log_from("_build/log.json");
    // if (config_build->all || build_log.release != config_build->release) {
    //   cmd_run((char *const[]){"make", "clean"}, config_build->quiet);
    // }
    
    project_config_t pc = project_config_from("");
    char ccompiler[PATH_MAX];
    char cppcompiler[PATH_MAX];
    char linker[PATH_MAX];
    snprintf(ccompiler, PATH_MAX, "CC=%s", pc.ccompiler);
    snprintf(cppcompiler, PATH_MAX, "CPPC=%s", pc.cppcompiler);
    snprintf(linker, PATH_MAX, "LINKER=%s", pc.linker);
    if (config_build->release) {
	cmd_run((char *const[]) {
		"make",
		"BUILD=release",
		ccompiler,
		cppcompiler,
		linker,
		NULL
	}, config_build->quiet);
    } else {
	cmd_run((char *const[]) {
	      "make",
	      ccompiler,
	      cppcompiler,
	      linker, 
	      NULL
	}, config_build->quiet);
    }
    return 0;
}

typedef struct option Option;

config_build_t config_build_of_args(int argc, char *argv[]) {
    config_build_t build_config;
    static Option long_options[] = {{"all", no_argument, NULL, 'a'},
                                    {"release", no_argument, NULL, 'r'},
                                    {"quiet", no_argument, NULL, 'q'},
                                    {"help", no_argument, NULL, 'h'},
                                    {"staticlib", no_argument, NULL, 'L'},
                                    {"dynlib", no_argument, NULL, 'l'},
                                    {"docs", no_argument, NULL, 'd'},
                                    {"tests", no_argument, NULL, 't'},
                                    {"examples", no_argument, NULL, 'e'},
                                    {"config-file", required_argument, NULL, 'C'},
                                    {NULL, 0, NULL, 0}};

    int c;
    int option_index = 0;
    while ((c = getopt_long(argc, argv, "arqhlLdteC:", long_options,
                            &option_index)) != -1) {
	switch (c) {
	case 'a':
	    build_config.all = true;
	    break;
	case 'r':
	    build_config.release = true;
	    break;
	case 'q':
	    build_config.quiet = true;
	    break;
	case 'h':
	    build_config.help = true;
	    break;
	case 'L':
	    build_config.staticlib = true;
	    break;
	case 'l':
	    build_config.dynlib = true;
	    break;
	case 'd':
	    build_config.docs = true;
	    break;
	case 't':
	    build_config.tests = true;
	    break;
	case '?':
	default:
	    break;
	}
    }
    return build_config;
}
