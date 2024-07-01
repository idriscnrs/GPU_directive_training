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
    #pragma acc exit data detach(pop->count, pop->birth_rate, pop->death_rate)
    #pragma acc exit data delete(pop->count, pop->birth_rate, pop->death_rate)    
    #pragma acc exit data delete(pop)
    free(pop->count);
    free(pop->birth_rate);
    free(pop->death_rate);
    free(pop);
}

void derivee(double* x, Population* pop, double* dx)
{
    #pragma acc serial present(pop, pop->birth_rate[0:2], pop->death_rate[0:2], x[0:2], dx[0:2])
    {
    dx[0] =  pop->birth_rate[0]*x[0] - pop->death_rate[0]*x[0]*x[1];
    dx[1] = -pop->death_rate[1]*x[1] + pop->birth_rate[1]*x[0]*x[1];       
    }
}

void rk4(Population* pop, double dt)
{
        double* x_temp = (double*) malloc(2*sizeof(double));
        double* k1     = (double*) malloc(2*sizeof(double));
        double* k2     = (double*) malloc(2*sizeof(double));
        double* k3     = (double*) malloc(2*sizeof(double));
        double* k4     = (double*) malloc(2*sizeof(double));
        double halfdt  = dt / 2.0;
 
        # pragma acc data create(k1[0:2], k2[0:2], k3[0:2], k4[0:2], x_temp[0:2]) present(pop, pop->birth_rate[0:2], pop->death_rate[0:2], pop->count[0:2])
        {

          #pragma acc parallel loop
          for(int i=0; i<2; i++)
             x_temp[i] = pop->count[i];

          derivee(x_temp, pop, k1);
          #pragma acc parallel loop
          for(int i=0; i<2; i++)
            x_temp[i] = pop->count[i] + k1[i]*halfdt;
        
          derivee(x_temp, pop, k2);
          #pragma acc parallel loop
          for(int i=0; i<2; i++)
            x_temp[i] = pop->count[i] + k2[i]*halfdt;

          derivee(x_temp, pop, k3);
          #pragma acc parallel loop
          for(int i=0; i<2; i++)
            x_temp[i] = pop->count[i] + k3[i]*dt;

          derivee(x_temp, pop, k4);
          #pragma acc parallel loop
          for(int i=0; i<2; i++)
            pop->count[i] = pop->count[i] + (dt/6.0)*(k1[i] + 2.0*k2[i] + 2.0*k3[i] + k4[i]);
        }
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
    pred_prey->death_rate[1] =   1.0;  // predator death rate

    pred_prey->count[0]      = 100.00; //prey count
    pred_prey->birth_rate[0] =   2.00; // prey birth rate 
    pred_prey->death_rate[0] =   0.02; // prey death rate

    #pragma acc enter data copyin(pred_prey->count[0:2], pred_prey->birth_rate[0:2], pred_prey->death_rate[0:2])    
    #pragma acc enter data copyin(pred_prey) attach(pred_prey->count, pred_prey->birth_rate, pred_prey->death_rate)

    FILE* fichier = fopen("output_solution", "w");
    while (ti < tmax)
    {
        ti += dt;
        rk4(pred_prey, dt);
        for(int i=0; i<2; i++)
        {
            #pragma acc update self(pred_prey->count[i:1])
	}
        fprintf(fichier, "%lf %s %lf %s %lf\n", ti, ";", pred_prey->count[0], ";", pred_prey->count[1]);
    }   
    fclose(fichier);
    free_pop(pred_prey);    
    return 0;
}
