/* get_mesh_mem: allocates memory for k times the computational      */
/*               grid and returns a pointer to a list with pointers  */
/*               to each individual mesh.                            */
/*   input:  int k       # of times the mesh is created              */
/*           int grid[3] # of points in all directions               */
/*   output: struct point **setlist   pointer to list of sets        */
/*           int get_mesh_mem         =0 for ok, <>0 for error       */

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "parheat.h"


int get_mesh_mem( int k, int *grid, struct point ***setlist )
{
    struct point *set;
    int i, istep;
    int x, y, z;
    
    x = grid[0];
    y = grid[1];
    z = grid[2];
    
    *setlist = (struct point **)malloc( k*sizeof( struct point *) );
    if( *setlist == NULL ) {
	printf( "get_mesh_mem: Not enough memory for struct point *setlist[]\n" );
	return 1;
    }
    
    set = (struct point *)malloc( k*x*y*z*sizeof( struct point ));
    if( set == NULL ) {
	printf( "get_mesh_mem: Not enough memory for struct point set[]\n" );
	return 1;
    }
    
    istep = x*y*z;
    for( i=0 ; i<k ; i++ ) {
	*(*setlist+i) = set + i*istep;
    }
    for( i=0 ; i<k*x*y*z ; i++ ) {
	set[i].x = 0.0;
	set[i].y = 0.0;
	set[i].z = 0.0;
    }

  return 0;
}
