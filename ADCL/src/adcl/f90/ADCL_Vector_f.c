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
#include "ADCL_fprototypes.h"

/* Note: Vector_allocate, vector_free and vector_get_data_ptr are
   intentionally not offered in the fortran interface. */

#ifndef _SX
//#pragma weak adcl_vector_register_  = adcl_vector_register
//#pragma weak adcl_vector_register__ = adcl_vector_register
//#pragma weak ADCL_VECTOR_REGISTER   = adcl_vector_register

#pragma weak adcl_vector_register_generic_  = adcl_vector_register_generic
#pragma weak adcl_vector_register_generic__ = adcl_vector_register_generic
#pragma weak ADCL_VECTOR_REGISTER_GENERIC   = adcl_vector_register_generic

#pragma weak adcl_vector_deregister_  = adcl_vector_deregister
#pragma weak adcl_vector_deregister__ = adcl_vector_deregister
#pragma weak ADCL_VECTOR_DEREGISTER   = adcl_vector_deregister

#pragma weak adcl_vectset_create_  = adcl_vectset_create
#pragma weak adcl_vectset_create__ = adcl_vectset_create
#pragma weak ADCL_VECTSET_CREATE   = adcl_vectset_create

#pragma weak adcl_vectset_free_  = adcl_vectset_free
#pragma weak adcl_vectset_free__ = adcl_vectset_free
#pragma weak ADCL_VECTSET_FREE   = adcl_vectset_free
#endif

//#ifdef _SX
//void adcl_vector_register_( int *ndims, int *dims, int *nc, int *vectype, int *hwidth,
//                            int *dat, void *data, int *vec, int *ierror)
//#else
//void adcl_vector_register ( int *ndims, int *dims, int *nc, int *vectype, int *hwidth,
//                            int *dat, void *data, int *vec, int *ierror)
//#endif
//{
//    int i;
//    ADCL_vector_t *cvec;
//    MPI_Datatype cdat;
//
//    if ( ( NULL == ndims ) ||
//         ( NULL == dims )  ||
//         ( NULL == nc   )  ||
//         ( NULL == hwidth) ||
//         ( NULL == dat )   ||
//         ( NULL == data )  ||
//         ( NULL == vec )   ) {
//        *ierror = ADCL_INVALID_ARG;
//        return;
//    }
//    /* Verification of the input parameters */
//    if ( 0 > *ndims ) {
//        *ierror = ADCL_INVALID_NDIMS;
//        return;
//    }
//    for ( i=0; i<*ndims; i++ ) {
//        if ( 0 > dims[i] ) {
//            *ierror = ADCL_INVALID_DIMS;
//            return;
//        }
//    }
//    if ( 0 > *hwidth ) {
//        *ierror = ADCL_INVALID_HWIDTH;
//        return;
//    }
//    if ( 0 > *nc ) {
//        *ierror = ADCL_INVALID_NC;
//        return;
//    }
//    if (  0 >= *vectype || 4 < *vectype ) {
//        *ierror = ADCL_INVALID_VECTYPE;
//        return;
//    }
//
//    cdat = MPI_Type_f2c (*dat);
//    if ( cdat == MPI_DATATYPE_NULL ) {
//        *ierror = ADCL_INVALID_TYPE;
//        return;
//    }
//
//    if ( cdat == MPI_DOUBLE_PRECISION ) {
//        cdat = MPI_DOUBLE;
//    }
//    else if ( cdat == MPI_REAL ) {
//        cdat = MPI_FLOAT;
//    }
//    else if ( cdat == MPI_INTEGER ) {
//        cdat = MPI_INT;
//    }
//
//    *ierror = ADCL_vector_register (*ndims, dims, *nc, *vectype, *hwidth,
//                                    cdat, data, &cvec );
//    if ( *ierror == ADCL_SUCCESS ) {
//        *vec = cvec->v_findex;
//    }
//    //free (cdat); 
//
//    return;
//}

#ifdef _SX
void adcl_vector_register_generic_ ( int *ndims, int *dims, int *nc, int *vmap,
                           int *dat, void *data, int *vec, int *ierror )
#else
void adcl_vector_register_generic  ( int *ndims, int *dims, int *nc, int *vmap,
                           int *dat, void *data, int *vec, int* ierror )
#endif
{
    int i;
    ADCL_vmap_t *cvmap;
    ADCL_vector_t *cvec;
    MPI_Datatype cdat;

    /* Verification of the input parameters */
    /* check first for vmap */
    if ( NULL == vmap ) {
       *ierror = ADCL_INVALID_VMAP;
       return;
    }
    cvmap = (ADCL_vmap_t *) ADCL_array_get_ptr_by_pos ( ADCL_vmap_farray, *vmap );
    if ( ADCL_VECTOR_INPLACE == cvmap->m_vectype ) {
       /* how do I verify that the user set data == MPI_IN_PLACE? */
       data = MPI_IN_PLACE;
       goto allocate;
    }

    /* if it is not ADCL_VECTOR_INPLACE, check rest of input parameters */
    if ( ( NULL == ndims ) ||
         ( NULL == dims )  ||
         ( NULL == nc   )  ||
         ( NULL == dat )   ||
         ( NULL == data )  ||
         ( NULL == vec )   ) {
        *ierror = ADCL_INVALID_ARG;
        return;
    }

    /* Verification of the input parameters */
    if ( 0 > *ndims ) {
        *ierror = ADCL_INVALID_NDIMS;
        return;
    }
    for ( i=0; i<*ndims; i++ ) {
        if ( 0 > dims[i] ) {
            *ierror = ADCL_INVALID_DIMS;
            return;
        }
    }
    if ( 0 > *nc ) {
        *ierror = ADCL_INVALID_NC;
        return;
    }

    cdat = MPI_Type_f2c (*dat);
    if ( cdat == MPI_DATATYPE_NULL ) {
        *ierror = ADCL_INVALID_TYPE;
        return;
    }

    if ( cdat == MPI_DOUBLE_PRECISION ) {
        cdat = MPI_DOUBLE;
    }
    else if ( cdat == MPI_REAL ) {
        cdat = MPI_FLOAT;
    }
    else if ( cdat == MPI_INTEGER ) {
        cdat = MPI_INT;
    }

allocate:
    *ierror = ADCL_vector_register_generic ( *ndims, dims, *nc, cvmap, cdat, data, &cvec );
    if ( *ierror == ADCL_SUCCESS ) {
        *vec = cvec->v_findex;
    }


}


#ifdef _SX
void adcl_vector_deregister_( int *vec, int *ierror )
#else
void adcl_vector_deregister ( int *vec, int *ierror )
#endif
{
    ADCL_vector_t *cvec;

    cvec = (ADCL_vector_t *) ADCL_array_get_ptr_by_pos ( ADCL_vector_farray,
                                                         *vec );
    *ierror = ADCL_vector_deregister ( &cvec );

    *vec = ADCL_FVECTOR_NULL;
    return;
}

#ifdef _SX
void adcl_vectset_create_( int *maxnum, int *svecs, int *rvecs,
                           int *vectset, int *ierror )
#else
void adcl_vectset_create ( int *maxnum, int *svecs, int *rvecs,
                           int *vectset, int *ierror )
#endif
{
    int i;
    ADCL_vector_t **csvecs;
    ADCL_vector_t **crvecs;
    ADCL_vectset_t *cvectset;

    if ( ( NULL == maxnum ) ||
         ( NULL == svecs )  ||
         ( NULL == rvecs )  ||
         ( NULL == vectset) ) {
        *ierror = ADCL_INVALID_ARG;
        return;
    }
    if ( 0 >= *maxnum ) {
        *ierror = ADCL_INVALID_ARG;
        return;
    }
    csvecs = ( ADCL_vector_t **) malloc ( *maxnum * sizeof ( ADCL_vector_t *));
    crvecs = ( ADCL_vector_t **) malloc ( *maxnum * sizeof ( ADCL_vector_t *));

    for ( i=0; i< *maxnum; i++) {
        csvecs[i] = (ADCL_vector_t *) ADCL_array_get_ptr_by_pos ( ADCL_vector_farray,
                                                                  svecs[i] );
        crvecs[i] = (ADCL_vector_t *) ADCL_array_get_ptr_by_pos ( ADCL_vector_farray,
                                                                  rvecs[i] );
        if ( ( NULL == csvecs[i] )   ||
             ( NULL == crvecs[i] )   ||
             ( 0 > csvecs[i]->v_id ) ||
             ( 0 > crvecs[i]->v_id ) ) {
            *ierror = ADCL_INVALID_VECTOR;
            return;
        }
    }

    *ierror = ADCL_vectset_create ( *maxnum, csvecs, crvecs, &cvectset );
    if ( *ierror == ADCL_SUCCESS ) {
        *vectset = cvectset->vs_findex;
    }
    free( csvecs );
    free( crvecs );

    return;
}

#ifdef _SX
void adcl_vectset_free_( int *vectset, int *ierror )
#else
void adcl_vectset_free ( int *vectset, int *ierror )
#endif
{
    ADCL_vectset_t *cvectset;

    if ( NULL == vectset )  {
        *ierror = ADCL_INVALID_ARG;
        return;
    }

    cvectset = (ADCL_vectset_t *) ADCL_array_get_ptr_by_pos ( ADCL_vectset_farray,
                                                              *vectset );
    if ( NULL == cvectset ) {
        *ierror = ADCL_INVALID_VECTSET;
        return;
    }

    *ierror = ADCL_vectset_free ( &cvectset );
    
    *vectset = ADCL_FVECTSET_NULL;
    
    return;
}
