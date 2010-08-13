/*
 * Copyright (c) 2009           HLRS. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include <stdio.h>
#include <string.h>

#include "ADCL.h"
#include "ADCL_internal.h"
#include "mpi.h"

void dump_vector_1D ( double *data, int rank, int dim);
void dump_vector_1D_mpi ( double *data, int dim, MPI_Comm comm );
void set_data_1D ( double *data, int rank, int dim);
int check_data_1D ( double *data, int* rcounts, int *rdispl, int rank, int size);

/**********************************************************************/
/**********************************************************************/
int check_data_1D ( double *data, int *rcounts, int *rdispl, int rank, int size) 
/**********************************************************************/
/**********************************************************************/
{
    int proc, j;
    int err = 0, gerr = 0; 

    for ( proc=0; proc<size; proc++) {
       for (j=0; j<rcounts[proc]; j++){
           if (data[ rdispl[proc]+j ] != proc ){
               printf("Wrong data: proc %d, pos %d, value %lf, expected value %lf\n", 
	          proc, rdispl[proc]+j, data[ rdispl[proc]+j ], (double) proc);
	       err++;
	   }
       }
    }

    MPI_Allreduce ( &err, &gerr, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
    if ( gerr == 0 ) {
	//if ( rank == 0 ) printf("1-D testsuite passed\n");
    }
    else {
	if ( rank == 0 ) printf("1-D testsuite failed\n");
	err = 1;
    }

    return err;
}


/**********************************************************************/
/**********************************************************************/
void set_data_1D ( double *data, int value, int dim) 
/**********************************************************************/
/**********************************************************************/
{
    int i;

    for ( i=0; i<dim; i++) {
	data[i] = value;
    }

    return;
}

/**********************************************************************/
/**********************************************************************/
void dump_vector_1D ( double *data, int rank, int dim)
/**********************************************************************/
/**********************************************************************/
{
    int i;
    
    printf("%d : ", rank);
    for (i=0; i<dim; i++) {
	printf("%lf ", data[i]);
    }
    printf ("\n");

    return;
}


/**********************************************************************/
void dump_vector_1D_mpi ( double *data, int dim, MPI_Comm comm )
/**********************************************************************/
{
    int i, iproc;
    int rank, size;

    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    for ( iproc=0; iproc<size; iproc++ ) {
        if ( iproc == rank ) {
            printf("%d : ", rank);
            for ( i=0; i<dim; i++ ) {
                printf("%lf ", data[i]);
            }
            printf ("\n");
        }
        MPI_Barrier ( comm );
    }

    return;
}


