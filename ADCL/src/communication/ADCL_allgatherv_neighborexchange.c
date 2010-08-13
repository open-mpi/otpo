/* Algorithms implementing ALLGATHERV operations. Taken from the Open MPI source code repository.
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
 * allgatherv_neighborexchange
 *
 * Function:     allgatherv using N/2 steps (O(N))
 * Accepts:      Same arguments as MPI_Allgatherv
 * Returns:      MPI_SUCCESS or error code
 *
 * Description:  Neighbor Exchange algorithm for allgather adapted for 
 *               allgatherv.
 *               Described by Chen et.al. in 
 *               "Performance Evaluation of Allgather Algorithms on 
 *                Terascale Linux Cluster with Fast Ethernet",
 *               Proceedings of the Eighth International Conference on 
 *               High-Performance Computing inn Asia-Pacific Region
 *               (HPCASIA'05), 2005
 * 
 *               Rank r exchanges message with one of its neighbors and
 *               forwards the data further in the next step.
 *
 *               No additional memory requirements.
 * 
 * Limitations:  Algorithm works only on even number of processes.
 *               For odd number of processes we switch to ring algorithm.
 * 
 * Example on 6 nodes:
 *  Initial state
 *    #     0      1      2      3      4      5
 *         [0]    [ ]    [ ]    [ ]    [ ]    [ ]
 *         [ ]    [1]    [ ]    [ ]    [ ]    [ ]
 *         [ ]    [ ]    [2]    [ ]    [ ]    [ ]
 *         [ ]    [ ]    [ ]    [3]    [ ]    [ ]
 *         [ ]    [ ]    [ ]    [ ]    [4]    [ ]
 *         [ ]    [ ]    [ ]    [ ]    [ ]    [5]
 *   Step 0:
 *    #     0      1      2      3      4      5
 *         [0]    [0]    [ ]    [ ]    [ ]    [ ]
 *         [1]    [1]    [ ]    [ ]    [ ]    [ ]
 *         [ ]    [ ]    [2]    [2]    [ ]    [ ]
 *         [ ]    [ ]    [3]    [3]    [ ]    [ ]
 *         [ ]    [ ]    [ ]    [ ]    [4]    [4]
 *         [ ]    [ ]    [ ]    [ ]    [5]    [5]
 *   Step 1:
 *    #     0      1      2      3      4      5
 *         [0]    [0]    [0]    [ ]    [ ]    [0]
 *         [1]    [1]    [1]    [ ]    [ ]    [1]
 *         [ ]    [2]    [2]    [2]    [2]    [ ]
 *         [ ]    [3]    [3]    [3]    [3]    [ ]
 *         [4]    [ ]    [ ]    [4]    [4]    [4]
 *         [5]    [ ]    [ ]    [5]    [5]    [5]
 *   Step 2:
 *    #     0      1      2      3      4      5
 *         [0]    [0]    [0]    [0]    [0]    [0]
 *         [1]    [1]    [1]    [1]    [1]    [1]
 *         [2]    [2]    [2]    [2]    [2]    [2]
 *         [3]    [3]    [3]    [3]    [3]    [3]
 *         [4]    [4]    [4]    [4]    [4]    [4]
 *         [5]    [5]    [5]    [5]    [5]    [5]
 */
void ADCL_allgatherv_neighborexchange(ADCL_request_t *req)
{
    int line = -1;
    int rank, size;
    int neighbor[2], offset_at_step[2], recv_data_from[2], send_data_from;
    int new_scounts[2], new_sdispls[2], new_rcounts[2], new_rdispls[2];
    int i, even_rank;
    int err = 0;
    MPI_Aint rlb, rext;
    char *tmpsend = NULL, *tmprecv = NULL;
    MPI_Datatype  new_rdtype, new_sdtype;

    ADCL_topology_t *topo = req->r_emethod->em_topo;
    ADCL_vmap_t *r_vmap = req->r_rvecs[0]->v_map;
    void *sbuf = req->r_svecs[0]->v_data;
    void *rbuf = req->r_rvecs[0]->v_data;
    MPI_Comm comm = topo->t_comm;

    MPI_Datatype sdtype;
    int scount;
    MPI_Datatype rdtype = req->r_rdats[0];

    int *rcounts = r_vmap->m_rcnts;
    int *rdispls = r_vmap->m_displ;

    size = topo->t_size;
    rank = topo->t_rank;

    if (size % 2) {
	/* ADCL: I Don't like this!!!! */
        ADCL_allgatherv_ring(req);
	return;
    }

    err = MPI_Type_get_extent (rdtype, &rlb, &rext);
    if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }

    /* Initialization step:
       - if send buffer is not MPI_IN_PLACE, copy send buffer to 
       the appropriate block of receive buffer
    */
    tmprecv = (char*) rbuf + rdispls[rank] * rext;
    if (MPI_IN_PLACE != sbuf) {
        sdtype  = req->r_sdats[0];
        scount = req->r_scnts[0];
        tmpsend = (char*) sbuf;
        err = MPI_Sendrecv (tmpsend, scount, sdtype, rank, ADCL_TAG_ALLGATHERV,
               tmprecv, rcounts[rank], rdtype, rank, ADCL_TAG_ALLGATHERV,
	       comm,  MPI_STATUS_IGNORE);
        if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl;  }
    } 

    /* Determine neighbors, order in which blocks will arrive, etc. */
    even_rank = !(rank % 2);
    if (even_rank) {
        neighbor[0] = (rank + 1) % size;
        neighbor[1] = (rank - 1 + size) % size;
        recv_data_from[0] = rank;
        recv_data_from[1] = rank;
        offset_at_step[0] = (+2);
        offset_at_step[1] = (-2);
    } else {
        neighbor[0] = (rank - 1 + size) % size;
        neighbor[1] = (rank + 1) % size;
        recv_data_from[0] = neighbor[0];
        recv_data_from[1] = neighbor[0];
        offset_at_step[0] = (-2);
        offset_at_step[1] = (+2);
    }

    /* Communication loop:
       - First step is special: exchange a single block with neighbor[0].
       - Rest of the steps: 
       update recv_data_from according to offset, and 
       exchange two blocks with appropriate neighbor.
       the send location becomes previous receve location.
       Note, we need to create indexed datatype to send and receive these
       blocks properly.
    */
    tmprecv = (char*)rbuf + rdispls[neighbor[0]] * rext;
    tmpsend = (char*)rbuf + rdispls[rank] * rext;
    err = MPI_Sendrecv(tmpsend, rcounts[rank], rdtype, 
		       neighbor[0], ADCL_TAG_ALLGATHERV,
		       tmprecv, rcounts[neighbor[0]], rdtype, 
		       neighbor[0], ADCL_TAG_ALLGATHERV,
		       comm, MPI_STATUS_IGNORE);
    if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }
   
    /* Determine initial sending counts and displacements*/
    if (even_rank) {
        send_data_from = rank;
    } else {
        send_data_from = recv_data_from[0];
    }

    for (i = 1; i < (size / 2); i++) {
        const int i_parity = i % 2;
        recv_data_from[i_parity] = 
            (recv_data_from[i_parity] + offset_at_step[i_parity] + size) % size;

        /* Create new indexed types for sending and receiving.
           We are sending data from ranks (send_data_from) and (send_data_from+1)
           We are receiving data from ranks (recv_data_from[i_parity]) and
           (recv_data_from[i_parity]+1).
        */
        new_scounts[0] = rcounts[send_data_from];
        new_scounts[1] = rcounts[(send_data_from + 1)];
        new_sdispls[0] = rdispls[send_data_from];
        new_sdispls[1] = rdispls[(send_data_from + 1)];
        err = MPI_Type_indexed(2, new_scounts, new_sdispls, rdtype, 
                                      &new_sdtype);
        if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }
        err = MPI_Type_commit(&new_sdtype);
        if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }

        new_rcounts[0] = rcounts[recv_data_from[i_parity]];
        new_rcounts[1] = rcounts[(recv_data_from[i_parity] + 1)];
        new_rdispls[0] = rdispls[recv_data_from[i_parity]];
        new_rdispls[1] = rdispls[(recv_data_from[i_parity] + 1)];
        err = MPI_Type_indexed(2, new_rcounts, new_rdispls, rdtype, 
                                      &new_rdtype);
        if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }
        err = MPI_Type_commit(&new_rdtype);
        if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }
      
        tmprecv = (char*)rbuf;
        tmpsend = (char*)rbuf;
      
        /* Sendreceive */
        err = MPI_Sendrecv(tmpsend, 1, new_sdtype, neighbor[i_parity],
			   ADCL_TAG_ALLGATHERV,
			   tmprecv, 1, new_rdtype, neighbor[i_parity],
			   ADCL_TAG_ALLGATHERV,
			   comm, MPI_STATUS_IGNORE);
        if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }
	
        send_data_from = recv_data_from[i_parity];
      
        MPI_Type_free(&new_sdtype);
        MPI_Type_free(&new_rdtype);
    }

 err_hndl:
    return;
}


