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

#define NIT 3000

#define DIM0  32
#define DIM1  32
#define DIM2  32

/* how many halo-cells for each neighbor ? */
#define HWIDTH 1


int main ( int argc, char ** argv ) 
{
    int i, rank, size;
    int dims[3]={DIM0+2*HWIDTH,DIM1+2*HWIDTH,DIM2+2*HWIDTH};
    int cdims[]={0,0,0};
    int periods[]={0,0,0};
    int ***data;

    ADCL_Vmap vmap;    
    ADCL_Vector vec;
    ADCL_Topology topo;
    ADCL_Request request;
    
    MPI_Comm cart_comm;

    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    ADCL_Init ();
    ADCL_Vmap_halo_allocate ( HWIDTH, &vmap );
    ADCL_Vector_allocate_generic ( 3, dims, 0, vmap, MPI_DOUBLE, &data, &vec );

    MPI_Dims_create ( size, 3, cdims );
    MPI_Cart_create ( MPI_COMM_WORLD, 3, cdims, periods, 0, &cart_comm);

    //ADCL_Topology_create (cart_comm, &topology);
    ADCL_Topology_create ( cart_comm, &topo );

    ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &request );
    

    for ( i=0; i<NIT; i++ ) {
	ADCL_Request_start( request );
    }

    ADCL_Vector_free ( &vec );
    ADCL_Vmap_free (&vmap);
    ADCL_Request_free ( &request );
    ADCL_Topology_free ( &topo );

    MPI_Comm_free ( &cart_comm );
    
    ADCL_Finalize ();
    MPI_Finalize ();
    return 0;
}

