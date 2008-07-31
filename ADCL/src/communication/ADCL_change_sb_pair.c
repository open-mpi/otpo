/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"

#define AAO_SB_TAG 11111


/* Neighborhood communication based on pairwise message exchange */
void ADCL_CHANGE_SB_PAIR ( ADCL_request_t *req )
{
    int i, dim;
    int nneighs = 2*req->r_emethod->em_topo->t_ndims;

    PREPARE_COMMUNICATION(req);

    for ( dim=0, i=0; i<nneighs ; i+=2, dim++ ) {

        if ( TOPO->t_coords[dim] % 2 == 0 ) {
            if ( MPI_PROC_NULL != TOPO->t_neighbors[i] ) {
                RECV_START(req, i, AAO_SB_TAG);
                SEND_START(req, i, AAO_SB_TAG);
                RECV_WAIT (req,i);
                SEND_WAIT (req,i);
            }
            if ( MPI_PROC_NULL != TOPO->t_neighbors[i+1] ) {
                RECV_START(req, i+1, AAO_SB_TAG);
                SEND_START(req, i+1, AAO_SB_TAG);
                RECV_WAIT (req,i+1);
                SEND_WAIT (req,i+1);
            }
        }
        else {
            if ( MPI_PROC_NULL != TOPO->t_neighbors[i+1] ) {
                SEND_START(req, i+1, AAO_SB_TAG);
                RECV_START(req, i+1, AAO_SB_TAG);
                RECV_WAIT (req,i+1);
                SEND_WAIT (req,i+1);
            }
            if ( MPI_PROC_NULL != TOPO->t_neighbors[i] ) {
                SEND_START(req, i, AAO_SB_TAG);
                RECV_START(req, i, AAO_SB_TAG);
                RECV_WAIT (req,i);
                SEND_WAIT (req,i);
            }
        }
    }

    STOP_COMMUNICATION(req);

    return;
}

