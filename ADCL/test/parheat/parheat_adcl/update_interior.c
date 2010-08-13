/* update_interior: computes the next time step for the */
/*                  interior of the domain              */
/*    input:  double c_fact                             */
/*            double delta_t                            */
/*            double delta_x                            */
/*            int grid[3]                               */
/*            int start[3]                              */
/*            int end[3]                                */
/*            double lambda                             */
/*            struct tstep *solution                    */
/*    output: int update_interior                       */

#include <stdio.h>
#include "parheat.h"

int update_interior( double c_fact, double delta_t, double delta_x,  \
            int *grid, int *start, int *end, \
            double lambda, struct tstep *solution )
{
    int i, j, k;
    int index;
    int xm1, xp1, ym1, yp1, zm1, zp1;
    double xdiff, ydiff, zdiff, diff;

    for( i=start[0]+1 ; i<end[0] ; i++ ) {
	for( j=start[1]+1 ; j<end[1] ; j++ ) {
	    for( k=start[2]+1 ; k<end[2] ; k++ ) {
		index = ( i * grid[1] + j ) * grid[2] + k;
		
		xm1 = index - grid[1] * grid[2];
		xp1 = index + grid[1] * grid[2];
		ym1 = index - grid[2];
		yp1 = index + grid[2];
		zm1 = index - 1;
		zp1 = index + 1;

		xdiff = (*solution).old[xp1] + (*solution).old[xm1] - 
		    (*solution).old[index]*2;
		
		ydiff = (*solution).old[yp1] + (*solution).old[ym1] -
		    (*solution).old[index]*2;
		
		zdiff = (*solution).old[zp1] + (*solution).old[zm1] -
		    (*solution).old[index]*2;
		
		diff = lambda*( xdiff + ydiff + zdiff );
		
		/* MIR */
		/* (*solution).neu[index] = (*solution).old[index] + diff; */
		(*solution).neu[index] = (*solution).old[index]*(1+delta_t*c_fact) + diff;
		/* MIR */
	    }
	}
    }

    return 0;
}

