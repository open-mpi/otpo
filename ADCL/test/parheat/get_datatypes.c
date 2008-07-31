/* get_datatypes: build the necessary datatypes */
/*                for sending the faces of the  */
/*                domain.                       */
/* input:  int grid[3]                          */
/*         int start[3]                         */
/*         int end[3]                           */
/*         MPI_Datatype faces[3]                */
/*         int msg_fac                          */

#include "mpi.h"

int get_datatypes( int *grid, int *start, int *end, \
                    MPI_Datatype *faces, int msg_fac )
{
  int count, blocklength;
  int stride;
  MPI_Aint extent, i;
  MPI_Datatype z_row, oneface[3];

  /* set up datatype for x_faces */
  count = end[1] - start[1] + 1;
  blocklength = end[2] - start[2] + 1;
  stride = grid[2];
  MPI_Type_vector( count, blocklength, stride, MPI_DOUBLE, \
                       &oneface[0] );

  /* set up datatype for y_faces */
  count = end[0] - start[0] + 1;
  blocklength = end[2] - start[2] + 1;
  stride = grid[1] * grid[2];
  MPI_Type_vector( count, blocklength, stride, MPI_DOUBLE, \
                       &oneface[1] );

  /* set up datatype for z_faces */
  count = end[1] - start[1] + 1;
  blocklength = 1;
  stride = grid[2];
  MPI_Type_vector( count, blocklength, stride, MPI_DOUBLE, \
                       &z_row );
  MPI_Type_commit( &z_row );

  count = end[0] - start[0] + 1;
  blocklength = 1;
  MPI_Type_extent( MPI_DOUBLE, &extent );
  extent = grid[1] * grid[2] * extent;
  MPI_Type_hvector( count, blocklength, extent, z_row, \
                       &oneface[2] );

  for( i=0 ; i<3 ; i++ )
  {
    MPI_Type_commit( &oneface[i] );
    MPI_Type_vector( msg_fac, 1, 0, oneface[i], &faces[i] ); 
    MPI_Type_commit( &faces[i] );
    MPI_Type_free( &oneface[i] );
  }

  /* Free the z_row Type */
  MPI_Type_free( &z_row );

  return 0;
}
