/* get_offset: computes offsets for send and receive */
/*             operations.                           */
/* input:  int grid[3]                               */
/*         int start[3]                              */
/*         int end[3]                                */
/*         int neighbor[6]                           */
/*         int s_off[6]                              */
/*         int r_off[6]                              */
/* output: int get_offset                            */

#include "mpi.h"

int get_offset( int *grid, int *start, int *end, \
               int *neighbor, int *s_off, int *r_off )
{
  int i;

  for( i=0 ; i<6 ; i++ )
  {
    s_off[i] = 0;
    r_off[i] = 0;
  }

  /*                              */
  /* for the x_min face           */
  /*                              */
  if( neighbor[0] != MPI_PROC_NULL )
  {
    s_off[0] = ( grid[1] + start[1] ) * grid[2] + start[2];
    r_off[0] =             start[1]   * grid[2] + start[2];
  }

  /*                              */
  /* for the y_min face           */
  /*                              */
  if( neighbor[1] != MPI_PROC_NULL )
  {
    s_off[1] = ( start[0] * grid[1] + 1 ) * grid[2] + start[2];
    r_off[1] =   start[0] * grid[1]       * grid[2] + start[2];
  }

  /*                              */
  /* for the z_min face           */
  /*                              */
  if( neighbor[2] != MPI_PROC_NULL )
  {
    s_off[2] = ( start[0] * grid[1] + start[1] ) * grid[2] + 1;
    r_off[2] = ( start[0] * grid[1] + start[1] ) * grid[2];
  }

  /*                              */
  /* for the x_max face           */
  /*                              */
  if( neighbor[3] != MPI_PROC_NULL )
  {
    s_off[3] = ( grid[1] *   end[0]       + start[1] ) * grid[2] + start[2];
    r_off[3] = ( grid[1] * ( end[0] + 1 ) + start[1] ) * grid[2] + start[2];
  }

  /*                              */
  /* for the y_max face           */
  /*                              */
  if( neighbor[4] != MPI_PROC_NULL )
  {
    s_off[4] = ( start[0] * grid[1] +   end[1]       ) * grid[2] + start[2];
    r_off[4] = ( start[0] * grid[1] + ( end[1] + 1 ) ) * grid[2] + start[2];
  }

  /*                              */
  /* for the z_max face           */
  /*                              */
  if( neighbor[5] != MPI_PROC_NULL )
  {
    s_off[5] = ( start[0] * grid[1] + start[1] ) * grid[2] + end[2];
    r_off[5] = ( start[0] * grid[1] + start[1] ) * grid[2] + end[2] + 1;
  }

  return 0;
}
