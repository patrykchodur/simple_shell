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

#include <fcntl.h>

int max_fork_count = 100;

const char* built_in_names[] = {
	"cd",
	"exit",
	"set",
};

int (*built_in_functions[])(const char(*)[]) = {
	built_in_cd,
	built_in_exit,
	built_in_set,
};

static int child_pipe[2];

static void set_redirections(Fdsimple* fd, Nfsimple* nf) {
	int fd_count = *(int*)fd;
	int nf_count = *(int*)nf;


	for (int iter = 0; iter < fd_count; ++iter) {
		dup2(fd[iter + 1].to, fd[iter + 1].from);
		fprintf(stderr, "from: %d to: %d", fd[iter + 1].from, fd[iter + 1].to);
	}

	for (int iter = 0; iter < nf_count; ++iter) {
		int result = 0;
		if (nf[iter + 1].append)
			result = open(nf[iter + 1].to, O_WRONLY | O_APPEND | O_CREAT);
		else
			result = open(nf[iter + 1].to, O_RDWR | O_CREAT);
		dup2(result, nf[iter + 1].from );
		// fprintf(stderr, "from: %d result: %d, file: %s\n, append: %d", nf[iter + 1].from, result, nf[iter + 1].to, nf[iter + 1].append);
	}
}

int execute(const char* path, char* argv[], Fdsimple* fd, Nfsimple* nf) {
	assert(--max_fork_count > 0);

	for (int iter = 0; iter < sizeof(built_in_names) / sizeof(built_in_names[0]); ++iter) {
		if (!strcmp(built_in_names[iter], path)) {
			int result = built_in_functions[iter](argv);
			char result_buf[20];
			sprintf(result_buf, "%d", result);
			set_env("?", result_buf);
			return 0;
		}
	}
	int pid = fork();
	if (pid == 0) {
		set_redirections(fd, nf);
		if (child_pipe[0]) {
			dup2(child_pipe[0], 0);
		}
		if (child_pipe[1]) {
			dup2(child_pipe[1], 1);
		}
		execvp(path, argv);
		fprintf(stderr, "Error: program %s not found\n", path);
		exit(-1);
	}
	else if (pid > 0) {
		if (child_pipe[0]) {
			close(child_pipe[0]);
		}
		if (child_pipe[1]) {
			close(child_pipe[1]);
		}
		return pid;
	}
	else {
		fprintf(stderr, "Error: fork unsuccessfull\n");
	}
}
void set_pipe_for_child(int arg[2]) {
	child_pipe[0] = arg[0];
	child_pipe[1] = arg[1];
}

int wait_for_process(int pid) {
	int result;
	waitpid(pid, &result, 0);
	result = WEXITSTATUS(result);

	char result_buf[20];
	sprintf(result_buf, "%d", result);
	set_env("?", result_buf);
	return result;
}

void open_pipe(int arg[2]) {
	pipe(arg);
}

void close_fd(int fd) {
	close(fd);
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

const char* get_env(const char* name) {
	return getenv(name);
}

int set_env(const char* name, const char* value) {
	return setenv(name, value, 1);

}
