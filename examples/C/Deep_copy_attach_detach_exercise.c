#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

typedef struct
{
    double* count;
    double* birth_rate;
    double* death_rate;    
} Population;

void free_pop(Population* pop)
{
    // Add openacc directives
    free(pop->count);
    free(pop->birth_rate);
    free(pop->death_rate);
    free(pop);
}

void derivee(double* x, Population* pop, double* dx)
{
    // Add openacc directives
    dx[0] =  pop->birth_rate[0]*x[0] - pop->death_rate[0]*x[0]*x[1];
    dx[1] = -pop->death_rate[1]*x[1] + pop->birth_rate[1]*x[0]*x[1];       
}

void rk4(Population* pop, double dt)
{
        double* x_temp = (double*) malloc(2*sizeof(double));
        double* k1     = (double*) malloc(2*sizeof(double));
        double* k2     = (double*) malloc(2*sizeof(double));
        double* k3     = (double*) malloc(2*sizeof(double));
        double* k4     = (double*) malloc(2*sizeof(double));
        double halfdt  = dt / 2.0;
 
	// Add openacc directives
          for(int i=0; i<2; i++)
             x_temp[i] = pop->count[i];

          derivee(x_temp, pop, k1);
          for(int i=0; i<2; i++)
            x_temp[i] = pop->count[i] + k1[i]*halfdt;
        
          derivee(x_temp, pop, k2);
          for(int i=0; i<2; i++)
            x_temp[i] = pop->count[i] + k2[i]*halfdt;

          derivee(x_temp, pop, k3);
          for(int i=0; i<2; i++)
            x_temp[i] = pop->count[i] + k3[i]*dt;

          derivee(x_temp, pop, k4);
          for(int i=0; i<2; i++)
            pop->count[i] = pop->count[i] + (dt/6.0)*(k1[i] + 2.0*k2[i] + 2.0*k3[i] + k4[i]);
}
                 
    
int main(void)                 
{    
    double ti   =   0.00; 
    double dt   =   0.05;
    double tmax = 100.00; 
    
    Population *pred_prey = (Population *) malloc(sizeof(Population));
    pred_prey->count      = (double*) malloc(2*sizeof(double));
    pred_prey->birth_rate = (double*) malloc(2*sizeof(double));
    pred_prey->death_rate = (double*) malloc(2*sizeof(double));
    
    pred_prey->count[1]      = 15.00;  // predator count
    pred_prey->birth_rate[1] =  0.01;  // predator birth rate
    pred_prey->death_rate[1] =  1.00;  // predator death rate

    pred_prey->count[0]      = 100.00; // prey count
    pred_prey->birth_rate[0] =   2.00; // prey birth rate 
    pred_prey->death_rate[0] =   0.02; // prey death rate

    FILE* fichier = fopen("output", "w");
    // Add openacc directives
    while (ti < tmax)
    {
        ti += dt;
        rk4(pred_prey, dt);
        for(int i=0; i<2; i++)
        {
	    // Add openacc directives
	}
        fprintf(fichier, "%lf %s %lf %s %lf\n", ti,";", pred_prey->count[0],";",pred_prey->count[1]);
    }   
    fclose(fichier);
    free_pop(pred_prey);
    return 0;
}
