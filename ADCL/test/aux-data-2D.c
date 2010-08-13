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



void dump_vector_2D ( double **data, int rank, int *dim);
void dump_vector_2D_mpi ( double **data, int *dim, MPI_Comm comm );
void dump_vector_2D_plus_nc_mpi ( double ***data, int *dim, int nc, MPI_Comm comm ); 
void set_data_2D    ( double **data, int rank, int *dim, int hwidth );
int check_data_2D  ( double **data, int rank, int *dim, 
			    int hwidth, int *neighbors ); 

void dump_vector_2D_int ( int **data, int rank, int *dim);
void set_data_2D_int    ( int **data, int rank, int *dim, int hwidth );
void check_data_2D_int  ( int **data, int rank, int *dim, 
			    int hwidth, int *neighbors ); 

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int check_data_2D ( double **data, int rank, int *dim, int hwidth, 
			    int *neighbors)
{
    int i, j, lres=1, gres;
    double should_be;


    should_be = neighbors[0]==MPI_PROC_NULL ? -1 : neighbors[0];
    for ( j=hwidth; j<dim[1]-hwidth; j++ ) {
	for ( i=0; i<hwidth; i++ ) {
	    if ( data[i][j] != should_be ){
		lres = 0;
	    }
	}
    }

    should_be = neighbors[1]==MPI_PROC_NULL ? -1 : neighbors[1];
    for ( j=hwidth; j<dim[1]-hwidth; j++ ) {
	for ( i=dim[0]-hwidth; i<dim[0]; i++ ) {
	    if ( data[i][j] != should_be ) {
		lres = 0;
	    }
	}
    }
    should_be = neighbors[2]==MPI_PROC_NULL ? -1 : neighbors[2];
    for (i=hwidth; i<dim[0]-hwidth; i++ ) {
	for (j=0; j<hwidth; j++ ){
	    if ( data[i][j] != should_be ) {
		lres = 0;
	    }
	}
    }

    should_be = neighbors[3]==MPI_PROC_NULL ? -1 : neighbors[3];
    for (i=hwidth; i<dim[0]-hwidth; i++ ) {
	for (j=dim[1]-hwidth; j<dim[1]; j++ ) {
	    if ( data[i][j] != should_be) {
		lres = 0;
	    }
	}
    }


    for ( i=hwidth; i<dim[0]-hwidth; i++) {
	for (j=hwidth; j<dim[1]-hwidth; j++ ){
	    if ( data[i][j] != rank ) {
		lres = 0;
	    }
	}
    }


    MPI_Allreduce ( &lres, &gres, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD );
    if ( gres == 1 ) {
        return 1; 
    }
    else {
        return 0; 
    }

}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
void set_data_2D ( double **data, int rank, int *dim, int hwidth ) 
{
    int i, j;

    for (i=0; i<dim[0]; i++ ) {
	for (j=0; j<hwidth; j++ ){
	    data[i][j] = -1;
	}
	for (j=dim[1]-hwidth; j<dim[1]; j++ ) {
	    data[i][j] = -1;
	}
    }
    for ( j=0; j<dim[1]; j++ ) {
	for ( i=0; i<hwidth; i++ ) {
	    data[i][j]=-1;
	}
	for ( i=dim[0]-hwidth; i<dim[0]; i++ ) {
	    data[i][j]=-1;
	}
    }

    for ( i=hwidth; i<dim[0]-hwidth; i++) {
	for (j=hwidth; j<dim[1]-hwidth; j++ ){
	    data[i][j] = rank;
	}
    }


    return;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
void dump_vector_2D ( double **data, int rank, int *dim)
{
    int i, j;
    
    for (i=0; i<dim[0]; i++) {
	printf("%d : ", rank);
	for ( j=0; j<dim[1]; j++ ) {
	    printf("%lf ", data[i][j]);
	}
	printf ("\n");
    }

    return;
}

/**********************************************************************/
void dump_vector_2D_mpi ( double **data, int *dim, MPI_Comm comm )
/**********************************************************************/
{
    int i, j, iproc;
    int rank, size;

    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    for ( iproc=0; iproc<size; iproc++ ) {
        if ( iproc == rank ) {
            for (i=0; i<dim[0]; i++) {
                printf("%d : ", rank);
                for ( j=0; j<dim[1]; j++ ) {
                    printf("%lf ", data[i][j]);
                }
                printf ("\n");
            }
        }
        MPI_Barrier ( comm );
    }

    return;
}

/********************************************************************************************************************************/
void dump_vector_2D_plus_nc_mpi ( double ***data, int *dim, int nc, MPI_Comm comm )
/********************************************************************************************************************************/
{
    /* data - 2-dimensional array
       dims - array of dimensions of data
       nc   - number of entries for each data point
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
                    for ( k=0; k<nc; k++ ) {
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


/********************************************************************************************************************************/


/**********************************************************************/
 void set_data_2D_int ( int **data, int rank, int *dim, int hwidth ) 
/**********************************************************************/
{
    int i, j;

    for (i=0; i<dim[0]; i++ ) {
	for (j=0; j<hwidth; j++ ){
	    data[i][j] = -1;
	}
	for (j=dim[1]-hwidth; j<dim[1]; j++ ) {
	    data[i][j] = -1;
	}
    }
    for ( j=0; j<dim[1]; j++ ) {
	for ( i=0; i<hwidth; i++ ) {
	    data[i][j]=-1;
	}
	for ( i=dim[0]-hwidth; i<dim[0]; i++ ) {
	    data[i][j]=-1;
	}
    }

    for ( i=hwidth; i<dim[0]-hwidth; i++) {
	for (j=hwidth; j<dim[1]-hwidth; j++ ){
	    data[i][j] = rank;
	}
    }


    return;
}


/**********************************************************************/
 void check_data_2D_int ( int **data, int rank, int *dim, int hwidth, 
			    int *neighbors)
/**********************************************************************/
{
    int i, j, lres=1, gres;
    double should_be;


    should_be = neighbors[0]==MPI_PROC_NULL ? -1 : neighbors[0];
    for ( j=hwidth; j<dim[1]-hwidth; j++ ) {
	for ( i=0; i<hwidth; i++ ) {
	    if ( data[i][j] != should_be ){
		lres = 0;
	    }
	}
    }

    should_be = neighbors[1]==MPI_PROC_NULL ? -1 : neighbors[1];
    for ( j=hwidth; j<dim[1]-hwidth; j++ ) {
	for ( i=dim[0]-hwidth; i<dim[0]; i++ ) {
	    if ( data[i][j] != should_be ) {
		lres = 0;
	    }
	}
    }
    should_be = neighbors[2]==MPI_PROC_NULL ? -1 : neighbors[2];
    for (i=hwidth; i<dim[0]-hwidth; i++ ) {
	for (j=0; j<hwidth; j++ ){
	    if ( data[i][j] != should_be ) {
		lres = 0;
	    }
	}
    }

    should_be = neighbors[3]==MPI_PROC_NULL ? -1 : neighbors[3];
    for (i=hwidth; i<dim[0]-hwidth; i++ ) {
	for (j=dim[1]-hwidth; j<dim[1]; j++ ) {
	    if ( data[i][j] != should_be) {
		lres = 0;
	    }
	}
    }


    for ( i=hwidth; i<dim[0]-hwidth; i++) {
	for (j=hwidth; j<dim[1]-hwidth; j++ ){
	    if ( data[i][j] != rank ) {
		lres = 0;
	    }
	}
    }


    MPI_Allreduce ( &lres, &gres, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD );
    if ( gres == 1 ) {
	if ( rank == 0 ) {
	 //   printf("2-D C testsuite: hwidth = %d, nc = 0 passed\n", 
	 //	   hwidth );
	}
    }
    else {
	if ( rank == 0 ) {
	    printf("2-D C testsuite: hwidth = %d, nc = 0 failed\n",
		   hwidth);
	}
	dump_vector_2D_int ( data, rank, dim );
	printf("%d: n[0]=%d n[1]=%d n[2]=%d n[3]=%d\n", rank, neighbors[0], 
	       neighbors[1], neighbors[2], neighbors[3] );
    }


    return;
}



/**********************************************************************/
 void dump_vector_2D_int ( int **data, int rank, int *dim)
/**********************************************************************/
{
    int i, j;
    
    for (i=0; i<dim[0]; i++) {
	printf("%d : ", rank);
	for ( j=0; j<dim[1]; j++ ) {
	    printf("%d ", data[i][j]);
	}
	printf ("\n");
    }

    return;
}
