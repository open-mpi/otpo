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

void ADCL_alltoall_linear_sync ( ADCL_request_t *req ) 
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

   int i, line = -1, error = 0;
   int rank, size;
   MPI_Aint sext, rext, lb;
    int ri, si;
    int nreqs, nrreqs, nsreqs, total_reqs;
    char *psnd;
    char *prcv;

    MPI_Request *reqs = NULL;

    /* Initialize. */
    size = topo->t_size;
    rank = topo->t_rank;

    error = MPI_Type_get_extent(sdtype, &lb, &sext);
    if ( MPI_SUCCESS != error ) { line = __LINE__; return; }
    sext *= scount;

    error = MPI_Type_get_extent(rdtype, &lb, &rext);
    if ( MPI_SUCCESS != error ) { line = __LINE__; return; }
    rext *= rcount;


    /* simple optimization */
    psnd = ((char *) sbuf) + (rank * sext);
    prcv = ((char *) rbuf) + (rank * rext);
    error = MPI_Sendrecv(psnd, scount, sdtype, rank, ADCL_TAG_ALLTOALL,
                         prcv, rcount, rdtype, rank, ADCL_TAG_ALLTOALL, comm, MPI_STATUS_IGNORE );
    if (MPI_SUCCESS != error) {
        return;
    }

    /* If only one process, we're done. */
    if (1 == size) {
        return;
    }

    /* Initiate send/recv to/from others. */
    total_reqs =  size - 1; 
    reqs = (MPI_Request*) malloc( 2 * total_reqs * sizeof(MPI_Request));
    if (NULL == reqs) { error = -1; goto error_hndl; }

    prcv = (char *) rbuf;
    psnd = (char *) sbuf;

    /* Post first batch or ireceive and isend requests  */
    for (nreqs = 0, nrreqs = 0, ri = (rank + 1) % size; nreqs < total_reqs;
         ri = (ri + 1) % size, ++nreqs, ++nrreqs) {
       error = MPI_Irecv (prcv + (ri * rext), rcount, rdtype, ri,
                        ADCL_TAG_ALLTOALL, comm, &reqs[nreqs]);
       if (MPI_SUCCESS != error) { line = __LINE__; goto error_hndl; }
    }
    for ( nsreqs = 0, si =  (rank + size - 1) % size; nreqs < 2 * total_reqs;
          si = (si + size - 1) % size, ++nreqs, ++nsreqs) {
       error = MPI_Isend (psnd + (si * sext), scount, sdtype, si, 
        ADCL_TAG_ALLTOALL, comm, &reqs[nreqs]);
       if (MPI_SUCCESS != error) { line = __LINE__; goto error_hndl; }
    }

    /* Wait for requests to complete */
    if (nreqs == 2 * (size - 1)) {
       /* Optimization for the case when all requests have been posted  */
       error = MPI_Waitall(nreqs, reqs, MPI_STATUSES_IGNORE);
       if (MPI_SUCCESS != error) { line = __LINE__; goto error_hndl; }

    } else {
       int ncreqs = 0;
       while (ncreqs < 2 * (size - 1)) {
          int completed;
          error = MPI_Waitany(2 * total_reqs, reqs, &completed,
                                        MPI_STATUS_IGNORE);
          if (MPI_SUCCESS != error) { line = __LINE__; goto error_hndl; }
          reqs[completed] = MPI_REQUEST_NULL;
          ncreqs++;
          if (completed < total_reqs) {
             if (nrreqs < (size - 1)) {
                error = MPI_Irecv (prcv + (ri * rext), rcount, rdtype, ri,
                                 ADCL_TAG_ALLTOALL, comm, &reqs[completed]);
                if (MPI_SUCCESS != error) { line = __LINE__; goto error_hndl; }
                ++nrreqs;
                ri = (ri + 1) % size;
             }
          } else {
             if (nsreqs < (size - 1)) {
                error = MPI_Isend (psnd + (si * sext), scount, sdtype, si,
                                      ADCL_TAG_ALLTOALL, comm, &reqs[completed]);
                ++nsreqs;
                si = (si + size - 1) % size;
             }
          }
       }
    }

error_hndl:
    if (NULL != reqs) free(reqs);
    return;
}
