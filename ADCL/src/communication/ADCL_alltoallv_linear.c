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

void ADCL_alltoallv_linear ( ADCL_request_t *req ) 
{
    ADCL_topology_t *topo = req->r_emethod->em_topo;
    MPI_Comm comm         = topo->t_comm;
    MPI_Datatype dtype    = req->r_rdats[0]; 
    
    void *sbuf = req->r_svecs[0]->v_data;
    void *rbuf = req->r_rvecs[0]->v_data;
    
    ADCL_vmap_t *svmap = req->r_svecs[0]->v_map;
    ADCL_vmap_t *rvmap = req->r_rvecs[0]->v_map;
    int *scounts       = svmap->m_rcnts;
    int *rcounts       = rvmap->m_rcnts;
    int *sdispls       = svmap->m_displ;
    int *rdispls       = rvmap->m_displ;
    
    MPI_Request *preq;
    int i, line = -1, err = 0;
    int rank, size;
    void *psnd, *prcv;
    MPI_Aint ext, lb;
    int nreqs;
    
    size = topo->t_size;
    rank = topo->t_rank;
    
    err = MPI_Type_get_extent(dtype, &lb, &ext);
    if ( MPI_SUCCESS != err) { line = __LINE__; return; }
    
    nreqs = 2 * size;
    preq = ( MPI_Request * )malloc ( nreqs * sizeof (MPI_Request));
    if ( NULL == preq ) {
	return;
    }
    
    psnd = ((char *) sbuf) + (sdispls[rank] * ext);
    prcv = ((char *) rbuf) + (rdispls[rank] * ext);
    
    if (0 != scounts[rank]) {
	err = ADCL_ddt_copy_content_same_ddt ( dtype, scounts[rank]*ext, 
					       prcv, psnd );
	if ( MPI_SUCCESS != err ) {
	    return;
	}
    }
    
    /* If only one process, we're done. */
    if (1 == size) {
	return;
    }
    
    /* Post all receives first */
    for (i = 0; i < size; ++i) {
	if (i == rank || 0 == rcounts[i]) {
	    preq[2*i] = MPI_REQUEST_NULL;
	    continue;
	}
	
	prcv = ((char *) rbuf) + (rdispls[i] * ext);
	MPI_Irecv (prcv, rcounts[i], dtype,
		   i, ADCL_TAG_ALLTOALLV, comm,
		   &preq[2*i]);
	if (MPI_SUCCESS != err) {
	    free ( preq );
	    return;
	}
    }
    
    /* Now post all sends */
    for (i = 0; i < size; ++i) {
	if (i == rank || 0 == scounts[i]) {
	    preq[2*i+1] = MPI_REQUEST_NULL;
	    continue;
	}
	
	psnd = ((char *) sbuf) + (sdispls[i] * ext);
	err = MPI_Isend (psnd, scounts[i], dtype,
			 i, ADCL_TAG_ALLTOALLV,
			 comm, &preq[2*i+1]);
	if (MPI_SUCCESS != err) {
	    free ( preq );
	    return;
	}
    }
    
    err = MPI_Waitall( nreqs, preq, MPI_STATUSES_IGNORE);
    
    free ( preq );
    return;
}

