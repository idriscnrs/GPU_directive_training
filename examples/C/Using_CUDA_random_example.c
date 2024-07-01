#include <stdio.h>
#include <stdlib.h>
#include <curand.h>
#include <openacc.h>

// Fill d_buffer with num random numbers
void fill_rand(unsigned int *d_buffer, size_t num, cudaStream_t stream)
{
    curandGenerator_t gen;
    int status;
    // Create generator
    status = curandCreateGenerator(&gen, CURAND_RNG_PSEUDO_DEFAULT);
    // Set CUDA stream
    status |= curandSetStream(gen, stream);
    // Set seed
    status |= curandSetPseudoRandomGeneratorSeed(gen, 1234ULL);
    // Generate num random numbers
    status |= curandGenerate(gen, d_buffer, num);
//  Peut essayer curandStatus_tcurandGeneratePoisson(curandGenerator_t generator, unsigned int *outputPtr, size_t n, double lambda)
    // Cleanup generator
    status |= curandDestroyGenerator(gen);

    if (status != CURAND_STATUS_SUCCESS) {
        printf ("curand failure!\n");
        exit (EXIT_FAILURE);
    }
}

int main(void) { 
    // Histogram allocation and initialization 
    int histo[10]; 
    for (int i=0; i<10; ++i) 
        histo[i] = 0; 

    size_t nshots = (size_t) 1e9; 
    cudaStream_t stream ;

    // Allocate memory for the random numbers 
    unsigned int* shots = (unsigned int*) malloc(nshots*sizeof(unsigned int));
    #pragma acc data create(shots[:nshots]) copyout(histo[:10]) 
    {
        #pragma acc host_data use_device(shots) 
        {
             stream = (cudaStream_t) acc_get_cuda_stream(acc_async_sync); 
             fill_rand(shots, nshots, stream);
        }

        // Count the number of time each number was drawn 
        #pragma acc parallel loop present(shots[:nshots]) 
        for (size_t i=0; i<nshots; ++i) 
        { 
            shots[i] = shots[i] % 10;
            #pragma acc atomic update 
            histo[shots[i]]++; 
        } 
    }// End acc data

    // Print results 
    for (int i=0; i<10; ++i) 
        printf("%3d: %10d (%5.3f)\n", i, histo[i], (double) histo[i]/1.e9); 

     return 0; 
}

