#include <stdio.h>
#include <stdlib.h>

void normalize_rows(double* mat, size_t size)
{
    double norms[size];
    #pragma acc declare device_resident(norms)
    double norm;
    // Compute the L1 norm of each row
    #pragma acc parallel loop present(mat[0:size*size])
    for (size_t i=0; i<size; ++i)
    {
        norm = 0.;
        #pragma acc loop reduction(+:norm)
        for (size_t j=0; j<size; ++j)
            norm += mat[i*size+j];
        norms[i] = norm;
    }
    // Divide each row element by the L1 norm 
    #pragma acc parallel loop present(mat[0:size*size])
    for (size_t i=0; i<size; ++i)
        for (size_t j=0; j<size; ++j)
            mat[i*size+j] /= norms[i];
}

int main(void)
{
    size_t size = 2000;
    double* mat = malloc(size*size*sizeof(double));
    double sum = 0.;
    srand((unsigned) 12345900);
    for (size_t i=0; i<size; ++i)
        for (size_t j=0; j<size; ++j)
            mat[i*size+j] = (double)rand() / (double) RAND_MAX;
    #pragma acc enter data copyin(mat[0:size*size])

    normalize_rows(mat, size);

    // Compute the sum of all elements in the matrix
    #pragma acc parallel loop present(mat[0:size*size]) reduction(+:sum)
    for (size_t i=0; i<size; ++i)
        for (size_t j=0; j<size; ++j)
            sum += mat[i*size+j];
    #pragma acc exit data delete(mat[0:size*size])
    free(mat);

    printf("%f == %d?\n", sum, size);

    return 0;
}
