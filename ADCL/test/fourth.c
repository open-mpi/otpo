/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include <stdio.h>

#include "ADCL.h"
#include "mpi.h"


/* Dimensions of the data matrix per process */
#define DIM0  2
#define DIM1  4
#define NC    1
/* how many halo-cells for each neighbor ? */
#define HWIDTH 1

/* just a helper routine */
static void matrix_init ( int dims[2], int cdims[2], 
			  double matrix[DIM0+2*HWIDTH][DIM1+2*HWIDTH][NC],  
			  MPI_Comm cart_comm );
static void matrix_dump ( double matrix[DIM0+2*HWIDTH][DIM1+2*HWIDTH][NC],  
			  MPI_Comm cart_comm, char *msg);

int main ( int argc, char ** argv ) 
{
    /* General variables */
    int rank, size;
    
    /* Definition of the 2-D vector */
    int dims[2]={DIM0+2*HWIDTH, DIM1+2*HWIDTH};
    double matrix[DIM0+2*HWIDTH][DIM1+2*HWIDTH][NC];
    ADCL_Topology topo;
    ADCL_Vector vec;

    /* Variables for the process topology information */
    int cdims[]={0,0};
    int periods[]={0,0};
    MPI_Comm cart_comm;
    ADCL_Request request;


    /* Initiate the MPI environment */
    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    /* Initiate the ADCL library and register a 2D vector with ADCL */
    ADCL_Init ();
    ADCL_Vector_register ( 2,  dims, NC, ADCL_VECTOR_HALO, 1, MPI_DOUBLE, matrix, &vec );

    /* Describe the neighborhood relations */
    MPI_Dims_create ( size, 2, cdims );
    MPI_Cart_create ( MPI_COMM_WORLD, 2, cdims, periods, 0, &cart_comm);
    ADCL_Topology_create ( cart_comm, &topo );

    /* Match the data type description and the process topology 
       to each other */
    ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &request );

    /* Initiate matrix to zero including halo-cells */
    matrix_init ( dims, cdims, matrix, cart_comm );

    /* Now this is the real communication */
//    ADCL_change_sb_aao_IsendIrecv ( request );
//    ADCL_change_sb_aao_SendIrecv ( request );
      ADCL_change_sb_aao_IsendIrecv_pack ( request );
//    ADCL_change_sb_pair_debug ( request );
//    ADCL_change_sb_pair_IsendIrecv ( request );
//    ADCL_change_sb_pair_SendRecv ( request );
//    ADCL_change_sb_pair_SendIrecv ( request );
//    ADCL_change_sb_pair_IsendIrecv_pack ( request );

   /* Dump the resulting matrix */
    matrix_dump ( matrix, cart_comm, "After the communication");

    ADCL_Request_free ( &request );
    ADCL_Topology_free ( &topo );
    ADCL_Vector_deregister ( &vec );
    MPI_Comm_free ( &cart_comm );
    
    ADCL_Finalize ();
    MPI_Finalize ();
    return 0;
}

static void matrix_init ( int dims[2], int cdims[2], 
			  double matrix[DIM0+2*HWIDTH][DIM1+2*HWIDTH][NC],    
			  MPI_Comm cart_comm )
{
    int i, j;
    double offset1, offset0;
    int rank;
    int coord[2];

    MPI_Comm_rank ( cart_comm, &rank );
    MPI_Cart_coords ( cart_comm, rank, 2, coord );

    for (i=0; i<DIM0+2; i++ ) {
        for ( j=0; j<DIM1+2; j++ ) {
	    matrix[i][j][0] = 0.0;
        }
    }

    /* Set now the values */
    offset1 = coord[1] * DIM1;
    offset0 = coord[0] * DIM0 * (DIM1 * cdims[1]);
    for (i=1; i<DIM0+1; i++ ) {
        for ( j=1; j<DIM1+1; j++ ){
            matrix[i][j][0] = (i-1)*DIM1*cdims[1]+offset0+offset1+(j-1);
        }
    }

#if VERBOSE
    matrix_dump ( matrix, cart_comm, "The original input matrix is");
#endif

    return;
}

static void matrix_dump ( double matrix[DIM0+2*HWIDTH][DIM1+2*HWIDTH][NC], 
			  MPI_Comm cart_comm, char *msg )
{
    int i, j, k;
    int rank, size;

    MPI_Comm_rank ( cart_comm, &rank );
    MPI_Comm_size ( cart_comm, &size );
    if (rank == 0 ) {
        printf("%s\n", msg);
    }

    for ( k=0 ; k < size; k++ ) {
        if (rank == k ) {
            for ( i=0; i<DIM0+2; i++ ) {
                printf("%d: ", rank );
                for ( j=0; j<DIM1+2; j++ ) {
                    printf("%lf ", matrix[i][j][0]);
                }
                printf("\n");
            }
                printf("\n");
        }
        MPI_Barrier ( cart_comm );
    }

    return;
}
