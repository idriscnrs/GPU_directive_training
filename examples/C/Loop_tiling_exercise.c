#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<openacc.h>

int main(void) {
    int    nx = 20000;
    int    ny = 10000;
    int    idx; 
    double T[nx*ny], T_new[nx*ny];
    double erreur;

    for(int i=1; i<nx-1; ++i) {
        for(int j=1; j<ny-1; ++j) {
        T[i*ny+j]     = 0.0;
        T_new[i*ny+j] = 0.0;
        }
    }

    for(int i=0; i<nx; ++i){
        T[i*ny     ] = 100.0;
        T[i*ny+ny-1] = 0.0;
    }

    for(int j=0; j<ny; ++j){
        T[j]           = 0.0;
        T[(nx-1)*ny+j] = 0.0;
    }

    // add acc directive
    for (int it = 0; it<10000; ++it){
        erreur = 0.0;	
        // add acc directive 
        for (int i=1; i<nx-1; ++i) {
            for (int j=1; j<ny-1; ++j) {
                idx = i*(ny)+j;
                T_new[idx] = 0.25*(T[idx+ny]+T[idx-ny] + T[idx+1]+T[idx-1]);
                erreur = fmax(erreur, fabs(T_new[idx]-T[idx]));
            }
         }
        if(it%100 == 0) fprintf(stderr,"it: %d, erreur: %e\n",it,erreur);

        // add acc directive
        for (int i=1; i<nx-1; ++i) {
            for (int j=1; j<ny-1; ++j) {
                T[i*(ny)+j] =  T_new[i*(ny)+j];
            }
        }
    }
    return 0;
}
