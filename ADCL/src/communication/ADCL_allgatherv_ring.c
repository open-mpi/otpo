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
 * allgatherv_ring
 *
 * Function:     allgatherv using O(N) steps.
 * Accepts:      Same arguments as MPI_Allgatherv
 * Returns:      MPI_SUCCESS or error code
 *
 * Description:  Ring algorithm for all gather.
 *               At every step i, rank r receives message from rank (r - 1)
 *               containing data from rank (r - i - 1) and sends message to rank
 *               (r + 1) containing data from rank (r - i), with wrap arounds.
 * Memory requirements:
 *               No additional memory requirements.
 * 
 */
void ADCL_allgatherv_ring( ADCL_request_t *req )
{
    int line = -1;
    int rank, size;
    int sendto, recvfrom, i, recvdatafrom, senddatafrom;
    int err = 0;
    MPI_Aint rlb, rext;
    char *tmpsend = NULL, *tmprecv = NULL;

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

    err = MPI_Type_get_extent (rdtype, &rlb, &rext);
    if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }

    /* Initialization step:
       - if send buffer is not MPI_IN_PLACE, copy send buffer to 
       the appropriate block of receive buffer
    */
    tmprecv = (char*) rbuf + rdispls[rank] * rext;
    if (MPI_IN_PLACE != sbuf) {
        sdtype = req->r_sdats[0];
        scount = req->r_scnts[0];
        tmpsend = (char*) sbuf;
        err = MPI_Sendrecv (tmpsend, scount, sdtype, rank, ADCL_TAG_ALLGATHERV,
                tmprecv, rcounts[rank], rdtype, rank, ADCL_TAG_ALLGATHERV,
	        comm,  MPI_STATUS_IGNORE);
        if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl;  }
    } 
   
    /* Communication step:
       At every step i: 0 .. (P-1), rank r:
       - receives message from [(r - 1 + size) % size] containing data from rank
       [(r - i - 1 + size) % size]
       - sends message to rank [(r + 1) % size] containing data from rank
       [(r - i + size) % size]
       - sends message which starts at begining of rbuf and has size 
    */
    sendto = (rank + 1) % size;
    recvfrom  = (rank - 1 + size) % size;

    for (i = 0; i < size - 1; i++) {
        recvdatafrom = (rank - i - 1 + size) % size;
        senddatafrom = (rank - i + size) % size;

        tmprecv = (char*)rbuf + rdispls[recvdatafrom] * rext;
        tmpsend = (char*)rbuf + rdispls[senddatafrom] * rext;

        /* Sendreceive */
        err = MPI_Sendrecv(tmpsend, rcounts[senddatafrom], rdtype, 
			   sendto, ADCL_TAG_ALLGATHERV,
			   tmprecv, rcounts[recvdatafrom], rdtype, 
			   recvfrom, ADCL_TAG_ALLGATHERV,
			   comm, MPI_STATUS_IGNORE);
        if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }

    }


err_hndl:
    return ;
}

