#include <xv6/types.h>
#include <xv6/user.h>

unsigned int freq[] = {
        0,
        262,
        294,
        330,
        349,
        392,
        440,
        494
};

int sound[] = {3, 3, 4, 5, 5, 4, 3, 2, 1, 1, 2, 3, 3, 2, 2,
               3, 3, 4, 5, 5, 4, 3, 2, 1, 1, 2, 3, 2, 1, 1};

int main(int argc, char *argv[])
{
    int i;
    for (i = 0; i < sizeof(sound) / sizeof(sound[0]); ++i) {
        playsound(freq[sound[i]]);
        sleep(30);
        nosound();
        sleep(10);
    }
    exit();
}