/* get_domain: computes the number of grid points per */
/*             compute node in each direction and the */
/*             coordinate borders.                    */
/* input:  int grid[3]     Global no. of mesh points in each direction */
/*         int coords[3]   coordinates of this proc in the topology    */
/*         int dimlist[3]  extent of the process topolgoy              */
/*         int ndim        number of dimensions of the proc. topolgoy  */
/*         double min[3]   global start of the physical domain         */
/*         double max[3]   global end of the physical domain           */
/* output: int ppnode[3]   number of meshpoints on this proc  */
/*         double lmin[3]  local start of the physical domain */
/*         double lmax[3]  local end of the physical domain   */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "parheat.h"

int get_domain( int *grid, int *coords, int *dimlist, 
                int ndim, double *min, double *max,   
                int *ppnode, double *lmin, double *lmax )
{
    int i, j, rest, left, right;
    int num_middle, offset, *help;
    double h;
    
    help = (int *)malloc( dimlist[0]*sizeof( int ) );
    if( help==NULL ) {
	printf( "Not enough memory for %d integers.\n", dimlist[0] );
	MPI_Abort ( MPI_COMM_WORLD, 1);
    }
    
    for( i=0 ; i<3 ; i++ ) {
	num_middle = (int)( ( grid[i] - 2 ) / dimlist[i] );
	for( j=0 ; j<dimlist[i] ; j++ ) {
	    help[j] = num_middle;
        }
	help[0]++;
	help[dimlist[i]-1]++;
	rest = grid[i] - ( num_middle * dimlist[i] + 2 );
	right = (int)( rest / 2 );
	left = rest - right;

	for( j=0 ; j<left ; j++ ) {
	    help[j]++;
	}
	for( j=dimlist[i]-right ; j<dimlist[i] ; j++ ) {
	    help[j]++;
        }
	ppnode[i] = help[ coords[i] ];
	
	offset = 0;
	for( j=0 ; j<coords[i] ; j++ ) {
	    offset += help[j];
        }
	h = ( max[i] - min[i] ) / (double)( grid[i] -1 );
	lmin[i] = h * offset;
	lmax[i] = lmin[i] + h * ( ppnode[i] - 1 );
    }
    
    free( (void *)help );
    
  return 0;
}
