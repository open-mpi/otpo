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

int ADCL_contiguous_init ( int vecndims, MPI_Datatype btype,
             MPI_Datatype **dats)
 {
     int err = ADCL_SUCCESS;
     MPI_Datatype *tdats=NULL;

     tdats = (MPI_Datatype *) malloc ( sizeof(MPI_Datatype) );
     if ( NULL == tdats  ) {
        return ADCL_NO_MEMORY;
     }

     /* !!! Makes only sense for sbuf != MPI_IN_PLACE */
     /* Loop over all topology dimensions */
     MPI_Type_contiguous ( vecndims, btype, &(tdats[0]));
     MPI_Type_commit ( &(tdats[0]) );


     *dats = tdats;
     
     return err;
 }

void ADCL_contiguous_free ( int num, MPI_Datatype **dats )
 {
     int i;
     MPI_Datatype *tdats = *dats;

     if ( NULL != tdats ) {
        for ( i=0; i<num; i++ ){
           if ( MPI_DATATYPE_NULL != tdats[i]) {
              MPI_Type_free ( &(tdats[i]) );
           }
        }
        free ( tdats );
     }

     *dats = NULL;
     return;
 }

int ADCL_contiguous_init_generic ( MPI_Datatype btype, int cnt,
             MPI_Datatype **dats, int** cnts)
{
    int err = ADCL_SUCCESS;
    MPI_Datatype *tdats=NULL; 
    int *tcnts = NULL;
    //MPI_Aint slb, rlb, sext, rext;

    tdats = (MPI_Datatype *) malloc ( sizeof(MPI_Datatype) );
    if ( NULL == tdats  ) {
       return ADCL_NO_MEMORY;
    }
    tcnts = (int *) malloc ( sizeof(int) );
    if ( NULL == tcnts  ) {
       return ADCL_NO_MEMORY;
    }

    /* !!! Makes only sense for sbuf != MPI_IN_PLACE */
    MPI_Type_contiguous ( cnt, btype, &(tdats[0]));
    MPI_Type_commit ( &(tdats[0]));
    tcnts[0] = 1;

    //err = MPI_Type_get_extent (tdats[0], &slb, &sext);

    *dats = tdats;
    *cnts = tcnts;

    return err;
}

void ADCL_contiguous_free_generic ( int num, MPI_Datatype **dats, int **cnts )
{
    int i;
    MPI_Datatype *tdats = *dats;
    int *tcnts = *cnts;

    if ( NULL != tdats ) {
       for ( i=0; i<num; i++ ){
          if ( MPI_DATATYPE_NULL != tdats[i]) {
             MPI_Type_free ( &tdats[i] );
          }
       }
       free ( tdats );
       free ( tcnts );
    }

    *dats = NULL;
    *cnts = NULL;
    return;
}

