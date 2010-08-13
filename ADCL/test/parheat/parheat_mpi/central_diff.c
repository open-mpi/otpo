/* central_diff: computes the new value for the solution*/
/*               at position index.                     */
/*    input:  double c_fact                             */
/*            double delta_t                            */
/*            double delta_x                            */
/*            int grid[3]                               */
/*            int index                                 */
/*            double lambda                             */
/*            struct tstep *solution                    */
/*    output: int central_diff                          */

#include "parheat.h"

int central_diff( double c_fact, double delta_t, double delta_x, 
                  int *grid, int index, double lambda, 
                  struct tstep *solution )
{
    int xm1, xp1, ym1, yp1, zm1, zp1;
    double xdiff, ydiff, zdiff, diff;
    
    xm1 = index - grid[1] * grid[2];
    xp1 = index + grid[1] * grid[2];
    ym1 = index - grid[2];
    yp1 = index + grid[2];
    zm1 = index - 1;
    zp1 = index + 1;
    
    xdiff = (*solution).old[xp1] + (*solution).old[xm1] -	\
	(*solution).old[index]*2;
    ydiff = (*solution).old[yp1] + (*solution).old[ym1] -	\
	(*solution).old[index]*2;
    zdiff = (*solution).old[zp1] + (*solution).old[zm1] -	\
	(*solution).old[index]*2;
    
    diff = lambda*( xdiff + ydiff + zdiff );
    
    
    /* MIR */
    /*(*solution).neu[index] = (*solution).old[index] + diff; */
    (*solution).neu[index] = (*solution).old[index]*(1+delta_t*c_fact) + diff;
    /* MIR */

    return 0;
}
