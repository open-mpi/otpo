/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include <stdio.h>
#include <string.h>

#include "ADCL.h"
#include "mpi.h"

#define DIM0  8

static void dump_vector_1D ( double *data, int rank, int dim);
static void dump_vector_2D ( double **data, int rank, int dim, int nc);
static void set_data_1D ( double *data, int rank, int dim, int hwidth );
static void set_data_2D ( double **data, int rank, int dim, int hwidth, int nc);
static void check_data_1D ( double *data, int rank, int size, int dim, int hwidth ); 
static void check_data_2D ( double **data, int rank, int size, int dim, int hwidth, int nc ); 

int main ( int argc, char ** argv ) 
{
    int rank, size;
    int dims, cdims=0;
    int periods=0, hwidth;
    double *data, **data2;
    
    ADCL_Vector vec;
    ADCL_Topology topo;
    ADCL_Request request;
    
    MPI_Comm cart_comm;

    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    ADCL_Init ();
    MPI_Dims_create ( size, 1, &cdims );
    MPI_Cart_create ( MPI_COMM_WORLD, 1, &cdims, &periods, 0, &cart_comm);

    ADCL_Topology_create ( cart_comm, &topo );

    /**********************************************************************/
    /* Test 1: hwidth=1, nc=0 */
    hwidth=1;
    dims = DIM0+2*hwidth;
    ADCL_Vector_allocate ( 1,  &dims, 0, ADCL_VECTOR_HALO, hwidth, MPI_DOUBLE, &data, &vec );
    ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &request );

    set_data_1D ( data, rank, DIM0+2*hwidth, hwidth);
#ifdef VERBOSE
    dump_vector_1D ( data, rank, dims);
#endif

    ADCL_Request_start( request );
    check_data_1D ( data, rank, size, DIM0+2*hwidth, hwidth );

    ADCL_Request_free ( &request );
    ADCL_Vector_free ( &vec );
    MPI_Barrier ( MPI_COMM_WORLD);

    /**********************************************************************/
    /* Test 2: hwidth=2, nc=0 */
    hwidth=2;
    dims = DIM0+2*hwidth;
    ADCL_Vector_allocate ( 1,  &dims, 0, ADCL_VECTOR_HALO, hwidth, MPI_DOUBLE, &data, &vec );
    ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &request );

    set_data_1D ( data, rank, DIM0+2*hwidth, hwidth);
#ifdef VERBOSE
    dump_vector_1D ( data, rank, dims);
#endif
    ADCL_Request_start( request );
    check_data_1D ( data, rank, size, DIM0+2*hwidth, hwidth );

    ADCL_Request_free ( &request );
    ADCL_Vector_free ( &vec );
    MPI_Barrier ( MPI_COMM_WORLD);


    /**********************************************************************/
    /* Test 3: hwidth=1, nc=1 */
    hwidth=1;
    dims = DIM0+2*hwidth;
    ADCL_Vector_allocate ( 1,  &dims, 1, ADCL_VECTOR_HALO, hwidth, MPI_DOUBLE, &data2, &vec );
    ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &request );

    set_data_2D ( data2, rank, dims, hwidth, 1 );
#ifdef VERBOSE    
    dump_vector_2D ( data2, rank, dims, 1);
#endif
    ADCL_Request_start( request );
    check_data_2D ( data2, rank, size, DIM0+2*hwidth, hwidth, 1 );

    ADCL_Request_free ( &request );
    ADCL_Vector_free ( &vec );
    MPI_Barrier ( MPI_COMM_WORLD);

    /**********************************************************************/
    /* Test 4: hwidth=2, nc=1 */
    hwidth=2;
    dims = DIM0+2*hwidth;
    ADCL_Vector_allocate ( 1,  &dims, 1, ADCL_VECTOR_HALO, hwidth, MPI_DOUBLE, &data2, &vec );
    ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &request );

    set_data_2D ( data2, rank, dims, hwidth, 1 );
#ifdef VERBOSE
    dump_vector_2D ( data2, rank, dims, 1);
#endif
    ADCL_Request_start( request );
    check_data_2D ( data2, rank, size, DIM0+2*hwidth, hwidth, 1 );

    ADCL_Request_free ( &request );
    ADCL_Vector_free ( &vec );
    MPI_Barrier ( MPI_COMM_WORLD);


    /**********************************************************************/
    /* Test 5: hwidth=2, nc=2 */
    hwidth=2;
    dims = DIM0+2*hwidth;
    ADCL_Vector_allocate ( 1,  &dims, 2, ADCL_VECTOR_HALO, hwidth, MPI_DOUBLE, &data2, &vec );
    ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &request );

    set_data_2D ( data2, rank, dims, hwidth, 2 );
#ifdef VERBOSE
    dump_vector_2D ( data2, rank, dims, 2);
#endif
    ADCL_Request_start( request );
    check_data_2D ( data2, rank, size, DIM0+2*hwidth, hwidth, 2 );

    ADCL_Request_free ( &request );
    ADCL_Vector_free ( &vec );


    /**********************************************************************/
    ADCL_Topology_free ( &topo );
    MPI_Comm_free ( &cart_comm );
    
    ADCL_Finalize ();
    MPI_Finalize ();
    return 0;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static void check_data_1D ( double *data, int rank, int size, int dim, int hwidth ) 
{
    int i, lres=1, gres;

    if ( rank == 0 ) {
	for (i=0; i< hwidth; i++ ) {
	    if ( data[i] != -1 ) {
		lres = 0;
	    }
	}
    }
    else {
	for (i=0; i< hwidth; i++ ) {
	    if ( data[i] != rank-1 ) {
		lres = 0;
	    }
	}
    }

    for ( i=hwidth; i<(DIM0+hwidth); i++) {
	if ( data[i] != rank) {
	    lres = 0;
	}
    }

    if ( rank == size -1 ) {
	for ( i=DIM0+hwidth; i<DIM0+2*hwidth; i++) {
	    if ( data[i] != -1 ) {
		lres = 0;
	    }
	}
    }
    else {
	for ( i=DIM0+hwidth; i<DIM0+2*hwidth; i++) {
	    if ( data[i] != rank+1 ) {
		lres = 0;
	    }
	}
    }

    MPI_Allreduce ( &lres, &gres, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD );
    if ( gres == 1 ) {
	if ( rank == 0 ) {
	    printf("1-D C testsuite: hwidth = %d, nc = 0 passed\n", hwidth);
	}
    }
    else {
	if ( rank == 0 ) {
	    printf("1-D C testsuite: hwidth = %d, nc = 0 failed\n", hwidth);
	}
	dump_vector_1D ( data, rank, dim);
    }

    return;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static void check_data_2D ( double **data, int rank, int size, int dim, int hwidth, int nc )
{
    int i, j, lres=1, gres;

    if ( rank == 0 )  {
	for (i=0; i< hwidth; i++ ) {
	    for (j=0; j<nc; j++ ){
		if ( data[i][j] != -1) {
		    lres = 0;
		}
	    }
	}
    }
    else {
	for (i=0; i< hwidth; i++ ) {
	    for (j=0; j<nc; j++ ){
		if ( data[i][j] != rank-1) {
		    lres = 0;
		}
	    }
	}
    }

    for ( i=hwidth; i<(DIM0+hwidth); i++) {
	for (j=0; j<nc; j++ ){
	    if ( data[i][j] != rank) {
		lres = 0;
	    }
	}
    }

    
    if ( rank == size -1 ) {
	for ( i=DIM0+hwidth; i<DIM0+2*hwidth; i++) {
	    for (j=0; j<nc; j++ ){
		if ( data[i][j] != -1 ) {
		    lres = 0;
		}
	    }
	}
    }
    else {
	for ( i=DIM0+hwidth; i<DIM0+2*hwidth; i++) {
	    for (j=0; j<nc; j++ ){
		if ( data[i][j] != rank+1 ) {
		    lres = 0;
		}
	    }
	}
    }

    MPI_Allreduce ( &lres, &gres, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD );
    if ( gres == 1 ) {
	if ( rank == 0 ) {
	    printf("1-D C testsuite: hwidth = %d, nc = %d passed\n", 
		   hwidth, nc);
	}
    }
    else {
	if ( rank == 0 ) {
	    printf("1-D C testsuite: hwidth = %d, nc = %d failed\n",
		   hwidth, nc);
	}
	dump_vector_2D ( data, rank, dim, 1);
    }


    return;
}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static void set_data_1D ( double *data, int rank, int dim, int hwidth ) 
{
    int i;

    for (i=0; i< hwidth; i++ ) {
	data[i] = -1;
    }
    for ( i=hwidth; i<(DIM0+hwidth); i++) {
	data[i] = rank;
    }
    for ( i=DIM0+hwidth; i<DIM0+2*hwidth; i++) {
	data[i] = -1;
    }

    return;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static void set_data_2D ( double **data, int rank, int dim, int hwidth, int nc ) 
{
    int i, j;

    for (i=0; i< hwidth; i++ ) {
	for (j=0; j<nc; j++ ){
	    data[i][j] = -1;
	}
    }
    for ( i=hwidth; i<(DIM0+hwidth); i++) {
	for (j=0; j<nc; j++ ){
	    data[i][j] = rank;
	}
    }
    for ( i=DIM0+hwidth; i<DIM0+2*hwidth; i++) {
	for (j=0; j<nc; j++ ){
	    data[i][j] = -1;
	}
    }

    return;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static void dump_vector_1D ( double *data, int rank, int dim)
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
/**********************************************************************/
/**********************************************************************/
static void dump_vector_2D ( double **data, int rank, int dim, int nc)
{
    int i, j;
    
    printf("%d : ", rank);
    for (i=0; i<dim; i++) {
	for (j=0; j<nc; j++) {
	    printf("%lf ", data[i][j]);
	}
    }
    printf ("\n");

    return;
}
