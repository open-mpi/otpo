/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL.h"
#include "ADCL_internal.h"

static int ADCL_vector_get_realdim ( int ndims, int *dims, int nc,
                     int *rndims, int **rdims );

ADCL_array_t* ADCL_vector_farray=NULL;
static int ADCL_local_vector_counter=0;

ADCL_array_t* ADCL_vectset_farray=NULL;
static int ADCL_local_vectset_counter=0;

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_vector_allocate ( int ndims, int *dims, int nc, int comtype, int hwidth,
               MPI_Datatype dat, ADCL_vector_t **vec )
{
    ADCL_vector_t *tvec=NULL;
    int rndims, *rdims=NULL;
          

    /* Verification of the input parameters has been done in the
       user level layer. Allocate now a new vector object */
    tvec = (ADCL_vector_t *) calloc ( 1, sizeof (ADCL_vector_t) );
    if ( NULL == tvec ) {
        return ADCL_NO_MEMORY;
    }

    /* Set the according elements of the structure */
    tvec->v_id     = ADCL_local_vector_counter++;
    tvec->v_rfcnt  = 1;
    
    ADCL_array_get_next_free_pos ( ADCL_vector_farray, &(tvec->v_findex) );
    ADCL_array_set_element ( ADCL_vector_farray,
                 tvec->v_findex,
                 tvec->v_id,
                 tvec );
    
    ADCL_vector_get_realdim ( ndims, dims, nc, &rndims, &rdims );
    tvec->v_ndims   = rndims;
    tvec->v_nc      = nc;
    tvec->v_comtype = comtype;
    tvec->v_hwidth  = hwidth;
    tvec->v_dims    = rdims;

    tvec->v_alloc = TRUE;
    tvec->v_dat   = dat;
    
    /* allocate the according data array */
    tvec->v_data = ADCL_allocate_matrix ( rndims, rdims, dat, &(tvec->v_matrix) );
    if ( NULL == tvec->v_data ) {
        free ( tvec );
        *vec = NULL;
        return ADCL_ERROR_INTERNAL;
    }
    
    *vec = tvec;
    return ADCL_SUCCESS;

    /* TBD later on */
    switch ( comtype ) {
    /* Halo cells are to be communicated */
    case ADCL_VECTOR_HALO :
        return ADCL_SUCCESS;
        break;
    /* All the matrix is to be communicated */
    case ADCL_VECTOR_ALL :
        /* Not supported until now */
        return ADCL_INVALID_COMTYPE;
        break;

    /* The upper triang is to be communicated */
    case ADCL_VECTOR_UP_TRIANG :
        /* Not supported until now */
        return ADCL_INVALID_COMTYPE;
        break;

    /* The lower triang is to be communicated */
    case ADCL_VECTOR_LO_TRIANG :
        /* Not supported until now */
        return ADCL_INVALID_COMTYPE;
        break;

    default:
        /* User error, should be tested in ADCL_Vector_allocate  */
        return ADCL_INVALID_COMTYPE;
        break;
    }
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_vector_free  ( ADCL_vector_t **vec )
{
    ADCL_vector_t *tvec=*vec;

    tvec->v_rfcnt--;
    if ( tvec->v_rfcnt == 0 ) {
        if ( NULL != tvec->v_dims ) {
            free ( tvec->v_dims);
        }

        ADCL_free_matrix ( tvec->v_ndims, tvec->v_dat, tvec->v_matrix );
        ADCL_array_remove_element ( ADCL_vector_farray, tvec->v_findex );
        free ( tvec );
    }
    else if ( tvec->v_rfcnt < 0 ) {
        return ADCL_ERROR_INTERNAL;
    }

    *vec = ADCL_VECTOR_NULL;
    return ADCL_SUCCESS;
}


/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_vector_register ( int ndims, int *dims, int nc, int comtype, int hwidth,
               MPI_Datatype dat, void *data,  ADCL_vector_t **vec )
{
    ADCL_vector_t *tvec;
    int rndims, *rdims=NULL;

    
    /* Allocate a new vector object */
    tvec = (ADCL_vector_t *) malloc (sizeof(ADCL_vector_t) );
    if ( NULL == tvec ) {
        return ADCL_NO_MEMORY;
    }
    
    /* Set the according elements of the structure */
    tvec->v_id     = ADCL_local_vector_counter++;
    tvec->v_rfcnt  = 1;
    tvec->v_alloc  = FALSE;
    
    ADCL_array_get_next_free_pos ( ADCL_vector_farray, &(tvec->v_findex) );
    ADCL_array_set_element ( ADCL_vector_farray,
                             tvec->v_findex,
                             tvec->v_id,
                             tvec );
    
    ADCL_vector_get_realdim ( ndims, dims, nc, &rndims, &rdims );
    tvec->v_ndims   = rndims;
    tvec->v_nc      = nc;
    tvec->v_comtype = comtype;
    tvec->v_hwidth  = hwidth;
    tvec->v_dims    = rdims;
    
    tvec->v_data    = data;
    tvec->v_matrix  = NULL;
    tvec->v_dat     = dat;
    
    /* Finally, set the returning handle correctly */
    *vec = tvec;
    return ADCL_SUCCESS;

    /* TBD later on */
    switch ( comtype ) {
    /* Halo cells are to be communicated */
    case ADCL_VECTOR_HALO :
        return ADCL_SUCCESS;
        break;
    /* All the matrix is to be communicated */
    case ADCL_VECTOR_ALL :
        /* Not supported until now */
        return ADCL_INVALID_COMTYPE;
        break;

    /* The upper triangle is to be communicated */
    case ADCL_VECTOR_UP_TRIANG :
        /* Not supported until now */
        return ADCL_INVALID_COMTYPE;
        break;

    /* The lower triangle is to be communicated */
    case ADCL_VECTOR_LO_TRIANG :
        /* Not supported until now */
        return ADCL_INVALID_COMTYPE;
        break;

    default:
        /* User error, should be tested in ADCL_Vector_register */
        return ADCL_INVALID_COMTYPE;
        break;
    }
}


/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_vector_deregister  ( ADCL_vector_t **vec )
{
    ADCL_vector_t *tvec=*vec;

    tvec->v_rfcnt--;
    if ( tvec->v_rfcnt == 0 ) {
        if ( NULL != tvec->v_dims ) {
            free ( tvec->v_dims );
        }
        ADCL_array_remove_element ( ADCL_vector_farray, tvec->v_findex );
        free ( tvec );
    }
    else if ( tvec->v_rfcnt < 0 ) {
        return ADCL_ERROR_INTERNAL;
    }

    *vec = ADCL_VECTOR_NULL;
    return ADCL_SUCCESS;
}

void*  ADCL_vector_get_data_ptr ( ADCL_vector_t *vec )
{
    return vec->v_matrix;
}


/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/* non-public function */
static int ADCL_vector_get_realdim ( int ndims, int *dims, int nc,
                     int *rndims, int **rdims )
{
    int *ldims=NULL;
    int i, lndims;

    if ( nc == 0 ) {
        lndims = ndims;
    }
    else {
        lndims = ndims + 1;
    }

    ldims = (int *) malloc ( lndims * sizeof ( int));
    if ( NULL == ldims ) {
        return ADCL_NO_MEMORY;
    }
    for ( i=0; i<ndims; i++ ) {
        ldims[i] = dims[i];
    }

    if ( nc != 0 ) {
        ldims[lndims-1] = nc;
    }

    *rndims = lndims;
    *rdims = ldims;
    return ADCL_SUCCESS;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_vectset_create ( int maxnum,
                          ADCL_vector_t **svecs,
                          ADCL_vector_t **rvecs,
                          ADCL_vectset_t **vectset )
{
    int i, ret = ADCL_SUCCESS;
    ADCL_vectset_t *newvectset = NULL;

    newvectset = ( ADCL_vectset_t *) calloc (1, sizeof (ADCL_vectset_t));
    if ( NULL == newvectset ) {
        return ADCL_NO_MEMORY;
    }

    newvectset->vs_id = ADCL_local_vectset_counter++;
    ADCL_array_get_next_free_pos ( ADCL_vectset_farray, &(newvectset->vs_findex));
    ADCL_array_set_element ( ADCL_vectset_farray, newvectset->vs_findex,
                             newvectset->vs_id, newvectset );
    /* Make sure all vectors have the same comtype */
    for (i=0; i<maxnum; i++ ) {
        if ( svecs[i]->v_comtype != svecs[0]->v_comtype ||
             rvecs[i]->v_comtype != svecs[0]->v_comtype ) {
            ADCL_printf("Error Generating a vector set: inconsistent comtype across "
                        "multiple vectors ");
            ret = ADCL_USER_ERROR;
            goto exit;
        }
    }
    
    newvectset->vs_maxnum = maxnum;
    
    newvectset->vs_svecs = (ADCL_vector_t **) calloc ( 1, maxnum*sizeof (ADCL_vector_t *));
    newvectset->vs_rvecs = (ADCL_vector_t **) calloc ( 1, maxnum*sizeof (ADCL_vector_t *));

    if ( NULL == newvectset->vs_svecs || NULL == newvectset->vs_rvecs ) {
        ret = ADCL_NO_MEMORY;
        goto exit;
    }

    memcpy ( newvectset->vs_svecs, svecs, maxnum * sizeof (ADCL_vector_t *));
    memcpy ( newvectset->vs_rvecs, rvecs, maxnum * sizeof (ADCL_vector_t *));

    exit:
    if ( ret != ADCL_SUCCESS ) {
        
        ADCL_array_remove_element ( ADCL_vectset_farray, newvectset->vs_findex );
        
        if ( NULL != newvectset->vs_svecs ) {
            free ( newvectset->vs_svecs );
        }
        if ( NULL != newvectset->vs_rvecs ) {
            free ( newvectset->vs_rvecs );
        }
        
        free ( newvectset );
    }

    *vectset = newvectset;
    return ret;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_vectset_free ( ADCL_vectset_t **vectset )
{
    ADCL_vectset_t *tvectset=*vectset;

    if ( NULL != tvectset ) {
        if ( NULL != tvectset->vs_svecs ) {
            free ( tvectset->vs_svecs );
        }
        if ( NULL != tvectset->vs_rvecs ) {
            free ( tvectset->vs_rvecs );
        }
        free ( tvectset );
    }

    *vectset = ADCL_VECTSET_NULL;
    return ADCL_SUCCESS;
}
