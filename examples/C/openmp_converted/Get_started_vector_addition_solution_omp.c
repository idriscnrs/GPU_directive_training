#include <stdio.h>
#include <stdlib.h>
#include <math.h>
int main(void)
{
    int size=1e5;
    double s[size];
    double c[size];
    double sum[size];
    #pragma acc data create(s, c) copyout(sum)
#pragma omp target data map(from:sum) map(alloc:s,c)
    {
    #pragma acc parallel
#pragma omp target teams
    {
        #pragma acc loop
#pragma omp loop
        for (int i=0; i<size;++i)
        {
            s[i] = sin(i*M_PI/size)*sin(i*M_PI/size);
            c[i] = cos(i*M_PI/size)*cos(i*M_PI/size);
        }    
    }
    #pragma acc parallel
#pragma omp target teams
    {
        #pragma acc loop
#pragma omp loop
        for (int i=1; i<size ; ++i)
            sum[i] = s[i] + c[size - i]; 
    }
    }// end of structured data region
    printf("sum[42] = %f\n", sum[42]);
}


// Code was translated using: /home/very/bin/acc2mp -async=nowait -specify-language=C -generate-report Get_started_vector_addition_solution.c
