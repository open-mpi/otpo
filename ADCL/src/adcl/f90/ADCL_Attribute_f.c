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

#ifndef _SX
#pragma weak adcl_attribute_create_  = adcl_attribute_create
#pragma weak adcl_attribute_create__ = adcl_attribute_create
#pragma weak ADCL_ATTRIBUTE_CREATE   = adcl_attribute_create

#pragma weak adcl_attribute_free_  = adcl_attribute_free
#pragma weak adcl_attribute_free__ = adcl_attribute_free
#pragma weak ADCL_ATTRIBUTE_FREE   = adcl_attribute_free


#pragma weak adcl_attrset_create_  = adcl_attrset_create
#pragma weak adcl_attrset_create__ = adcl_attrset_create
#pragma weak ADCL_ATTRSET_CREATE   = adcl_attrset_create

#pragma weak adcl_attrset_free_  = adcl_attrset_free
#pragma weak adcl_attrset_free__ = adcl_attrset_free
#pragma weak ADCL_ATTRSET_FREE   = adcl_attrset_free
#endif

#ifdef _SX
void adcl_attribute_create_ ( int* maxnvalues, int *array_of_values, 
			    int *attr, int *ierr )
#else
void adcl_attribute_create ( int* maxnvalues, int *array_of_values, 
			    int *attr, int *ierr )
#endif
{
    ADCL_attribute_t *tattr;

    if ( ( NULL == maxnvalues )      ||
	 ( NULL == array_of_values ) ||
	 ( NULL == attr ) ){
	*ierr = ADCL_INVALID_ARG;
	return;
    }
	 
    *ierr = ADCL_attribute_create ( *maxnvalues, array_of_values, NULL, NULL, &tattr );
    if ( *ierr == ADCL_SUCCESS ) {
	*attr = tattr->a_findex;
    }

    return;
}

#ifdef _SX
void adcl_attribute_free_  ( int *attr, int *ierr )
#else
void adcl_attribute_free   ( int *attr, int *ierr )
#endif
{
    ADCL_attribute_t *tattr;

    if ( NULL == attr ) {
	*ierr = ADCL_INVALID_ARG;
	return;
    }

    tattr = (ADCL_attribute_t *) ADCL_array_get_ptr_by_pos ( ADCL_attribute_farray, *attr);
    *ierr = ADCL_attribute_free (&tattr);
    *attr = ADCL_FATTRIBUTE_NULL;

    return;
}

#ifdef _SX
void adcl_attrset_create_( int* maxnum, int *array_of_attributes, 
			   int *attrset, int *ierr )
#else
void adcl_attrset_create ( int* maxnum, int *array_of_attributes, 
			   int *attrset, int *ierr )
#endif
{
    int i;
    ADCL_attrset_t *tattr;
    ADCL_attribute_t **cattr;

    if ( ( NULL == maxnum )      ||
	 ( NULL == array_of_attributes ) ||
	 ( NULL == attrset ) ){
	*ierr = ADCL_INVALID_ARG;
	return;
    }

    cattr = (ADCL_attribute_t **) malloc ( *maxnum * sizeof(ADCL_attribute_t *));
    if ( NULL == cattr ) {
	*ierr = ADCL_NO_MEMORY;
	return;
    }

    for (i=0; i<*maxnum; i++ ) {
	cattr[i] = (ADCL_attribute_t *) ADCL_array_get_ptr_by_pos ( ADCL_attribute_farray,
								    array_of_attributes[i] );
    }

    *ierr = ADCL_attrset_create ( *maxnum, cattr, &tattr );
    if ( *ierr == ADCL_SUCCESS ) {
	*attrset = tattr->as_findex;
    }

    return;

}

#ifdef _SX
void adcl_attrset_free_( int *attrset, int *ierr )
#else
void adcl_attrset_free ( int *attrset, int *ierr )
#endif
{
    ADCL_attrset_t *tattr;

    if ( NULL == attrset ) {
	*ierr = ADCL_INVALID_ARG;
	return;
    }

    tattr = (ADCL_attrset_t *) ADCL_array_get_ptr_by_pos ( ADCL_attrset_farray, *attrset);
    *ierr = ADCL_attrset_free (&tattr);
    *attrset = ADCL_FATTRSET_NULL;

    return;
}
