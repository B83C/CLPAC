#include "config.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

#define ERR_BUF 0
char* err[] = {"Buffer overflow detected!\n!"};
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

    
