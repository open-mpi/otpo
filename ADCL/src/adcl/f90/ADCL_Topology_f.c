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
#include "ADCL_fprototypes.h"


#pragma weak adcl_topology_create_  = adcl_topology_create
#pragma weak adcl_topology_create__ = adcl_topology_create
#pragma weak ADCL_TOPOLOGY_CREATE   = adcl_topology_create

#pragma weak adcl_topology_create_generic_   = adcl_topology_create_generic
#pragma weak adcl_topology_create_generic__  = adcl_topology_create_generic
#pragma weak ADCL_TOPOLOGY_CREATE_GENERIC    = adcl_topology_create_generic

#pragma weak adcl_topology_free_  = adcl_topology_free
#pragma weak adcl_topology_free__ = adcl_topology_free
#pragma weak ADCL_TOPOLOGY_FREE   = adcl_topology_free


#ifdef _SX
void adcl_topology_create_generic_ ( int *ndims, int *lneighb, int *rneighb, int *coords, 
				    int *direction, int *comm, int *topo, int *ierror )
#else
void adcl_topology_create_generic ( int *ndims, int *lneighb, int *rneighb, int *coords, 
				    int *direction, int *comm, int *topo, int *ierror )
#endif
{
    ADCL_topology_t *ctopo;
    MPI_Comm ccomm;
    
    if ( ( NULL == ndims )    || 
	 ( NULL == lneighb )  ||
	 ( NULL == rneighb  ) ||
	 ( NULL == coords )   ||
	 ( NULL == comm )     ||
	 ( NULL == topo ) )   {
	*ierror = ADCL_INVALID_ARG;
	return;
    }

    ccomm = MPI_Comm_f2c (*comm);
    if ( ccomm == MPI_COMM_NULL ) {
	*ierror = ADCL_INVALID_COMM;
	return;
    }
    
    if ( 0 == *ndims ) {
	*ierror = ADCL_topology_create_generic ( *ndims, NULL, NULL, 
						 NULL, *direction, ccomm,
						 &ctopo );
    }
    else {
        *ierror = ADCL_topology_create_generic ( *ndims, lneighb, rneighb, 
						 coords, *direction, ccomm,
						 &ctopo );
    }

    if ( *ierror == ADCL_SUCCESS ) {
	*topo = ctopo->t_findex;
    }

    return;
}

#ifdef _SX
void adcl_topology_create_  ( int* cart_comm, int *topo, int *ierror )
#else
void adcl_topology_create   ( int* cart_comm, int *topo, int *ierror )
#endif
{
    int topo_type;
    ADCL_topology_t *ctopo;
    MPI_Comm ccomm;

    if ( ( NULL == cart_comm ) ||
	 ( NULL == topo ) )    {
	*ierror = ADCL_INVALID_ARG;
	return;
    }

    ccomm = MPI_Comm_f2c (*cart_comm);
    if ( ccomm == MPI_COMM_NULL ) {
	*ierror = ADCL_INVALID_COMM;
	return;
    }
    MPI_Topo_test ( ccomm, &topo_type );
    if ( MPI_UNDEFINED==topo_type || MPI_GRAPH==topo_type ) {
        *ierror = ADCL_INVALID_COMM;
        return;
    }

    *ierror = ADCL_topology_create ( ccomm, &ctopo );
    if ( *ierror == ADCL_SUCCESS ) {
	*topo = ctopo->t_findex;
    } 
    
    return;
}

#ifdef _SX
void adcl_topology_free_ ( int *topo, int *ierror )
#else
void adcl_topology_free ( int *topo, int *ierror )
#endif
{
    ADCL_topology_t *ctopo;

    ctopo = (ADCL_topology_t *) ADCL_array_get_ptr_by_pos (ADCL_topology_farray, 
							   *topo );
    *ierror = ADCL_Topology_free ( &ctopo );
    return;
}

