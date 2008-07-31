#include <stdio.h>
#include "mpi.h"
#include "parheat.h"

int read_input( int *grid, 
                int *mem_fac, int *msg_fac, int *cpt_fac, 
                double *accuracy, double *tstep_fac, double *c_fact,   
                double *min, double *max )
{
  FILE *fp;

  fp = fopen( "parheat.in", "r" );
  fscanf( fp, "%d %d %d", &grid[0], &grid[1], &grid[2] );
  fscanf( fp, "%d", mem_fac );
  fscanf( fp, "%d", msg_fac );
  fscanf( fp, "%d", cpt_fac );
  fscanf( fp, "%lf %lf %lf", accuracy, tstep_fac, c_fact );
  fscanf( fp, "%lf %lf", &min[0], &max[0] );
  fscanf( fp, "%lf %lf", &min[1], &max[1] );
  fscanf( fp, "%lf %lf", &min[2], &max[2] );
  fclose( fp );
#ifdef DEBUG
  printf( "done reading input file\n" );
#endif
  return 0;
} 
