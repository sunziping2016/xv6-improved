/** @file
 * @brief Kernel extended string functions
 * @author Qifan Lu
 * @date June 26, 2016
 * @version 1.0.0
 */

//[ Header Files ]
//Xv6 kernel extra
#include "etypes.h"
#include "edefs.h"

/**
 * Compare two string
 *
 * @param str1 String I
 * @param str2 String II
 * @return -1 if str1<str2, 0 if str1==str2 or 1 if str1>str2
 */
int strcmp(const char* str1, const char* str2)
{   unsigned int i = 0;

    while ((str1[i]==str2[i])&&(str1[i]!='\0'))
        i++;
    //Result
    if (str1[i]<str2[i])
        return -1;
    else if (str1[i]>str2[i])
        return 1;
    else
        return 0;
}
