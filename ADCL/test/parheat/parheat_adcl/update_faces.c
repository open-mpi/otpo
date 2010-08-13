/* update_faces: computes the next time step for the    */
/*               faces of the cube                      */
/*    input:  double c_fact                             */
/*            double delta_t                            */
/*            double delta_x                            */
/*            int grid[3]                               */
/*            int start[3]                              */
/*            int end[3]                                */
/*            int neighbor[6]                           */
/*            double lambda                             */
/*            struct tstep *solution                    */
/*    output: int update_faces                          */

#include "mpi.h"
#include "parheat.h"

int update_faces( double c_fact, double delta_t, double delta_x, 
                  int *grid, int *start, int *end, int *neighbor,
                  double lambda, struct tstep *solution )
{
    int i, j, k;
    int ymax, zmax;
    int index;
    
    /*                              */
    /* xmin_face                    */
    /*                              */
    if( neighbor[0] != MPI_PROC_NULL ) {
	ymax = grid[1] - 1;
	zmax = grid[2] - 1;
	for( j=1 ; j<ymax ; j++ ) {
	    for( k=1 ; k<zmax ; k++ ) {
		index = ( start[0] * grid[1] + j ) * grid[2] + k;
		central_diff( c_fact, delta_t, delta_x, grid, index, lambda, solution );
	    }
	}
    }

    /*                              */
    /* ymin_face                    */
    /*                              */
    if( neighbor[1] != MPI_PROC_NULL ) {
	zmax = grid[2] - 1;
	for( i=start[0]+1 ; i<end[0] ; i++ ) {
	    for( k=1 ; k<zmax ; k++ ) {
		index = ( i * grid[1] + start[1] ) * grid[2] + k;
		central_diff( c_fact, delta_t, delta_x, grid, index, lambda, solution );
	    }
	}
    }
    
    /*                              */
    /* zmin_face                    */
    /*                              */
    if( neighbor[2] != MPI_PROC_NULL ) {
	for( i=start[0]+1 ; i<end[0] ; i++ ) {
	    for( j=start[1]+1 ; j<end[1] ; j++ ) {
		index = ( i * grid[1] + j ) * grid[2] + start[2];
		central_diff( c_fact, delta_t, delta_x, grid, index, lambda, solution );
	    }
	}
    }
    
    /*                              */
    /* xmax_face                    */
    /*                              */
    if( neighbor[3] != MPI_PROC_NULL ) {
	ymax = grid[1] - 1;
	zmax = grid[2] - 1;
	for( j=1 ; j<ymax ; j++ ) {
	    for( k=1 ; k<zmax ; k++ ) {
                index = ( end[0] * grid[1] + j ) * grid[2] + k;
                central_diff( c_fact, delta_t, delta_x, grid, index, lambda, solution );
            }
        }
    }
    
    /*                              */
    /* ymax_face                    */
    /*                              */
    if( neighbor[4] != MPI_PROC_NULL ) {
	zmax = grid[2] - 1;
	for( i=start[0]+1 ; i<end[0] ; i++ ) {
	    for( k=1 ; k<zmax ; k++ ) {
		index = ( i * grid[1] + end[1] ) * grid[2] + k;
		central_diff( c_fact, delta_t, delta_x, grid, index, lambda, solution );
	    }
	}
    }
    
    /*                              */
    /* zmax_face                    */
    /*                              */
    if( neighbor[5] != MPI_PROC_NULL ) {
	for( i=start[0]+1 ; i<end[0] ; i++ ) {
	    for( j=start[1]+1 ; j<end[1] ; j++ ) {
		index = ( i * grid[1] + j ) * grid[2] + end[2];
		central_diff( c_fact, delta_t, delta_x, grid, index, lambda, solution );
	    }
	}
    }

    return 0;
}

