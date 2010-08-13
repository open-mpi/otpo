/* write_step: writes the solution of one particular      */
/*             time step to a specified file              */
/*    input:  int grid[3]                                 */
/*            struct point *set                           */
/*            double *step                                */
/*            char *filename                              */
/*    output: int write_step     =0 for ok, <>0 for error */

#include <stdio.h>
#include "mpi.h"
#include "parheat.h"

int write_step( int *grid, struct point *set, \
                double *step, char *filename )
{
  int slice, i, j,k, index;
  int x, y, z;
  FILE *fp;

  x = grid[0];
  y = grid[1];
  z = grid[2];

  slice = x*y;

  printf( "Writing to file %s ...\n", filename );

  if( (fp = fopen( filename, "w" )) == NULL )
    {
    fprintf( fp, "write_step: couldn't open file %s to write!\n", filename );
    return 1;
    }

  index = 0;
  for( i=0 ; i<x ; i++ )
    {
    for( j=0 ; j<y ; j++ )
      {
      for( k=0 ; k<z ; k++ )
	{
        fprintf( fp, "%6d %10.5f %10.5f %10.5f %10.5f\n", \
               index, set[index].x, set[index].y, set[index].z, step[index] );
        index += 1;
        }
      fprintf( fp, "\n" );
      }
    fprintf( fp, "\n" );
    }

  fclose( fp );

  return 0;
}
