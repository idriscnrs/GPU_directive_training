#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
int main(void)
{
    // current position and value
    double x,y,x_p;
    // Number of divisions of the function
    int nsteps = 1e9;
    // x min
    double begin = 0.;
    // x max
    double end = M_PI;
    // Sum of elements
    double sum = 0.;
    // Length of the step
    double step_l = (end-begin)/nsteps;

    double dmin = DBL_MAX;
    double dmax = DBL_MIN;
#pragma acc parallel loop reduction(+:sum) reduction(min:dmin) reduction(max:dmax)
    for (int i=0 ; i < nsteps ; ++i )
    {
        x = i*step_l;
        x_p = (i+1)*step_l;
        y = (exp(x)+exp(x_p))/2;
        sum += y;
        if (y < dmin)
            dmin = y;
        if (y > dmax)
            dmax = y;
    }
    // Print the stats
    printf("The MINimum value of the function is: %f\n",dmin);
    printf("The MAXimum value of the function is: %f\n",dmax);
    printf("The integral of the function on [%f,%f] is: %f\n",begin,end,sum*step_l);
    printf("   difference is: %5.2e",exp(end)-exp(begin)-sum*step_l);
}
