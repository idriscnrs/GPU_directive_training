#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

int main(int argc, char** argv)
{
    size_t size=50000;
    double table[size];
    double sum_val;
    double res;

    unsigned int ngangs = (unsigned int) atoi(argv[1]);

    res = 0.0;
    for (size_t i=0; i<size; ++i)
    {
        for(size_t j=0; j<size; ++j)
        {
            table[j] = (i+j);
        }
        sum_val = 0.0;
        for(size_t j=0; j<size; ++j)
        {
            sum_val += table[j];
        }
        res += sum_val;
    }
    printf("result: %lf \n",res);
    return 0;
}
