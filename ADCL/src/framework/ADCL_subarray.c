/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"

int ADCL_subarray_init ( int topodims, int vecndims, int *vecdims,
             int hwidth, int nc, int order, MPI_Datatype btype,
             MPI_Datatype **senddats, MPI_Datatype **recvdats)
{
    int i, j, k;
    int ret = ADCL_SUCCESS;
    int *subdims=NULL, *sstarts=NULL, *rstarts=NULL;
    MPI_Datatype *sdats=NULL, *rdats=NULL;

    subdims  = ( int*) malloc ( vecndims * sizeof(int) );
    if ( NULL == subdims ) {
    return ADCL_NO_MEMORY;
    }

    sstarts  = ( int*) malloc ( vecndims * sizeof(int) );
    rstarts  = ( int*) malloc ( vecndims * sizeof(int) );
    if ( NULL == sstarts || NULL == rstarts  ) {
    ret = ADCL_NO_MEMORY;
    goto exit;
    }

    sdats = ( MPI_Datatype *) malloc ( topodims * 2 * sizeof(MPI_Datatype));
    rdats = ( MPI_Datatype *) malloc ( topodims * 2 * sizeof(MPI_Datatype));
    if ( NULL == sdats || NULL == rdats  ) {
    ret = ADCL_NO_MEMORY;
    goto exit;
    }

    /* Loop over all topology dimensions */
    for ( i = 0; i < topodims; i++ ) {

    /* handle left and right neighbor separatly */
    for ( j=2*i; j<= 2*i+1; j++ ) {

        /* Set subdims and starts arrays. Basically,
           subdims is in each direction the total extent of the
           according dimension of the data array without the halo-cells
           except for the dimension which we are currently dealing
           with. For this dimension it is 1.

           The starts arrays are 1 for all dimensions except
           for the dimension (lets say k)  which we are dealing with.
           There it is for sending:
           - 1 for the left neighbor,
           - ldims[k]-2*HWIDTH for the right neighbor
           for receiving:
           - 0 for the left neighbor
           - ldims[k]-HWDITH for the right neighbor
        */
        if  ( nc > 0 ) {
        for ( k=0; k < vecndims-1; k++ ) {
            if ( k == i ) {
            subdims[k] = hwidth;
            sstarts[k] = (j == 2*i) ? hwidth : (vecdims[k]-2*hwidth);
            rstarts[k] = (j == 2*i) ? 0 : (vecdims[k]-hwidth);
            }
            else {
            subdims[k] = vecdims[k]- 2*hwidth;
            sstarts[k] = hwidth;
            rstarts[k] = hwidth;
            }
        }
        subdims[vecndims-1] = nc;
        sstarts[vecndims-1] = 0;
        rstarts[vecndims-1] = 0;
            }
        else {
        for ( k=0; k < vecndims; k++ ) {
            if ( k == i ) {
            subdims[k] = hwidth;
            sstarts[k] = (j == 2*i) ? hwidth : (vecdims[k]-2*hwidth);
            rstarts[k] = (j == 2*i) ? 0 : (vecdims[k]-hwidth);
            }
            else {
            subdims[k] = vecdims[k]- 2*hwidth;
            sstarts[k] = hwidth;
            rstarts[k] = hwidth;
            }
        }
        }
        MPI_Type_create_subarray ( vecndims, vecdims, subdims, sstarts,
                       order, btype, &(sdats[j]));
        MPI_Type_create_subarray ( vecndims, vecdims, subdims, rstarts,
                       order, btype, &(rdats[j]));
        MPI_Type_commit ( &(sdats[j]));
        MPI_Type_commit ( &(rdats[j]));
    }
    }

 exit:
    if ( ret != ADCL_SUCCESS ) {
    if ( NULL != subdims ) {
        free ( subdims ) ;
    }
    if ( NULL != sstarts ) {
        free ( sstarts );
    }
    if ( NULL != rstarts ) {
        free ( rstarts );
    }
    if ( NULL != sdats ) {
        free ( sdats );
    }
    if ( NULL != rdats ) {
        free ( rdats );
    }
    }

    *senddats = sdats;
    *recvdats = rdats;
    return ret;
}

void ADCL_subarray_free ( int num, MPI_Datatype **senddats,
              MPI_Datatype **recvdats )
{
    int i;
    MPI_Datatype *sdats = *senddats;
    MPI_Datatype *rdats = *recvdats;

    if ( NULL != rdats ) {
    for ( i=0; i<num; i++ ){
        if ( MPI_DATATYPE_NULL != rdats[i]) {
        MPI_Type_free ( &(rdats[i]));
        }
    }
    free ( rdats );
    }

    if ( NULL != sdats ) {
    for ( i=0; i<num; i++ ){
        if ( MPI_DATATYPE_NULL != sdats[i]) {
        MPI_Type_free ( &(sdats[i]));
        }
    }
    free ( sdats );
    }

    *senddats = NULL;
    *recvdats = NULL;
    return;
}


