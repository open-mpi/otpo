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

void dump_vector_4D_mpi ( double ****data, int *dim, int nc, MPI_Comm comm );

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
void dump_vector_4D_mpi ( double ****data, int *dim, int nc, MPI_Comm comm )
{
    int i, j, k, l, iproc;
    int rank, size;

    MPI_Comm_rank ( comm, &rank );
    MPI_Comm_size ( comm, &size );

    for (iproc=0; iproc<size; iproc++) {
        if ( iproc == rank ) {
            for ( k=0; k<dim[2]; k++ ) {
                for ( j=0; j<dim[1]; j++ ) {
                    printf("Rank %d dim[2]=%d dim[1]=%d: ", rank, k, j);
                    for (i=0; i<dim[0]; i++) {
                        for ( l=0; l<nc; l++ ) {
                            printf("%lf ", data[i][j][k][l]);
                        }
                    }
                    printf ("\n");
                }
            }
        }
        MPI_Barrier( comm );
    }

    return;
}

