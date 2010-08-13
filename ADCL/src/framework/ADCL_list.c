/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2008           High Performance Computing Center Stuttgart. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"

/* these function make no sense, since the OpenMPI functions only 
 * require the basic data types */

int ADCL_list_init ( int size, int* rcnts, MPI_Datatype btype,
             MPI_Datatype **dats )
{
    int err = ADCL_SUCCESS;
    MPI_Datatype *tdats=NULL; 

    tdats = ( MPI_Datatype *) malloc ( size * sizeof(MPI_Datatype));
    if ( NULL == tdats  ) {
       return ADCL_NO_MEMORY;
    }

    tdats[0] = btype;

    *dats = tdats;
    return err;
}

void ADCL_list_free ( int num, MPI_Datatype **dats )
{
    MPI_Datatype *tdats = *dats;

    if ( NULL != tdats ) {
    //for ( i=0; i<num; i++ ){
    //    if ( MPI_DATATYPE_NULL != tdats[i]) {
    //    MPI_Type_free ( &(tdats[i]));
    //    }
    //}
    free ( tdats );
    }

    *dats = NULL;
    return;
}

