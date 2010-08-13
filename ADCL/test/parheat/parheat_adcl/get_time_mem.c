/* get_time_mem: allocates memory for two solution steps for           */
/*               the whole domain.                                     */
/*    input:  int x                    # of points in x-direction      */
/*            int y                    # of points in y-direction      */
/*            int z                    # of points in z-direction      */
/*            struct tstep *solution    pointer to structure with       */
/*                                     pointers to time levels         */
/*    output: get_time_mem   =0 for ok, <>0 for error                  */

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "parheat.h"

int get_time_mem( int *grid, struct tstep *solution )
{
    int x, y, z;

    x = grid[0];
    y = grid[1];
    z = grid[2];

    (*solution).start = (double *)malloc( 2*x*y*z*sizeof( double ) );
    if( (*solution).start == NULL ) {
        printf( "get_time_mem: Not enough memory for solution->start\n" );
        return 1;
    }

    (*solution).old = (*solution).start;
    (*solution).neu = (*solution).start + x*y*z;
    return 0;
}
