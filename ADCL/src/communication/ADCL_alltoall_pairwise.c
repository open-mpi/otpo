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

void ADCL_alltoall_pairwise( ADCL_request_t *req ) 
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

   int line = -1, err = 0;
   int rank, size, step;
   int sendto, recvfrom;
   void * tmpsend, *tmprecv;
   MPI_Aint sext, rext, lb;


    size = topo->t_size;
    rank = topo->t_rank;

    err = MPI_Type_get_extent(sdtype, &lb, &sext);
    if ( MPI_SUCCESS != err) { line = __LINE__; return; }

    err = MPI_Type_get_extent(rdtype, &lb, &rext);
    if ( MPI_SUCCESS != err) { line = __LINE__; return; }

    //    err = ADCL_ddt_copy_content_same_ddt ( sdtype, scount*ext, 
    //    				       tmprecv, tmpsend );

    /* If only one process, we're done. */
    if (1 == size) {
        return;
    }

    /* Perform pairwise exchange - starting from 1 so the local copy is last */
    for (step = 1; step < size + 1; step++) {

        /* Determine sender and receiver for this step. */
        sendto  = (rank + step) % size;
        recvfrom = (rank + size - step) % size;

        /* Determine sending and receiving locations */
        tmpsend = (char*)sbuf + sendto * sext * scount;
        tmprecv = (char*)rbuf + recvfrom * rext * rcount;

        /* send and receive */
        err = MPI_Sendrecv ( tmpsend, scount, sdtype, sendto, ADCL_TAG_ALLTOALL,
			     tmprecv, rcount, rdtype, recvfrom, ADCL_TAG_ALLTOALL,
			     comm, MPI_STATUS_IGNORE);
        if (err != MPI_SUCCESS) { line = __LINE__; return;  }
   }

   return;
}

