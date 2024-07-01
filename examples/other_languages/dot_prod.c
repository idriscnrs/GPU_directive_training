#include <stdio.h>
#include <CL/cl.h>
#include <time.h>        // For timing

#define NUM_ELEMENTS 1024 * 1024 * 16  // Vector size, you can change it to your needs
#define BLOCK_SIZE 512                  // Block size for CUDA kernel

const char* source =
__kernel void dot(__global float* a, __global float* b, __global float *c) {
    const int BLOCK_SIZE = 512;
    __local float temp[BLOCK_SIZE];
    int idx = get_global_id(0); // This calculates the global index of the current thread.
    temp[get_local_id(0)] = a[idx] * b[idx];
    barrier(CLK_LOCAL_MEM_FENCE); // This synchronizes all threads in the block.
    if (get_local_id(0) == 0) { // Only the first thread computes the sum
        float sum = 0.0;
       for(int i = 0; i < BLOCK_SIZE; ++i)
            sum += temp[i];
        AtomicAdd(&c[0], sum); // This adds the sum to the value pointed by the pointer c.
   }
};
int main(void) {
    cl_platform_id platform_id;
    cl_device_id device_id;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel dot;
    cl_int err;


    cl_mem d_a, d_b, d_c;

    float *a, *b, *c;

    // Allocate host memory for the vectors a, b and c.
    a = (float*)malloc(sizeof(float) * NUM_ELEMENTS);
    b = (float*)malloc(sizeof(float) * NUM_ELEMENTS);
    c = (float*)malloc(sizeof(float));
    c[0] = 0.0;

    srand(42);

    // Initialize the vectors a and b
    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        a[i] = 1.0;
        b[i] = (float)rand() / (float)RAND_MAX;
    }
    // Compute the reference value
    float sum = 0.;
    for (int i = 0; i < NUM_ELEMENTS; ++i){
        sum += b[i];
    }
    printf("Sum of b %.6e\n", sum);

    err = clGetPlatformIDs(1, &platform_id, NULL);
    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, NULL);
    printf("Using device: %d\n", device_id);

    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    queue = clCreateCommandQueueWithProperties(context, device_id, 0, &err);
    
    // Allocate the memory objects for the vectors a and b on the device.
    d_a = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * NUM_ELEMENTS, NULL, &err);
    d_b = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * NUM_ELEMENTS, NULL, &err);
    d_c = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float), NULL, &err);
    // Initialize the vectors with the value they have on the host
    clEnqueueWriteBuffer(queue, d_a, CL_TRUE, 0, sizeof(float) * NUM_ELEMENTS, a, 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, d_b, CL_TRUE, 0, sizeof(float) * NUM_ELEMENTS, b, 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, d_c, CL_TRUE, 0, sizeof(float), c, 0, NULL, NULL);

    // Create the program and kernel objects.
    program = clCreateProgramWithSource(context, 1, &source, NULL, &err);
    clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    char result[4096];
    size_t size;
    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(result), result, &size);
    printf("%s\n", result);
    dot = clCreateKernel(program, "dot", &err);
    clSetKernelArg(dot, 0, sizeof(cl_mem), &d_a);
    clSetKernelArg(dot, 1, sizeof(cl_mem), &d_b);
    clSetKernelArg(dot, 2, sizeof(cl_mem), &d_c);

    size_t global_item_size = NUM_ELEMENTS;
    size_t local_item_size = BLOCK_SIZE;

    // Execute the kernel.
    clEnqueueNDRangeKernel(queue, dot, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
    clFinish(queue);
    // Read the result back to host.
    clEnqueueReadBuffer(queue, d_c, CL_TRUE, 0, sizeof(float), c, 0, NULL, NULL);

    printf("Result: %.6e; diff: %.6e\n", c[0], sum - c[0]);

    // Free the memory
    free(a); free(b);
    clReleaseMemObject(d_a); clReleaseMemObject(d_b); clReleaseMemObject(d_c);
    clReleaseKernel(dot); clReleaseProgram(program);
    clReleaseCommandQueue(queue); clReleaseContext(context);
    return 0;
}
