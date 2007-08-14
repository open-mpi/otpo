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

int ADCL_Topology_create_generic ( int ndims, int *lneighbors, int *rneighbors,
                   int *coords, int direction, MPI_Comm comm,
                   ADCL_Topology *topo)
{
    /*
    ** Please note, that if ndims == 0  it is legal to pass in
    ** NULL pointers for the neighbors list and for the coords
    */

    if ( 0 != ndims ) {
        if ( NULL == lneighbors || NULL == rneighbors ||
             NULL == coords ) {
            return ADCL_INVALID_ARG;
        }
    }

    if ( NULL == topo ) {
        return ADCL_INVALID_TOPOLOGY;
    }

    if ( direction != ADCL_DIRECTION_BOTH &&
         direction != ADCL_DIRECTION_LEFT_TO_RIGHT &&
         direction != ADCL_DIRECTION_RIGHT_TO_LEFT ){
        return ADCL_INVALID_DIRECTION;
    }

    return ADCL_topology_create_generic ( ndims, lneighbors, rneighbors,
                      coords, direction, comm, topo );
}

int ADCL_Topology_create ( MPI_Comm cart_comm, ADCL_Topology *topo)
{
    int topo_type;

    /* Right now we can only handle cartesian topologies! */
    MPI_Topo_test ( cart_comm, &topo_type );
    if ( MPI_UNDEFINED==topo_type || MPI_GRAPH==topo_type ) {
        return ADCL_INVALID_COMM;
    }

    if ( NULL == topo ) {
        return ADCL_INVALID_TOPOLOGY;
    }

    return ADCL_topology_create ( cart_comm, topo );
}


int ADCL_Topology_free ( ADCL_Topology *topo )
{
    ADCL_topology_t *ptopo = *topo;

    if ( NULL == topo  ) {
        return ADCL_INVALID_ARG;
    }
    if ( ptopo->t_id < 0 ) {
        return ADCL_INVALID_TOPOLOGY;
    }

    return ADCL_topology_free ( topo );
}

