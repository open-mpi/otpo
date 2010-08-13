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
 * ompi_coll_tuned_allreduce_intra_nonoverlapping
 *
 * This function just calls a reduce followed by a broadcast
 * both called functions are tuned but they complete sequentially,
 * i.e. no additional overlapping
 * meaning if the number of segments used is greater than the topo depth
 * then once the first segment of data is fully 'reduced' it is not broadcast
 * while the reduce continues (cost = cost-reduce + cost-bcast + decision x 3)
 *
 */
void
ADCL_allreduce_nonoverlapping(ADCL_request_t *req)
{
   int rank, err;
   
   ADCL_topology_t *topo = req->r_emethod->em_topo;
   MPI_Comm comm = topo->t_comm;
   ADCL_vmap_t *rvmap = req->r_rvecs[0]->v_map;

   void *sbuf = req->r_svecs[0]->v_data;
   void *rbuf = req->r_rvecs[0]->v_data;

   int  count = req->r_rvecs[0]->v_dims[0];
   MPI_Datatype dtype = req->r_rdats[0]; 
   MPI_Op op = rvmap->m_op;
 
   rank = topo->t_rank;

   /* Reduce to 0 and broadcast. */
   if (MPI_IN_PLACE == sbuf) {
       if (0 == rank) {
            err = MPI_Reduce (MPI_IN_PLACE, rbuf, count, dtype,
                                            op, 0, comm);
        } else {
            err = MPI_Reduce (rbuf, NULL, count, dtype, op, 0,
                                            comm);
        }
    } else {
      err = MPI_Reduce (sbuf, rbuf, count, dtype, op, 0, comm);
    }

    err = MPI_Bcast (rbuf, count, dtype, 0, comm);
    return;
}
