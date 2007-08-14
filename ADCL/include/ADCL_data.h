/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef __ADCL_DATA_H__
#define __ADCL_DATA_H__

#if ADCL_DUMMY_MPI
#include "ADCL_dummy_mpi.h"
#else
#include "mpi.h"
#endif

struct ADCL_data_s{
    int                     d_id; /* id of the object */
    int                 d_findex; /* index of this object in the fortran array */
    int                 d_refcnt; /* reference counter of this object */
    /* Topology information */
    int                 d_tndims; /* Topology number of dimensions */
    int              *d_tperiods; /* periodicity for each cartesian dimension */
    /* Vector information */
    int                 d_vndims; /* Vector number of dimensions */
    int                 *d_vdims; /* Vector extent of each the dimensions */
    int                     d_nc; /* Extent of each data point  */
    int                 d_hwidth; /* Halo cells width */
    int                d_comtype; /* Communication type */
    /* Function set and winner function */
    char               *d_fsname; /* Function set name */
    char               *d_wfname; /* Winner function name */
};
typedef struct ADCL_data_s ADCL_data_t;

extern ADCL_array_t *ADCL_data_array;

int  ADCL_data_create ( ADCL_emethod_t *e );
void ADCL_data_free   ( void );
int  ADCL_data_find   ( ADCL_emethod_t *e, ADCL_data_t **data );
void ADCL_data_read_from_file ( void );

#endif /* __ADCL_DATA_H__ */
