/* get_coords: computes the coordinates for every point */
/*             and stores them                          */
/*    input:  int grid[3]                               */
/*            int start[3]                              */
/*            int end[3]                                */
/*            double lmin[3]                             */
/*            double lmax[3]                             */
/*            struct point *set                         */
/*    output: int get_coords                            */

#include "mpi.h"
#include "parheat.h"

int get_coords( int *grid, int *start, int *end, \
                double *lmin, double *lmax, struct point *set )
{
    double x, y, z; 
    double step[3];
    int i, j, k;
    int index;
    
    for( i=0 ; i<3 ; i++ )
	step[i] = ( lmax[i] - lmin[i] ) / (double)( end[i] - start[i] );
    
    x = lmin[0];
    for( i=start[0] ; i<=end[0] ; i++ )
    {
	y = lmin[1];
	for( j=start[1] ; j<=end[1] ; j++ )
	{
	    z = lmin[2];
	    for( k=start[2] ; k<=end[2] ; k++ )
	    {
		index = ( i * grid[1] + j ) * grid[2] + k;
		set[index].x = x;
		set[index].y = y;
		set[index].z = z;
		z += step[2];
	    }
	    y += step[1];
	}
	x += step[0];
    }
  
    return 0;
}
