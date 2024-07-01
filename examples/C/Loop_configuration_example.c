#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>
int main(void)
{ 
    int n = 200;
    int ngangs = 1, nworkers = 2, nvectors = 32;
    size_t table[n*n*n];
    
#pragma acc parallel loop gang num_gangs(ngangs) num_workers(nworkers) vector_length(nvectors) copyout(table[0:n*n*n])
    for (int i=0; i<n; ++i)
    {
#pragma acc loop worker
        for (int j=0; j<n; ++j)
        {
            #pragma acc loop vector
            for (int k=0; k<n; ++k) table[i*n*n + j*n + k] = k + 1000*j + 1000*1000*i;
        }
    }
    printf("%d %d\n",table[0],table[n*n*n-1]);
}
