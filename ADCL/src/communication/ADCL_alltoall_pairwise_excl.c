/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
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

void ADCL_alltoall_pairwise_excl( ADCL_request_t *req ) 
{
    ADCL_topology_t *topo = req->r_emethod->em_topo;
    MPI_Comm comm         = topo->t_comm;
    MPI_Datatype sdtype    = req->r_sdats[0]; 
    MPI_Datatype rdtype    = req->r_rdats[0]; 

    void *sbuf = req->r_svecs[0]->v_data;
    void *rbuf = req->r_rvecs[0]->v_data;

    ADCL_vmap_t *vmap = req->r_svecs[0]->v_map;
    int scount        = vmap->m_scnt;
    int rcount        = vmap->m_rcnt;

    int line = -1, err = 0, i;
    int rank, size, step;
    int src, dst;
    int rsend, ssend; 
    MPI_Aint sext, rext, lb;
    MPI_Status status;
    int pof2; 

    size = topo->t_size;
    rank = topo->t_rank;

    err = MPI_Type_get_extent(sdtype, &lb, &sext);
    if ( MPI_SUCCESS != err) { line = __LINE__; return; }
    ssend = scount*sext; 

    err = MPI_Type_get_extent(rdtype, &lb, &rext);
    if ( MPI_SUCCESS != err) { line = __LINE__; return; }
    rsend = rcount*rext; 
    //    err = ADCL_ddt_copy_content_same_ddt ( sdtype, scount*ext, 
    //    				       tmprecv, tmpsend );

    /* Long message. If comm_size is a power-of-two, do a pairwise
       exchange using exclusive-or to create pairs. Else send to
       rank+i, receive from rank-i. */

    /* Make local copy first */
    err = MPI_Sendrecv(((char *)sbuf + rank*ssend), scount, sdtype, rank, ADCL_TAG_ALLTOALL,
            ((char *)rbuf + rank*rsend), rcount, rdtype, rank, ADCL_TAG_ALLTOALL, comm, &status);
    if (err != MPI_SUCCESS) { line = __LINE__; return;  }

    /* Is size a power-of-two? */
    i = 1;
    while (i < size)
        i *= 2;
    if (i == size)
        pof2 = 1;
    else 
        pof2 = 0;

    /* Do the pairwise exchanges */
    for (i=1; i<size; i++) {
        if (pof2 == 1) {
            /* use exclusive-or algorithm */
            src = dst = rank ^ i;
        }
        else {
            src = (rank - i + size) % size;
            dst = (rank + i) % size;
        }

        err = MPI_Sendrecv(((char *)sbuf + dst*ssend), scount, sdtype, dst, ADCL_TAG_ALLTOALL, 
                ((char *)rbuf + src*rsend), rcount, rdtype, src, ADCL_TAG_ALLTOALL, comm, &status);
        if (err != MPI_SUCCESS) { line = __LINE__; return;  }
    }


    return;
}



