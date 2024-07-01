#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

int main(int argc, char** argv)
{
    size_t size=50000;
    double array[size];
    double table[size];
    double sum_val;
    double res;

    unsigned int ngangs = (unsigned int) atoi(argv[1]);

    res = 0.0;
#pragma acc parallel num_gangs(ngangs) copyout(array[0:size]) private(table[0:size])
{
    #pragma acc loop gang reduction(+:res)
    for (size_t i=0; i<size; ++i)
    {
        #pragma acc loop vector
        for(size_t j=0; j<size; ++j)
        {
           table[j] = (i+j);
        }
        sum_val = 0.0;
        #pragma acc loop vector reduction(+:sum_val)
        for(size_t j=0; j<size; ++j)
        {
            sum_val += table[j];
        }
        array[i] = sum_val;
        res += sum_val;
    }
}

    printf("result: %lf\n",res);
    return 0;
}
