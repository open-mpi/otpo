/* get_dimlist.c: breaks down the node numbers to a set of   */
/*                prime numbers to make a good topology      */
/* input:  int num_nodes                                     */
/*         int *dim                                          */
/* output: int get_dimlist                                   */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"
#include "parheat.h"

int get_dimlist( int num_nodes, int *ndim, int *dim )
{
    *ndim = 3;
    dim[0] = 0;
    dim[1] = 0;
    dim[2] = 0;

    MPI_Dims_create ( num_nodes, *ndim, dim );
    return 0;
}
