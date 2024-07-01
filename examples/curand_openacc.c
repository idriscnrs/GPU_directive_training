#include<stdio.h>
#include<math.h>
#include<openacc.h>
#include<stdlib.h>
#include "openacc_curand.h"

int main(void) {
// Histogram allocation and initialization 

 int histo[10];
 for (int i=0; i<10; ++i) histo[i] = 0;
 size_t nshots = (size_t) 1e9;

// cudaStream_t stream ;

// Allocate memory for the random numbers 
 unsigned int* shots = (unsigned int*) malloc(nshots*sizeof(unsigned int));
#pragma acc data create(shots[:nshots]) copy(histo[0:10]) 
{


curandState_t state;
unsigned long long seed = 1234ULL;
unsigned long long seq = 0ULL;
unsigned long long offset = 0ULL;
#pragma acc parallel present(shots[:nshots])
{
 #pragma acc loop 
 for (size_t i=0; i<nshots; ++i) {
  curand_init(seed + i, seq, offset, &state);    // the same seed always produces the same sequence
  shots[i] =  (int) floor(10*curand_uniform(&state)) ;
 }
}

// Count the number of time each number was drawn 
#pragma acc parallel loop present(shots[:nshots], histo[0:10]) 
 for (size_t i=0; i<nshots; ++i) {
        shots[i] = shots[i] % 10;
#pragma acc atomic update 
        histo[shots[i]]++;
 }
}

// Print results 
 for (int i=0; i<10; ++i) printf("%3d: %10d (%5.3f)\n", i, histo[i], (double) histo[i]/1.e9);

 return 0;
}
