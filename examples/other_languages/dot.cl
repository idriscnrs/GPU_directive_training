__kernel void dot(__global float* a, __global float* b, __global float *c) {

    __local float temp[BLOCK_SIZE];
    int idx = get_global_id(0); // This calculates the global index of the current thread.
    
    temp[get_local_id(0)] = a[idx] * b[idx]; 
    barrier(CL_LOCAL_MEM_FENCE); // This synchronizes all threads in the block.
    
    if (get_local_id(0) == 0) { // Only the first thread computes the sum
        float sum = 0.0; 
        for(int i = 0; i < BLOCK_SIZE; ++i)
            sum += temp[i];
        atomic_add(c, sum); // This adds the sum to the value pointed by the pointer "c". The addition is performed in a thread-safe way, preventing race conditions
    }
}