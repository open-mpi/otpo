/* get_bnode_mem: computes the number of surface nodes and allocates */
/*                the necessary memory for a list containing         */
/*                the indices to these nodes                         */
/*    input:  int num_bnodes                                         */
/*            int **mem                                              */
/*    output: int get_bnode_mem    =0 for ok, <>0 for error          */

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "parheat.h"

int get_bcnode_mem( int num_bnodes, int **mem )
{
    if( num_bnodes != 0 ) {
        *mem = (int *)malloc( num_bnodes * sizeof(int) );
        if( *mem == NULL ) {
            printf( "get_bnode_mem: Not enough memory for list of boundary nodes\n" );
            return 1;
        }
    }
    return 0;
}
