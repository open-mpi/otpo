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
#define DIM0  8
#define DIM1  4

static void dump_vector_2D ( double **data, int rank, int *dim);
static void dump_vector_3D ( double ***data, int rank, int *dim, int nc);
static void set_data_2D ( double **data, int rank, int *dim, int hwidth );
static void set_data_3D ( double ***data, int rank, int *dim, int hwidth, int nc);
static void check_data_2D ( double **data, int rank, int *dim, 
			    int hwidth, int *neighbors ); 
static void check_data_3D ( double ***data, int rank, int *dim, 
			    int hwidth, int nc, int *neighbors ); 


int main ( int argc, char ** argv ) 
{
    /* General variables */
    int hwidth, rank, size;
    
    /* Definition of the 2-D vector */
    int dims[2], neighbors[4];
    double **data, ***data2;
    ADCL_Vector vec;

    /* Variables for the process topology information */
    int cdims[]={0,0};
    int periods[]={0,0};
    MPI_Comm cart_comm;
    ADCL_Topology topo;
    ADCL_Request request;

    /* Initiate the MPI environment */
    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    /* Describe the neighborhood relations */
    MPI_Dims_create ( size, 2, cdims );
    MPI_Cart_create ( MPI_COMM_WORLD, 2, cdims, periods, 0, &cart_comm);
    MPI_Cart_shift ( cart_comm, 0, 1, &(neighbors[0]), &(neighbors[1]));
    MPI_Cart_shift ( cart_comm, 1, 1, &(neighbors[2]), &(neighbors[3]));

    /* Initiate the ADCL library and register a topology object with ADCL */
    ADCL_Init ();
    ADCL_Topology_create ( cart_comm, &topo );

    /**********************************************************************/
    /* Test 1: hwidth=1, nc=0 */
    hwidth=1;
    dims[0] = DIM0 + 2*hwidth;
    dims[1] = DIM1 + 2*hwidth;
    ADCL_Vector_allocate ( 2,  dims, 0, ADCL_VECTOR_HALO, hwidth, MPI_DOUBLE, &data, &vec );
    ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &request );
    
    set_data_2D ( data, rank, dims, hwidth );
#ifdef VERBOSE
    dump_vector_2D ( data, rank, dims );
#endif

    ADCL_Request_start ( request );
    check_data_2D ( data, rank, dims, hwidth, neighbors );

    ADCL_Request_free ( &request );
    ADCL_Vector_free ( &vec );

    /**********************************************************************/
    /* Test 2: hwidth=2, nc=0 */
    hwidth=2;
    dims[0] = DIM0 + 2*hwidth;
    dims[1] = DIM1 + 2*hwidth;
    ADCL_Vector_allocate ( 2,  dims, 0, ADCL_VECTOR_HALO, hwidth, MPI_DOUBLE, &data, &vec );
    ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &request );
    
    set_data_2D ( data, rank, dims, hwidth );
#ifdef VERBOSE
    dump_vector_2D ( data, rank, dims );
#endif

    ADCL_Request_start ( request );
    check_data_2D ( data, rank, dims, hwidth, neighbors );

    ADCL_Request_free ( &request );
    ADCL_Vector_free ( &vec );

    /**********************************************************************/
    /* Test 3: hwidth=1, nc=1 */
    hwidth=1;
    dims[0] = DIM0 + 2*hwidth;
    dims[1] = DIM1 + 2*hwidth;
    ADCL_Vector_allocate ( 2,  dims, 1, ADCL_VECTOR_HALO, hwidth, MPI_DOUBLE, &data2, &vec );
    ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &request );
    
    set_data_3D ( data2, rank, dims, hwidth, 1 );
#ifdef VERBOSE
    dump_vector_3D ( data2, rank, dims, 1 );
#endif

    ADCL_Request_start ( request );
    check_data_3D ( data2, rank, dims, hwidth, 1, neighbors);

    ADCL_Request_free ( &request );
    ADCL_Vector_free ( &vec );

    /**********************************************************************/
    /* Test 4: hwidth=2, nc=1 */
    hwidth=2;
    dims[0] = DIM0 + 2*hwidth;
    dims[1] = DIM1 + 2*hwidth;
    ADCL_Vector_allocate ( 2,  dims, 1, ADCL_VECTOR_HALO, hwidth, MPI_DOUBLE, &data2, &vec );
    ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &request );
    
    set_data_3D ( data2, rank, dims, hwidth, 1 );
#ifdef VERBOSE
    dump_vector_3D ( data2, rank, dims, 1 );
#endif

    ADCL_Request_start ( request );
    check_data_3D ( data2, rank, dims, hwidth, 1, neighbors );

    ADCL_Request_free ( &request );
    ADCL_Vector_free ( &vec );

    /**********************************************************************/
    /* Test 5: hwidth=2, nc=2 */
    hwidth=2;
    dims[0] = DIM0 + 2*hwidth;
    dims[1] = DIM1 + 2*hwidth;
    ADCL_Vector_allocate ( 2,  dims, 2, ADCL_VECTOR_HALO, hwidth, MPI_DOUBLE, &data2, &vec );
    ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &request );
    
    set_data_3D ( data2, rank, dims, hwidth, 2 );
#ifdef VERBOSE
    dump_vector_3D ( data2, rank, dims, 2 );
#endif

    ADCL_Request_start ( request );
    check_data_3D ( data2, rank, dims, hwidth, 2, neighbors );

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
static void check_data_3D ( double ***data, int rank, int *dim, 
			    int hwidth, int nc, int *neighbors ) 
{
    int i, j, k, lres=1, gres;
    double should_be;

    should_be = neighbors[0]==MPI_PROC_NULL ? -1 : neighbors[0];
    for ( j=hwidth; j<dim[1]-hwidth; j++ ) {
	for ( i=0; i<hwidth; i++ ) {
	    for ( k=0; k<nc; k++ ) {
		if ( data[i][j][k] != should_be ){
		    lres = 0;
		}
	    }
	}
    }

    should_be = neighbors[1]==MPI_PROC_NULL ? -1 : neighbors[1];
    for ( j=hwidth; j<dim[1]-hwidth; j++ ) {
	for ( i=dim[0]-hwidth; i<dim[0]; i++ ) {
	    for ( k=0; k<nc; k++ ) {
		if ( data[i][j][k] != should_be ) {
		    lres = 0;
		}
	    }
	}
    }
    should_be = neighbors[2]==MPI_PROC_NULL ? -1 : neighbors[2];
    for (i=hwidth; i<dim[0]-hwidth; i++ ) {
	for (j=0; j<hwidth; j++ ){
	    for ( k=0; k<nc; k++ ) {
		if ( data[i][j][k] != should_be ) {
		    lres = 0;
		}
	    }
	}
    }

    should_be = neighbors[3]==MPI_PROC_NULL ? -1 : neighbors[3];
    for (i=hwidth; i<dim[0]-hwidth; i++ ) {
	for (j=dim[1]-hwidth; j<dim[1]; j++ ) {
	    for ( k=0; k<nc; k++ ) {
		if ( data[i][j][k] != should_be) {
		    lres = 0;
		}
	    }
	}
    }


    for ( i=hwidth; i<dim[0]-hwidth; i++) {
	for (j=hwidth; j<dim[1]-hwidth; j++ ){
	    for ( k=0; k<nc; k++ ) {
		if ( data[i][j][k] != rank ) {
		    lres = 0;
		}
	    }
	}
    }

    MPI_Allreduce ( &lres, &gres, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD );
    if ( gres == 1 ) {
	if ( rank == 0 ) {
	    printf("2-D C testsuite: hwidth = %d, nc = %d passed\n", 
		   hwidth, nc);
	}
    }
    else {
	if ( rank == 0 ) {
	    printf("2-D C testsuite: hwidth = %d, nc = %d failed\n", 
		   hwidth, nc);
	}
	dump_vector_3D ( data, rank, dim, nc);
    }

    return;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static void check_data_2D ( double **data, int rank, int *dim, int hwidth, 
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
	if ( rank == 0 ) {
	    printf("2-D C testsuite: hwidth = %d, nc = 0 passed\n", 
		   hwidth );
	}
    }
    else {
	if ( rank == 0 ) {
	    printf("2-D C testsuite: hwidth = %d, nc = 0 failed\n",
		   hwidth);
	}
	dump_vector_2D ( data, rank, dim );
	printf("%d: n[0]=%d n[1]=%d n[2]=%d n[3]=%d\n", rank, neighbors[0], 
	       neighbors[1], neighbors[2], neighbors[3] );
    }


    return;
}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static void set_data_2D ( double **data, int rank, int *dim, int hwidth ) 
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
static void set_data_3D ( double ***data, int rank, int *dim, int hwidth, int nc ) 
{
    int i, j, k;

    for (i=0; i<dim[0]; i++ ) {
	for (j=0; j<hwidth; j++ ){
	    for ( k=0; k<nc; k++ ) {
		data[i][j][k] = -1;
	    }
	}
	for (j=dim[1]-hwidth; j<dim[1]; j++ ) {
	    for ( k=0; k<nc; k++ ) {
		data[i][j][k] = -1;
	    }
	}
    }
    for ( j=0; j<dim[1]; j++ ) {
	for ( i=0; i<hwidth; i++ ) {
	    for ( k=0; k<nc; k++ ) {
		data[i][j][k]=-1;
	    }
	}
	for ( i=dim[0]-hwidth; i<dim[0]; i++ ) {
	    for ( k=0; k<nc; k++ ) {
		data[i][j][k]=-1;
	    }
	}
    }

    for ( i=hwidth; i<dim[0]-hwidth; i++) {
	for (j=hwidth; j<dim[1]-hwidth; j++ ){
	    for ( k=0; k<nc; k++ ) {
		data[i][j][k] = rank;
	    }
	}
    }


    return;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static void dump_vector_2D ( double **data, int rank, int *dim)
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
/**********************************************************************/
/**********************************************************************/
static void dump_vector_3D ( double ***data, int rank, int *dim, int nc)
{
    int i, j, k;
    
    for (i=0; i<dim[0]; i++) {
	printf("%d : ", rank);
	for ( j=0; j<dim[1]; j++ ) {
	    for ( k=0; k<nc; k++ ) {
		printf("%lf ", data[i][j][k]);
	    }
	}
	printf ("\n");
    }


    return;
}
