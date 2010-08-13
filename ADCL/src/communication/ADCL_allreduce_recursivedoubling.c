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
 *   ompi_coll_tuned_allreduce_intra_recursivedoubling
 *
 *   Function:       Recursive doubling algorithm for allreduce operation
 *   Accepts:        Same as MPI_Allreduce()
 *   Returns:        MPI_SUCCESS or error code
 *
 *   Description:    Implements recursive doubling algorithm for allreduce.
 *                   Original (non-segmented) implementation is used in MPICH-2
 *                   for small and intermediate size messages.
 *                   The algorithm preserves order of operations so it can
 *                   be used both by commutative and non-commutative operations.
 *
 *         Example on 7 nodes:
 *         Initial state
 *         #      0       1      2       3      4       5      6
 *               [0]     [1]    [2]     [3]    [4]     [5]    [6]
 *         Initial adjustment step for non-power of two nodes.
 *         old rank      1              3              5      6
 *         new rank      0              1              2      3
 *                     [0+1]          [2+3]          [4+5]   [6]
 *         Step 1
 *         old rank      1              3              5      6
 *         new rank      0              1              2      3
 *                     [0+1+]         [0+1+]         [4+5+]  [4+5+]
 *                     [2+3+]         [2+3+]         [6   ]  [6   ]
 *         Step 2
 *         old rank      1              3              5      6
 *         new rank      0              1              2      3
 *                     [0+1+]         [0+1+]         [0+1+]  [0+1+]
 *                     [2+3+]         [2+3+]         [2+3+]  [2+3+]
 *                     [4+5+]         [4+5+]         [4+5+]  [4+5+]
 *                     [6   ]         [6   ]         [6   ]  [6   ]
 *         Final adjustment step for non-power of two nodes
 *         #      0       1      2       3      4       5      6
 *              [0+1+] [0+1+] [0+1+]  [0+1+] [0+1+]  [0+1+] [0+1+]
 *              [2+3+] [2+3+] [2+3+]  [2+3+] [2+3+]  [2+3+] [2+3+]
 *              [4+5+] [4+5+] [4+5+]  [4+5+] [4+5+]  [4+5+] [4+5+]
 *              [6   ] [6   ] [6   ]  [6   ] [6   ]  [6   ] [6   ]
 *
 */

void ADCL_allreduce_recursivedoubling ( ADCL_request_t *req )
{
   int ret, line;
   int rank, size, adjsize, remote, distance;
   int newrank, newremote, extra_ranks;
   char *tmpsend = NULL, *tmprecv = NULL, *tmpswap = NULL, *inplacebuf = NULL;
   int error_hndl, bcount;
   MPI_Aint true_lb, true_extent, lb, extent;
   MPI_Request reqs[2];

   ADCL_topology_t *topo = req->r_emethod->em_topo;
   MPI_Comm comm = topo->t_comm;
   ADCL_vmap_t *svmap = req->r_svecs[0]->v_map;
   ADCL_vmap_t *rvmap = req->r_rvecs[0]->v_map;

   void *sbuf = req->r_svecs[0]->v_data;
   void *rbuf = req->r_rvecs[0]->v_data;

   /* use receive data */
   MPI_Datatype dtype = req->r_rdats[0];
   int  count = req->r_rvecs[0]->v_dims[0];
   MPI_Op op = rvmap->m_op;

   rank = topo->t_rank;
   size = topo->t_size;

   /* Allocate and initialize temporary send buffer */
   ret = MPI_Type_get_extent(dtype, &lb, &extent);
   if (MPI_SUCCESS != ret) { line = __LINE__; goto error_hndl; }
   ret = MPI_Type_get_true_extent(dtype, &true_lb, &true_extent);
   if (MPI_SUCCESS != ret) { line = __LINE__; goto error_hndl; }
   bcount = true_extent + (count - 1) * extent;

   /* Special case for size == 1 */
   if (1 == size) {
      if (MPI_IN_PLACE != sbuf) {
         ret = ADCL_ddt_copy_content_same_ddt(dtype, bcount, (char*)rbuf, (char*)sbuf);
         if (ret < 0) { line = __LINE__; goto error_hndl; }
      }
      return;
   }

   inplacebuf = (char*) malloc(bcount);
   if (NULL == inplacebuf) { ret = -1; line = __LINE__; goto error_hndl; }


   if (MPI_IN_PLACE == sbuf) {
      ret = ADCL_ddt_copy_content_same_ddt(dtype, bcount, inplacebuf, (char*)rbuf);
      if (ret < 0) { line = __LINE__; goto error_hndl; }
   } else {
      ret = ADCL_ddt_copy_content_same_ddt(dtype, bcount, inplacebuf, (char*)sbuf);
      if (ret < 0) { line = __LINE__; goto error_hndl; }
   }

   tmpsend = (char*) inplacebuf;
   tmprecv = (char*) rbuf;

   /* Determine nearest power of two less than or equal to size */
   for (adjsize = 0x1; adjsize <= size; adjsize <<= 1); adjsize = adjsize >> 1;

   /* Handle non-power-of-two case:
      - Even ranks less than 2 * extra_ranks send their data to (rank + 1), and
        sets new rank to -1.
      - Odd ranks less than 2 * extra_ranks receive data from (rank - 1),
        apply appropriate operation, and set new rank to rank/2
      - Everyone else sets rank to rank - extra_ranks
    */
   extra_ranks = size - adjsize;
   if (rank <  (2 * extra_ranks)) {
      if (0 == (rank % 2)) {
         ret = MPI_Send(tmpsend, count, dtype, (rank + 1),
                                 ADCL_TAG_ALLREDUCE, comm);
         if (MPI_SUCCESS != ret) { line = __LINE__; goto error_hndl; }
         newrank = -1;
      } else {
         ret = MPI_Recv(tmprecv, count, dtype, (rank - 1),
                                 ADCL_TAG_ALLREDUCE, comm, MPI_STATUS_IGNORE);
         if (MPI_SUCCESS != ret) { line = __LINE__; goto error_hndl; }
         /* tmpsend = tmprecv (op) tmpsend */
         ADCL_op_reduce(op, tmprecv, tmpsend, count, dtype);
         newrank = rank >> 1;
      }
   } else {
      newrank = rank - extra_ranks;
   }

   /* Communication/Computation loop
      - Exchange message with remote node.
      - Perform appropriate operation taking in account order of operations:
        result = value (op) result
    */
   for (distance = 0x1; distance < adjsize; distance <<=1) {
      if (newrank < 0) break;
      /* Determine remote node */
      newremote = newrank ^ distance;
      remote = (newremote < extra_ranks)?
         (newremote * 2 + 1):(newremote + extra_ranks);

      /* Exchange the data */
      ret = MPI_Irecv(tmprecv, count, dtype, remote,
                               ADCL_TAG_ALLREDUCE, comm, &reqs[0]);
      if (MPI_SUCCESS != ret) { line = __LINE__; goto error_hndl; }
      ret = MPI_Isend(tmpsend, count, dtype, remote,
                               ADCL_TAG_ALLREDUCE, comm, &reqs[1]);
      if (MPI_SUCCESS != ret) { line = __LINE__; goto error_hndl; }
      ret = MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);
      if (MPI_SUCCESS != ret) { line = __LINE__; goto error_hndl; }

      /* Apply operation */
      if (rank < remote) {
         /* tmprecv = tmpsend (op) tmprecv */
         ADCL_op_reduce(op, tmpsend, tmprecv, count, dtype);
         tmpswap = tmprecv;
         tmprecv = tmpsend;
         tmpsend = tmpswap;
      } else {
         /* tmpsend = tmprecv (op) tmpsend */
         ADCL_op_reduce(op, tmprecv, tmpsend, count, dtype);
      }
   }

   /* Handle non-power-of-two case:
      - Odd ranks less than 2 * extra_ranks send result from tmpsend to
        (rank - 1)
      - Even ranks less than 2 * extra_ranks receive result from (rank + 1)
   */
   if (rank < (2 * extra_ranks)) {
      if (0 == (rank % 2)) {
         ret = MPI_Recv(rbuf, count, dtype, (rank + 1),
                                 ADCL_TAG_ALLREDUCE, comm, MPI_STATUS_IGNORE);
         if (MPI_SUCCESS != ret) { line = __LINE__; goto error_hndl; }
         tmpsend = (char*)rbuf;
      } else {
         ret = MPI_Send(tmpsend, count, dtype, (rank - 1),
                                 ADCL_TAG_ALLREDUCE, comm);
         if (MPI_SUCCESS != ret) { line = __LINE__; goto error_hndl; }
      }
   }

   /* Ensure that the final result is in rbuf */
   if (tmpsend != rbuf) {
      ret = ADCL_ddt_copy_content_same_ddt(dtype, bcount, (char*)rbuf, tmpsend);
      if (ret < 0) { line = __LINE__; goto error_hndl; }
   }

error_hndl:
   if (NULL != inplacebuf) free(inplacebuf);
   return;
}

