/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2009           HLRS. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include <stdio.h>

#include "ADCL.h"
#include "mpi.h"

void dump_vector_3D_mpi ( double ***data, int *dim, MPI_Comm comm );


/********************************************************************************************************************************/
void dump_vector_3D_mpi ( double ***data, int *dim, MPI_Comm comm )
/********************************************************************************************************************************/
{
    /* data - 3-dimensional array
       dims - array of dimensions of data
       comm - MPI communicator 
       purpose: prints 3d array data */
    int i, j, k, iproc;
    int rank, size;

    MPI_Comm_rank ( comm, &rank );
    MPI_Comm_size ( comm, &size );

    for (iproc=0; iproc<size; iproc++) {
        if ( iproc == rank ) {
            for (i=0; i<dim[0]; i++) {
                for ( j=0; j<dim[1]; j++ ) {
                    printf("Rank %d : dim[0]=%d dim[1]=%d ", rank, i, j);
                    for ( k=0; k<dim[2]; k++ ) {
                        printf("%lf ", data[i][j][k]);
                    }
                    printf ("\n");
                }
            }
        }
        MPI_Barrier( comm );
    }
    return;
}

