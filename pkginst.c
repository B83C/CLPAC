#include "helper.h"

int pkgfd;
int rootfd;

struct linux_dirent64 {
    ino64_t	    d_ino;
    off64_t	    d_off;
    unsigned short  d_reclen;
    unsigned char   d_type;
    char	    d_name[];
};

struct existing_path {
    int	    fd;
    char    path[256];
} ep_path;

#define EPSTACK_SIZE 256
struct ep_path epstack[EPSTACK_SIZE];
struct ep_path* ephead = epstack;
struct ep_path* eptail = ephead;

#define EP_PUSH(x,y) ({ \
	eptail->fd = x; eptail->path = y; \
	eptail = (eptail + 1) & (EPSTACK_SIZE - 1); \
	})    
#define EP_POP(x,y) ({ \
	x = ephead->fd; y = ephead->path; \
	ephead = (ephead + 1) & (EPSTACK_SIZE - 1); \
	})

static inline
void usage(char* path) {
    printf("CLPAC Package Installer \n"
	    "Usage: %s package_name [package_version] \n"
	    "	-h --help to display this message \n"
	    , path);
    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
    check_root();

    if(argc <= 1) 
	usage(*argv);

    for(int i = 1; i < argc; i++) {
	register uint16_t res = *((uint16_t*)(*(argv + i))) ^ 0x2D2D;

	if(res == 0)
	{
	}
	else if((uint8_t)res == 0)
	{
	    switch(*(*(argv + i) + 1))
	    {
		case 'h':
		    usage(*argv);
		    break;
		default:
		    usage(*argv);
		    break;
	    }
	}
    }

    char path[PATH_MAX];

    register char* tmp = mempcpy(path, PKG_ROOT, strlen(PKG_ROOT));
    *tmp = '/';
    tmp = mempcpy(tmp + 1, *(argv + 1), strlen(*(argv + 1)));
    if(argc > 2) {
	if(tmp - path + strlen(*(argv + 2)) > PATH_MAX) 
	    EXIT_ERROR(ERR_BUF);
	*tmp = '/';
	tmp = mempcpy(tmp + 1, *(argv + 2), strlen(*(argv + 2)));
    }
    *tmp = 0;

    rootfd = open("/", O_PATH);
    if(rootfd < 0)
    {
	fprintf(stderr, "Unable to open /\n");
	exit(EXIT_FAILURE);
    }

    pkgfd = open(path, O_PATH);
    if(pkgfd < 0) {
	fprintf(stderr, "%s does not exist!\n", path);
	exit(EXIT_FAILURE);
    }

    char* pkgdent = calloc(DIRENT_MAX_BUF, 1);
    if(!pkgdent)
	EXIT_ERROR(ERR_NOBUF);

    do {
	register int bufcnt = 0;
	while((bufcnt = getdents64(pkgfd, pkgdent, DIRENT_MAX_BUF))) {
	    
	}
    } while(ephead != eptail);





    return 0;
}
