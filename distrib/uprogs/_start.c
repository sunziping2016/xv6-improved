#include <xv6/user.h>

int main(int argc, char *argv[]);

int _start(int argc, char *argv[]) {
    main(argc, argv);
    exit();
}