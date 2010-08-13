/* 
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


void ADCL_allgatherv_native ( ADCL_request_t *req )
{
    int size, rank;
    int err;
    MPI_Aint extent;
    MPI_Aint lb;
    char *send_buf = NULL;
    MPI_Datatype newtype, send_type;

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

    if (MPI_IN_PLACE != sbuf) {
        scount = req->r_scnts[0];
        sdtype = req->r_sdats[0];
    }

    err = MPI_Allgatherv(sbuf, scount, sdtype, 
                      rbuf, rcounts, rdispls, rdtype, 
		      comm);
    
    if (MPI_SUCCESS != err) {
        return;
    }
}

