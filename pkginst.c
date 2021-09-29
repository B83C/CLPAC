#include "helper.h"

int pkgfd;

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

    pkgfd = open(path, O_PATH);

    if(pkgfd < 0) {
	fprintf(stderr, "%s does not exist!\n", path);
	exit(EXIT_FAILURE);
    }
    
    

      

    

    return 0;
}
