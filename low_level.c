#include "low_level.h"
#include "built_in.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <sys/types.h>
#include <pwd.h>
#include <uuid/uuid.h>

int max_fork_count = 100;

const char* built_in_names[] = {
	"cd",
	"exit",
};

int (*built_in_functions[])(const char(*)[]) = {
	built_in_cd,
	built_in_exit,
};

int execute(const char* path, char* argv[]) {
	assert(--max_fork_count > 0);

	for (int iter = 0; iter < sizeof(built_in_names) / sizeof(built_in_names[0]); ++iter) {
		if (!strcmp(built_in_names[iter], path))
			return built_in_functions[iter](argv);
	}
	int pid = fork();
	if (pid == 0) {
		execvp(path, argv);
		fprintf(stderr, "Error: program %s not found\n", path);
		exit(-1);
	}
	else if (pid > 0) {
		return pid;
	}
	else {
		fprintf(stderr, "Error: fork unsuccessfull\n");
	}
}

int wait_for_process(int pid) {
	int result;
	waitpid(pid, &result, 0);
	return WEXITSTATUS(result);
}

static char* wd_path;

static int get_wd_size(void) {
	return 4096;
}

static void free_wd_path(void) {
	free(wd_path);
}

const char* get_wd(void) {
	if (!wd_path) {
		wd_path = malloc(get_wd_size());
		atexit(free_wd_path);
	}
	getcwd(wd_path, get_wd_size());
	return wd_path;
}

const char* get_hd(void) {
	return getpwuid(getuid())->pw_dir;
}

const char* get_login(void) {
	return getpwuid(getuid())->pw_name;
}
