#include "xv6/types.h"
#include "xv6/stat.h"
#include "xv6/user.h"

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        printf(2, "Usage: mount path...\n");
        exit();
    }

    mkdir(argv[1]);
    mount(1, argv[1], 0);
    exit();
}
