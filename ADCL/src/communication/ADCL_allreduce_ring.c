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
 *   ompi_coll_tuned_allreduce_intra_ring
 *
 *   Function:       Ring algorithm for allreduce operation
 *   Accepts:        Same as MPI_Allreduce()
 *   Returns:        MPI_SUCCESS or error code
 *
 *   Description:    Implements ring algorithm for allreduce: the message is
 *                   automatically segmented to segment of size M/N.
 *                   Algorithm requires 2*N - 1 steps.
 *
 *   Limitations:    The algorithm DOES NOT preserve order of operations so it
 *                   can be used only for commutative operations.
 *                   In addition, algorithm cannot work if the total count is
 *                   less than size.
 *         Example on 5 nodes:
 *         Initial state
 *   #      0              1             2              3             4
 *        [00]           [10]          [20]           [30]           [40]
 *        [01]           [11]          [21]           [31]           [41]
 *        [02]           [12]          [22]           [32]           [42]
 *        [03]           [13]          [23]           [33]           [43]
 *        [04]           [14]          [24]           [34]           [44]
 *
 *        COMPUTATION PHASE
 *         Step 0: rank r sends block r to rank (r+1) and receives bloc (r-1)
 *                 from rank (r-1) [with wraparound].
 *    #     0              1             2              3             4
 *        [00]          [00+10]        [20]           [30]           [40]
 *        [01]           [11]         [11+21]         [31]           [41]
 *        [02]           [12]          [22]          [22+32]         [42]
 *        [03]           [13]          [23]           [33]         [33+43]
 *      [44+04]          [14]          [24]           [34]           [44]
 *
 *         Step 1: rank r sends block (r-1) to rank (r+1) and receives bloc
 *                 (r-2) from rank (r-1) [with wraparound].
 *    #      0              1             2              3             4
 *         [00]          [00+10]     [01+10+20]        [30]           [40]
 *         [01]           [11]         [11+21]      [11+21+31]        [41]
 *         [02]           [12]          [22]          [22+32]      [22+32+42]
 *      [33+43+03]        [13]          [23]           [33]         [33+43]
 *        [44+04]       [44+04+14]       [24]           [34]           [44]
 *
 *         Step 2: rank r sends block (r-2) to rank (r+1) and receives bloc
 *                 (r-2) from rank (r-1) [with wraparound].
 *    #      0              1             2              3             4
 *         [00]          [00+10]     [01+10+20]    [01+10+20+30]      [40]
 *         [01]           [11]         [11+21]      [11+21+31]    [11+21+31+41]
 *     [22+32+42+02]      [12]          [22]          [22+32]      [22+32+42]
 *      [33+43+03]    [33+43+03+13]     [23]           [33]         [33+43]
 *        [44+04]       [44+04+14]  [44+04+14+24]      [34]           [44]
 *
 *         Step 3: rank r sends block (r-3) to rank (r+1) and receives bloc
 *                 (r-3) from rank (r-1) [with wraparound].
 *    #      0              1             2              3             4
 *         [00]          [00+10]     [01+10+20]    [01+10+20+30]     [FULL]
 *        [FULL]           [11]        [11+21]      [11+21+31]    [11+21+31+41]
 *     [22+32+42+02]     [FULL]          [22]         [22+32]      [22+32+42]
 *      [33+43+03]    [33+43+03+13]     [FULL]          [33]         [33+43]
 *        [44+04]       [44+04+14]  [44+04+14+24]      [FULL]         [44]
 *
 *        DISTRIBUTION PHASE: ring ALLGATHER with ranks shifted by 1.
 *
 */
void
ADCL_allreduce_ring( ADCL_request_t* req )
{
   int ret, line;
   int rank, size, k, recvfrom, sendto;
   int segcount, lastsegcount, maxsegcount;
   int blockcount, inbi;
   int typelng;
   char *tmpsend = NULL, *tmprecv = NULL;
   char *inbuf[2] = {NULL, NULL};
   MPI_Aint true_lb, true_extent, lb, extent;
   int realsegsize, maxrealsegsize;
   MPI_Request reqs[2];

   ADCL_topology_t *topo = req->r_emethod->em_topo;
   MPI_Comm comm = topo->t_comm;
   void *sbuf = req->r_svecs[0]->v_data;
   void *rbuf = req->r_rvecs[0]->v_data;
   int bcount;

   /* use receive data */
   ADCL_vmap_t *rvmap = req->r_rvecs[0]->v_map;
   MPI_Datatype dtype = req->r_rdats[0]; 
   int count = req->r_rvecs[0]->v_dims[0];
   MPI_Op op = rvmap->m_op;

   size = topo->t_size;
   rank = topo->t_rank;

   /* Allocate and initialize temporary buffers */
   ret = MPI_Type_get_extent(dtype, &lb, &extent);
   if (MPI_SUCCESS != ret) { line = __LINE__; goto error_hndl; }
   ret = MPI_Type_get_true_extent(dtype, &true_lb, &true_extent);
   if (MPI_SUCCESS != ret) { line = __LINE__; goto error_hndl; }
   MPI_Type_size(dtype, &typelng);
   bcount = true_extent + (count - 1) * extent;

   /* Special case for size == 1 */
   if (1 == size) {
      if (MPI_IN_PLACE != sbuf) {
         ret = ADCL_ddt_copy_content_same_ddt(dtype, bcount, (char*)rbuf, (char*)sbuf);
         if (ret < 0) { line = __LINE__; goto error_hndl; }
      }
      return;
   }

   /* Special case for count less than size - 1 - use recursive doubling */
   if (count < size - 1) {
      printf("WARNING: ADCL_allreduce_ring falls back to ADCL_allreduce_recursivedoubling.\n");
      ADCL_allreduce_recursivedoubling( req );
      return;
   }


   /* Determine number of elements per block.
      This is not the same computation as the one for number of elements
      per segment - and we may end up having last block larger any other block.
    */
   segcount = count / size;
   if (0 != count % size) segcount++;
   lastsegcount = count - (size - 1) * segcount;
   if (lastsegcount <= 0) {
      segcount--;
      lastsegcount = count - (size - 1) * segcount;
   }
   realsegsize = segcount * extent;
   maxsegcount = (segcount > lastsegcount)? segcount : lastsegcount;
   maxrealsegsize = true_extent + (maxsegcount - 1) * extent;

   inbuf[0] = (char*)malloc(maxrealsegsize);
   if (NULL == inbuf[0]) { ret = -1; line = __LINE__; goto error_hndl; }
   if (size > 2) {
      inbuf[1] = (char*)malloc(maxrealsegsize);
      if (NULL == inbuf[1]) { ret = -1; line = __LINE__; goto error_hndl; }
   }

   /* Handle MPI_IN_PLACE */
   if (MPI_IN_PLACE != sbuf) {
      ret = ADCL_ddt_copy_content_same_ddt(dtype, bcount, (char*)rbuf, (char*)sbuf);
      if (ret < 0) { line = __LINE__; goto error_hndl; }
   }

   /* Computation loop */

   /*
      For each of the remote nodes:
      - post irecv for block (r-1)
      - send block (r)
      - in loop for every step k = 2 .. n
         - post irecv for block (r + n - k) % n
         - wait on block (r + n - k + 1) % n to arrive
         - compute on block (r + n - k + 1) % n
         - send block (r + n - k + 1) % n
     - wait on block (r + 1)
     - compute on block (r + 1)
     - send block (r + 1) to rank (r + 1)
     Note that for send operations and computation we must compute the exact
     block size.
    */
   sendto = (rank + 1) % size;
   recvfrom = (rank + size - 1) % size;

   inbi = 0;
   tmpsend = ((char*)rbuf) + rank * realsegsize;
   /* Initialize first receive from the neighbor on the left */

   ret = MPI_Irecv(inbuf[inbi], maxsegcount, dtype, recvfrom,
                            ADCL_TAG_ALLREDUCE, comm, &reqs[inbi]);
   if (MPI_SUCCESS != ret) { line = __LINE__; goto error_hndl; }
   /* Send first block to the neighbor on the right */
   blockcount = segcount;
   if ((size - 1) == rank) { blockcount = lastsegcount; }
   ret = MPI_Send(tmpsend, blockcount, dtype, sendto,
                           ADCL_TAG_ALLREDUCE, comm);
   if (MPI_SUCCESS != ret) { line = __LINE__; goto error_hndl; }

   for (k = 2; k < size; k++) {
      const int prevblock = (rank + size - k + 1) % size;

      inbi = inbi ^ 0x1;
      /* Post irecv for the current block */
      ret = MPI_Irecv(inbuf[inbi], maxsegcount, dtype, recvfrom,
                               ADCL_TAG_ALLREDUCE, comm, &reqs[inbi]);
      if (MPI_SUCCESS != ret) { line = __LINE__; goto error_hndl; }

      /* Wait on previous block to arrive */
      ret = MPI_Wait(&reqs[inbi ^ 0x1], MPI_STATUS_IGNORE);
      if (MPI_SUCCESS != ret) { line = __LINE__; goto error_hndl; }

      /* Apply operation on previous block: result goes to rbuf
         rbuf[prevblock] = inbuf[inbi ^ 0x1] (op) rbuf[prevblock]
      */
      blockcount = segcount;
      if ((size - 1) == prevblock) { blockcount = lastsegcount; }
      tmprecv = ((char*)rbuf) + prevblock * realsegsize;
      ADCL_op_reduce(op, inbuf[inbi ^ 0x1], tmprecv, blockcount, dtype);

      /* send previous block to sendto */
      ret = MPI_Send(tmprecv, blockcount, dtype, sendto,
                              ADCL_TAG_ALLREDUCE, comm);
      if (MPI_SUCCESS != ret) { line = __LINE__; goto error_hndl; }
   }

   /* Wait on the last block to arrive */
   ret = MPI_Wait(&reqs[inbi], MPI_STATUS_IGNORE);
   if (MPI_SUCCESS != ret) { line = __LINE__; goto error_hndl; }

   /* Apply operation on the last block (from neighbor (rank + 1)
      rbuf[rank+1] = inbuf[inbi] (op) rbuf[rank + 1] */
   blockcount = segcount;
   if ((size - 1) == (rank + 1) % size) { blockcount = lastsegcount; }
   tmprecv = ((char*)rbuf) + ((rank + 1) % size) * realsegsize;
   ADCL_op_reduce(op, inbuf[inbi], tmprecv, blockcount, dtype);

   /* Distribution loop - variation of ring allgather */
   for (k = 0; k < size - 1; k++) {
      const int recvdatafrom = (rank + size - k) % size;
      const int senddatafrom = (rank + 1 + size - k) % size;

      blockcount = segcount;
      if ((size - 1) == senddatafrom) blockcount = lastsegcount;
      tmprecv = (char*)rbuf + recvdatafrom * realsegsize;
      tmpsend = (char*)rbuf + senddatafrom * realsegsize;

      /* ompi tuned sendrecv */
      ret = MPI_Sendrecv(tmpsend, blockcount, dtype, sendto,
                                     ADCL_TAG_ALLREDUCE,
                                     tmprecv, maxsegcount, dtype, recvfrom,
                                     ADCL_TAG_ALLREDUCE,
                                     comm, MPI_STATUS_IGNORE);
      if (MPI_SUCCESS != ret) { line = __LINE__; goto error_hndl;}

   }

   if (NULL != inbuf[0]) free(inbuf[0]);
   if (NULL != inbuf[1]) free(inbuf[1]);

   return;

error_hndl:
   if (NULL != inbuf[0]) free(inbuf[0]);
   if (NULL != inbuf[1]) free(inbuf[1]);
   return;
}
