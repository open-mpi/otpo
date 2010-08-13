/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2008           HLRS. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL.h"
#include "ADCL_internal.h"


/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_Vmap_halo_allocate ( int hwidth, ADCL_Vmap *vmap )
{
    ADCL_vmap_t *tvmap=NULL;
    int err = ADCL_SUCCESS;         
          
    if (hwidth < 0){
        return ADCL_INVALID_HWIDTH;
    }	

    err = ADCL_vmap_halo_allocate ( hwidth, &tvmap );

    *vmap = tvmap;

    return err;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_Vmap_list_allocate ( int size, int* rcnts, int* displ, ADCL_Vmap *vmap )
{
    ADCL_vmap_t *tvmap=NULL;
    int err = ADCL_SUCCESS;         
 
    if ( NULL == rcnts){
        return ADCL_INVALID_ARG;
    }	

    if ( NULL == displ) {
        return ADCL_INVALID_ARG; 
    }

    err = ADCL_vmap_list_allocate ( size, rcnts, displ, &tvmap );

    *vmap = tvmap;
    return err;
} 
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_Vmap_allreduce_allocate ( MPI_Op op, ADCL_Vmap *vmap )
{
    ADCL_vmap_t *tvmap=NULL;
    int err = ADCL_SUCCESS;         
 
    if ( op != MPI_MIN && op != MPI_MAX && op != MPI_SUM){
       return ADCL_INVALID_ARG;
    }

    err = ADCL_vmap_allreduce_allocate ( op, &tvmap );

    *vmap = tvmap;
    return err;
}

int ADCL_Vmap_reduce_allocate ( MPI_Op op, ADCL_Vmap *vmap )
{
    ADCL_vmap_t *tvmap=NULL;
    int err = ADCL_SUCCESS;         
 
    if ( op != MPI_MIN && op != MPI_MAX && op != MPI_SUM){
       return ADCL_INVALID_ARG;
    }

    err = ADCL_vmap_reduce_allocate ( op, &tvmap );

    *vmap = tvmap;
    return err;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_Vmap_alltoall_allocate ( int scnt, int rcnt, ADCL_Vmap *vmap )
{
    ADCL_vmap_t *tvmap=NULL;
    int err = ADCL_SUCCESS;         
 
    if ( 0 >= scnt || 0 >= rcnt ) {
        return ADCL_INVALID_ARG;
    }	

    err = ADCL_vmap_alltoall_allocate ( scnt, rcnt, &tvmap );

    *vmap = tvmap;
    return err;
} 
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_Vmap_all_allocate ( ADCL_Vmap *vmap )
{
    ADCL_vmap_t *tvmap=NULL;
    int err = ADCL_SUCCESS;         
          
    err = ADCL_vmap_all_allocate ( &tvmap );

    *vmap = tvmap;
    return err;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_Vmap_inplace_allocate ( ADCL_Vmap *vmap )
{
    ADCL_vmap_t *tvmap=NULL;
    int err = ADCL_SUCCESS;         
          
    err = ADCL_vmap_inplace_allocate ( &tvmap );

    *vmap = tvmap;
    return err;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_Vmap_free  ( ADCL_Vmap *vmap )
{
    int ret; 

    if ( NULL == vmap ) {
       return ADCL_INVALID_ARG;
    }

    ret =  ADCL_vmap_free ( vmap );

    // whatever happens internally, object has to appear deleted
    vmap = ADCL_VMAP_NULL; 

    return ret;
}


