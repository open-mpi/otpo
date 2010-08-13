/* apply_bc: apllies the boundary condition specified     */
/*           by the test problem to all the points        */
/*           that are listed in and therefore pointed     */
/*           to by the bnode list (i.e all surface nodes) */
/*    input:  int num_bnodes                              */
/*            struct point *set                           */
/*            double *step                                */
/*            int *bnode                                  */
/*    output: int apply_bc                                */

#include "mpi.h"
#include "parheat.h"

int apply_bc( int num_bnodes, struct point *set, \
              double *step, int *bnode )
{
    int i;
    double val;

    for( i=0 ; i<num_bnodes ; i++ ) {
	val = set[bnode[i]].x;
	step[bnode[i]] = val;
    }

    return 0;
}
