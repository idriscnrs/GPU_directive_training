/**
 * This is an example of a cuda program that performs a scalar product.
  * It uses the CUDA runtime API to manage memory and execute kernel functions.
  * The vector size can be changed by changing the value of NUM_ELEMENTS.
  */
#include <stdio.h>
#include "driver_types.h"
#include <cuda_runtime.h>
#include <time.h>        // For timing

#define NUM_ELEMENTS 1024 * 1024 * 16  // Vector size, you can change it to your needs
#define BLOCK_SIZE 512                  // Block size for CUDA kernel

__global__ void dot(float* a, float* b, float *c) {

    __shared__ float temp[BLOCK_SIZE];
    int idx = threadIdx.x + blockDim.x * blockIdx.x; // This calculates the global index of the current thread.
    
    temp[threadIdx.x] = a[idx] * b[idx]; 
    __syncthreads(); // This synchronizes all threads in the block.
    
    if (threadIdx.x == 0) { // Only the first thread computes the sum
        float sum = 0.0; 
        for(int i = 0; i < BLOCK_SIZE; ++i)
            sum += temp[i];
        atomicAdd(c, sum); // This adds the sum to the value pointed by the pointer "c". The addition is performed in a thread-safe way, preventing race conditions
    }
}

int main(void) {
    float *a, *b, *c;  // host copies of vectors a, b
    float *d_a, *d_b, *d_c;  // device copies of vectors a, b, c
    
    clock_t start, end;  // variables for time measurement

    // Allocate space for device copies of a, b, c
    cudaMalloc((void **)&d_a, sizeof(float) * NUM_ELEMENTS);
    cudaMalloc((void **)&d_b, sizeof(float) * NUM_ELEMENTS);
    cudaMalloc((void **)&d_c, sizeof(float));
    
    // Allocate space for host copies of a, b, c and setup input values
    a = (float*)malloc(sizeof(float) * NUM_ELEMENTS);
    b = (float*)malloc(sizeof(float) * NUM_ELEMENTS);
    c = (float*)malloc(sizeof(float));
    c[0] = 0.0;
    
    // Fix the seed of the PRNG to get reproducible results.
    srand(42);    
    
    // Initialize the vectors a and b with some values.
    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        a[i] = 1.0;  // random numbers between [0,1]
        b[i] = (float)rand() / (float)RAND_MAX;  // random numbers between [0,1]
    }

    // To check if the dot product is correct
    float sum = 0.;
    for (int i = 0; i < NUM_ELEMENTS; ++i){
        sum += b[i];
    }
    printf("Sum of b %.6e\n", sum);
    start = clock();   // Start timing

    // Copy inputs to device
    cudaMemcpy(d_a, a, sizeof(float) * NUM_ELEMENTS, cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, b, sizeof(float) * NUM_ELEMENTS, cudaMemcpyHostToDevice);
    cudaMemcpy(d_c, c, sizeof(float), cudaMemcpyHostToDevice);
    // Launch dot() kernel on GPU with N blocks
    int threads = BLOCK_SIZE;
    int blocks = (NUM_ELEMENTS) / threads;
    printf("Number of elements: %d; BLOCK_SIZE: %d; Number of blocks: %d\n", NUM_ELEMENTS, BLOCK_SIZE, blocks);

    // Execute the kernel
    dot<<<blocks,threads>>>(d_a, d_b, d_c);
    
    // Copy result back to host
    cudaMemcpy(c, d_c, sizeof(float), cudaMemcpyDeviceToHost);

    end = clock();   // End timing

    printf("Result: %.6e; diff: %.6e\n", c[0], sum - c[0]);
    printf("Time elapsed: %.2f ms\n", (double)(end - start) / CLOCKS_PER_SEC * 1000);

    // Cleanup
    free(a); free(b);
    cudaFree(d_a); cudaFree(d_b); cudaFree(d_c);

    return 0;
}