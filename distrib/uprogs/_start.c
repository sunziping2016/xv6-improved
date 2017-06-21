#include <time.h>

int main(int argc, char *argv[]);
int exit();
int _start(int argc, char *argv[]) {
    process_start_time = uptime();
    main(argc, argv);
    exit();
}