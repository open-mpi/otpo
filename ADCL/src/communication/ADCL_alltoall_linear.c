/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2006 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

/*
** Codes have been adapted to be used in ADCL
** Following are the ADCL copyrights
*/

/*
 * Copyright (c) 2008 - 2009  University of Houston. All rights reserved.
 * Copyright (c) 2008 - 2009  High Performance Computing Center Stuttgart,
 *                            University of Stuttgart.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"

void ADCL_alltoall_linear ( ADCL_request_t *req ) 
{
    ADCL_topology_t *topo = req->r_emethod->em_topo;
    MPI_Comm comm         = topo->t_comm;
    MPI_Datatype sdtype   = req->r_sdats[0]; 
    MPI_Datatype rdtype   = req->r_rdats[0]; 
    
    void *sbuf = req->r_svecs[0]->v_data;
    void *rbuf = req->r_rvecs[0]->v_data;
    
    ADCL_vmap_t *vmap = req->r_svecs[0]->v_map;
    int scount       = vmap->m_scnt;
    int rcount       = vmap->m_rcnt;
    
    MPI_Request *sreqs;  //= &req->r_sreqs[0];
    MPI_Request *rreqs;  //= &req->r_rreqs[0];
    MPI_Request *mpireq; // = rreqs;

    int i, line = -1, err = 0;
    int rank, size;
    void *psnd, *prcv;
    MPI_Aint sext, rext, lb;
    int nreqs;
    
    size = topo->t_size;
    rank = topo->t_rank;
    
   /* Initialize. */

    err = MPI_Type_get_extent(sdtype, &lb, &sext);
    if ( MPI_SUCCESS != err) { line = __LINE__; return; }
    sext *= scount;

    err = MPI_Type_get_extent(rdtype, &lb, &rext);
    if ( MPI_SUCCESS != err) { line = __LINE__; return; }
    rext *= rcount;

    /* simple optimization */
    psnd = ((char *) sbuf) + (rank * sext);
    prcv = ((char *) rbuf) + (rank * rext);

    err = MPI_Sendrecv(psnd, scount, sdtype, rank, ADCL_TAG_ALLTOALL, 
                       prcv, rcount, rdtype, rank, ADCL_TAG_ALLTOALL, comm, MPI_STATUS_IGNORE );
    if ( MPI_SUCCESS != err) { line = __LINE__; return; }

    /* If only one process, we're done. */
    if (1 == size) {
        return;
    }

    /* Initiate all send/recv to/from others. */
    nreqs = (size - 1) * 2;
    mpireq = rreqs = ( MPI_Request * ) malloc ( nreqs * sizeof (MPI_Request));
    if ( NULL == rreqs ) {
        return;
    }
    sreqs = rreqs + size - 1;

    prcv = (char *) rbuf;
    psnd = (char *) sbuf;



    /* Post all receives first */
    for (nreqs = 0, i = (rank + 1) % size; i != rank; i = (i + 1) % size, ++rreqs, ++nreqs) {
        err = MPI_Irecv ( (char*) prcv + (i * rext), rcount, rdtype, i, ADCL_TAG_ALLTOALL, comm, rreqs );
        if (MPI_SUCCESS != err) {
            free ( mpireq );
            return;
        }
    }

    /* Now post all sends in reverse order */
    for (nreqs = 0, i = (rank + size - 1) % size; i != rank; i = (i + size - 1) % size, ++sreqs, ++nreqs) {
        err = MPI_Isend ( (char*) psnd + (i * sext), scount, sdtype, i,
                          ADCL_TAG_ALLTOALL, comm, sreqs );
        if (MPI_SUCCESS != err) {
            free ( mpireq );
            return;
        }
    }

    nreqs = (size - 1) * 2;
    err = MPI_Waitall( nreqs, mpireq, MPI_STATUSES_IGNORE);
    
    free ( mpireq );

    return;
}

