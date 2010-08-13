/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2008           High Performance Computing Center Stuttgart. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef __ADCL_CONTIGUOUS_H__
#define __ADCL_CONTIGUOUS_H__

#ifdef ADCL_DUMMY_MPI
#include "ADCL_dummy_mpi.h"
#else
#include "mpi.h"
#endif


int ADCL_contiguous_init ( int vecndims, MPI_Datatype btype,
             MPI_Datatype **dats);
void ADCL_contigous_free ( int num, MPI_Datatype **dats );


int ADCL_contiguous_init_generic ( MPI_Datatype btype, int cnt,
             MPI_Datatype **dats, int** cnts);
void ADCL_contiguous_free_generic ( int num, MPI_Datatype **dats, int **cnts );

			     //
#endif /* __ADCL_CONTIGUOUS_H__ */
