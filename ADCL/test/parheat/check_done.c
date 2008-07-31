/* check_done: check, if steady state is already */
/*             reached                           */
/* input:  double eps                            */
/*         int *flag                             */
/*         int grid[3]                           */
/*         int start[3]                          */
/*         int end[3]                            */
/*         double *value                         */
/*         struct point *set                     */
/* output: int check_done                        */

#include <stdio.h>
#include <math.h>
#include "parheat.h"

int check_done( double eps, int *flag, int *grid, int *start, \
                int *end, struct tstep *solution, struct point *set )
{
    int i, j, k;
    int index;
    int count = 0;
    double diff;
    
    *flag = 0;
    for( i=start[0] ; i<=end[0]&&count==0 ; i++ )
    {
	for( j=start[1] ; j<=end[1]&&count==0 ; j++ )
	{
	    for( k=start[2] ; k<=end[2] ; k++ )
	    {
		index = ( i * grid[1] + j ) * grid[2] + k;
		diff = fabs( (*solution).neu[index] - (*solution).old[index] ); 
		if( diff > eps ) 
		{
		    count++;
		    break;
		}
	    }
	}
    }

    if( count == 0 ) *flag = 1;
    return 0;
}
