/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"

ADCL_array_t *ADCL_topology_farray;
static int ADCL_local_id_counter=0;

int ADCL_topology_create_generic ( int ndims, int *lneighbors, int *rneighbors,
                   int *coords, int direction, MPI_Comm comm,
                   ADCL_topology_t **topo)
{
    ADCL_topology_t *newtopo=NULL;
    int i, j;

    newtopo = (ADCL_topology_t *) calloc ( 1, sizeof ( ADCL_topology_t ));
    if ( NULL == newtopo ) {
    return ADCL_NO_MEMORY;
    }

    newtopo->t_id = ADCL_local_id_counter++;
    ADCL_array_get_next_free_pos ( ADCL_topology_farray, &(newtopo->t_findex));
    ADCL_array_set_element ( ADCL_topology_farray,
                 newtopo->t_findex,
                 newtopo->t_id,
                 newtopo );

    newtopo->t_ndims = ndims;
    newtopo->t_comm  = comm;
    if ( ndims > 0 ) {
    newtopo->t_neighbors = (int *) malloc ( 2 * ndims * sizeof(int));
    newtopo->t_coords    = (int *) malloc ( ndims * sizeof (int));
    if ( NULL == newtopo->t_coords || NULL == newtopo->t_neighbors ) {
        free ( newtopo );
        return ADCL_NO_MEMORY;
    }

    for ( i=0, j=0; i< ndims; i++ ) {
        newtopo->t_neighbors[j++] = lneighbors[i];
        newtopo->t_neighbors[j++] = rneighbors[i];
        newtopo->t_coords[i]      = coords[i];
    }
    }

    MPI_Comm_rank ( comm, &(newtopo->t_rank) );
    MPI_Comm_size ( comm, &(newtopo->t_size) );

    *topo = newtopo;
    return ADCL_SUCCESS;
}

int ADCL_topology_create ( MPI_Comm cart_comm, ADCL_topology_t **topo )
{
    ADCL_topology_t *newtopo=NULL;
    int cartdim, i;

    newtopo = (ADCL_topology_t *) calloc ( 1, sizeof ( ADCL_topology_t ));
    if ( NULL == newtopo ) {
    return ADCL_NO_MEMORY;
    }

    newtopo->t_id = ADCL_local_id_counter++;
    ADCL_array_get_next_free_pos ( ADCL_topology_farray, &(newtopo->t_findex));
    ADCL_array_set_element ( ADCL_topology_farray,
                 newtopo->t_findex,
                 newtopo->t_id,
                 newtopo );

    MPI_Comm_rank ( cart_comm, &(newtopo->t_rank) );
    MPI_Comm_size ( cart_comm, &(newtopo->t_size) );
    MPI_Cartdim_get ( cart_comm, &cartdim );

    newtopo->t_comm  = cart_comm;
    newtopo->t_ndims = cartdim;
    newtopo->t_coords = (int *)malloc ( cartdim * sizeof(int));
    newtopo->t_neighbors = (int *) malloc ( 2*cartdim *sizeof(int));
    if ( NULL == newtopo->t_neighbors || NULL == newtopo->t_coords  ) {
    free ( newtopo );
    return ADCL_NO_MEMORY;
    }

    MPI_Cart_coords ( cart_comm, newtopo->t_rank, cartdim, newtopo->t_coords );
    for ( i=0; i< cartdim; i++ ) {
    MPI_Cart_shift ( cart_comm, i, 1, &(newtopo->t_neighbors[2*i]),
             &(newtopo->t_neighbors[2*i+1]) );
    }

    *topo = newtopo;
    return ADCL_SUCCESS;
}

int ADCL_topology_free ( ADCL_topology_t **topo)
{
    ADCL_topology_t *ttopo=*topo;

    if ( NULL != ttopo )
    {
        ADCL_array_remove_element ( ADCL_topology_farray, ttopo->t_findex);
        if ( NULL != ttopo->t_coords ) {
            free ( ttopo->t_coords);
        }
        if ( NULL != ttopo->t_neighbors ) {
            free ( ttopo->t_neighbors );
        }
        free(ttopo);
    }

    *topo = ADCL_TOPOLOGY_NULL;
    return ADCL_SUCCESS;
}
