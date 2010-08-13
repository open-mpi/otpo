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


/* For medium size messages and (short messages for comm_size < 8), we
   use an algorithm that posts all irecvs and isends and then does a
   waitall. We scatter the order of sources and destinations among the
   processes, so that all processes don't try to send/recv to/from the
   same process at the same time.

 *** Modification: We post only a small number of isends and irecvs 
 at a time and wait on them as suggested by Tony Ladd. *** */


void ADCL_ALLTOALL_LADD ( ADCL_request_t *req ) 
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

    MPI_Request *reqs;  //= &req->r_sreqs[0];
    MPI_Status *stats; 

    int i, line = -1, err = 0;
    int rank, size;
    MPI_Aint sext, rext, lb;
    int dst, ii, ss, bblock;

    size = topo->t_size;
    rank = topo->t_rank;

    /* Initialize. */
    bblock = BBLOCK;
    if ( bblock == 0 || bblock > size ) bblock = size;

    reqs = (MPI_Request *) malloc(2*bblock*sizeof(MPI_Request));
    if (!reqs)  { goto errhdl; }

    stats = (MPI_Status *) malloc(2*bblock*sizeof(MPI_Status));
    if (!stats) { goto errhdl; }

    err = MPI_Type_get_extent(sdtype, &lb, &sext);
    if ( MPI_SUCCESS != err) { goto errhdl; }
    sext *= scount;

    err = MPI_Type_get_extent(rdtype, &lb, &rext);
    if ( MPI_SUCCESS != err) { goto errhdl; }
    rext *= rcount;

    for (ii=0; ii<size; ii+=bblock) {
        ss = size-ii < bblock ? size-ii : bblock;
        /* do the communication -- post ss sends and receives: */
        for ( i=0; i<ss; i++ ) { 
            dst = (rank+i+ii) % size;
            err = MPI_Irecv((char *)rbuf + dst*rext, rcount, rdtype, dst,
                    ADCL_TAG_ALLTOALL, comm, &reqs[i]);
            if ( MPI_SUCCESS != err) { goto errhdl; }
        }

        for ( i=0; i<ss; i++ ) { 
            dst = (rank-i-ii+size) % size;
            err = MPI_Isend((char *)sbuf + dst*sext, scount, sdtype, dst,
                    ADCL_TAG_ALLTOALL, comm, &reqs[i+ss]);
            if ( MPI_SUCCESS != err) { goto errhdl; }

        }
        /* ... then wait for them to finish: */
        err = MPI_Waitall(2*ss,reqs,stats);
        if ( MPI_SUCCESS != err) { goto errhdl; }
    }

errhdl: 
    if ( NULL != reqs )   free ( reqs );
    if ( NULL != stats )  free ( stats );

    return;
}


