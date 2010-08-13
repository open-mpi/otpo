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

void ADCL_alltoallv_pairwise( ADCL_request_t *req ) 
{
   ADCL_topology_t *topo = req->r_emethod->em_topo;
   MPI_Comm comm         = topo->t_comm;
   MPI_Datatype dtype    = req->r_rdats[0]; 

   void *sbuf = req->r_svecs[0]->v_data;
   void *rbuf = req->r_rvecs[0]->v_data;

   ADCL_vmap_t *svmap = req->r_svecs[0]->v_map;
   ADCL_vmap_t *rvmap = req->r_rvecs[0]->v_map;
   int *scounts       = svmap->m_rcnts;
   int *rcounts       = rvmap->m_rcnts;
   int *sdispls       = svmap->m_displ;
   int *rdispls       = rvmap->m_displ;

   int line = -1, err = 0;
   int rank, size, step;
   int sendto, recvfrom;
   void *psnd, *prcv;
   MPI_Aint ext, lb;


    size = topo->t_size;
    rank = topo->t_rank;

    err = MPI_Type_get_extent(dtype, &lb, &ext);
    if ( MPI_SUCCESS != err) { line = __LINE__; return; }


    psnd = ((char *) sbuf) + (sdispls[rank] * ext);
    prcv = ((char *) rbuf) + (rdispls[rank] * ext);

    if (0 != scounts[rank]) {
	err = ADCL_ddt_copy_content_same_ddt ( dtype, scounts[rank]*ext, 
					       prcv, psnd );
    }

    /* If only one process, we're done. */
    if (1 == size) {
        return;
    }

    /* Perform pairwise exchange starting from 1 since local exhange is done */
    for (step = 1; step < size + 1; step++) {

        /* Determine sender and receiver for this step. */
        sendto  = (rank + step) % size;
        recvfrom = (rank + size - step) % size;

        /* Determine sending and receiving locations */
        psnd = (char*)sbuf + sdispls[sendto] * ext;
        prcv = (char*)rbuf + rdispls[recvfrom] * ext;

        /* send and receive */
        err = MPI_Sendrecv ( psnd, scounts[sendto], dtype, sendto,
			     ADCL_TAG_ALLTOALLV,
			     prcv, rcounts[recvfrom], dtype, recvfrom,
			     ADCL_TAG_ALLTOALLV,
			     comm, MPI_STATUS_IGNORE);
    }


   return;
}

