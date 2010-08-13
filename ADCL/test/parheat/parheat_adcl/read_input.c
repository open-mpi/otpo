#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"
#include "parheat.h"

int read_input( int *nb_of_problems, int ***grid, 
                int *mem_fac, int *msg_fac, int *cpt_fac, 
                double *accuracy, double *tstep_fac, double *c_fact,   
                double *min, double *max, int *max_iter )
{
    FILE *fp;
    int i;
    int **tgrid = NULL;

    fp = fopen( "parheat.in", "r" );
    fscanf( fp, "%d", nb_of_problems );
    tgrid = (int **)calloc(*nb_of_problems , sizeof(int *) );
    if ( NULL == tgrid) {
        return -1;
    }
    for (i=0; i<*nb_of_problems;i++) {  
        tgrid[i] = (int *)calloc(3 , sizeof(int) );
        if (NULL == tgrid[i] ) {
            return -1;
        }
        fscanf( fp, "%d %d %d", &(tgrid[i][0]), &(tgrid[i][1]), &(tgrid[i][2]) );
    }
    *grid = tgrid;

    fscanf( fp, "%d", mem_fac );
    fscanf( fp, "%d", msg_fac );
    fscanf( fp, "%d", cpt_fac );
    fscanf( fp, "%lf %lf %lf", accuracy, tstep_fac, c_fact );
    fscanf( fp, "%lf %lf", &min[0], &max[0] );
    fscanf( fp, "%lf %lf", &min[1], &max[1] );
    fscanf( fp, "%lf %lf", &min[2], &max[2] );
    /* Reading the max number of iterations */
    fscanf( fp, "%d", max_iter );
    fclose( fp );
#ifdef DEBUG
    printf( "done reading input file\n" );
#endif
  
    return 0;
}
