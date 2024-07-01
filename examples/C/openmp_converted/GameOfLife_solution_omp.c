#include <stdio.h>
#include <stdlib.h>

void output_world(int* restrict world, int rows, int cols, int generation)
{
    /**
     * Write a file with the world inside
     * @param world: a pointer to the storage for the current step
     * @param rows: the number of rows without the border
     * @param cols: the number of columns without the border
     */
    char* path = (char*) malloc(sizeof(char)*80);
    sprintf(path, "generation%05d.gray", generation);
    FILE* f = fopen(path, "wb");
    unsigned char* mat = (unsigned char*) malloc(sizeof(unsigned char)*(rows+2)*(cols+2));
    #pragma acc parallel loop copyout(mat[(rows+2)*(cols+2)]) present(world[(rows+2)*(cols+2)])
#pragma omp target teams loop map(from:mat[(rows+2)*(cols+2)]) map(present,\
            alloc:world[(rows+2)*(cols+2)])
    for (int i=0; i<rows+2; ++i)
        for (int j=0; j<cols+2; ++j)
            mat[i*cols+j] = (unsigned char) world[i*cols+j] * 255;
    fwrite(world, sizeof(unsigned char), (rows+2)*(cols+2), f);
    fclose(f);
}
void next(int* restrict world, int* restrict  oworld, int rows, int cols)
{
    /**
     * Apply the rules and compute the next generation
     * @param world: a pointer to the storage for the current step
     * @param oworld: a pointer to the storage for the previous step
     * @param rows: the number of rows without the border
     * @param cols: the number of columns without the border
     */
    int neigh = 0;
    int row_current = 0;
    int row_above = 0;
    int row_below = 0;
#pragma acc parallel loop present(world[:(rows+2)*(cols+2)], oworld[:(rows+2)*(cols+2)])
#pragma omp target teams loop map(present,alloc:world[:(rows+2)*(cols+2)],\
            oworld[:(rows+2)*(cols+2)])
    for (int r=1; r<=rows; ++r)
        for (int c=1; c<=cols; ++c)
        {
            row_current = r*(cols+2);
            row_above = (r-1)*(cols+2);
            row_below = (r+1)*(cols+2);
            neigh = oworld[row_above + c-1] + oworld[row_above + c]   + oworld[row_above + c+1] +
                    oworld[row_current + c+1]+                        oworld[row_current + c-1] + 
                    oworld[row_below + c-1] + oworld[row_below + c] + oworld[row_below + c+1];
            if (oworld[r*(cols+2)+c] == 1 && (neigh<2||neigh>3))
                world[r*(cols+2)+c] = 0;
            else if (neigh==3)
                world[r*(cols+2)+c] = 1;
        }
} 

void save(int* restrict world, int* restrict oworld, int rows, int cols)
{
    /**
     * Save the current world to oworld
     * @param world: a pointer to the storage for the current step
     * @param oworld: a pointer to the storage for the previous step
     * @param rows: the number of rows without the border
     * @param cols: the number of columns without the border
     */
#pragma acc parallel loop collapse(2) present(world[:(rows+2)*(cols+2)], oworld[:(rows+2)*(cols+2)])
#pragma omp target teams loop collapse(2) map(present,\
            alloc:world[:(rows+2)*(cols+2)],oworld[:(rows+2)*(cols+2)])
    for (int r=1; r<=rows; ++r)
        for (int c=1; c <= cols; ++c)
            oworld[r*(cols+2) + c] = world[r*(cols+2) + c];
}

int alive(int* restrict world, int rows, int cols)
{
    /**
     * Compute the number of cells alive at the current generation
     * @param world: a pointer to the storage for the current step
     * @param rows: the number of rows without the border
     * @param cols: the number of columns without the border
     */
    int cells = 0;
#pragma acc parallel loop collapse(2) reduction(+:cells) present(world[:(rows+2)*(cols+2)])
#pragma omp target teams loop reduction(+:cells) collapse(2) map(present,\
            alloc:world[:(rows+2)*(cols+2)])
    for (int r=1; r <= rows; ++r)
        for (int c=1; c <= cols; ++c)
            cells += world[r*(cols+2) + c];
    return cells;
}

void fill_world(int* restrict world, int rows, int cols)
{
    /**
     *  Set the initial state of the world
     * @param world: a pointer to the storage for the current step
     * @param rows: the number of rows without the border
     * @param cols: the number of columns without the border
     */
    for (int r=1; r <= rows; ++r)
        for (int c=1; c <= cols; ++c)
            world[r*(cols+2) + c] = rand()%4==0 ?1 : 0;
    // The border of the world is a dead zone
    for (int i=0;i<=rows;++i)
    {
        world[i*(cols+2)] = 0;
        world[i*(cols+2)+cols+1] = 0;
    }
    for (int j=0; j<cols; ++j)
    {
        world[j] = 0;
        world[(rows+1)*(cols+2)+j] = 0;
    }
}

int* allocate(int rows, int cols)
{
    /**
     * Allocate memory for a 2D array
     * @param rows: the number of rows without the border
     * @param cols: the number of columns without the border
     * @return a pointer to the matrix
     */
    
    int* mat = (int*) malloc((rows+2)*(cols+2)*sizeof(int));
#pragma acc enter data create(mat[0:(rows+2)*(cols+2)])
#pragma omp target enter data map(alloc:mat[0:(rows+2)*(cols+2)])
    
    return mat;
}

void destroy(int* mat, int rows, int cols)
{
    /**
     * Free memory for a 2D array
     * @param mat: a pointer to the matrix to free
     * @param rows: the number of rows without the border
     * @param cols: the number of columns without the border 
     */
#pragma acc exit data delete(mat[0:(rows+2)*(cols+2)])
#pragma omp target exit data map(delete:mat[0:(rows+2)*(cols+2)])
    free(mat);
}

int main(int argc, char** argv)
{
    int rows, cols, generations;
    int* world;
    int* oworld;

    if (argc < 4)
    {
        printf("Wrong number of arguments: Please give rows cols and generations\n");
        return 1;
    }
    rows = strtol(argv[1], NULL, 10);
    cols = strtol(argv[2], NULL, 10);
    generations = strtol(argv[3], NULL, 10);
    
    world = allocate(rows, cols);
    oworld = allocate(rows, cols);
    fill_world(world, rows, cols);
    printf("Initial state set\n");
#pragma acc update device(world[0:(rows+2)*(cols+2)])
#pragma omp target update to(world[0:(rows+2)*(cols+2)])
    printf("Cells alive at generation %d: %d\n", 0, alive(world, rows, cols));
    for (int g=1; g <= generations; ++g)
    {
        save(world, oworld, rows, cols);
        next(world, oworld, rows, cols);
        output_world(world, rows, cols, g);
        printf("Cells alive at generation %4d: %d\n", g, alive(world, rows, cols));
    }

    destroy(world, rows, cols);
    destroy(oworld, rows, cols);

    return 0;
}

// Code was translated using: /home/very/bin/acc2mp -async=nowait -specify-language=C -generate-report GameOfLife_solution.c
