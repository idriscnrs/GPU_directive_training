#include <stdio.h>
#include <stdlib.h>
#include <math.h>
int main(void)
{
    int size=1e5;
    double s[size];
    double c[size];
    double sum[size];
    #pragma acc parallel
    {
        #pragma acc loop
        for (int i=0; i<size;++i)
        {
            s[i] = sin(i*M_PI/size)*sin(i*M_PI/size);
            c[i] = cos(i*M_PI/size)*cos(i*M_PI/size);
        }    
    }
    #pragma acc parallel
    {
        #pragma acc loop
        for (int i=1; i<size ; ++i)
            sum[i] = s[i] + c[size - i]; 
    }
    
    printf("sum[42] = %f", sum[42]);
}

