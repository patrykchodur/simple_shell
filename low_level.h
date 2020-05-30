#ifndef LOW_LEVEL_H
#define LOW_LEVEL_H

#ifdef __cplusplus
extern "C" {
#endif

int execute(const char* path, char* argv[]);

int wait_for_process(int pid);

const char* get_wd(void);
const char* get_hd(void);
const char* get_login(void);


#ifdef __cplusplus
}
#endif

#endif
