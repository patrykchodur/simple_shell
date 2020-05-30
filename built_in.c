#include "built_in.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "low_level.h"

int built_in_cd(const char* argv[]) {
	int result;
	if (!argv[1])
		result = chdir(get_hd());
	else if (argv[1][0] == '~') {
		const char* home = get_hd();

		int tmp_size = strlen(argv[1]);
		tmp_size += strlen(home);

		char* path = malloc(tmp_size * sizeof(char));

		strcpy(stpcpy(path, home), argv[1] + 1);
		result = chdir(path);
		free(path);
	}
	else {
		result = chdir(argv[1]);
	}
	if (result)
		fprintf(stderr, "Error: directory %s not found\n", argv[1]);
	else
		result = set_env("PWD", get_wd());
	return result;
}

int built_in_exit(const char* argv[]) {
	if (!argv[1])
		exit(0);
	exit(atoi(argv[1]));
}

int built_in_set(const char* argv[]) {
	if (!argv[1]) {
		fprintf(stderr, "Error: set requires variable");
		return -1;
	}
	const char* equals = strchr(argv[1], '=');
	if (!equals) {
		fprintf(stderr, "Error: wrong variable format for set");
		return -1;
	}

	char* var_name = malloc(equals - argv[1] + 1);
	strncpy(var_name, argv[1], equals - argv[1]);
	var_name[equals - argv[1]] = 0;

	int result = set_env(var_name, equals + 1);

	free(var_name);
	return result;
}
