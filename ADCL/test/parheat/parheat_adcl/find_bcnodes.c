/* find_bnodes: find the indices of surface nodes and store them */
/*              the bnode array                                  */            
/*    input:  int grid[3]                                        */
/*            int start[3]                                       */
/*            int end[3]                                         */
/*            int neighbor[6]                                    */
/*            int *bnode                                         */
/*    output: int num_bcnodes                                    */
/*            int *bnode                                         */

#include "mpi.h"
#include "parheat.h"

int find_bcnodes( int *grid, int *start, int *end, 
                  int *neighbor, int *bnode, int *num_bcnodes )
{
    int i, j, k, index;

    *num_bcnodes = 0;

    /* check the xmin surface */
    if( neighbor[0] == MPI_PROC_NULL ) {
	for( j=start[1] ; j<=end[1] ; j++ ) {
	    for( k=start[2] ; k<=end[2] ; k++ ) {
		index = j * grid[2] + k;
		bnode[(*num_bcnodes)++] = index;
	    }
	}
    }

    /* check interior x planes */
    for( i=1 ; i<grid[0]-1 ; i++ )  {
	/* Is ymin boundary? */
	if( neighbor[1] == MPI_PROC_NULL ) {
	    for( k=start[2] ; k<=end[2] ; k++ ) {
		index = i * grid[1] * grid[2] + k;
		bnode[(*num_bcnodes)++] = index;
	    }
	}

        /* interior columns */
	for( j=1 ; j<grid[1]-1 ; j++ ) {
	    if( neighbor[2] == MPI_PROC_NULL ) {
		index = ( i * grid[1] + j ) * grid[2];
		bnode[(*num_bcnodes)++] = index;
	    }
	    if( neighbor[5] == MPI_PROC_NULL ) {
		index = ( i * grid[1] + j + 1 ) * grid[2] - 1;
		bnode[(*num_bcnodes)++] = index;
	    }
	}

	/* Is ymax boundary? */
	if( neighbor[4] == MPI_PROC_NULL ) {
	    for( k=start[2] ; k<=end[2] ; k++ ) {
		index = ( ( i + 1 ) * grid[1] - 1 ) * grid[2] + k;
		bnode[(*num_bcnodes)++] = index;
	    }
	}
    }

    /* check the xmax surface */
    if( neighbor[3] == MPI_PROC_NULL ) {
	for( j=start[1] ; j<=end[1] ; j++ ) {
	    for( k=start[2] ; k<=end[2] ; k++ ) {
		index = ( ( grid[0] - 1 ) * grid[1] + j ) * grid[2] + k;
		bnode[(*num_bcnodes)++] = index;
	    }
	}
    }

    return 0;
}
