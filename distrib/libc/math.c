#include <math.h>

int isnan(double d)
{
    register struct IEEEdp {
        unsigned int manl : 32;
        unsigned int manh : 20;
        unsigned int  exp : 11;
        unsigned int sign :  1;
    } *p = (struct IEEEdp *)&d;

    return(p->exp == 2047 && (p->manh || p->manl));
}

int isinf(double d)
{
    register struct IEEEdp {
        unsigned int manl : 32;
        unsigned int manh : 20;
        unsigned int  exp : 11;
        unsigned int sign :  1;
    } *p = (struct IEEEdp *)&d;

    return(p->exp == 2047 && !p->manh && !p->manl);
}
