#include "files.h"
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char *file_path_concat(char *buffer, pathname_t base, pathname_t filepath) {
	strncpy(buffer, base, PATH_MAX);
	strncat(buffer, "/", 2);
	strncat(buffer, filepath, PATH_MAX);
	return buffer;
}

int file_text_copy(FILE *out, FILE *in) {
	char buffer[1024];
	while (fgets(buffer, sizeof(buffer), in) != NULL) {
		fprintf(out, "%s", buffer);
	}
	if (fflush(out)) {
		perror("file_text_copy.fflush");
		return -1;
	}
	return 0;
}

int file_text_copy_from_pathnames(pathname_t dest_path, pathname_t src_path) {
	FILE *out = fopen(dest_path, "w");
	if (out == NULL) {
		perror("file_text_copy.fopen[out]");
		return -1;
	}

	FILE *in = fopen(src_path, "r");
	if (in == NULL) {
		perror("file_text_copy.fopen[in]");
		fclose(out);
		return -1;
	}

	int status = file_text_copy(out, in);

	if (fclose(out) || fclose(in)) {
		perror("file_text_copy.fclose");
	}
	return status;
}

char *get_executable_path() {
	static char exe_path[PATH_MAX] = {0};
	if (exe_path[0] == 0) {
		ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
		if (len == -1) {
			perror("get_executable_path.readlink");
			return NULL;
		}
		exe_path[len] = '\0';
	}
	return exe_path;
}

char *get_executable_dir() {
#ifdef NDEBUG
	static char exe_dir[PATH_MAX] = {0};
	if (exe_dir[0] == 0) {
		char *exe_path = get_executable_path();
		if (exe_path == NULL) {
			return NULL;
		}
		size_t len = strlen(exe_path);
		while (len != 0 && exe_path[--len] != '/')
			;
		if (len == 0) {
			return NULL;
		}
		strncpy(exe_dir, exe_path, len);
		exe_dir[len] = '\0';
	}
	return exe_dir;
#else
	return __PROJECT_DIRECTORY__;
#endif
}
