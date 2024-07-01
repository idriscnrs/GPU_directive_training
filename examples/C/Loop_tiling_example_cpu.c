#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define MIN(a,b) ( ((a)<(b))?(a):(b) )

double double_random(){
    return (double) (rand()) / RAND_MAX;
}	

void nullify(int ni, int nj, double* d){
    for (int i=0; i<ni; i++){
       for (int j=0; j<nj; j++){
           d[j+i*nj] = 0.0;
       }
    }
}

double checksum(int ni, int nj, double* d){
    double dsum = 0.0;
    for (int i=0; i<ni; i++){
       for (int j=0; j<nj; j++){
           dsum = dsum + d[j+i*nj];
       }
    }
    return dsum;
}	

void naive_matmul(int ni, int nj, int nk, double* a, double* b, double* c, double* d){
    for (int i=0; i<ni; i++){
       for (int j=0; j<nj; j++){
           for (int k=0; k<nk; k++){
                d[i*nj +j] = d[i*nj +j] + a[k+i*nk] * b[j+k*nj];
           }
           d[j+i*nj]= d[j+i*nj] + c[j+i*nj];
       }
    }
}

void tiled_matmul(int tile, int ni, int nj, int nk, double* a, double* b, double* c, double* d){
     for (int i=0; i<ni; i+=tile){	
       for (int j=0; j<nj; j+=tile){	     
          for (int ii=i; ii< MIN(i+tile,ni); ii++){
            for (int jj=j; jj<MIN(j+tile,nj); jj++){
	         for (int k=0; k<nk; k++){
                       d[ii*nj +jj] = d[ii*nj +jj] + a[k+ii*nk] * b[jj+k*nj];
                 }
             }
          }
       }
    }	
    for (int i=0; i<ni; i++){
       for (int j=0; j<nj; j++){
           d[j+i*nj]= d[j+i*nj] + c[j+i*nj];
       }
    }
	     
}

int main(void)
{
    int ni=4280, nj=4024, nk=1960;
 
    clock_t t1, t2;

    double* a = (double*) malloc(ni*nk*sizeof(double));
    double* b = (double*) malloc(nk*nj*sizeof(double));
    double* c = (double*) malloc(ni*nj*sizeof(double));
    double* d = (double*) malloc(ni*nj*sizeof(double));
    double test;

    unsigned int seed = 1234;
    srand(seed);

    for (int i=0; i<ni; i++){
       for (int k=0; k<nk; k++){   
           a[k+i*nk] = double_random();
       }
    }

    for (int k=0; k<nk; k++){
       for (int j=0; j<nj; j++){   
           b[j+k*nj] = double_random();
       }   
    }  

    for (int i=0; i<ni; i++){
       for (int j=0; j<nj; j++){
           c[j+i*nj] = 2.0;
       }
    }

    nullify(ni, nj, d);
    
    t1 = clock();
    naive_matmul(ni, nj, nk, a, b, c, d);
    t2 = clock();
    test = checksum(ni, nj, d);
    fprintf(stderr, "CPU naive Elapsed: %lf\n", (double) (t2-t1) /CLOCKS_PER_SEC);
    fprintf(stderr, "\tchecksum=%lf\n\n", test);
    nullify(ni, nj, d);

    int tile = 512;
    t1 = clock();
    tiled_matmul(tile, ni, nj, nk, a, b, c, d);
    t2 = clock();
    test = checksum(ni, nj, d);
    fprintf(stderr, "CPU Manually tiled Elapsed: %lf\n", (double) (t2-t1) /CLOCKS_PER_SEC);
    fprintf(stderr, "\tchecsum=%lf\n\n", test);
    nullify(ni, nj, d);

    free(a);
    free(b);
    free(c);
    free(d);

    return 0;
}	
