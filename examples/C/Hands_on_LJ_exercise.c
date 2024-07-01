#include <stdio.h>
#include <stdlib.h>
#include <string.h>
# include <math.h>
#include <float.h>
// Mass of the atoms (only 1 kind and normalized)
const double mass = 1.0;
// Boltzmann constant
const double kb = 0.831451115;

typedef struct
{
    size_t size;
    double* data;
} array;

typedef struct
{
    array* Fx;
    array* Fy;
    array* Fz;
} Forces;

typedef struct
{
    array* vx;
    array* vy;
    array* vz;
    array* x;
    array* y;
    array* z;
} dynamic;

typedef struct
{
    size_t nsteps; // number of steps
    size_t dump_dyn; // number of steps between dumps
    char dump_file[20]; // number of steps between dumps
    double dt; // time step
    double lattice_length; // length of the box
    double Berendsen_T; // Temperature of the thermostat
    double Berendsen_coupling; // Temperature of the thermostat
    size_t NAtoms; // Number of atoms
    double LJ_sigma; // sigma parameter of the Lennard-Jones potential
    double LJ_epsilon; // epsilon parameter of the Lennard-Jones potential
    double LJ_cutoff; // cutoff of the Lennard-Jones potential
    double LJ_tolerance; // cutoff of the Lennard-Jones potential
} Config;

/**
 * Management of the dynamic struct
 */
dynamic* initialize_dyn(Config* conf, int random); //done
void free_dyn(dynamic* dyn); //done
array* allocate_array(size_t size); //done
void free_array(array* ar); //done

/**
 * Dynamic
 */
void velocity_verlet(dynamic* dyn, Forces* forces, Config* conf); //done
double LJ_pot(double rij2, double sigma2, double epsilon2);// done
void forces_from_LJ(dynamic* dyn, Forces* forces, Config* conf); //done
double berendsen_thermostat(dynamic* dyn, Config*);
void sd(dynamic* dyn,
        Forces* forces,
        Config* conf, 
        double step_length, 
        double threshold, 
        size_t max_steps);
double stat_forces(Forces* forces, Config* conf);

#pragma acc routine seq
inline double LJ_pot(double rij2, double sigma2, double epsilon)
{
    return epsilon * pow((2.0 * (sigma2/rij2)), 6) - pow((2.0 * (sigma2/rij2)), 3);
}

void forces_from_LJ(dynamic* dyn, Forces* forces, Config* conf)
{
    double sigma2 = conf->LJ_sigma*conf->LJ_sigma;
    double rij2, xij, yij, zij = 0.0;
    double potential_energy=0.0;

    for (size_t i=0; i<conf->NAtoms; ++i)
    {
        forces->Fx->data[i] = 0.;
        forces->Fy->data[i] = 0.;
        forces->Fz->data[i] = 0.; 
    }

    for (size_t i=0; i<conf->NAtoms; ++i)
        for (size_t j=0; j<conf->NAtoms; ++j)
        {
            xij =  (dyn->x->data[j] - dyn->x->data[i]);
            yij =  (dyn->y->data[j] - dyn->y->data[i]);
            zij =  (dyn->z->data[j] - dyn->z->data[i]);
            // Apply Periodic Boundary Conditions
            xij -= floor(xij/conf->lattice_length + 0.5) *conf->lattice_length;
            yij -= floor(yij/conf->lattice_length + 0.5) *conf->lattice_length;
            zij -= floor(zij/conf->lattice_length + 0.5) *conf->lattice_length;
            rij2 = xij*xij + yij*yij + zij*zij;

            if ((rij2 > conf->LJ_tolerance) && (rij2 < conf->LJ_cutoff * conf->LJ_cutoff))
            {
                potential_energy += LJ_pot(rij2, sigma2, conf->LJ_epsilon);
                forces->Fx->data[i] += 24.0*LJ_pot(rij2, sigma2, conf->LJ_epsilon)*xij/sqrt(rij2);
                forces->Fy->data[i] += 24.0*LJ_pot(rij2, sigma2, conf->LJ_epsilon)*yij/sqrt(rij2);
                forces->Fz->data[i] += 24.0*LJ_pot(rij2, sigma2, conf->LJ_epsilon)*zij/sqrt(rij2);
                forces->Fx->data[j] -= 24.0*LJ_pot(rij2, sigma2, conf->LJ_epsilon)*xij/sqrt(rij2);
                forces->Fy->data[j] -= 24.0*LJ_pot(rij2, sigma2, conf->LJ_epsilon)*yij/sqrt(rij2);
                forces->Fz->data[j] -= 24.0*LJ_pot(rij2, sigma2, conf->LJ_epsilon)*zij/sqrt(rij2);
            }
        }
    printf("Epot= %15.5e ", potential_energy);
}


double stat_forces(Forces* forces, Config* conf)
{
    double Fmax = 0.;
    double Fmin = DBL_MAX;
    double Fnorm = 0.;
    double F = 0.;
    for (int i =0; i< conf->NAtoms; ++i)
    {
        F = forces->Fx->data[i]*forces->Fx->data[i]+
            forces->Fy->data[i]*forces->Fy->data[i]+
            forces->Fz->data[i]*forces->Fz->data[i];
        if (F < Fmin) Fmin = F;
        if (F > Fmax) Fmax = F;
        Fnorm += F;
    }
    printf("<F>= %10.3e min(F)= %10.3e max(F)= %10.3e ", sqrt(Fnorm)/conf->NAtoms, sqrt(Fmin), sqrt(Fmax));
    return sqrt(Fnorm)/conf->NAtoms;
}

void velocity_verlet(dynamic* dyn, Forces* forces, Config* conf)
{
    for (size_t i=0; i < conf->NAtoms; ++i)
    {
        dyn->vx->data[i] += 0.5 * conf->dt * forces->Fx->data[i];
        dyn->vy->data[i] += 0.5 * conf->dt * forces->Fy->data[i];
        dyn->vz->data[i] += 0.5 * conf->dt * forces->Fz->data[i];

        dyn->x->data[i] += conf->dt*dyn->vx->data[i];
        dyn->y->data[i] += conf->dt*dyn->vy->data[i];
        dyn->z->data[i] += conf->dt*dyn->vz->data[i];

        // Apply the Periodic Boundary Conditions
        dyn->x->data[i] -= floor(dyn->x->data[i]/conf->lattice_length + 0.5) * conf->lattice_length;
        dyn->y->data[i] -= floor(dyn->y->data[i]/conf->lattice_length + 0.5) * conf->lattice_length;
        dyn->z->data[i] -= floor(dyn->z->data[i]/conf->lattice_length + 0.5) * conf->lattice_length;
    }

    forces_from_LJ(dyn, forces, conf);

    for (size_t i=0; i < conf->NAtoms; ++i)
    {

        dyn->vx->data[i] += 0.5 * conf->dt * forces->Fx->data[i];
        dyn->vy->data[i] += 0.5 * conf->dt * forces->Fy->data[i];
        dyn->vz->data[i] += 0.5 * conf->dt * forces->Fz->data[i];
    }
}

/**
 * Read/write configuration
 */
Config* read_params(char* filepath); //done
void read_initial(char* filepath, dynamic* dyn);
void write_step(char* filepath, dynamic* dyn);

void free_array(array* ar)
{
    free(ar->data);
    free(ar);
}

array* allocate_array(size_t size)
{
    array* ar = (array*) malloc(sizeof(array));
    ar->size = size;
    ar->data = (double*) malloc(size*sizeof(double));
    
    return ar;
}

void free_forces(Forces* forces)
{
    free_array(forces->Fx);
    free_array(forces->Fy);
    free_array(forces->Fz);
    
    free(forces);
}

void free_dyn(dynamic* dyn)
{
    free_array(dyn->x);
    free_array(dyn->y);
    free_array(dyn->z);
    free_array(dyn->vx);
    free_array(dyn->vy);
    free_array(dyn->vz);
    
    free(dyn);
}

void update_array(array* ar, size_t size, int gpu)
{
    if (gpu)
    {
        
    }
    else
    {
        
    }
}

void update_dyn(dynamic* dyn, Config* conf, int gpu)
{
    update_array(dyn->x, conf->NAtoms, gpu);
    update_array(dyn->y, conf->NAtoms, gpu);
    update_array(dyn->z, conf->NAtoms, gpu);
    update_array(dyn->vx, conf->NAtoms, gpu);
    update_array(dyn->vy, conf->NAtoms, gpu);
    update_array(dyn->vz, conf->NAtoms, gpu);
}

/**
 * Initialize the structures for the dynamic
 * If random is >0 we generate a grid on which we place the atoms
 */
dynamic* initialize_dyn(Config* conf, int random)
{
    size_t id = 0;
    size_t n = floor(pow(conf->NAtoms,1./3.))+1;
    size_t leftover = conf->NAtoms - n*n*n;
    printf("%d %d %d\n", leftover, leftover/n/n, leftover%(n*n));
    double s = conf->lattice_length/(double) n;
    dynamic* dyn = (dynamic*) malloc(sizeof(dynamic));
    
    dyn->x  = allocate_array(conf->NAtoms);
    dyn->y  = allocate_array(conf->NAtoms);
    dyn->z  = allocate_array(conf->NAtoms);
    dyn->vx = allocate_array(conf->NAtoms);
    dyn->vy = allocate_array(conf->NAtoms);
    dyn->vz = allocate_array(conf->NAtoms);
    if (random > 0)
    {
        srand(47329);
        for (size_t i=0; i<n; ++i)
        {
            for (size_t j=0; j<n; ++j)
            {
                for (size_t k=0; k<n; ++k)
                {
                    id = i*n*n + j*n + k;
                    if (id >= conf->NAtoms) break;
                    dyn->x->data[id] = s*((double)i + 0.5) + (double)rand()/RAND_MAX * 0.3*s;
                    dyn->y->data[id] = s*((double)j + 0.5) + (double)rand()/RAND_MAX * 0.3*s;
                    dyn->z->data[id] = s*((double)k + 0.5) + (double)rand()/RAND_MAX * 0.3*s;
                    dyn->vx->data[id] = 0.;//(double)rand()/RAND_MAX * 5.0 - 2.5;
                    dyn->vy->data[id] = 0.;//(double)rand()/RAND_MAX * 5.0 - 2.5;
                    dyn->vz->data[id] = 0.;//(double)rand()/RAND_MAX * 5.0 - 2.5;
                }
                if (id >= conf->NAtoms) break;
            }
            if (id >= conf->NAtoms) break;
        }
        // Apply PBC
        for (int i=0; i<conf->NAtoms; ++i)
        {
            dyn->x->data[i] -= floor(dyn->x->data[i]/conf->lattice_length + 0.5) * conf->lattice_length;
            dyn->y->data[i] -= floor(dyn->y->data[i]/conf->lattice_length + 0.5) * conf->lattice_length;
            dyn->z->data[i] -= floor(dyn->z->data[i]/conf->lattice_length + 0.5) * conf->lattice_length;
        }
    }

    int gpu=1;
    update_dyn(dyn, conf, gpu);
    return dyn;
}

/**
 * Initialize Forces
 */
Forces* initialize_forces(Config* conf)
{
    Forces* forces = (Forces*) malloc(sizeof(Forces));
    
    forces->Fx = allocate_array(conf->NAtoms);
    forces->Fy = allocate_array(conf->NAtoms);
    forces->Fz = allocate_array(conf->NAtoms);

    for (size_t i=0; i<conf->NAtoms; ++i)
    {
        forces->Fx->data[i] = 0.;
        forces->Fy->data[i] = 0.;
        forces->Fz->data[i] = 0.;
    }
    return forces;
}

/**
 * Read the configuration
 */
Config* read_params(char* filepath)
{
    FILE* fp = fopen(filepath, "r");
    char* line = NULL;
    size_t len = 0;
    char key[20], val[20];
    
    Config* conf = (Config*) malloc(sizeof(Config));
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((getline(&line, &len, fp)) != -1)
    {
        sscanf(line, "%s %s", key, val);
        if (strcmp(key, "T") == 0) 
        {
            conf->Berendsen_T = atof(val);
        } else if (strcmp(key, "nsteps") == 0){
            conf->nsteps = atoi(val);
        } else if (strcmp(key, "dump_dyn") == 0){
            conf->dump_dyn = atoi(val);
        } else if (strcmp(key, "dump_file") == 0){
             strcpy(conf->dump_file, val);
        } else if (strcmp(key, "dt") == 0){
            conf->dt = atof(val);
        } else if (strcmp(key, "tau") == 0){
            conf->Berendsen_coupling = atof(val);
        } else if (strcmp(key, "lattice") == 0){
            conf->lattice_length = atof(val);
        } else if (strcmp(key, "LJ_sigma") == 0){
            conf->LJ_sigma = atof(val);
        } else if (strcmp(key, "LJ_epsilon") == 0){
            conf->LJ_epsilon = atof(val);
        } else if (strcmp(key, "LJ_cutoff") == 0){
            conf->LJ_cutoff = atof(val);
        } else if (strcmp(key, "LJ_tolerance") == 0){
            conf->LJ_tolerance = atof(val);
        } else if (strcmp(key, "natoms") == 0){
            conf->NAtoms = atoi(val);
        }
    } 
    fclose(fp);
    
    return conf;
}

void print_conf(Config* conf)
{
    printf("natoms %d\n", conf->NAtoms);
    printf("dt %f\n", conf->dt);
    printf("nsteps %d\n", conf->nsteps);
    printf("tau %f\n", conf->Berendsen_coupling);
    printf("T %f\n", conf->Berendsen_T);
    printf("lattice %f\n", conf->lattice_length);
    printf("LJ_sigma %f\n", conf->LJ_sigma);
    printf("LJ_epsilon %f\n", conf->LJ_epsilon);
    printf("LJ_cutoff %f\n", conf->LJ_cutoff);
}

void dump_dyn(dynamic* dyn, Config* conf, char* mode)
{
    FILE* fp = fopen(conf->dump_file, mode);
    fprintf(fp, "%d\n", conf->NAtoms);
    fprintf(fp, "%10.5f\n", conf->lattice_length);
    for (int i=0; i<conf->NAtoms; ++i)
    {
        fprintf(fp, "Ne %15.10f %15.10f %15.10f %15.8e %15.8e %15.8e\n", 
                dyn->x->data[i], dyn->y->data[i], dyn->z->data[i],
                dyn->vx->data[i], dyn->vy->data[i], dyn->vz->data[i]);
    }
    fclose(fp);
}

double berendsen_thermostat(dynamic* dyn, Config* conf)
{
    double kinetic_E = 0.0;

    for (size_t i=0; i<conf->NAtoms; ++i)
        kinetic_E += 1.0 * (dyn->vx->data[i]*dyn->vx->data[i])
                         + (dyn->vy->data[i]*dyn->vy->data[i])
                         + (dyn->vz->data[i]*dyn->vz->data[i]);

    kinetic_E *= 0.5;
    double T = 2.0 * kb * kinetic_E/(3.0 * conf->NAtoms -3);
    double lambda_scaling = sqrt(1 + (conf->dt/conf->Berendsen_coupling) * (conf->Berendsen_T/T-1));
    printf("T= %15.6e l= %10.3e ", T, lambda_scaling);

    for (size_t i=0; i < conf->NAtoms; ++i)
    {
        dyn->vx->data[i] *= lambda_scaling;
        dyn->vy->data[i] *= lambda_scaling;
        dyn->vz->data[i] *= lambda_scaling;
    }

    return T;
}

int main(int argc, char** argv)
{
    double T;
    int cpu=0;
    Config* conf = read_params("conf.dat"); 
    dynamic* dyn = initialize_dyn(conf, 1);
    Forces* forces = initialize_forces(conf);
    forces_from_LJ(dyn, forces, conf);
    dump_dyn(dyn, conf, "w");
//    sd(dyn, forces, conf, 0.0001, 0.0001, 2000);
    for (int i=0; i<conf->nsteps; ++i)
    {
        printf("Step %6d ",i);
        velocity_verlet(dyn, forces, conf);
        stat_forces(forces, conf);
        T = berendsen_thermostat(dyn, conf);
        if (i > 100 && T > conf->Berendsen_T*1000)
        {
            fprintf(stderr, "Oups something went wrong with T\n");
            break;
        }
        if (i%100 == 0)
        {
            update_dyn(dyn, conf, cpu);
            dump_dyn(dyn, conf, "a");
        }
        printf("\n");
    }
    update_dyn(dyn, conf, cpu);
    dump_dyn(dyn, conf, "a");
    free_dyn(dyn);
    free_forces(forces);
    return 0;
}
