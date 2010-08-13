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

#define NDIM  3
/* Dimensions of the data matrix per process */
#define DIM0  4
#define DIM1  5
#define DIM2  6

extern void dump_vector_3D_mpi ( double ***data, int *dim, MPI_Comm cart_comm );
extern void dump_vector_4D_mpi ( double ****data, int *dim, int nc, MPI_Comm cart_comm );

static void set_data_3D ( double ***data, int rank, MPI_Comm cart_comm, int *dim, int hwidth );
static void set_data_4D ( double ****data, int rank, MPI_Comm cart_comm, int *dim, int hwidth, int nc);

static int check_data_3D ( double ***data, int rank, MPI_Comm cart_comm, int *dim, int hwidth, int *neighbors ); 
static int check_data_4D ( double ****data, int rank, MPI_Comm cart_comm, int *dim, int hwidth, int nc, int *neighbors ); 

static int calc_entry3D ( int control_x, int control_y, int control_z,  double ***data, int rank, 
        MPI_Comm cart_comm, int *dim, int hwidth, int *neighbors);
static int calc_entry4D ( int control_x, int control_y, int control_z,  double ****data, int rank, 
        MPI_Comm cart_comm, int *dim, int hwidth, int *neighbors, int nc);


int main ( int argc, char ** argv ) 
{
    /* General variables */
    int hwidth, nc, rank, size, err;

    /* Definition of the 2-D vector */
    int dims[3], neighbors[6];
    double ***data, ****data2;
    ADCL_Vector vec;

    /* Variables for the process topology information */
    int cdims[]={0,0,0};
    int periods[]={0,0,0};
    ADCL_Vmap vmap;
    MPI_Comm cart_comm;
    ADCL_Topology topo;
    ADCL_Request request;

    int i, niter = 50; 
    int ntests_3D, ntests_3D_plus_nc; 
    int itest, isok; 

    /* Initiate the MPI environment */
    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    /* Describe the neighborhood relations */
    MPI_Dims_create ( size, 3, cdims );
    MPI_Cart_create ( MPI_COMM_WORLD, 3, cdims, periods, 0, &cart_comm);
    MPI_Cart_shift ( cart_comm, 0, 1, &(neighbors[0]), &(neighbors[1]));
    MPI_Cart_shift ( cart_comm, 1, 1, &(neighbors[2]), &(neighbors[3]));
    MPI_Cart_shift ( cart_comm, 2, 1, &(neighbors[4]), &(neighbors[5]));


    /* Initiate the ADCL library and register a topology object with ADCL */
    ADCL_Init ();
    err = ADCL_Topology_create_extended ( cart_comm, &topo );
    if ( ADCL_SUCCESS != err) {
        printf("topology_create not successful\n");
        goto exit; 
    }

    ntests_3D = 2;
    for ( itest = 0; itest<ntests_3D; itest++) {
        isok = 1;

        if ( itest == 0 ) {
            /**********************************************************************/
            /* Test 1: hwidth=1, nc=0 */
            hwidth=1;
            nc = 0; 
            dims[0] = DIM0 + 2*hwidth;
            dims[1] = DIM1 + 2*hwidth;
            dims[2] = DIM2 + 2*hwidth;
        }
        else {
            /**********************************************************************/
            /* Test 2: hwidth=2, nc=0 */
            hwidth=2;
            nc = 0; 
            dims[0] = DIM0 + 2*hwidth;
            dims[1] = DIM1 + 2*hwidth;
            dims[2] = DIM2 + 2*hwidth;
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
        err = ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &request );
        if ( ADCL_SUCCESS != err) {
            printf("request_create not successful\n");
            goto exit; 
        }

        for ( i=0; i<niter; i++) {
            set_data_3D ( data, rank, cart_comm, dims, hwidth );
#ifdef VERBOSE3D
            dump_vector_3D_mpi ( data, dims, cart_comm );
#endif

            err = ADCL_Request_start ( request );
            if ( ADCL_SUCCESS != err) {
                printf("request_start not successful\n");
                goto exit; 
            }
            isok = check_data_3D ( data, rank, cart_comm, dims, hwidth, neighbors );
            if ( ! isok )  {
                if ( rank == 0 ) {
                    printf("3D C testsuite failed at iteration %d: hwidth = %d, nc = %d\n", i, hwidth, nc);
                }
                dump_vector_3D_mpi ( data, dims, cart_comm );
                exit;
            }
        } // niter

        err = ADCL_Request_free ( &request );
        if ( ADCL_SUCCESS != err) {
            printf("request_free not successful\n");
            goto exit; 
        }
        err = ADCL_Vector_free ( &vec );
        if ( ADCL_SUCCESS != err) {
            printf("vector_free not successful\n");
            goto exit; 
        }
        err = ADCL_Vmap_free ( &vmap ); 
        if ( ADCL_SUCCESS != err) {
            printf("vmap_free not successful\n");
            goto exit; 
        }

        if ( rank == 0 && isok ) {
            printf("3D C testsuite: hwidth = %d, nc = %d passed\n", hwidth, nc);
        }
    }

    ntests_3D_plus_nc = 3;
    for (itest = 0; itest<ntests_3D_plus_nc; itest++) {
        isok = 1;

        if ( itest == 0 ) {
            /**********************************************************************/
            /* Test 3: hwidth=1, nc=1 */
            hwidth=1;
            nc = 1; 
            dims[0] = DIM0 + 2*hwidth;
            dims[1] = DIM1 + 2*hwidth;
            dims[2] = DIM2 + 2*hwidth;
        }
        else if ( itest == 1 ) {
            /**********************************************************************/
            /* Test 4: hwidth=2, nc=1 */
            hwidth=2;
            nc = 1; 
            dims[0] = DIM0 + 2*hwidth;
            dims[1] = DIM1 + 2*hwidth;
            dims[2] = DIM2 + 2*hwidth;
        }
        else {
            /**********************************************************************/
            /* Test 5: hwidth=2, nc=2 */
            hwidth=1;
            nc = 2; 
            dims[0] = DIM0 + 2*hwidth;
            dims[1] = DIM1 + 2*hwidth;
            dims[2] = DIM2 + 2*hwidth;
        }

        err = ADCL_Vmap_halo_allocate ( hwidth, &vmap );
        if ( ADCL_SUCCESS != err) {
            printf("vmap_create not successful\n");
            goto exit; 
        }
        err = ADCL_Vector_allocate_generic ( 3,  dims, nc, vmap, MPI_DOUBLE, &data2, &vec );
        if ( ADCL_SUCCESS != err) {
            printf("vector_create not successful\n");
            goto exit; 
        }
        err = ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &request );
        if ( ADCL_SUCCESS != err) {
            printf("request_create not successful\n");
            goto exit; 
        }

        for ( i = 0; i<niter; i++) {
            set_data_4D ( data2, rank, cart_comm, dims, hwidth, 1 );
#ifdef VERBOSE4D
            dump_vector_4D ( data2, dims, nc, cart_comm );
#endif

            err = ADCL_Request_start ( request );
            if ( ADCL_SUCCESS != err) {
                printf("request_start not successful\n");
                goto exit; 
            }
#ifdef VERBOSE4D
            dump_vector_4D ( data2, dims, nc, cart_comm );
#endif

            isok = check_data_4D ( data2, rank, cart_comm, dims, hwidth, 1, neighbors);
            if ( ! isok )  {
                if ( rank == 0 ) {
                    printf("3D C testsuite failed at iteration %d: hwidth = %d, nc = %d\n", i, hwidth, nc);
                    dump_vector_4D_mpi ( data2, dims, nc, cart_comm );
                }
                exit;
            }
        }

        err = ADCL_Request_free ( &request );
        if ( ADCL_SUCCESS != err) {
            printf("request_free not successful\n");
            goto exit; 
        }
        err = ADCL_Vector_free ( &vec );
        if ( ADCL_SUCCESS != err) {
            printf("vector_free not successful\n");
            goto exit; 
        }
        err = ADCL_Vmap_free ( &vmap ); 
        if ( ADCL_SUCCESS != err) {
            printf("vmap_free not successful\n");
            goto exit; 
        }

        if ( rank == 0 && isok ) {
            printf("3D C testsuite: hwidth = %d, nc = %d passed\n", hwidth, nc);
        }
    }

    /**********************************************************************/

exit:
    err = ADCL_Topology_free ( &topo );
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

static int check_data_3D ( double ***data, int rank, MPI_Comm cart_comm, int *dim, int hwidth, int *neighbors)
{
    int control_x, control_y, control_z, lres=1, gres;
    double prod; 

    // check for each of the 27 possible locations
    for(control_x=-1; control_x<=1; control_x++){
        for(control_y=-1; control_y<=1; control_y++){
            for(control_z=-1; control_z<=1; control_z++){
                prod = control_x * control_y * control_z; 
#ifdef INCCORNER
                if ( prod != 0){
                    // corner
                    lres = calc_entry3D ( control_x, control_y, control_z, data, rank, cart_comm, dim, hwidth, neighbors);
                }
#endif
                if( prod == 0) {
                    // edge, face or inside
                    lres = calc_entry3D ( control_x, control_y, control_z, data, rank, cart_comm, dim, hwidth, neighbors);
                }
                MPI_Allreduce ( &lres, &gres, 1, MPI_INT, MPI_MIN, cart_comm );
                if ( gres != 1 ) {
                    return 0; 
                }
            }
        }
    }

    /*if ( gres == 1 ) {
      if ( rank == 0 ) {
      printf("3-D C testsuite: hwidth = %d, nc = 0 passed\n", hwidth );
      }
      }
      else {
      if ( rank == 0 ) {
      printf("3-D C testsuite: hwidth = %d, nc = 0 failed\n", hwidth);
      }
      dump_vector_3D ( data, dim, cart_comm );
      }*/

    return 1;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/

static int check_data_4D ( double ****data, int rank, MPI_Comm cart_comm, int *dim, int hwidth, int nc, int *neighbors) 
{
    int control_x, control_y, control_z, lres=1, gres;
    double prod; 

    for(control_x=-1; control_x<=1; control_x++){
        for(control_y=-1; control_y<=1; control_y++){
            for(control_z=-1; control_z<=1; control_z++){
                prod = control_x * control_y * control_z;
#ifdef INCCORNER
                if( prod != 0){
                    lres = calc_entry4D ( control_x, control_y, control_z, data, rank, cart_comm, dim, hwidth, neighbors, nc);
                }
#endif
                if( prod == 0){
                    lres = calc_entry4D ( control_x, control_y, control_z, data, rank, cart_comm, dim, hwidth, neighbors, nc);
                }
            }
            MPI_Allreduce ( &lres, &gres, 1, MPI_INT, MPI_MIN, cart_comm );
            if ( gres != 1 ) {
                return 0; 
            }
        }
    }

    /* if ( gres == 1 ) {
       if ( rank == 0 ) {
       printf("4-D C testsuite: hwidth = %d, nc = %d passed\n", 
       hwidth, nc );
       }
       }
       else {
       if ( rank == 0 ) {
       printf("4-D C testsuite: hwidth = %d, nc = %d failed\n",
       hwidth, nc);
       }
       dump_vector_4D ( data, dim, nc, cart_comm);
       } */

    return 1;

}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static void set_data_3D ( double ***data, int rank, MPI_Comm cart_comm, int *dim, int hwidth ) 
{
    int i, j, k;
    int coords[3];
    int dims[3]; //size of each dimension
    int period[3];

    MPI_Cart_get(cart_comm, 3, dims, period, coords);

    for (i=0; i<dim[0]; i++ ) {
        for (j=0; j<dim[1]; j++ ){
            for (k=0; k<hwidth; k++ ){
                data[i][j][k] = -1;
            }
            for (k=dim[2]-hwidth; k<dim[2]; k++ ) {
                data[i][j][k] = -1;
            }
        }
    }

    for (i=0; i<dim[0]; i++ ) {
        for (k=0; k<dim[2]; k++ ){
            for (j=0; j<hwidth; j++ ){
                data[i][j][k] = -1;
            }
            for (j=dim[1]-hwidth; j<dim[1]; j++ ) {
                data[i][j][k] = -1;
            }
        }
    }

    for (j=0; j<dim[1]; j++ ) {
        for (k=0; k<dim[2]; k++ ){
            for (i=0; i<hwidth; i++ ){
                data[i][j][k] = -1;
            }
            for (i=dim[0]-hwidth; i<dim[0]; i++ ) {
                data[i][j][k] = -1;
            }
        }
    }

    for ( i=hwidth; i<dim[0]-hwidth; i++) {
        for (j=hwidth; j<dim[1]-hwidth; j++ ){
            for (k=hwidth; k<dim[2]-hwidth; k++ ){
                //printf("rank = %d, coords[0] = %d, coords[1] = %d, coords[2] = %d \n", rank, coords[0], coords[1], coords[2]);
                data[i][j][k] = (coords[2] * (dim[2]-hwidth*2) + (k-hwidth) ) 
                    + (dims[2] * (dim[2] - hwidth*2)) * ((dim[1] - hwidth*2) * coords[1] + (j-hwidth)) 
                    + (dims[2] * (dim[2] - hwidth*2)) * (dims[1] * (dim[1] - hwidth*2)) * ((dim[0] - hwidth*2) * coords[0]+(i-hwidth)) ;
            }  
        }
    }


    return;
}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static void set_data_4D ( double ****data, int rank,  MPI_Comm cart_comm, int *dim, int hwidth, int nc ) 
{
    int i, j, k, l;
    int coords[3];
    int dims[3]; //size of each dimension
    int period[3];

    //Get the coordinate of the process
    //MPI_Cart_coords(cart_comm, rank, 2, coords);
    MPI_Cart_get(cart_comm, 3, dims, period, coords);

    for (i=0; i<dim[0]; i++ ) {
        for (j=0; j<dim[1]; j++ ){
            for (k=0; k<hwidth; k++ ){
                for (l=0; l<nc; l++ ) {
                    data[i][j][k][l] = -1;
                }
            }
            for (k=dim[2]-hwidth; k<dim[2]; k++ ) {
                for (l=0; l<nc; l++ ) {
                    data[i][j][k][l] = -1;
                }
            }
        }
    }

    for (i=0; i<dim[0]; i++ ) {
        for (k=0; k<dim[2]; k++ ){
            for (j=0; j<hwidth; j++ ){
                for (l=0; l<nc; l++ ) {
                    data[i][j][k][l] = -1;
                }
            }
            for (j=dim[1]-hwidth; j<dim[1]; j++ ) {
                for (l=0; l<nc; l++ ) {
                    data[i][j][k][l] = -1;
                }
            }
        }
    }

    for (j=0; j<dim[1]; j++ ) {
        for (k=0; k<dim[2]; k++ ){
            for (i=0; i<hwidth; i++ ){
                for (l=0; l<nc; l++ ) {
                    data[i][j][k][l] = -1;
                }
            }
            for (i=dim[0]-hwidth; i<dim[0]; i++ ) {
                for (l=0; l<nc; l++ ) {
                    data[i][j][k][l] = -1;
                }
            }
        }
    }


    for ( i=hwidth; i<dim[0]-hwidth; i++) {
        for (j=hwidth; j<dim[1]-hwidth; j++ ){
            for (k=hwidth; k<dim[2]-hwidth; k++ ){
                for (l=0; l<nc; l++ ) {
                    //printf("rank = %d, coords[0] = %d, coords[1] = %d, coords[2] = %d \n", rank, coords[0], coords[1], coords[2]);
                    data[i][j][k][l] = (coords[2] * (dim[2]-hwidth*2) + (k-hwidth) ) 
                        + (dims[2] * (dim[2] - hwidth*2)) * ((dim[1] - hwidth*2) * coords[1] + (j-hwidth)) 
                        + (dims[2] * (dim[2] - hwidth*2)) * (dims[1] * (dim[1] - hwidth*2)) * ((dim[0] - hwidth*2) * coords[0]+(i-hwidth)) ;
                }          
            }  
        }
    }



    return;
}


static int calc_entry3D ( int control_x, int control_y, int control_z,  double ***data, int rank, MPI_Comm cart_comm, int *dim, int hwidth, 
        int *neighbors)
{
    int i, j, k;
    int lres=1;
    double should_be;
    int coords[3], n_coords[3], c_coords[3];
    int dims[3]; //size of each dimension
    int period[3];
    int compensate_i, compensate_j, compensate_k; 
    int compensate[3]; //are used to calculate compensate_i,j,k
    int	loopstart[3], loopend[3];
    int neighbor_cond[3];

    MPI_Cart_get(cart_comm, 3, dims, period, coords); 
    neighbor_cond[0] = neighbor_cond[1] = neighbor_cond[2] = 0;

    switch(control_x){
        case 0:
            loopstart[0] = hwidth;
            loopend[0] = dim[0] - hwidth;
            compensate[0] = 0;
            c_coords[0] = coords[0];
            neighbor_cond[0] = 1;
            break;

        case 1:
            loopstart[0] = dim[0] - hwidth *2;
            loopend[0] = dim[0]-hwidth ;
            compensate[0] = - (dim[0] - hwidth*2) ;
            if(neighbors[0]!=MPI_PROC_NULL){
                MPI_Cart_coords (cart_comm, neighbors[0], 3, n_coords);
                c_coords[0] = n_coords[0];
                neighbor_cond[0] = 1;
            }
            break;

        case -1:
            loopstart[0] = hwidth;
            loopend[0] = hwidth * 2;	
            compensate[0] = (dim[0] - hwidth*2) ;
            if(neighbors[1]!=MPI_PROC_NULL){
                MPI_Cart_coords (cart_comm, neighbors[1], 3, n_coords);
                c_coords[0] = n_coords[0];
                neighbor_cond[0] = 1;
            }
    }

    switch(control_y){
        case 0:		
            loopstart[1] = hwidth;
            loopend[1] = dim[1] - hwidth;
            compensate[1] = 0;
            c_coords[1] = coords[1];
            neighbor_cond[1] = 1;
            break;

        case 1:		
            loopstart[1] = dim[1] - hwidth *2;
            loopend[1] = dim[1]-hwidth ; 
            compensate[1] = - (dim[1] - hwidth*2) ;
            if(neighbors[2]!=MPI_PROC_NULL){
                MPI_Cart_coords (cart_comm, neighbors[2], 3, n_coords);
                c_coords[1] = n_coords[1];
                neighbor_cond[1] = 1;
            }
            break;

        case -1:	
            loopstart[1] = hwidth;
            loopend[1] = hwidth * 2;		
            compensate[1] = (dim[1] - hwidth*2) ;
            if(neighbors[3]!=MPI_PROC_NULL){
                MPI_Cart_coords (cart_comm, neighbors[3], 3, n_coords);
                c_coords[1] = n_coords[1];
                neighbor_cond[1] = 1;
            }
    }

    switch(control_z){
        case 0:		
            loopstart[2] = hwidth;
            loopend[2] = dim[2] - hwidth;
            compensate[2] = 0;
            c_coords[2] = coords[2];
            neighbor_cond[2] = 1;
            break;

        case 1:		
            loopstart[2] = dim[2] - hwidth *2;
            loopend[2] = dim[2]-hwidth ; 
            compensate[2] = - (dim[2] - hwidth*2) ;
            if(neighbors[4]!=MPI_PROC_NULL){
                MPI_Cart_coords (cart_comm, neighbors[4], 3, n_coords);
                c_coords[2] = n_coords[2];
                neighbor_cond[2] = 1;
            }
            break;

        case -1:	
            loopstart[2] = hwidth;
            loopend[2] = hwidth * 2;	 	
            compensate[2] = (dim[2] - hwidth*2) ;
            if(neighbors[5]!=MPI_PROC_NULL){
                MPI_Cart_coords (cart_comm, neighbors[5], 3, n_coords);
                c_coords[2] = n_coords[2];
                neighbor_cond[2] = 1;
            }
    }

    for ( i = loopstart[0]; i < loopend[0]; i++){
        for ( j= loopstart[1]; j < loopend[1]; j++){
            for (k = loopstart[2]; k < loopend[2]; k++){
                if(neighbor_cond[0] && neighbor_cond[1] && neighbor_cond[2]){				
                    should_be = (c_coords[2] * (dim[2]-hwidth*2) + (k-hwidth) ) 
                        + (dims[2] * (dim[2] - hwidth*2)) * ((dim[1] - hwidth*2) * c_coords[1] + (j-hwidth)) 
                        + (dims[2] * (dim[2] - hwidth*2)) * (dims[1] * (dim[1] - hwidth*2)) * ((dim[0] - hwidth*2) * c_coords[0]+(i-hwidth)) ;
                }
                else{
                    should_be = -1;
                }

                compensate_i = i + compensate[0];
                compensate_j = j + compensate[1];
                compensate_k = k + compensate[2];

                if ( data[compensate_i][compensate_j][compensate_k] != should_be ){
                    lres = 0;
                    printf("3D shouldbe = %f data= %f control_x= %d control_y= %d control_z= %d rank = %d\n", 
                            should_be, data[compensate_i][compensate_j][compensate_k], control_x, control_y, control_z, rank);
                }
            }
        }
    }
    return lres;
}

static int calc_entry4D ( int control_x, int control_y, int control_z,  double ****data, int rank, MPI_Comm cart_comm, int *dim, int hwidth, 
        int *neighbors, int nc)
{
    int i, j, k, l;
    int lres=1;
    double should_be;
    int coords[3], n_coords[3], c_coords[3];
    int dims[3]; //size of each dimension
    int period[3];
    int compensate_i, compensate_j, compensate_k; 
    int compensate[3];
    int	loopstart[3], loopend[3];
    int neighbor_cond[3];

    MPI_Cart_get(cart_comm, 3, dims, period, coords); 
    neighbor_cond[0] = neighbor_cond[1] = neighbor_cond[2] = 0;

    switch(control_x){
        case 0:		
            loopstart[0] = hwidth;
            loopend[0] = dim[0] - hwidth;
            compensate[0] = 0;
            c_coords[0] = coords[0];
            neighbor_cond[0] = 1;
            break;

        case 1:		
            loopstart[0] = dim[0] - hwidth *2;
            loopend[0] = dim[0]-hwidth ;
            compensate[0] = - (dim[0] - hwidth*2) ;
            if(neighbors[0]!=MPI_PROC_NULL){
                MPI_Cart_coords (cart_comm, neighbors[0], 3, n_coords);
                c_coords[0] = n_coords[0];
                neighbor_cond[0] = 1;
            }
            break;

        case -1:	
            loopstart[0] = hwidth;
            loopend[0] = hwidth * 2;	
            compensate[0] = (dim[0] - hwidth*2) ;
            if(neighbors[1]!=MPI_PROC_NULL){
                MPI_Cart_coords (cart_comm, neighbors[1], 3, n_coords);
                c_coords[0] = n_coords[0];
                neighbor_cond[0] = 1;
            }
    }

    switch(control_y){
        case 0:		
            loopstart[1] = hwidth;
            loopend[1] = dim[1] - hwidth;
            compensate[1] = 0;
            c_coords[1] = coords[1];
            neighbor_cond[1] = 1;
            break;

        case 1:		
            loopstart[1] = dim[1] - hwidth *2;
            loopend[1] = dim[1]-hwidth ; 
            compensate[1] = - (dim[1] - hwidth*2) ;
            if(neighbors[2]!=MPI_PROC_NULL){
                MPI_Cart_coords (cart_comm, neighbors[2], 3, n_coords);
                c_coords[1] = n_coords[1];
                neighbor_cond[1] = 1;
            }
            break;

        case -1:	
            loopstart[1] = hwidth;
            loopend[1] = hwidth * 2;		
            compensate[1] = (dim[1] - hwidth*2) ;
            if(neighbors[3]!=MPI_PROC_NULL){
                MPI_Cart_coords (cart_comm, neighbors[3], 3, n_coords);
                c_coords[1] = n_coords[1];
                neighbor_cond[1] = 1;
            }
    }

    switch(control_z){
        case 0:		
            loopstart[2] = hwidth;
            loopend[2] = dim[2] - hwidth;
            compensate[2] = 0;
            c_coords[2] = coords[2];
            neighbor_cond[2] = 1;
            break;

        case 1:		
            loopstart[2] = dim[2] - hwidth *2;
            loopend[2] = dim[2]-hwidth ; 
            compensate[2] = - (dim[2] - hwidth*2) ;
            if(neighbors[4]!=MPI_PROC_NULL){
                MPI_Cart_coords (cart_comm, neighbors[4], 3, n_coords);
                c_coords[2] = n_coords[2];
                neighbor_cond[2] = 1;
            }
            break;

        case -1:	
            loopstart[2] = hwidth;
            loopend[2] = hwidth * 2;	 	
            compensate[2] = (dim[2] - hwidth*2) ;
            if(neighbors[5]!=MPI_PROC_NULL){
                MPI_Cart_coords (cart_comm, neighbors[5], 3, n_coords);
                c_coords[2] = n_coords[2];
                neighbor_cond[2] = 1;
            }
    }

    for ( i = loopstart[0]; i < loopend[0]; i++){
        for ( j= loopstart[1]; j < loopend[1]; j++){
            for (k = loopstart[2]; k < loopend[2]; k++){
                for (l = 0; l < nc; l++){
                    if(neighbor_cond[0] && neighbor_cond[1] && neighbor_cond[2]){				
                        should_be = (c_coords[2] * (dim[2]-hwidth*2) + (k-hwidth) ) 
                            + (dims[2] * (dim[2] - hwidth*2)) * ((dim[1] - hwidth*2) * c_coords[1] + (j-hwidth)) 
                            + (dims[2] * (dim[2] - hwidth*2)) * (dims[1] * (dim[1] - hwidth*2)) * ((dim[0] - hwidth*2) * c_coords[0]+(i-hwidth)) ;
                    }
                    else{
                        should_be = -1;
                    }

                    compensate_i = i + compensate[0];
                    compensate_j = j + compensate[1];
                    compensate_k = k + compensate[2];

                    if ( data[compensate_i][compensate_j][compensate_k][l] != should_be ){
                        lres = 0;
                        printf("4D shouldbe = %f data= %f control_x= %d control_y= %d control_z= %d rank = %d nc= %d\n", 
                                should_be, data[compensate_i][compensate_j][compensate_k][l], control_x, control_y, control_z, rank, nc);
                    }
                }
            }
        }
    }
    return lres;
}


