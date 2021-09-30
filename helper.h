#include "config.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fcntl.h>
#include <dirent.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include <errno.h>

#define PATH_MAX 256
#define not_root() ({ \
    getuid() | geteuid(); \
})

#define check_root() ({ \
    if(not_root() != 0) { \
	printf("Not running as root, quitting!\n"); \
	return 1; \
    }\
})

#ifdef DEBUG 
#define DEBUGI(fmt, ...) \
	printf(fmt, __VA_ARGS__) 
#else
#define DEBUGI(fmt, ...)
#endif

#define ERR_BUF 0
#define ERR_NOBUF 1
char* err[] = {"Buffer overflow detected!\n", "Buffer allocation failed\n"};
#define EXIT_ERROR(x) ({ \
	fprintf(stderr, err[x]); \
	exit(EXIT_FAILURE); \
})

int pr_fd;

static inline
void init_fd() {
    if((pr_fd = open(PKG_ROOT, O_PATH)) == -1) {
	close(pr_fd);
	perror(PKG_ROOT" not found");
	exit(1);
    }
    
}

    
