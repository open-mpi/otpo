/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"


int ADCL_array_init (ADCL_array_t **arr, const char name[64], int size )
{
    ADCL_array_t *tarr;

    tarr = (ADCL_array_t *) malloc ( sizeof(ADCL_array_t) );
    if ( NULL == tarr ) {
    return ADCL_NO_MEMORY;
    }

    strncpy ( tarr->name, name, 64 );
    tarr->last = -1;
    tarr->size = size;

    tarr->array=(ADCL_array_elem_t*)calloc(1,size*sizeof(ADCL_array_elem_t));
    if ( NULL == tarr->array ) {
    free ( tarr );
    return ADCL_NO_MEMORY;
    }

    *arr = tarr;
    return ADCL_SUCCESS;
}

/*
** Right now, we are not checking whether the element array is
** really free
*/
int ADCL_array_free (ADCL_array_t **arr )
{
    ADCL_array_t *tarr=*arr;

    if ( NULL != tarr) {
    if ( NULL != tarr->array ) {
        free ( tarr->array );
    }
    free ( tarr );
    }
    *arr = NULL;
    return ADCL_SUCCESS;
}

/* Please note, we do not reserve this field at this step! */
int ADCL_array_get_next_free_pos  ( ADCL_array_t *arr, int *pos)
{
    ADCL_array_elem_t *oldarray=NULL;
    int oldsize;
    int i;
    int thispos=-1;

    if ( arr->last < (arr->size -1) ) {
    for (i=0; i< arr->size; i++ ) {
        if ( arr->array[i].in_use == 0 ) {
        thispos = i;
        break;
        }
    }
    }
    else {
    oldarray = arr->array;
    oldsize  = arr->size;

    arr->size *= 2;
    arr->array = (ADCL_array_elem_t*) calloc ( 1, arr->size *
                           sizeof(ADCL_array_elem_t));
    if ( NULL == arr->array ) {
        *pos = -1;
        return ADCL_NO_MEMORY;
    }

    memcpy ( arr->array, oldarray, oldsize * sizeof(ADCL_array_elem_t));
    free (oldarray);
    thispos = oldsize;
    }


    *pos = thispos;
    return ADCL_SUCCESS;
}

void * ADCL_array_get_ptr_by_pos ( ADCL_array_t *arr, int pos )
{
    if ( pos < arr->size ) {
    if ( arr->array[pos].in_use == 1 ) {
        return arr->array[pos].ptr;
    }
    }

    return NULL;
}

void * ADCL_array_get_ptr_by_id  ( ADCL_array_t *arr, int id  )
{
    int i;

    for (i=0; i<arr->size; i++ ) {
    if ( arr->array[i].in_use == 1 && arr->array[i].id == id ) {
        return arr->array[i].ptr;
    }
    }

    return NULL;
}

int ADCL_array_get_size ( ADCL_array_t *arr )
{
    return arr->size;
}

int ADCL_array_get_last ( ADCL_array_t *arr )
{
    return arr->last;
}

int ADCL_array_set_element (ADCL_array_t *arr, int pos, int id, void *ptr)
{
    int ret = ADCL_ERROR_INTERNAL;

    if ( pos < arr->size ) {
    if ( 0 == arr->array[pos].in_use ) {
        arr->array[pos].id      = id;
        arr->array[pos].in_use  = 1;
        arr->array[pos].ptr     = ptr;
        ret = ADCL_SUCCESS;
        if ( pos > arr->last ) {
        arr->last = pos;
        }
    }
    }

    return ret;
}

int ADCL_array_remove_element ( ADCL_array_t *arr, int pos )
{
    if ( 1 == arr->array[pos].in_use ) {
    arr->array[pos].ptr    = NULL;
    arr->array[pos].id     = MPI_UNDEFINED;
    arr->array[pos].in_use = FALSE;

    if ( pos == arr->last ) {
        arr->last--;
    }
    }


    return ADCL_SUCCESS;
}
