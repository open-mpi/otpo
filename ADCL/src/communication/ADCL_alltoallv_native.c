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

void ADCL_alltoallv_native( ADCL_request_t *req ) 
{
   int err;

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

   err = MPI_Alltoallv (sbuf, scounts, sdispls, dtype,
			rbuf, rcounts, rdispls, dtype, comm);
   return;
}

