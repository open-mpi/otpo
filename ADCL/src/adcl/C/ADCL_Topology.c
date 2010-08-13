/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2009           HLRS. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL.h"
#include "ADCL_internal.h"

int ADCL_Topology_create_generic ( int ndims, int nneigh, int *lneighbors, int *rneighbors, int* flip, 
                   int *coords, int direction, MPI_Comm comm,
                   ADCL_Topology *topo)
{
    /*
    ** Please note, that if ndims == 0  it is legal to pass in
    ** NULL pointers for the neighbors list and for the coords
    */

    if ( 0 != ndims ) {
        if ( NULL == lneighbors || NULL == rneighbors ||
             NULL == flip || NULL == coords || 0 >= nneigh ) {
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

    return ADCL_topology_create_generic ( ndims, nneigh, lneighbors, rneighbors, flip, 
                      coords, direction, comm, topo );
}

int ADCL_Topology_create ( MPI_Comm comm, ADCL_Topology *topo)
{
    int topo_type;

    ///* Right now we can only handle cartesian topologies! */
    //MPI_Topo_test ( cart_comm, &topo_type );
    //if ( MPI_UNDEFINED==topo_type || MPI_GRAPH==topo_type ) {
    //    return ADCL_INVALID_COMM;
    //}

    if ( NULL == topo ) {
        return ADCL_INVALID_TOPOLOGY;
    }

    return ADCL_topology_create ( comm, topo );
}

int ADCL_Topology_create_extended ( MPI_Comm cart_comm, ADCL_Topology *topo)
{
    int topo_type;
    int ndims;

    ///* Right now we can only handle cartesian topologies! */
    //MPI_Topo_test ( cart_comm, &topo_type );
    //if ( MPI_UNDEFINED==topo_type || MPI_GRAPH==topo_type ) {
    //    return ADCL_INVALID_COMM;
    //}

    if ( NULL == topo ) {
        return ADCL_INVALID_TOPOLOGY;
    }

    MPI_Cartdim_get ( cart_comm, &ndims );
    if ( 1 > ndims || 3 < ndims ) {
        return ADCL_INVALID_TOPOLOGY;
    }

    return ADCL_topology_create_extended ( cart_comm, topo );
}

int ADCL_Topology_free ( ADCL_Topology *topo )
{
    ADCL_topology_t *ptopo = *topo;
    int ret;

    if ( NULL == topo  ) {
        return ADCL_INVALID_ARG;
    }
    if ( ptopo->t_id < 0 ) {
        return ADCL_INVALID_TOPOLOGY;
    }

    ret = ADCL_topology_free ( topo );
    topo = ADCL_TOPOLOGY_NULL;

    return ret; 
}

int ADCL_Topology_dump ( ADCL_Topology topo)
{
    if ( NULL == topo ) {
        return ADCL_INVALID_TOPOLOGY;
    }

    return ADCL_topology_dump ( topo );
}

int ADCL_Topology_get_cart_number_neighbors ( int ndims, int extended, int* nneigh )
{
    /* needed, since memory management should be at user's side */ 
    if ( 1 > ndims || 3 < ndims ) {
        return ADCL_INVALID_ARG;
    }

    if ( extended != 0 || extended != 1 ) {
        return ADCL_INVALID_ARG;
    }

    ADCL_topology_get_cart_number_neighbors ( ndims, extended, nneigh );

    return ADCL_SUCCESS;
}

int ADCL_Topology_get_cart_neighbors ( int nneigh, int* lneighbors, int* rneighbors, int* flip, 
        MPI_Comm cart_comm )
{
    /*IN: nneigh                 - dimension of lneighbors and rneighbors
          cart_comm              - cartesian communicator 
      OUT: lneighbors, rneighbors - arrays with left and right neighbors */

    int ndims, topo_type;
    int *coords, *periods, *cdims;
    int nneigh_trad, nneigh_ext;

    if ( 0 >= nneigh || NULL == lneighbors || NULL == rneighbors ||
         NULL == flip || NULL == (void*) cart_comm || MPI_COMM_NULL == cart_comm ) {
        return ADCL_INVALID_ARG;
    }

    /* Right now we can only handle cartesian topologies with dimension 1 to 3 ! */
    MPI_Topo_test ( cart_comm, &topo_type );
    if ( MPI_UNDEFINED==topo_type || MPI_GRAPH==topo_type ) {
        return ADCL_INVALID_COMM;
    }
    MPI_Cartdim_get ( cart_comm, &ndims );
    if ( 1 > ndims || 3 < ndims ) {
        return ADCL_INVALID_TOPOLOGY;
    }

    ADCL_topology_get_cart_number_neighbors ( ndims, 0, &nneigh_trad ); 
    ADCL_topology_get_cart_number_neighbors ( ndims, 1, &nneigh_ext );

    if ( nneigh != nneigh_trad && nneigh != nneigh_ext  ) {
        return ADCL_INVALID_ARG;
    }
        
    cdims   = (int*) malloc( ndims * sizeof(int) );
    periods = (int*) malloc( ndims * sizeof(int) );
    coords  = (int*) malloc( ndims * sizeof(int) );
    MPI_Cart_get (cart_comm, ndims, cdims, periods, coords);

    ADCL_topology_get_cart_neighbors ( ndims, nneigh, lneighbors, rneighbors, flip, 
        cdims, periods, coords, cart_comm );

    free( cdims );
    free( periods );
    free( coords );

    return ADCL_SUCCESS;
}
