/* 
** Algorithms implementing ALLGATHERV operations. Taken from the Open MPI source code repository.
** Following are the Open MPI copyrights
*/

/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2007 The University of Tennessee and The University
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
 * Copyright (c) 2008      University of Houston. All rights reserved.
 * Copyright (c) 2008      High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"

/*
 * Linear functions are copied from the BASIC coll module
 * they do not segment the message and are simple implementations
 * but for some small number of nodes and/or small data sizes they
 * are just as fast as tuned/tree based segmenting operations
 * and as such may be selected by the decision functions
 * These are copied into this module due to the way we select modules
 * in V1. i.e. in V2 we will handle this differently and so will not
 * have to duplicate code.
 * GEF Oct05 after asking Jeff.
 */

/* copied function (with appropriate renaming) starts here */

/*
 *  bcast_lin_intra
 *
 *  Function:   - broadcast using O(N) algorithm
 *  Accepts:    - same arguments as MPI_Bcast()
 *  Returns:    - MPI_SUCCESS or error code
 */
void
ADCL_bcast_linear ( ADCL_request_t *req )
{
   int i;
   int size;
   int rank;
   int err;
   int root = 0; 

   ADCL_topology_t *topo = req->r_emethod->em_topo;
   MPI_Comm comm = topo->t_comm;
   ADCL_vmap_t *svmap = req->r_svecs[0]->v_map;
   void *sbuf = req->r_svecs[0]->v_data; 
   void *rbuf = req->r_rvecs[0]->v_data;
   void *buf; 
   MPI_Request *mpi_reqs;
 
   /* use receive data */
   MPI_Datatype btype =  req->r_rdats[0];
   int count = req->r_rvecs[0]->v_dims[0];

   size = topo->t_size;
   rank = topo->t_rank;

   mpi_reqs = (MPI_Request *) calloc(size, sizeof(MPI_Request));
    /* Non-root receive the data. */
    if (rank != root) {
        MPI_Recv(rbuf, count, btype, root,
            ADCL_TAG_BCAST, comm, MPI_STATUS_IGNORE);
       return;
    }


    /* Other processes returned, root sends data to all others. */
    if ( MPI_IN_PLACE == sbuf ) {
       /* this is a hack in case allgatherv_linear calls this routine */
       buf = rbuf;
    }
    else {
       buf = sbuf;
    }
    for (i = 0; i < size; i++) {
       if (i == rank) {
	  mpi_reqs[i] = MPI_REQUEST_NULL;
	  continue;
       }
       err = MPI_Isend(buf, count, btype, i, ADCL_TAG_BCAST, comm, &mpi_reqs[i]);
    }
    err = MPI_Waitall(size, mpi_reqs, MPI_STATUSES_IGNORE);

    /* Free the reqs */
    free(mpi_reqs);

    /* All done */

    return;
}
