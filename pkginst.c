#include "helper.h"

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

char pkgdent[DIRENT_MAX_BUF] = {0};
//powerf of 2 please
#define EP_RINGBUF_SIZE 512
struct ep_path eprb[EP_RINGBUF_SIZE] = {0};
int ephead_off, eptail_off = 0;

#define EP_INC(x) ({ \
	x = (x + 1) & (EP_RINGBUF_SIZE - 1); \
	})
#define EP_PUSH(x, n) ({ \
	struct ep_path* eptail = eprb + eptail_off; \
	memcpy(eptail->path, x, n); \
	*(eptail->path + n) = 0; \
	eptail->len = n; \
	EP_INC(eptail_off); \
	})    
#define EP_POP(x, n) ({ \
	struct ep_path* ephead = eprb + ephead_off; \
	n = ephead->len; \
	*(x + n) = 0; \
	memcpy(x, ephead->path, n); \
	EP_INC(ephead_off); \
	})
//#define EP_POP(x, n) ({ \
//	struct ep_path* ephead = eprb + ephead_off; \
//	x = ephead->path; \
//	n = ephead->n; \
//	EP_INC(ephead_off); \
//	})

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
	register uint16_t res = *((uint16_t*)(*(argv + i))) ^ CHR2('-', '-');

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
    *(uint16_t*)tmp = CHR2('/', 0);
    int path_slen = tmp - path + 1;

    int pkgfd = open(path, O_PATH);
    if(pkgfd < 0) {
	fprintf(stderr, "%s does not exist!\n", path);
	exit(EXIT_FAILURE);
    }

    struct open_how oh = {.flags= O_DIRECTORY, .resolve = RESOLVE_IN_ROOT};
    EP_INC(eptail_off);

    char* pk = path + path_slen;
    do {
	int len;
	EP_POP(pk, len);
	register int bufcnt = 0;
	int tmp_ = syscall(SYS_openat2, pkgfd, pk - 1, &oh, sizeof(struct open_how));

	DEBUGI("Pk at %s len : %d\n", pk - 1, len);

	while((bufcnt = getdents64(tmp_, pkgdent, DIRENT_MAX_BUF)) > 0) {
	    int off = 0;
	    while(off < bufcnt) {
		struct linux_dirent64* dent = pkgdent + off;
		int tmp_result = *((uint16_t*)(dent->d_name)) ^ CHR2('.', '.');
		if(tmp_result == 0 || (tmp_result & 0xFF) == 0) goto next;

		DEBUGI("Found %s len %d \n", dent->d_name, strlen(dent->d_name));

		register int len_ = dent->d_reclen - offsetof(struct linux_dirent64, d_name) - 2;
		register char* addr = len_ + memcpy(pk + len, dent->d_name, len_);
		*(uint16_t*)addr = 0;

		DEBUGI("Symlinking %s to %s\n", pk - 1, path);

		if(symlink(path, pk - 1) && errno == EEXIST) {
		    if(dent->d_type == DT_DIR) {
			*addr = '/';
			EP_PUSH(pk, len_ + len + 1);
		    DEBUGI("last Pk at %s len : %d\n", pk, len_ + 1);
		    }
		    else if(dent->d_type == DT_REG) {
			DEBUGI("Duplicated file at %s\n", pk);
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
