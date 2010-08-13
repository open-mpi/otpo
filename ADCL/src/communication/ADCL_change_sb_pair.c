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

#define AAO_SB_TAG 11111


/* Neighborhood communication based on pairwise message exchange */
void ADCL_CHANGE_SB_PAIR ( ADCL_request_t *req )
{
    int i, j;
    int nneigh  = req->r_emethod->em_topo->t_nneigh;
    int ndims   = req->r_emethod->em_topo->t_ndims;
    int offset;  

    if ( nneigh == ndims ) {
       offset = 0; 
    } 
    else {
       offset = 2*nneigh;
    }

    PREPARE_COMMUNICATION(req);

    for ( i=0, j=0; i<2*ndims; i+=2, j++ ) {
        
        if ( TOPO->t_flip[j] == 0 ) {
            if ( MPI_PROC_NULL != TOPO->t_neighbors[i] ) {
                RECV_START(req, i, AAO_SB_TAG);
                SEND_START(req, i, AAO_SB_TAG);
                RECV_WAIT (req, i);
                SEND_WAIT (req, i);
            }
            if ( MPI_PROC_NULL != TOPO->t_neighbors[i+1] ) {
                RECV_START(req, i+1, AAO_SB_TAG);
                SEND_START(req, i+1, AAO_SB_TAG);
                RECV_WAIT (req, i+1);
                SEND_WAIT (req, i+1);
            }
        }
        else {
            if ( MPI_PROC_NULL != TOPO->t_neighbors[i+1] ) {
                SEND_START(req, i+1, AAO_SB_TAG);
                RECV_START(req, i+1, AAO_SB_TAG);
                RECV_WAIT (req, i+1);
                SEND_WAIT (req, i+1);
            }
            if ( MPI_PROC_NULL != TOPO->t_neighbors[i] ) {
                SEND_START(req, i, AAO_SB_TAG);
                RECV_START(req, i, AAO_SB_TAG);
                RECV_WAIT (req, i);
                SEND_WAIT (req, i);
            }
        }
    }

    STOP_COMMUNICATION(req);

    return;
}

