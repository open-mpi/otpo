/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"


void* ADCL_allocate_matrix ( int ndims, int *dims, MPI_Datatype dat, void *matpt )
{

    if ( dat == MPI_DOUBLE ) {
        return ADCL_allocate_double_matrix ( ndims, dims, matpt );
    }
    else if ( dat == MPI_FLOAT ) {
        return ADCL_allocate_float_matrix ( ndims, dims, matpt );
    }
    else if ( dat == MPI_INT ) {
        return ADCL_allocate_int_matrix  (ndims, dims, matpt );
    }
    else {
        ADCL_printf("Datatype not supported by ADCL right now!\n");
    }

    return NULL;
}

void ADCL_free_matrix ( int ndims, MPI_Datatype dat, void *mat )
{

    if ( dat == MPI_DOUBLE ) {
        ADCL_free_double_matrix ( ndims, mat);
    }
    else if ( dat == MPI_FLOAT ) {
        ADCL_free_float_matrix ( ndims, mat );
    }
    else if ( dat == MPI_INT ) {
        ADCL_free_int_matrix ( ndims, mat );
    }
    else {
        ADCL_printf("Datatype not supported by ADCL right now!\n");
    }

    return;
}
