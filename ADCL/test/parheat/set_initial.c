/* set_initial: stores initial values on all points of the      */
/*              computational grid                              */
/*              (very simple and dumb routine)                  */
/*    input:  int grid[3]         # of points in all directions */
/*            double *domain      Pointer to the memory         */
/*    output: int set_initial     =0 for ok, <>0 for error      */

#include "mpi.h"
#include "parheat.h"

int set_initial( int *grid, double *mem )
{
    int i, imax;

    imax = grid[0]*grid[1]*grid[2];
    for( i=0 ; i<imax ; i++ )
    {
	mem[i] = 0.0;
    }
    return 0;
}

