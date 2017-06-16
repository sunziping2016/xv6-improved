#include <ctype.h>

int iscntrl(int c) {
    if (c >= 0 && c <= 31 || c == 127) {
        return 1;
    }
    else {
        return 0;
    }
}

int isprint(int c) {
    if (c >= 32 && c <= 126) {
        return 1;
    }
    else {
        return 0;
    }
}

int isspace(int c) {
    if (c >= 9 && c <= 13 || c == 32) {
        return 1;
    }
    else {
        return 0;
    }
}

int isblank(int c) {
    if (c == 9 || c == 32) {
        return 1;
    }
    else {
        return 0;
    }
}

int isgraph(int c) {
    if (c >= 33 && c <= 126) {
        return 1;
    }
    else {
        return 0;
    }
}

int ispunct(int c) {
    if (c >= 33 && c <= 47
            || c >= 58 && c <= 64
            || c >= 91 && c <= 96
            || c >= 123 && c <= 126) {
        return 1;
    }
    else {
        return 0;
    }
}

int isalnum(int c) {
    if (c >= 48 && c <= 57
            || c >= 65 && c <= 90
            || c >= 97 && c <= 122) {
        return 1;
    }
    else {
        return 0;
    }
}

int isalpha(int c) {
    if (c >= 65 && c <= 90
            || c >= 97 && c <= 122) {
        return 1;
    }
    else {
        return 0;
    }
}

int isupper(int c) {
    if (c >= 65 && c <= 90) {
        return 1;
    }
    else {
        return 0;
    }
}

int islower(int c) {
    if (c >= 97 && c <= 122) {
        return 1;
    }
    else {
        return 0;
    }
}

int isdigit(int c) {
    if (c >= 48 && c <= 57) {
        return 1;
    }
    else {
        return 0;
    }
}

int isxdigit(int c) {
    if (c >= 48 && c <= 57
            || c >= 65 && c <= 70
            || c >= 97 && c <= 102) {
        return 1;
    }
    else {
        return 0;
    }
}

int toupper(int c) {
    if (c >= 97 && c <= 122) {
        return (c - 32);
    }
}

int tolower(int c) {
    if (c >= 65 && c <= 90) {
        return (c + 32);
    }
}