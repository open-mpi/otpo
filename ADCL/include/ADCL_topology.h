/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef __ADCL_TOPOLOGY_H__
#define __ADCL_TOPOLOGY_H__

#if ADCL_DUMMY_MPI
#include "ADCL_dummy_mpi.h"
#else
#include "mpi.h"
#endif

struct ADCL_topology_s{
    int         t_id; /* id of the object */
    int     t_findex; /* index of this object in the fortran array */
    MPI_Comm  t_comm; /* communicator used for data exchange */
    int       t_rank; /* rank of this process in the t_comm */
    int       t_size; /* size of t_comm */
    int      t_ndims; /* number of dimension of this process topology */
    int *t_neighbors; /* array of neighboring processes, dimension
                         2*t_ndims */
    int    *t_coords; /* coordinate of this proc in the proc-topology,
                         dimension t_ndims */
};
typedef struct ADCL_topology_s ADCL_topology_t;
extern ADCL_array_t *ADCL_topology_farray;

int ADCL_topology_create ( MPI_Comm cart_comm, ADCL_topology_t **topo );
int ADCL_topology_free   ( ADCL_topology_t **topo);
int ADCL_topology_create_generic ( int ndims, int *lneighbors, int *rneighbors,
                                   int *coords,  int direction, MPI_Comm comm,
                                   ADCL_topology_t **topo);

#endif
