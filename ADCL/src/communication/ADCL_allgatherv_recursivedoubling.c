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

#include "ADCL_internal.h"

/*
 * ompi_coll_tuned_allgather_intra_recursivedoubling
 *
 * Function:     allgather using O(log(N)) steps.
 * Accepts:      Same arguments as MPI_Allgather
 * Returns:      MPI_SUCCESS or error code
 *
 * Description:  Recursive doubling algorithm for MPI_Allgather implementation.
 *               This algorithm is used in MPICH-2 for small- and medium-sized
 *               messages on power-of-two processes.
 *
 * Limitation:   Current implementation only works on power-of-two number of
 *               processes.
 *               In case this algorithm is invoked on non-power-of-two
 *               processes, Bruck algorithm will be invoked.
 *
 * Memory requirements:
 *               No additional memory requirements beyond user-supplied buffers.
 *
 * Example on 4 nodes:
 *   Initialization: everyone has its own buffer at location rank in rbuf
 *    #     0      1      2      3
 *         [0]    [ ]    [ ]    [ ]
 *         [ ]    [1]    [ ]    [ ]
 *         [ ]    [ ]    [2]    [ ]
 *         [ ]    [ ]    [ ]    [3]
 *   Step 0: exchange data with (rank ^ 2^0)
 *    #     0      1      2      3
 *         [0]    [0]    [ ]    [ ]
 *         [1]    [1]    [ ]    [ ]
 *         [ ]    [ ]    [2]    [2]
 *         [ ]    [ ]    [3]    [3]
 *   Step 1: exchange data with (rank ^ 2^1) (if you can)
 *    #     0      1      2      3
 *         [0]    [0]    [0]    [0]
 *         [1]    [1]    [1]    [1]
 *         [2]    [2]    [2]    [2]
 *         [3]    [3]    [3]    [3]
 *
 *  TODO: Modify the algorithm to work with any number of nodes.
 *        We can modify code to use identical implementation like MPICH-2:
 *        - using recursive-halving algorith, at the end of each step,
 *          determine if there are nodes who did not exchange their data in that
 *          step, and send them appropriate messages.
 */

void ADCL_allgatherv_recursivedoubling(ADCL_request_t *req)
{
   int line = -1;
   int rank, size, pow2size, i;
   int remote, distance;
   int r, *sendblocklocation;
   int err = 0;
   MPI_Aint rlb, rext;
   char *tmpsend = NULL, *tmprecv = NULL;

   ADCL_topology_t *topo = req->r_emethod->em_topo;
   MPI_Comm comm = topo->t_comm;
   ADCL_vmap_t *r_vmap = req->r_rvecs[0]->v_map;
   void *sbuf = req->r_svecs[0]->v_data;
   void *rbuf = req->r_rvecs[0]->v_data;

   MPI_Datatype sdtype; 
   int scount;
   MPI_Datatype rdtype = req->r_rdats[0];

   int *rcounts = r_vmap->m_rcnts;
   int *rdispls = r_vmap->m_displ;
   int rcount; 

   size = topo->t_size;
   rank = topo->t_rank;

   sendblocklocation = (int *) calloc( size, sizeof(int) );

   for (pow2size  = 1; pow2size <= size; pow2size <<=1);
   pow2size >>=1;

   /* Current implementation only handles power-of-two number of processes.
      If the function was called on non-power-of-two number of processes,
      print warning and call bruck allgather algorithm with same parameters.
    */
   if (pow2size != size) {
      ADCL_allgatherv_bruck(req);
      return;
   }

   err = MPI_Type_get_extent (rdtype, &rlb, &rext);
   if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }

   /* Initialization step:
      - if send buffer is not MPI_IN_PLACE, copy send buffer to block 0 of
      receive buffer
   */
   if (MPI_IN_PLACE != sbuf) {
      sdtype = req->r_sdats[0];
      scount = req->r_scnts[0];
      tmpsend = (char*) sbuf;
      tmprecv = (char*) rbuf + rdispls[rank] * rext;
      err = MPI_Sendrecv (tmpsend, scount, sdtype, rank, ADCL_TAG_ALLGATHERV, 
                    tmprecv, rcounts[rank], rdtype, rank, ADCL_TAG_ALLGATHERV,
	            comm,  MPI_STATUS_IGNORE);
      if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl;  }
   }

   /* Communication step:
      At every step i, rank r:
      - exchanges message with rank remote = (r ^ 2^i).

   */


   //sendblocklocation = rank;
   for (r=0; r<size; r++){
       sendblocklocation[r] = r;
   }

   for (distance = 0x1; distance < size; distance<<=1) {
      remote = rank ^ distance;

      scount = 0;
      rcount = 0;
      for (i=0; i<distance; i++){
          scount += rcounts[sendblocklocation[rank]  +i];
          rcount += rcounts[sendblocklocation[remote]+i];
      }

      if (rank < remote) {
         tmpsend = (char*)rbuf + rdispls[sendblocklocation[rank]] * rext;
         tmprecv = (char*)rbuf + rdispls[sendblocklocation[rank] + distance] * rext;
      } else {
         tmpsend = (char*)rbuf + rdispls[sendblocklocation[rank]] * rext;
         tmprecv = (char*)rbuf + rdispls[sendblocklocation[rank] - distance] * rext;
         //sendblocklocation -= distance;
      }

      /* Sendreceive */
      err = MPI_Sendrecv(tmpsend, scount, rdtype, remote, ADCL_TAG_ALLGATHERV,
                 tmprecv, rcount, rdtype, remote, ADCL_TAG_ALLGATHERV,
                 comm, MPI_STATUS_IGNORE);

      if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }

      for (r=0; r<size; r++){
         remote = r ^ distance;
         if (r >= remote) {
             sendblocklocation[r] -= distance;
         }
      }

   }

 err_hndl:
   free( sendblocklocation );
   return;
}
