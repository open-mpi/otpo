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
 * allgatherv_bruck
 *
 * Function:     allgather using O(log(N)) steps.
 * Accepts:      Same arguments as MPI_Allgather
 * Returns:      MPI_SUCCESS or error code
 *
 * Description:  Variation to All-to-all algorithm described by Bruck et al.in
 *               "Efficient Algorithms for All-to-all Communications
 *                in Multiport Message-Passing Systems"
 * Note:         Unlike in case of allgather implementation, we relay on
 *               indexed datatype to select buffers appropriately.
 *               The only additional memory requirement is for creation of 
 *               temporary datatypes.
 * Example on 7 nodes (memory lay out need not be in-order)
 *   Initial set up:
 *    #     0      1      2      3      4      5      6
 *         [0]    [ ]    [ ]    [ ]    [ ]    [ ]    [ ]
 *         [ ]    [1]    [ ]    [ ]    [ ]    [ ]    [ ]
 *         [ ]    [ ]    [2]    [ ]    [ ]    [ ]    [ ]
 *         [ ]    [ ]    [ ]    [3]    [ ]    [ ]    [ ]
 *         [ ]    [ ]    [ ]    [ ]    [4]    [ ]    [ ]
 *         [ ]    [ ]    [ ]    [ ]    [ ]    [5]    [ ]
 *         [ ]    [ ]    [ ]    [ ]    [ ]    [ ]    [6]
 *   Step 0: send message to (rank - 2^0), receive message from (rank + 2^0)
 *    #     0      1      2      3      4      5      6
 *         [0]    [ ]    [ ]    [ ]    [ ]    [ ]    [0]
 *         [1]    [1]    [ ]    [ ]    [ ]    [ ]    [ ]
 *         [ ]    [2]    [2]    [ ]    [ ]    [ ]    [ ]
 *         [ ]    [ ]    [3]    [3]    [ ]    [ ]    [ ]
 *         [ ]    [ ]    [ ]    [4]    [4]    [ ]    [ ]
 *         [ ]    [ ]    [ ]    [ ]    [5]    [5]    [ ]
 *         [ ]    [ ]    [ ]    [ ]    [ ]    [6]    [6]
 *   Step 1: send message to (rank - 2^1), receive message from (rank + 2^1).
 *           message contains all blocks from (rank) .. (rank + 2^2) with 
 *           wrap around.
 *    #     0      1      2      3      4      5      6
 *         [0]    [ ]    [ ]    [ ]    [0]    [0]    [0]
 *         [1]    [1]    [ ]    [ ]    [ ]    [1]    [1]
 *         [2]    [2]    [2]    [ ]    [ ]    [ ]    [2]
 *         [3]    [3]    [3]    [3]    [ ]    [ ]    [ ]
 *         [ ]    [4]    [4]    [4]    [4]    [ ]    [ ]
 *         [ ]    [ ]    [5]    [5]    [5]    [5]    [ ]
 *         [ ]    [ ]    [ ]    [6]    [6]    [6]    [6]
 *   Step 2: send message to (rank - 2^2), receive message from (rank + 2^2).
 *           message size is "all remaining blocks" 
 *    #     0      1      2      3      4      5      6
 *         [0]    [0]    [0]    [0]    [0]    [0]    [0]
 *         [1]    [1]    [1]    [1]    [1]    [1]    [1]
 *         [2]    [2]    [2]    [2]    [2]    [2]    [2]
 *         [3]    [3]    [3]    [3]    [3]    [3]    [3]
 *         [4]    [4]    [4]    [4]    [4]    [4]    [4]
 *         [5]    [5]    [5]    [5]    [5]    [5]    [5]
 *         [6]    [6]    [6]    [6]    [6]    [6]    [6]
 */
void ADCL_allgatherv_bruck( ADCL_request_t *req )
{
   int line = -1, err = 0;
   int rank, size;
   int sendto, recvfrom, distance, blockcount, i;
   int *new_rcounts = NULL, *new_rdispls = NULL;
   int *new_scounts = NULL, *new_sdispls = NULL;
   MPI_Aint rlb, rext;
   char *tmpsend = NULL, *tmprecv = NULL;
   MPI_Datatype new_rdtype, new_sdtype;

   ADCL_topology_t *topo = req->r_emethod->em_topo;
   MPI_Comm comm = topo->t_comm;
   ADCL_vmap_t *r_vmap = req->r_rvecs[0]->v_map;
   void *sbuf = req->r_svecs[0]->v_data;
   void *rbuf = req->r_rvecs[0]->v_data;

   MPI_Datatype sdtype; 
   int scount;
   MPI_Datatype rdtype = req->r_rdats[0];

   //int scount = req->r_svecs[0]->v_dims[0];
   int *rcounts = r_vmap->m_rcnts;
   int *rdispls = r_vmap->m_displ;

   size = topo->t_size;
   rank = topo->t_rank;

   err = MPI_Type_get_extent (rdtype, &rlb, &rext);
   if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }

   /* Initialization step:
      - if send buffer is not MPI_IN_PLACE, copy send buffer to block rank of 
        the receive buffer.
   */
   tmprecv = (char*) rbuf + rdispls[rank] * rext;
   if (MPI_IN_PLACE != sbuf) {
      scount = req->r_scnts[0];
      sdtype = req->r_sdats[0];
      tmpsend = (char*) sbuf;
      err = MPI_Sendrecv (tmpsend, scount, sdtype, rank, ADCL_TAG_ALLGATHERV, 
                    tmprecv, rcounts[rank], rdtype, rank, ADCL_TAG_ALLGATHERV,
	            comm,  MPI_STATUS_IGNORE);
      if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl;  }

   }

   /* Communication step:
      At every step i, rank r:
      - doubles the distance
      - sends message with blockcount blocks, (rbuf[rank] .. rbuf[rank + 2^i])
        to rank (r - distance)
      - receives message of blockcount blocks, 
        (rbuf[r + distance] ... rbuf[(r+distance) + 2^i]) from 
        rank (r + distance)
      - blockcount doubles until the last step when only the remaining data is 
      exchanged.
   */
   blockcount = 1;
   tmpsend = (char*) rbuf;

   new_rcounts = (int*) calloc(size, sizeof(int));
   if (NULL == new_rcounts) { err = -1; line = __LINE__; goto err_hndl; }
   new_rdispls = (int*) calloc(size, sizeof(int));
   if (NULL == new_rdispls) { err = -1; line = __LINE__; goto err_hndl; }
   new_scounts = (int*) calloc(size, sizeof(int));
   if (NULL == new_scounts) { err = -1; line = __LINE__; goto err_hndl; }
   new_sdispls = (int*) calloc(size, sizeof(int));
   if (NULL == new_sdispls) { err = -1; line = __LINE__; goto err_hndl; }

   for (distance = 1; distance < size; distance<<=1) {

      recvfrom = (rank + distance) % size;
      sendto = (rank - distance + size) % size;

      if (distance <= (size >> 1)) {
         blockcount = distance;
      } else { 
         blockcount = size - distance;
      }

      /* create send and receive datatypes */
      for (i = 0; i < blockcount; i++) {
          const int tmp_srank = (rank + i) % size;
          const int tmp_rrank = (recvfrom + i) % size;
          new_scounts[i] = rcounts[tmp_srank];
          new_sdispls[i] = rdispls[tmp_srank];
          new_rcounts[i] = rcounts[tmp_rrank];
          new_rdispls[i] = rdispls[tmp_rrank];
      }
      err = MPI_Type_indexed(blockcount, new_scounts, new_sdispls, 
                                    rdtype, &new_sdtype);
      if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }
      err = MPI_Type_indexed(blockcount, new_rcounts, new_rdispls,
                                    rdtype, &new_rdtype);

      err = MPI_Type_commit(&new_sdtype);
      if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }
      err = MPI_Type_commit(&new_rdtype);
      if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }

      /* Sendreceive */
      err = MPI_Sendrecv(rbuf, 1, new_sdtype, sendto,
			 ADCL_TAG_ALLGATHERV,
			 rbuf, 1, new_rdtype, recvfrom,
			 ADCL_TAG_ALLGATHERV,
			 comm, MPI_STATUS_IGNORE);
      MPI_Type_free(&new_sdtype);
      MPI_Type_free(&new_rdtype);

   }

 err_hndl:
   if( NULL != new_rcounts ) free(new_rcounts);
   if( NULL != new_rdispls ) free(new_rdispls);
   if( NULL != new_scounts ) free(new_scounts);
   if( NULL != new_sdispls ) free(new_sdispls);

   return;
}


