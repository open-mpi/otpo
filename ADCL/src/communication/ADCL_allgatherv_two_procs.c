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



void ADCL_allgatherv_two_procs(ADCL_request_t *req )
{
    int line = -1, err = 0;
    int rank;
    int remote;
    char *tmpsend = NULL, *tmprecv = NULL;
    MPI_Aint sext, rext, lb;

    ADCL_topology_t *topo = req->r_emethod->em_topo;
    //ADCL_vmap_t *s_vmap = req->r_svecs[0]->v_map;
    ADCL_vmap_t *r_vmap = req->r_rvecs[0]->v_map;
    void *sbuf = req->r_svecs[0]->v_data;
    void *rbuf = req->r_rvecs[0]->v_data;
    MPI_Comm comm = topo->t_comm;
    
    /* Caution, this might be a hack */
    MPI_Datatype sdtype = req->r_sdats[0];
    MPI_Datatype rdtype = req->r_rdats[0];
    int scount = req->r_svecs[0]->v_dims[0];
    int *rcounts = r_vmap->m_rcnts;
    int *rdispls = r_vmap->m_displ;
	        
    scount = 1; // for ddt

    rank = topo->t_rank;

    err = MPI_Type_get_extent (sdtype, &lb, &sext);
    if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }

    err = MPI_Type_get_extent (rdtype, &lb, &rext);
    if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }

    /* Exchange data:
       - compute source and destinations
       - send receive data
    */
    remote  = rank ^ 0x1;

    tmpsend = (char*) sbuf;
    if (MPI_IN_PLACE == sbuf) {
        tmpsend = (char*)rbuf + rdispls[rank] * rext;
        scount = rcounts[rank];
        sdtype = rdtype;
    }
    tmprecv = (char*)rbuf + rdispls[remote] * rext;

    err = MPI_Sendrecv(tmpsend, scount, sdtype, remote,
		       ADCL_TAG_ALLGATHERV,
		       tmprecv, rcounts[remote], rdtype, remote,
		       ADCL_TAG_ALLGATHERV,
		       comm, MPI_STATUS_IGNORE);
    if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }

    /* Place your data in correct location if necessary */
    if (MPI_IN_PLACE != sbuf) {
      tmprecv = (char*) rbuf + rdispls[rank] * rext;
      err = MPI_Sendrecv (tmpsend, scount, sdtype, rank, ADCL_TAG_ALLGATHERV,
           tmprecv, rcounts[rank], rdtype, rank, ADCL_TAG_ALLGATHERV,
	   comm,  MPI_STATUS_IGNORE);
        if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl;  }
    }


 err_hndl:

    return;
}

