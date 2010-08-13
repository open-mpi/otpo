/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2008-2009      HLRS. All rights reserved.
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

static void alltoall_test(int cnt, int dims, int rank, int size, ADCL_Topology topo); 

extern void dump_vector_1D ( double *data, int rank, int dim);
extern void dump_vector_1D_mpi ( double *data, int dim, MPI_Comm comm );
extern void set_data_1D ( double *data, int rank, int dim);
extern int check_data_1D ( double *data, int* rcounts, int *rdispl, int rank, int size);

int main ( int argc, char ** argv ) 
{
    int cnt, dims, err; 
    int rank, size;
    int cdims=0;
    int periods=0;

    ADCL_Topology topo;

    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    ADCL_Init ();

    err = ADCL_Topology_create ( MPI_COMM_WORLD, &topo );
    if ( ADCL_SUCCESS != err) goto exit;   

    cnt = 15;
    dims = 13;

    dims=3; 

    alltoall_test(cnt, dims, rank, size, topo);

exit:
    if ( ADCL_TOPOLOGY_NULL != topo)   ADCL_Topology_free ( &topo );
    ADCL_Finalize ();
    MPI_Finalize ();
    return 0;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
void alltoall_test(int cnt, int dims, int rank, int size, 
		     ADCL_Topology topo)
{
    double *sdata, *rdata;
    int dim, i; 
    int* cnts, *displ;
    int err, errc; 
    ADCL_Vector svec, rvec;
    ADCL_Vmap vmap;
    ADCL_Request request;

    /* set up arrays for verification */ 
    cnts = (int*) calloc ( size, sizeof(int) );
    displ = (int*) calloc ( size, sizeof(int) );

    for ( i=0;i<size;i++){
        cnts[i] = dims;
        displ[i] = dims*i;
    }

    err = ADCL_Vmap_alltoall_allocate( cnts[0], cnts[0], &vmap ); 
    if ( ADCL_SUCCESS != err) goto exit;   

    dim = dims*size;
    err = ADCL_Vector_allocate_generic ( 1,  &dim, 0, vmap, MPI_DOUBLE, &sdata, &svec );
    if ( ADCL_SUCCESS != err) goto exit;   
    err = ADCL_Vector_allocate_generic ( 1,  &dim, 0, vmap, MPI_DOUBLE, &rdata, &rvec );
    if ( ADCL_SUCCESS != err) goto exit;   

    err = ADCL_Request_create_generic ( svec, rvec, topo, ADCL_FNCTSET_ALLTOALL, &request );
    if ( ADCL_SUCCESS != err) goto exit;   

    for (i=0; i<cnt; i++){
	set_data_1D ( sdata, rank, dim );
	set_data_1D ( rdata, -1,   dim);

#ifdef VERBOSE
       dump_vector_1D_mpi ( sdata, dim, MPI_COMM_WORLD );
       dump_vector_1D_mpi ( rdata, dim, MPI_COMM_WORLD );
#endif

       err = ADCL_Request_start( request );
       if ( ADCL_SUCCESS != err) goto exit;   

#ifdef VERBOSE
       dump_vector_1D_mpi ( sdata, dim, MPI_COMM_WORLD );
       dump_vector_1D_mpi ( rdata, dim, MPI_COMM_WORLD );
#endif

       /* check data */
       errc = check_data_1D ( rdata, cnts, displ, rank, size);
       if (errc) goto exit;   
    }

    MPI_Barrier ( MPI_COMM_WORLD);

exit:
    if ( ADCL_SUCCESS != err) { printf("ADCL error nr. %d\n", err); } 

    if ( NULL != cnts) free(cnts);
    if ( NULL != displ) free(displ);
    if ( ADCL_REQUEST_NULL != request) ADCL_Request_free ( &request );
    if ( ADCL_VECTOR_NULL  != svec)    ADCL_Vector_free ( &svec );
    if ( ADCL_VECTOR_NULL  != rvec)    ADCL_Vector_free ( &rvec );
    if ( ADCL_VMAP_NULL    != vmap)    ADCL_Vmap_free (&vmap);

    return;
}

