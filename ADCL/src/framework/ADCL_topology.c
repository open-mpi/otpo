/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2009           HLRS. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"

ADCL_array_t *ADCL_topology_farray;
static int ADCL_local_id_counter=0;
static int ADCL_get_cart_neighbor_pair ( MPI_Comm cart_comm, int ndims, int* coords, int* direction, 
     int* periods, int *cdims, int* rank1, int* rank2 );
static int ADCL_topology_flip_neighbors ( int* coords, int idim, int* flip, int* lneighbor, int* rneighbor );

int ADCL_topology_create_generic ( int ndims, int nneigh, int *lneighbors, int *rneighbors, int* flip, 
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

    MPI_Comm_rank ( comm, &(newtopo->t_rank) );
    MPI_Comm_size ( comm, &(newtopo->t_size) );

    newtopo->t_comm  = comm;
    newtopo->t_ndims = ndims;

    if ( nneigh > 0 ) {
       newtopo->t_nneigh = nneigh;
       newtopo->t_neighbors = (int *) malloc ( 2 * nneigh * sizeof(int));
       newtopo->t_flip      = (int *) malloc ( nneigh * sizeof(int));
       newtopo->t_coords    = (int *) malloc ( ndims * sizeof (int));
       if ( NULL == newtopo->t_coords || NULL == newtopo->t_neighbors ) {
           free ( newtopo );
           return ADCL_NO_MEMORY;
       }

       for ( i=0, j=0; i< ndims; i++ ) {
           newtopo->t_coords[i]      = coords[i];
       }
       for ( i=0, j=0; i< nneigh; i++ ) {
           newtopo->t_neighbors[j++] = lneighbors[i];
           newtopo->t_neighbors[j++] = rneighbors[i];
           newtopo->t_flip[i]        = flip[i];
       }
    }

    *topo = newtopo;
    return ADCL_SUCCESS;
}

int ADCL_topology_create ( MPI_Comm comm, ADCL_topology_t **topo )
{
    ADCL_topology_t *newtopo=NULL;
    int cartdim, i, topo_type;

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

    MPI_Comm_rank ( comm, &(newtopo->t_rank) );
    MPI_Comm_size ( comm, &(newtopo->t_size) );
    newtopo->t_comm  = comm;

    MPI_Topo_test ( comm, &topo_type );
    if ( MPI_CART == topo_type ) {
        MPI_Cartdim_get ( comm, &cartdim );

        newtopo->t_ndims = cartdim;
        newtopo->t_nneigh = cartdim;
        newtopo->t_coords = (int *)malloc ( cartdim * sizeof(int));
        newtopo->t_neighbors = (int *) malloc ( 2*cartdim *sizeof(int));
        newtopo->t_flip      = (int *) malloc ( cartdim * sizeof(int));
        if ( NULL == newtopo->t_neighbors || NULL == newtopo->t_coords  ) {
            free ( newtopo );
            return ADCL_NO_MEMORY;
        }

        MPI_Cart_coords ( comm, newtopo->t_rank, cartdim, newtopo->t_coords );
        for ( i=0; i< cartdim; i++ ) {
            MPI_Cart_shift ( comm, i, 1, &(newtopo->t_neighbors[2*i]),
                    &(newtopo->t_neighbors[2*i+1]) );
            if ( newtopo->t_coords[i] % 2 == 0 ) newtopo->t_flip[i] = 0; else newtopo->t_flip[i] = 1;
        }
    }

    *topo = newtopo;
    return ADCL_SUCCESS;
}

int ADCL_topology_create_extended ( MPI_Comm cart_comm, ADCL_topology_t **topo) 
{
    int *coords, *periods, *cdims;
    int *lneighbors, *rneighbors, *flip;
    int nneigh, ndims; 

    MPI_Cartdim_get ( cart_comm, &ndims );

    cdims   = (int*) malloc( ndims * sizeof(int) );
    periods = (int*) malloc( ndims * sizeof(int) );
    coords  = (int*) malloc( ndims * sizeof(int) );
    MPI_Cart_get (cart_comm, ndims, cdims, periods, coords); 

    ADCL_topology_get_cart_number_neighbors ( ndims, 1, &nneigh );
    lneighbors = (int*) malloc ( nneigh * sizeof(int) );
    rneighbors = (int*) malloc ( nneigh * sizeof(int) );
    flip       = (int*) malloc ( nneigh * sizeof(int) );

    ADCL_topology_get_cart_neighbors ( ndims, nneigh, lneighbors, rneighbors, flip, 
         cdims, periods, coords, cart_comm );

    ADCL_topology_create_generic ( ndims, nneigh, lneighbors, rneighbors, flip,
                   coords, ADCL_DIRECTION_BOTH, cart_comm, topo);

    free(lneighbors);
    free(rneighbors);
    free(flip);

    free( cdims );
    free( periods );
    free( coords );

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
        if ( NULL != ttopo->t_flip ) {
            free ( ttopo->t_flip );
        }
        free(ttopo);
    }

    *topo = ADCL_TOPOLOGY_NULL;
    return ADCL_SUCCESS;
}


int ADCL_topology_dump ( ADCL_topology_t *topo )
{
    int i, iproc;

    for ( iproc=0; iproc<topo->t_size; iproc++ ) {
        if ( iproc == topo->t_rank ) { 
            printf("rank %d, topology %d: ndims: %d size= %d\n", topo->t_rank, topo->t_id, topo->t_ndims, topo->t_size );
            for ( i=0; i<topo->t_ndims; i++ ) {
                printf("rank %d, topology %d: coords[%d]=%d\n", topo->t_rank, topo->t_id, i, topo->t_coords[i] );
            }
            for ( i=0; i<topo->t_nneigh; i++ ) {
                printf("rank %d, topology %d: lneighbor[%d]=%d , rneighbor[%d]=%d, flip=%d\n",
                        topo->t_rank, topo->t_id, 2*i, topo->t_neighbors[2*i], 2*i+1, 
                        topo->t_neighbors[2*i+1], topo->t_flip[i] );
            }
        }
        MPI_Barrier ( topo->t_comm );
    }
    return ADCL_SUCCESS;
}

int ADCL_topology_get_cart_number_neighbors ( int ndims, int extended, int *nneigh ) 
{
    /* returns the number of neighbors in a cartesian topology
       IN: ndims \in {1,2,3} - number of dimensions of topology
           extended          - 0 - if only neighbors parallel to axes are required
                               1 - if also neighbors at corners are required
       OUT: nneigh           - number of neighbors */

    if ( !extended ) {
       *nneigh = ndims; 
    }
    else {
        if      ( ndims == 1 ) *nneigh = ndims;
        else if ( ndims == 2 ) *nneigh = ndims + 2;
        else if ( ndims == 3)  *nneigh = ndims + 6;
    }
    return ADCL_SUCCESS; 
}

int ADCL_topology_get_cart_neighbors ( int ndims, int nneigh, int* lneighbors, int* rneighbors,
        int* flip, int* comm_dims, int* periods, int* coords, MPI_Comm cart_comm )
/* IN:  ndims \in {1,2,3} - dimension of communication cart_comm
        nneigh            - number of pairs of neighbors
        
   OUT: lneighbors, rneighbors, flip
   purpose: gets for a cartesian communicator cart_comm its neighbors, which are stored in lneighbors and rneighbors
            if ndims == nneigh, neighbors are parallel to coordinate axes 
            if ndims <  nneigh, neighbors are also in 2D corners and in 3D sides and egdes */
{
    int *direction;
    int i, rank; 

    MPI_Comm_rank ( cart_comm, &rank );

    /* communication partners in x-, y-, z-direction */
    for ( i=0; i< ndims; i++ ) {
       MPI_Cart_shift ( cart_comm, i, 1, &(lneighbors[i]),
             &(rneighbors[i]) );
       if ( coords[i] % 2 == 0 ) flip[i] = 0; else flip[i] = 1;
    }

    if ( nneigh > ndims ) {
        direction   = (int*) malloc( ndims * sizeof(int) );

        if ( ndims == 2 ) {
            /* (-1,-1) - (+1,+1) */
            direction[0] = +1; direction[1] = +1; 
            ADCL_get_cart_neighbor_pair ( cart_comm, 2, coords, direction, periods, comm_dims, 
                &lneighbors[i], &rneighbors[i] );
            if ( coords[0] % 2 == 0 ) flip[i] = 0; else flip[i] = 1;
            i++; 

            /* (+1,-1) - (-1,+1) */
            direction[0] = -1; direction[1] = +1; 
            ADCL_get_cart_neighbor_pair ( cart_comm, 2, coords, direction, periods, comm_dims, 
                &lneighbors[i], &rneighbors[i] );
            if ( coords[0] % 2 == 0 ) flip[i] = 0; else flip[i] = 1;
            i++; 
        }
        else if ( ndims == 3 ) {
            /* VERTICAL EDGES */

            /* (-1,-1,0) - (+1,+1,0) */
            direction[0] = 1; direction[1] = 1;  direction[2] = 0; 
            ADCL_get_cart_neighbor_pair ( cart_comm, 3, coords, direction, periods, comm_dims, 
                &lneighbors[i], &rneighbors[i] );
            if ( coords[1] % 2 == 0 ) flip[i] = 0; else flip[i] = 1;
            i++; 

            /* (-1,+1,0) - (+1,-1,0) */
            direction[0] = +1; direction[1] = -1; direction[2] = 0; 
            ADCL_get_cart_neighbor_pair ( cart_comm, 3, coords, direction, periods, comm_dims, 
                &lneighbors[i], &rneighbors[i] );
            if ( coords[1] % 2 == 0 ) flip[i] = 0; else flip[i] = 1;
            i++; 

            /* HORIZONTAL EDGES */

            /* (-1,0,-1) - (+1,0,+1) */
            direction[0] = 1; direction[1] = 0; direction[2] = 1; 
            ADCL_get_cart_neighbor_pair ( cart_comm, 3, coords, direction, periods, comm_dims, 
                &lneighbors[i], &rneighbors[i] ); 
            if ( coords[2] % 2 == 0 ) flip[i] = 0; else flip[i] = 1;
            i++;

            /* (0,-1,1) - (0,1,-1) */
            direction[0] = 0; direction[1] = -1; direction[2] = 1; 
            ADCL_get_cart_neighbor_pair ( cart_comm, 3, coords, direction, periods, comm_dims, 
                &lneighbors[i], &rneighbors[i] );
            if ( coords[2] % 2 == 0 ) flip[i] = 0; else flip[i] = 1;
            i++; 

            /* (1,0,-1) - (-1,0,+1) */
            direction[0] = -1; direction[1] = 0; direction[2] = 1; 
            ADCL_get_cart_neighbor_pair ( cart_comm, 3, coords, direction, periods, comm_dims, 
                &lneighbors[i], &rneighbors[i] ); 
            if ( coords[2] % 2 == 0 ) flip[i] = 0; else flip[i] = 1;
            i++;

            /* (0,-1,-1) - (0,+1,+1) */
            direction[0] = 0; direction[1] = 1; direction[2] = 1; 
            ADCL_get_cart_neighbor_pair ( cart_comm, 3, coords, direction, periods, comm_dims, 
                &lneighbors[i], &rneighbors[i] ); 
            if ( coords[2] % 2 == 0 ) flip[i] = 0; else flip[i] = 1;
            i++;


            /* CORNERS */
            /* (-1,-1,-1) - (+1,+1,+1) */
            /* (+1,-1,-1) - (-1,+1,+1) */
            /* (+1,+1,-1) - (-1,-1,+1) */
            /* (-1,+1,-1) - (+1,-1,+1) */
        }
        else if ( 3 < ndims ) {
            printf("ADCL_topology_create_extended: ndims > 3 not implemented.\n");
            exit(-1); 
        }
        free( direction );
    }
    return ADCL_SUCCESS; 
}

int ADCL_get_cart_neighbor_pair ( MPI_Comm cart_comm, int ndims, int* coords, int* direction, 
    int* periods, int *cdims, int* rank1, int *rank2 )
{
  int i; 
  int *coords_up, *coords_down; 
  int inrange1=1, inrange2=1; 

  coords_down = (int*) malloc( ndims * sizeof(int) );
  coords_up   = (int*) malloc( ndims * sizeof(int) );


    for ( i=0; i<ndims; i++) {
        coords_down[i] = coords[i] - direction[i];
        if ( ( coords_down[i] < 0 || coords_down[i] >= cdims[i] ) && periods[i] == 0 ) {
            inrange1 = 0;
        }

        coords_up[i]   = coords[i] + direction[i]; 
            if ( ( coords_up[i] < 0 || coords_up[i] >= cdims[i] ) && periods[i] == 0 ) {
                inrange2 = 0;
            }
    }

    if ( inrange1 ) {
        MPI_Cart_rank (cart_comm, coords_down, rank1 );
    }
    else {
        *rank1 = MPI_PROC_NULL;
    }

    if ( inrange2 ) {
        MPI_Cart_rank (cart_comm, coords_up, rank2 );
    }
    else {
        *rank2 = MPI_PROC_NULL;
    }


    free( coords_down );
    free( coords_up ); 
}


