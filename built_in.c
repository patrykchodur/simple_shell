#include "built_in.h"

#include <unistd.h>
#include <stdio.h>

#include "low_level.h"

int built_in_cd(const char* argv[]) {
	int result;
	if (!argv[1])
		result = chdir(get_hd());
	else
		result = chdir(argv[1]);
	if (result)
		fprintf(stderr, "Error: directory %s not found\n", argv[1]);
}

int built_in_exit(const char* argv[]) {
	if (!argv[1])
		exit(0);
	exit(atoi(argv[1]));
}
