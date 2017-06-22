#include "etypes.h"
#include "edefs.h"

int strcmp(const char* str1, const char* str2)
{   unsigned int i = 0;

    while ((str1[i]==str2[i])&&(str1[i]!='\0'))
        i++;
    if (str1[i]<str2[i])
        return -1;
    else if (str1[i]>str2[i])
        return 1;
    else
        return 0;
}
