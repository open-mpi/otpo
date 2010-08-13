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
#include <stdlib.h> 

#include "ADCL.h"
#include "mpi.h"

#define NDIM 2
/* Dimensions of the data matrix per process */
#define DIM0  8
#define DIM1  4

void dump_vector_2D ( double **data, int rank, int *dim);
static void dump_vector_3D ( double ***data, int rank, int *dim, int nc);
void set_data_2D ( double **data, int rank, int *dim, int hwidth );
static void set_data_3D ( double ***data, int rank, int *dim, int hwidth, int nc);
int check_data_2D ( double **data, int rank, int *dim, 
			    int hwidth, int *neighbors ); 
static int check_data_3D ( double ***data, int rank, int *dim, 
			    int hwidth, int nc, int *neighbors ); 


int main ( int argc, char ** argv ) 
{
    /* General variables */
    int hwidth, rank, size, err;
    int itest, ntests_2D, ntests_2D_plus_nc; 
    
    /* Definition of the 2-D vector */
    int dims[2], neighbors[4], niter=50, i, isok, nc;
    double **data, ***data2;
    ADCL_Vector vec;

    /* Variables for the process topology information */
    int cdims[]={0,0};
    int periods[]={0,0};
    ADCL_Vmap vmap;
    MPI_Comm cart_comm;
    ADCL_Topology topo;
    ADCL_Request request;

    /* Initiate the MPI environment */
    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    /* Describe the neighborhood relations */
    MPI_Dims_create ( size, NDIM, cdims );
    MPI_Cart_create ( MPI_COMM_WORLD, NDIM, cdims, periods, 0, &cart_comm);
    MPI_Cart_shift ( cart_comm, 0, 1, &(neighbors[0]), &(neighbors[1]));
    MPI_Cart_shift ( cart_comm, 1, 1, &(neighbors[2]), &(neighbors[3]));

    /* Initiate the ADCL library and register a topology object with ADCL */
    ADCL_Init ();
    err = ADCL_Topology_create ( cart_comm, &topo );
    if ( ADCL_SUCCESS != err) {
        printf("topology_create not successful\n");
        goto exit; 
    }

    ntests_2D = 2;
    for ( itest = 0; itest<ntests_2D; itest++) {
        isok = 1;
        if ( itest == 0 ) {

            /**********************************************************************/
            /* Test 1: hwidth=1, nc=0 */
            hwidth=1;
            nc = 0;
            dims[0] = DIM0 + 2*hwidth;
            dims[1] = DIM1 + 2*hwidth;
        }
        else {
            /**********************************************************************/
            /* Test 2: hwidth=2, nc=0 */
            nc = 0; 
            hwidth=2;
            dims[0] = DIM0 + 2*hwidth;
            dims[1] = DIM1 + 2*hwidth;
        }

        err = ADCL_Vmap_halo_allocate ( hwidth, &vmap );
        if ( ADCL_SUCCESS != err) {
            printf("vmap_halo_allocate not successful\n");
            goto exit; 
        }
        err = ADCL_Vector_allocate_generic ( NDIM,  dims, nc, vmap, MPI_DOUBLE, &data, &vec );
        if ( ADCL_SUCCESS != err) {
            printf("vector_allocate not successful\n");
            goto exit; 
        }
        ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &request );
        if ( ADCL_SUCCESS != err) {
            printf("request_create not successful\n");
            goto exit; 
        }

        for ( i=0; i<niter; i++) {
            set_data_2D ( data, rank, dims, hwidth );
#ifdef VERBOSE
            dump_vector_2D ( data, rank, dims );
#endif

            ADCL_Request_start ( request );
            if ( ADCL_SUCCESS != err) {
                printf("request_start not successful\n");
                goto exit; 
            }
            isok = check_data_2D ( data, rank, dims, hwidth, neighbors );
            if ( ! isok )  {
                if ( rank == 0 ) {
                    printf("2D C testsuite failed at iteration %d: hwidth = %d\n", i, hwidth);
                    dump_vector_2D_mpi ( data, dims, cart_comm );
                }
                exit;
            }
        }

        ADCL_Request_free ( &request );
        if ( ADCL_SUCCESS != err) {
            printf("request_free not successful\n");
            goto exit; 
        }
        ADCL_Vector_free ( &vec );
        if ( ADCL_SUCCESS != err) {
            printf("vector_free not successful\n");
            goto exit; 
        }
        ADCL_Vmap_free ( &vmap ); 
        if ( ADCL_SUCCESS != err) {
            printf("vmap_free not successful\n");
            goto exit; 
        }

        if ( rank == 0 ) {
            printf("2D C testsuite: hwidth = %d, nc = %d passed\n", hwidth, nc);
        }
    }


    ntests_2D_plus_nc = 3;
    for (itest = 0; itest<ntests_2D_plus_nc; itest++) {
        isok = 1;

        if ( itest == 0 ) {

            /**********************************************************************/
            /* Test 3: hwidth=1, nc=1 */
            hwidth=1;
            nc = 1; 
            dims[0] = DIM0 + 2*hwidth;
            dims[1] = DIM1 + 2*hwidth;
        }
        else if ( itest == 1 ) {
            /**********************************************************************/
            /* Test 4: hwidth=2, nc=1 */
            hwidth=2;
            nc = 1; 
            dims[0] = DIM0 + 2*hwidth;
            dims[1] = DIM1 + 2*hwidth;


        }
        else {
            /**********************************************************************/
            /* Test 5: hwidth=2, nc=2 */
            hwidth=2;
            nc = 2; 
            dims[0] = DIM0 + 2*hwidth;
            dims[1] = DIM1 + 2*hwidth;



        }
        err = ADCL_Vmap_halo_allocate ( hwidth, &vmap );
        if ( ADCL_SUCCESS != err) {
            printf("vmap_create not successful\n");
            goto exit; 
        }
        err = ADCL_Vector_allocate_generic ( NDIM,  dims, nc, vmap, MPI_DOUBLE, &data2, &vec );
        if ( ADCL_SUCCESS != err) {
            printf("vector_create not successful\n");
            goto exit; 
        }
        ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &request );
        if ( ADCL_SUCCESS != err) {
            printf("request_create not successful\n");
            goto exit; 
        }

        for ( i=0; i<niter; i++) {
            set_data_3D ( data2, rank, dims, hwidth, 1 );
#ifdef VERBOSE
            dump_vector_3D ( data2, rank, dims, 1 );
#endif

            ADCL_Request_start ( request );
            if ( ADCL_SUCCESS != err) {
                printf("request_start not successful\n");
                goto exit; 
            }
            isok = check_data_3D ( data2, rank, dims, hwidth, 1, neighbors);
            if ( ! isok )  {
                if ( rank == 0 ) {
                    printf("2D C testsuite failed at iteration %d: hwidth = %d, nc = %d\n", i, hwidth, nc);
                    dump_vector_2D_mpi ( data, dims, cart_comm );
                }
                exit;
            }
        }

        ADCL_Request_free ( &request );
        if ( ADCL_SUCCESS != err) {
            printf("request_free not successful\n");
            goto exit; 
        }
        ADCL_Vector_free ( &vec );
        if ( ADCL_SUCCESS != err) {
            printf("vector_free not successful\n");
            goto exit; 
        }
        ADCL_Vmap_free ( &vmap ); 
        if ( ADCL_SUCCESS != err) {
            printf("vmap_free not successful\n");
            goto exit; 
        }

        if ( rank == 0 ) {
            printf("2D C testsuite: hwidth = %d, nc = %d passed\n", hwidth, nc);
        }
    }

    /**********************************************************************/
exit: 
    ADCL_Topology_free ( &topo );
    if ( ADCL_SUCCESS != err) {
        printf("topology_free not successful\n");
        goto exit; 
    } 
    MPI_Comm_free ( &cart_comm );

    ADCL_Finalize ();
    MPI_Finalize ();
    return 0;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static int check_data_3D ( double ***data, int rank, int *dim, 
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
	return 1; 
    }
    else {
        return 0; 
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
