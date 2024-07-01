#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
int main(int argc, char** argv)
{
    unsigned int steps = UINT_MAX/2;
    double pi = 0.0;
    double diff = 0.0;
    size_t i = 0;
    // Modifications from here
    for (i=0; i<steps; ++i)
    {   
        pi +=pow(-1.0,i)*1./(2.*i+1);
    }
    diff = 4*pi - M_PI;
    printf("Difference with M_PI is %10.4e with %u steps\n", diff, steps);
    if (abs(diff) < 1.e-5 )
        printf("Results seems correct");
    else
        printf("Results seems wrong");
    return 0;
}

