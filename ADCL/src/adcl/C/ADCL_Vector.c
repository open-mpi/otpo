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

//int ADCL_Vector_allocate ( int ndims, int *dims, int nc, int vectype, int hwidth,
//                           MPI_Datatype dat, void *data, ADCL_Vector *vec )
//{
//    int i, err;
//    int numints, numaddr, numdats, combiner;
//
//    /* Verification of the input parameters */
//    if ( 0 > ndims ) {
//        return ADCL_INVALID_NDIMS;
//    }
//    if ( NULL == dims ) {
//        return ADCL_INVALID_DIMS;
//    }
//    for ( i=0; i<ndims; i++ ) {
//        if ( 0 > dims[i] ) {
//            return ADCL_INVALID_DIMS;
//        }
//    }
//    if ( 0 > hwidth ) {
//        return ADCL_INVALID_HWIDTH;
//    }
//    if ( 0 > nc ) {
//        return ADCL_INVALID_NC;
//    }
//    if ( MPI_DATATYPE_NULL == dat ) {
//        return ADCL_INVALID_DAT;
//    }
//
//    if ( NULL == data ) {
//        return ADCL_INVALID_ARG;
//    }
//
//    /* Datatype has to be a basic datatype */
//    MPI_Type_get_envelope ( dat, &numints, &numaddr, &numdats, &combiner );
//    if ( MPI_COMBINER_NAMED != combiner ) {
//        return ADCL_INVALID_DAT;
//    }
//    if ( NULL == vec ) {
//        return ADCL_INVALID_ARG;
//    }
//
//    /* Call the backend function */
//    err = ADCL_vector_allocate ( ndims, dims, nc, vectype, hwidth, dat, vec );
//    *((void **) data ) = ADCL_vector_get_data_ptr ( *vec );
//    return err;
//}

int ADCL_Vector_allocate_generic ( int ndims, int *dims, int nc, ADCL_Vmap vmap,
                           MPI_Datatype dat, void *data, ADCL_Vector *vec )
{
    int i, err;
    int numints, numaddr, numdats, combiner;

    /* Verification of the input parameters */
    /* check first for vmap */
    if ( NULL == vmap ) {
        return ADCL_INVALID_VMAP;
    }
    if ( ADCL_VECTOR_INPLACE == vmap->m_vectype ) {
       goto allocate;
    }

    /* if it is not ADCL_VECTOR_INPLACE, verify rest of input parameters */
    if ( 0 > ndims ) {
        return ADCL_INVALID_NDIMS;
    }
    if ( NULL == dims ) {
        return ADCL_INVALID_DIMS;
    }
    for ( i=0; i<ndims; i++ ) {
        if ( 0 > dims[i] ) {
            return ADCL_INVALID_DIMS;
        }
    }
    if ( 0 > nc ) {
        return ADCL_INVALID_NC;
    }
    if ( MPI_DATATYPE_NULL == dat ) {
        return ADCL_INVALID_DAT;
    }

    if ( NULL == data ) {
        return ADCL_INVALID_ARG;
    }

    /* Datatype has to be a basic datatype */
    MPI_Type_get_envelope ( dat, &numints, &numaddr, &numdats, &combiner );
    if ( MPI_COMBINER_NAMED != combiner ) {
        return ADCL_INVALID_DAT;
    }
    if ( NULL == vec ) {
        return ADCL_INVALID_ARG;
    }

allocate:
    /* Call the backend function */
    err = ADCL_vector_allocate_generic ( ndims, dims, nc, vmap, dat, vec );

    if ( ADCL_VECTOR_INPLACE != vmap->m_vectype  ) {
       *((void **) data ) = ADCL_vector_get_data_ptr ( *vec );
    }
    else{
       data = NULL; 
    }
    return err;
}


int ADCL_Vector_free  ( ADCL_Vector *vec )
{
    ADCL_vector_t *tvec;
    int ret; 

    /* Verification of input parameters */
    if ( NULL == vec ) {
        return ADCL_INVALID_ARG;
    }
    tvec = *vec;
    if ( TRUE != tvec->v_alloc ) {
    /* this object has been registered with ADCL_vector_register and
       should therefore use ADCL_vector_deregister instead of the current
       routine! */
        return ADCL_INVALID_DATA;
    }

    ret = ADCL_vector_free ( vec );

    // whatever happens internally, object has to appear deleted
    vec = ADCL_VECTOR_NULL; 

    return ret;
}

//int ADCL_Vector_register ( int ndims, int *dims, int nc, int vectype, int hwidth,
//                           MPI_Datatype dat, void *data, ADCL_Vector *vec )
//{
//    int i;
//    int numints, numaddr, numdats, combiner;
//
//    /* Verification of the input parameters */
//    if ( 0 > ndims ) {
//        return ADCL_INVALID_NDIMS;
//    }
//    if ( NULL == dims ) {
//        return ADCL_INVALID_DIMS;
//    }
//    for ( i=0; i<ndims; i++ ) {
//        if ( 0 > dims[i] ) {
//            return ADCL_INVALID_DIMS;
//        }
//            }
//    if ( 0 > hwidth ) {
//        return ADCL_INVALID_HWIDTH;
//    }
//    if ( 0 > nc ) {
//        return ADCL_INVALID_NC;
//    }
//    if ( MPI_DATATYPE_NULL == dat ) {
//        return ADCL_INVALID_DAT;
//    }
//    if (  0 >= vectype || 4 < vectype ) {
//        return ADCL_INVALID_VECTYPE;
//    }
//    /* Datatype has to be a basic datatype */
//    MPI_Type_get_envelope ( dat, &numints, &numaddr, &numdats, &combiner );
//    if ( MPI_COMBINER_NAMED != combiner ) {
//        return ADCL_INVALID_DAT;
//    }
//    if ( NULL == data ) {
//        return ADCL_INVALID_DATA;
//    }
//    if ( NULL == vec ) {
//        return ADCL_INVALID_ARG;
//    }
//
//    return ADCL_vector_register ( ndims, dims, nc, vectype, hwidth, dat, data, vec );
//}

int ADCL_Vector_register_generic ( int ndims, int *dims, int nc, ADCL_Vmap vmap, 
                           MPI_Datatype dat, void *data, ADCL_Vector *vec )
{
    int i;
    int numints, numaddr, numdats, combiner;

    /* Verification of the input parameters */
    /* check first for vmap */
    if ( NULL == vmap ) {
        return ADCL_INVALID_VMAP;
    }
    if ( ADCL_VECTOR_INPLACE == vmap->m_vectype ) {
       if ( MPI_IN_PLACE != data ) {
          return ADCL_INVALID_DAT; 
       } 
       else {
          goto allocate;
       }
    }

    /* if it is not ADCL_VECTOR_INPLACE, verify rest of input parameters */
    if ( 0 > ndims ) {
        return ADCL_INVALID_NDIMS;
    }
    if ( NULL == dims ) {
        return ADCL_INVALID_DIMS;
    }
    for ( i=0; i<ndims; i++ ) {
        if ( 0 > dims[i] ) {
            return ADCL_INVALID_DIMS;
        }
            }
    if ( 0 > nc ) {
        return ADCL_INVALID_NC;
    }
    if ( MPI_DATATYPE_NULL == dat ) {
        return ADCL_INVALID_DAT;
    }
    if (  0 >= vmap->m_vectype || 9 < vmap->m_vectype ) {
        return ADCL_INVALID_VECTYPE;
    }
    /* Datatype has to be a basic datatype */
    MPI_Type_get_envelope ( dat, &numints, &numaddr, &numdats, &combiner );
    if ( MPI_COMBINER_NAMED != combiner ) {
        return ADCL_INVALID_DAT;
    }
    if ( NULL == data ) {
        return ADCL_INVALID_DATA;
    }
    if ( NULL == vec ) {
        return ADCL_INVALID_ARG;
    }

allocate:
    return ADCL_vector_register_generic ( ndims, dims, nc, vmap, dat, data, vec );
}

int ADCL_Vector_deregister  ( ADCL_Vector *vec )
{
    ADCL_vector_t *tvec;

    /* Verification of input parameters */
    if ( NULL == vec ) {
    return ADCL_INVALID_ARG;
    }
    tvec = *vec;
    if ( FALSE != tvec->v_alloc ) {
    /* this object has been allocated with ADCL_vector_allocate and
       should therefore use ADCL_vector_free instead of the current
       routine! */
    return ADCL_INVALID_DATA;
    }

    return ADCL_vector_deregister ( vec );
}

int ADCL_Vectset_create ( int maxnum,
                          ADCL_Vector  *svecs,
                          ADCL_Vector  *rvecs,
                          ADCL_Vectset *vectset )
{
    int i;

    if ( 0 >= maxnum ) {
        return ADCL_INVALID_ARG;
    }
    if ( NULL == svecs || NULL == rvecs ) {
        return ADCL_INVALID_ARG;
    }
    for ( i = 0; i < maxnum; i++ ) {
        if ( 0 > svecs[i]->v_id ||
             0 > rvecs[i]->v_id) {
            return ADCL_INVALID_VECTOR;
        }
    }

    return ADCL_vectset_create ( maxnum, svecs, rvecs, vectset );
}

int ADCL_Vectset_free ( ADCL_Vectset *vectset )
{
    if ( NULL == vectset ) {
        return ADCL_INVALID_ARG;
    }

    if ( 0 > (*vectset)->vs_id ) {
        return ADCL_INVALID_VECTSET;
    }

    return ADCL_vectset_free ( vectset );
}
