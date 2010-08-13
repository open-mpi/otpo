/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2008           High Performance Computing Center Stuttgart. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef __ADCL_SUBARRAY_H__
#define __ADCL_SUBARRAY_H__

#ifdef ADCL_DUMMY_MPI
#include "ADCL_dummy_mpi.h"
#else
#include "mpi.h"
#endif

int ADCL_list_init ( int size, int* rcnts, MPI_Datatype btype,
             MPI_Datatype **dats); 

void ADCL_list_free ( int num, MPI_Datatype **dats ); 


#endif /* __ADCL_SUBARRAY_H__ */
