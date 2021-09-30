#include "helper.h"

int pkgfd;
int rootfd;

struct linux_dirent64 {
    uint64_t	    d_ino;
    uint64_t	    d_off;
    unsigned short  d_reclen;
    unsigned char   d_type;
    char	    d_name[];
};

struct ep_path {
    char    len;
    char    path[PATH_MAX - 1];
}; 

//powerf of 2 please
#define EP_RINGBUF_SIZE 512
struct ep_path eprb[EP_RINGBUF_SIZE];
int ephead_off, eptail_off = 0;

#define EP_PUSH(x, n) ({ \
	struct ep_path* eptail = eprb + eptail_off; \
	memcpy(eptail->path, x, n); \
	*(eptail->path + n + 1) = 0; \
	eptail->len = n; \
	eptail_off = (eptail_off + 1) & (EP_RINGBUF_SIZE - 1); \
	})    
#define EP_POP(x, n) ({ \
	struct ep_path* ephead = eprb + ephead_off; \
	x = ephead->path; \
	n = ephead->n; \
	ephead_off = (ephead_off + 1) & (EP_RINGBUF_SIZE - 1); \
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
    int path_slen = tmp - path;

    pkgfd = open(path, O_PATH);
    if(pkgfd < 0) {
	fprintf(stderr, "%s does not exist!\n", path);
	exit(EXIT_FAILURE);
    }

    char* pkgdent = calloc(DIRENT_MAX_BUF, 1);
    if(!pkgdent)
	EXIT_ERROR(ERR_NOBUF);

    EP_PUSH(".", 1);

    do {
	char* pk;
	int len;
	EP_POP(pk, len);
	register int bufcnt = 0;
	int tmp_ = openat(pkgfd, pk, O_DIRECTORY);
	*(pk + len) = '/';
	DEBUGI("Pk %s tmp %d\n", pk, tmp_);
	while((bufcnt = getdents64(tmp_, pkgdent, DIRENT_MAX_BUF)) > 0) {
	    int off = 0;
	    while(off < bufcnt) {
		struct linux_dirent64* dent = pkgdent + off;
		int tmp_result = *((uint16_t*)(dent->d_name)) ^ 0x2e2e;
		if(tmp_result == 0 || (tmp_result & 0xFF) == 0) goto next;
		DEBUGI("Found %s\n", dent->d_name);
		register int len_ = dent->d_reclen - offsetof(struct linux_dirent64, d_name);
		register char* addr = len_ + memcpy(pk + len + 1, dent->d_name, len_);
		*addr = 0;
		*(path + path_slen) = '/';
		memcpy(path + path_slen + 1 , pk, len_ + len + 2);
		DEBUGI("Symlinking %s to %s\n", pk, path);
		if(symlink(path, pk) && errno == EEXIST) {
		    if(dent->d_type == DT_DIR) {
			EP_PUSH(pk, len_);
		    }
		    else if(dent->d_type == DT_REG) {
			printf("Duplicated file at %s\n", pk);
		    }
		}
next:
		off += dent->d_reclen;
	    }

	    
	}
	close(tmp_);
    } while(ephead_off != eptail_off);





    return 0;
}
