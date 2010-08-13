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

ADCL_array_t* ADCL_vmap_farray=NULL;
static int ADCL_local_vmap_counter=0;

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_vmap_halo_allocate ( int hwidth, ADCL_vmap_t **vmap )
{
    ADCL_vmap_t *tvmap=NULL;
          
    /* Allocate a new vmap object */
    tvmap = (ADCL_vmap_t *) calloc ( 1, sizeof (ADCL_vmap_t) );
    if ( NULL == tvmap ) {
        return ADCL_NO_MEMORY;
    }

    /* Set the according elements of the structure */
    tvmap->m_id     = ADCL_local_vmap_counter++;
    tvmap->m_rfcnt  = 1;
    
    ADCL_array_get_next_free_pos ( ADCL_vmap_farray, &(tvmap->m_findex) );
    ADCL_array_set_element ( ADCL_vmap_farray,
                 tvmap->m_findex,
                 tvmap->m_id,
                 tvmap );
    
    tvmap->m_vectype = ADCL_VECTOR_HALO;
    tvmap->m_hwidth  = hwidth;
    
    *vmap = tvmap;
    return ADCL_SUCCESS;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_vmap_list_allocate ( int size, int *rcnts, int *displ, 
   ADCL_vmap_t **vmap )
{
    ADCL_vmap_t *tvmap=NULL;
    int i;
    int *trcnts, *tdispl;
 
    /* Allocate a new vmap object */
    tvmap = (ADCL_vmap_t *) calloc ( 1, sizeof (ADCL_vmap_t) );
    if ( NULL == tvmap ) {
        return ADCL_NO_MEMORY;
    }

    /* Set the according elements of the structure */
    tvmap->m_id     = ADCL_local_vmap_counter++;
    tvmap->m_rfcnt  = 1;
    
    ADCL_array_get_next_free_pos ( ADCL_vmap_farray, &(tvmap->m_findex) );
    ADCL_array_set_element ( ADCL_vmap_farray,
                 tvmap->m_findex,
                 tvmap->m_id,
                 tvmap );
    
    tvmap->m_vectype = ADCL_VECTOR_LIST;

    trcnts = (int *) calloc ( size, sizeof (int) );
    if ( NULL == trcnts ) {
        return ADCL_NO_MEMORY;
    }
    tdispl = (int *) calloc ( size, sizeof (int) );
    if ( NULL == tdispl ) {
        return ADCL_NO_MEMORY;
    }

    for ( i=0; i<size; i++ ) {
       trcnts[i] = rcnts[i];
       tdispl[i] = displ[i];
    }
    tvmap->m_rcnts = trcnts;
    tvmap->m_displ = tdispl;

    *vmap = tvmap;
    return ADCL_SUCCESS;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_vmap_allreduce_allocate ( MPI_Op op, ADCL_vmap_t **vmap )
{
    ADCL_vmap_t *tvmap=NULL;
          
    /* Allocate a new vmap object */ 
    tvmap = (ADCL_vmap_t *) calloc ( 1, sizeof (ADCL_vmap_t) );
    if ( NULL == tvmap ) {
        return ADCL_NO_MEMORY;
    }

    /* Set the according elements of the structure */
    tvmap->m_id     = ADCL_local_vmap_counter++;
    tvmap->m_rfcnt  = 1;
    
    ADCL_array_get_next_free_pos ( ADCL_vmap_farray, &(tvmap->m_findex) );
    ADCL_array_set_element ( ADCL_vmap_farray,
                 tvmap->m_findex,
                 tvmap->m_id,
                 tvmap );
    
    tvmap->m_vectype = ADCL_VECTOR_ALLREDUCE;
    tvmap->m_op      = op;
    
    *vmap = tvmap;
    return ADCL_SUCCESS;
}

int ADCL_vmap_reduce_allocate ( MPI_Op op, ADCL_vmap_t **vmap )
{
    ADCL_vmap_t *tvmap=NULL;
          
    /* Allocate a new vmap object */ 
    tvmap = (ADCL_vmap_t *) calloc ( 1, sizeof (ADCL_vmap_t) );
    if ( NULL == tvmap ) {
        return ADCL_NO_MEMORY;
    }

    /* Set the according elements of the structure */
    tvmap->m_id     = ADCL_local_vmap_counter++;
    tvmap->m_rfcnt  = 1;
    
    ADCL_array_get_next_free_pos ( ADCL_vmap_farray, &(tvmap->m_findex) );
    ADCL_array_set_element ( ADCL_vmap_farray,
                 tvmap->m_findex,
                 tvmap->m_id,
                 tvmap );
    
    tvmap->m_vectype = ADCL_VECTOR_REDUCE;
    tvmap->m_op      = op;
    
    *vmap = tvmap;
    return ADCL_SUCCESS;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_vmap_alltoall_allocate ( int scnt, int rcnt, ADCL_vmap_t **vmap )
{
    ADCL_vmap_t *tvmap=NULL;
          
    /* Allocate a new vmap object */ 
    tvmap = (ADCL_vmap_t *) calloc ( 1, sizeof (ADCL_vmap_t) );
    if ( NULL == tvmap ) {
        return ADCL_NO_MEMORY;
    }

    /* Set the according elements of the structure */
    tvmap->m_id     = ADCL_local_vmap_counter++;
    tvmap->m_rfcnt  = 1;
    
    ADCL_array_get_next_free_pos ( ADCL_vmap_farray, &(tvmap->m_findex) );
    ADCL_array_set_element ( ADCL_vmap_farray,
                 tvmap->m_findex,
                 tvmap->m_id,
                 tvmap );
    
    tvmap->m_vectype = ADCL_VECTOR_ALLTOALL;
    tvmap->m_scnt    = scnt;
    tvmap->m_rcnt    = rcnt;
    
    *vmap = tvmap;
    return ADCL_SUCCESS;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_vmap_all_allocate ( ADCL_vmap_t **vmap )
{
    ADCL_vmap_t *tvmap=NULL;
          
    /* Allocate a new vmap object */ 
    tvmap = (ADCL_vmap_t *) calloc ( 1, sizeof (ADCL_vmap_t) );
    if ( NULL == tvmap ) {
        return ADCL_NO_MEMORY;
    }

    /* Set the according elements of the structure */
    tvmap->m_id     = ADCL_local_vmap_counter++;
    tvmap->m_rfcnt  = 1;
    
    ADCL_array_get_next_free_pos ( ADCL_vmap_farray, &(tvmap->m_findex) );
    ADCL_array_set_element ( ADCL_vmap_farray,
                 tvmap->m_findex,
                 tvmap->m_id,
                 tvmap );
    
    tvmap->m_vectype = ADCL_VECTOR_ALL;
    
    *vmap = tvmap;
    return ADCL_SUCCESS;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_vmap_inplace_allocate ( ADCL_vmap_t **vmap )
{
    ADCL_vmap_t *tvmap=NULL;
          
    /* Allocate a new vmap object */ 
    tvmap = (ADCL_vmap_t *) calloc ( 1, sizeof (ADCL_vmap_t) );
    if ( NULL == tvmap ) {
        return ADCL_NO_MEMORY;
    }

    /* Set the according elements of the structure */
    tvmap->m_id     = ADCL_local_vmap_counter++;
    tvmap->m_rfcnt  = 1;
    
    ADCL_array_get_next_free_pos ( ADCL_vmap_farray, &(tvmap->m_findex) );
    ADCL_array_set_element ( ADCL_vmap_farray,
                 tvmap->m_findex,
                 tvmap->m_id,
                 tvmap );
    
    tvmap->m_vectype = ADCL_VECTOR_INPLACE;
    
    *vmap = tvmap;
    return ADCL_SUCCESS;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_vmap_free  ( ADCL_vmap_t **vmap )
{
    ADCL_vmap_t *tvmap=*vmap;

    tvmap->m_rfcnt--;
    if ( tvmap->m_rfcnt == 0 ) {
        ADCL_array_remove_element ( ADCL_vmap_farray, tvmap->m_findex );

        if (NULL != tvmap->m_rcnts) { 
           free ( tvmap->m_rcnts );
	}
        if (NULL != tvmap->m_displ) { 
           free ( tvmap->m_displ );
	}
	free ( tvmap );
	// internally, vmap has to exist as long as rfcnt > 0, 
	// externally, vmap has to appear deleted after first call to Vmap_free
        *vmap = ADCL_VMAP_NULL;
    }
    else if ( tvmap->m_rfcnt < 0 ) {
        return ADCL_ERROR_INTERNAL;
    }

    return ADCL_SUCCESS;
}


