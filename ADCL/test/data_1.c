/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include <stdio.h>
#include <stdlib.h>

#include "ADCL.h"
#include "mpi.h"

#define NIT 3000

#define DIM0  52 
#define DIM1  52
#define DIM2  44 

/* how many halo-cells for each neighbor ? */
#define HWIDTH 1


int main ( int argc, char ** argv ) 
{
    int i, rank, size, err;
    int dims0[3]={DIM0+2*HWIDTH,DIM1+2*HWIDTH,DIM2+2*HWIDTH};
    int dims1[2]={DIM0+2*HWIDTH,DIM1+2*HWIDTH};
    int cdims0[]={0,0,0};
    int cdims1[]={0,0,0};
    int periods0[]={0,0,0};
    int periods1[]={0,0};
    double ***data0;
    double **data1;

    ADCL_Vmap vmap;
    /* ADCL Vector objects */
    ADCL_Vector vec0;
    ADCL_Vector vec1;
    /* ADCL Topology objects */
    ADCL_Topology topo0;
    ADCL_Topology topo1;
    /* ADCL Request Objects */
    ADCL_Request request0;
    ADCL_Request request1;
    /* MPI catesian cmmunicator */
    MPI_Comm cart_comm0;
    MPI_Comm cart_comm1;

    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    ADCL_Init ();

    err = ADCL_Vmap_halo_allocate ( HWIDTH, &vmap );
    if ( ADCL_SUCCESS != err) goto exit;

    err = ADCL_Vector_allocate_generic ( 3,  dims0, 0, vmap, MPI_DOUBLE, &data0, &vec0 );
    if ( ADCL_SUCCESS != err) goto exit;

    err = ADCL_Vector_allocate_generic ( 2,  dims1, 0, vmap, MPI_DOUBLE, &data1, &vec1 );
    if ( ADCL_SUCCESS != err) goto exit;

    MPI_Dims_create ( size, 3, cdims0 );
    MPI_Cart_create ( MPI_COMM_WORLD, 3, cdims0, periods0, 0, &cart_comm0);
    ADCL_Topology_create ( cart_comm0, &topo0 );

    MPI_Dims_create ( size, 2, cdims1 );
    MPI_Cart_create ( MPI_COMM_WORLD, 2, cdims1, periods1, 0, &cart_comm1);
    ADCL_Topology_create ( cart_comm1, &topo1 );

    ADCL_Request_create ( vec0, topo0, ADCL_FNCTSET_NEIGHBORHOOD, &request0 );

    ADCL_Request_create ( vec1, topo1, ADCL_FNCTSET_NEIGHBORHOOD, &request1 );

    /* Run 3D test */
    printf("3D test \n");
    for ( i=0; i<NIT; i++ ) {
	ADCL_Request_start( request0 );
    }
    /* Run 2D test */
    printf("2D test \n");
    for ( i=0; i<NIT; i++ ) {
	ADCL_Request_start( request1 );
    }
    ADCL_Request_free ( &request0 );
    ADCL_Request_free ( &request1 );

exit:
    ADCL_Vector_free ( &vec0 );
    ADCL_Vector_free ( &vec1 );
    ADCL_Vmap_free ( &vmap ); 
    ADCL_Topology_free ( &topo0 );
    ADCL_Topology_free ( &topo1 );
    MPI_Comm_free ( &cart_comm0 );
    MPI_Comm_free ( &cart_comm1 );

    ADCL_Finalize ();
    MPI_Finalize ();
    return 0;
}
