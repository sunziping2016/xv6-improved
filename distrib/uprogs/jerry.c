#include <ctype.h>
#include <xv6/types.h>
#include <xv6/user.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    char a = ' ';
    char b = '1';
    char c = 'A';
    char d = 'a';
    char e = tolower(c);
    char f = toupper(d);
    if (isspace(a)) {
        myprintf("a is a space\n");
    }
    if (isdigit(b)) {
        myprintf("b is a number\n");
    }
    if (isalpha(c)) {
        myprintf("c is a letter\n");
    }
    myprintf("e is %c\n", e);
    myprintf("f is %c\n", f);

    exit();
}