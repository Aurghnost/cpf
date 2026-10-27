#include "cmd.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int cmd_run(char *const commandline[], bool quiet) {
	int status = -1;
	int stdout_back = dup(1);
	if (quiet) {
		fflush(stdout);
		int std_tmp = open("/dev/null", O_WRONLY);
		dup2(std_tmp, 1);
		close(std_tmp); // not needed anymore
	}
	pid_t child = fork();
	switch (child) {
	case -1:
		perror("fork");
		exit(EXIT_FAILURE);
	case 0:

		if (-1 == execvp(commandline[0], commandline)) {
		perror("execvp");
		exit(EXIT_FAILURE);
		}
		break;
	default:
		waitpid(child, &status, 0);
		break;
	}
	if (quiet) {
		fflush(stdout);
		dup2(stdout_back, 1);
		close(stdout_back);
	}
	return status;
}
