#include <stdio.h>
#include <stdlib.h>
int main(void)
{
    // Histogram allocation and initialization
    int histo[10];
    for (int i=0; i<10; ++i)
        histo[i] = 0;
    size_t nshots = (size_t) 1e9;
    
    // Allocate memory for the random numbers
    int* shots = (int*) malloc(nshots*sizeof(int));

    srand((unsigned) 12345900);     
    
    // Fill the array on the CPU (rand is not available on GPU with Nvidia Compilers)
    for (size_t i=0; i< nshots; ++i)
    {
        shots[i] = (int) rand() % 10;
    }
    
    // Count the number of time each number was drawn 
    for (size_t i=0; i<nshots; ++i)
    {
        histo[shots[i]]++;
    }
    
    // Print results
    
    for (int i=0; i<10; ++i)
        printf("%3d: %10d (%5.3f)\n", i, histo[i], (double) histo[i]/1.e9);
      
    return 0;
}

