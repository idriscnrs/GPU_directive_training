#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

typedef struct
{
    size_t size;
    double* data;
} Array;

typedef struct
{
    Array* x;
    Array* y;
    Array* z;
} Coordinates;

typedef struct
{
    size_t natoms;    // number of atoms
    double lx;        // box length in each dimension
    double ly;         
    double lz;         
} Config;

Array* allocate_array(size_t size);
void free_array(Array* arr);
Coordinates* read_coords(char* filepath, Config *config);
void free_coords(Coordinates* coords);

Array* allocate_array(size_t size)
{
    Array* arr = (Array*) malloc(sizeof(Array));
    arr->size = size;
    arr->data = (double*) malloc(size*sizeof(double));
    #pragma acc enter data create(arr, arr->data[:size]) copyin(arr->size)
    return arr;
}

Coordinates* allocate_coords(size_t size)
{
    Coordinates* coords = (Coordinates*) malloc(sizeof(Coordinates));
    #pragma acc enter data create(coords)
    coords->x  = allocate_array(size);
    coords->y  = allocate_array(size);
    coords->z  = allocate_array(size);
    return coords;
 }

void free_array(Array* arr)
{
    free(arr->data);
    free(arr);
    #pragma acc exit data delete(arr->data,arr)
}

void free_coords(Coordinates* coords)
{
    free_array(coords->x);
    free_array(coords->y);
    free_array(coords->z);
    #pragma acc exit data delete(coords)
    free(coords);
}

Coordinates* read_coords(char* filepath, Config* conf)
{
    FILE* fptr = fopen(filepath, "r");
    char* line = NULL;
    size_t len = 0;
    size_t i = 0;
    char n[20], x[20], y[20], z[20], vx[20], vy[20], vz[20];
    
    if (fptr == NULL)
        exit(EXIT_FAILURE);

    getline(&line, &len, fptr);
    sscanf(line, "%s", n);
    conf->natoms = atoi(n);
    getline(&line, &len, fptr);
    sscanf(line, "%s", n);
    conf->lx = atof(n);
    conf->ly = atof(n);
    conf->lz = atof(n);
    #pragma acc enter data copyin(conf)
    printf("Number of atoms in %s file: %d\n", filepath, conf->natoms);
    
    Coordinates* coords = allocate_coords(conf->natoms);
    
    while (i < conf->natoms)
    {
        getline(&line, &len, fptr);
        sscanf(line, "%s %s %s %s %s %s %s", n, x, y, z, vx, vy, vz);
        coords->x->data[i] = atof(x);
        coords->y->data[i] = atof(y);
        coords->z->data[i] = atof(z);
	++i;
    }
    #pragma acc update device(coords->x->data[:conf->natoms],coords->y->data[:conf->natoms],coords->z->data[:conf->natoms])
    fclose(fptr);
    return coords;
}

int main(int argc, char** argv)
{
    double deltaR, rCutOff;
    FILE* fPtr;
    char* input;
    double xij, yij, zij, rij;
    int d;
    
    if (argc < 3 || argc > 4) 
    {
        fprintf(stderr, "%s", "ERROR: Wrong number of parameters.\n");
        fprintf(stderr, "%s", "ERROR: The program requires at least two parameters:\n");
        fprintf(stderr, "%s", "ERROR: deltaR, the length of each bin, and \n");
        fprintf(stderr, "%s", "ERROR: rCutoff, the total length (rcut < box_length/2).\n");
        fprintf(stderr, "%s", "ERROR: Usage example: ./rdf 0.5 15.5 [input]\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        deltaR = atof(argv[1]);
        rCutOff = atof(argv[2]);
        if (argc == 4)
            input = argv[3];
        else
            input = "./dyn.xyz";
    }
    
    int maxbin = rCutOff/deltaR + 1;
    int* hist = (int*) malloc(maxbin*sizeof(int));
    double* gr = (double*) malloc(maxbin*sizeof(double));
    #pragma acc enter data create(hist[:maxbin],gr[:maxbin])
    
    #pragma acc parallel loop present(hist[:maxbin])
    for (size_t i=0; i<maxbin; ++i)
        hist[i] = 0;
    
    Config* conf = (Config*) malloc(sizeof(Config));
    Coordinates* coords = read_coords(input, conf);  

    #pragma acc parallel loop present(conf,coords,coords->x,coords->y,coords->z)\
                              present(coords->x->data[:conf->natoms],coords->y->data[:conf->natoms],coords->z->data[:conf->natoms])
    for (int j = 0; j < conf->natoms; ++j)
        #pragma acc loop private(xij,yij,zij,rij,d)
        for (int i = 0; i < conf->natoms; ++i)
            if (i != j)		
            {
	        xij = coords->x->data[j]-coords->x->data[i];
                yij = coords->y->data[j]-coords->y->data[i];
                zij = coords->z->data[j]-coords->z->data[i];
                xij -= floor(xij/conf->lx + 0.5) *conf->lx;
                yij -= floor(yij/conf->ly + 0.5) *conf->ly;
                zij -= floor(zij/conf->lz + 0.5) *conf->lz;
		rij = xij*xij + yij*yij + zij*zij;
                d = (int) (sqrt(rij)/deltaR);
                if (d < maxbin)
                    #pragma acc atomic update
                    ++hist[d];
        }
    
    double rho = ((double) conf->natoms) / ((double)(conf->lx * conf->ly * conf->lz)) * 4.0 / 3.0 * acos(-1.0);
    #pragma acc parallel loop present (hist[:maxbin],gr[:maxbin])
    for (int i = 0; i < maxbin; ++i)
    {
        double nideal  = rho * ( pow((i+1)*deltaR,3) - pow(i*deltaR,3) );
        gr[i] = ((double) hist[i]) / (nideal*conf->natoms);
    }
    #pragma acc update self(gr[:maxbin])
         
    fPtr = fopen("RDF","w");
    for (int i = 0; i < maxbin; ++i)
      fprintf(fPtr,"%lf %lf\n", i*deltaR, gr[i]);
    fclose(fPtr);
    #pragma acc exit data delete(hist,gr)
    free(hist);
    free(gr);
    #pragma acc exit data delete(conf)
    free(conf);
    free_coords(coords);
    return 0;
}
