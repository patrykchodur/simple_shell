#ifndef LOW_LEVEL_H
#define LOW_LEVEL_H

#include "simple_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

int execute(const char* path, char* argv[], Fdsimple* fd, Nfsimple* nf);
void set_pipe_for_child(int arg[2]);

int wait_for_process(int pid);

void open_pipe(int arg[2]);
void close_fd(int fd);

const char* get_wd(void);
const char* get_hd(void);
const char* get_login(void);

const char* get_env(const char* name);
int set_env(const char* name, const char* value);


#ifdef __cplusplus
}
#endif

#endif
