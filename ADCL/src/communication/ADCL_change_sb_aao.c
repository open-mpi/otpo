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


/* Neighborhood communication initiating all operations at-once */
void ADCL_CHANGE_SB_AAO ( ADCL_request_t *req )
{
    int i, nneighs=2*TOPO->t_ndims;

    PREPARE_COMMUNICATION(req);

    for ( i=0; i<nneighs; i++ ) {
        if ( MPI_PROC_NULL != TOPO->t_neighbors[i] ) {
            RECV_START(req, i, AAO_SB_TAG);
            SEND_START(req, i, AAO_SB_TAG);
        }
        else {
            req->r_sreqs[i] = MPI_REQUEST_NULL;
            req->r_rreqs[i] = MPI_REQUEST_NULL;
        }
    }

    SEND_WAITALL(req);
    RECV_WAITALL(req);

    STOP_COMMUNICATION(req);

    return;
}

