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
 * Linear functions are copied from the BASIC coll module
 * they do not segment the message and are simple implementations
 * but for some small number of nodes and/or small data sizes they
 * are just as fast as tuned/tree based segmenting operations
 * and as such may be selected by the decision functions
 * These are copied into this module due to the way we select modules
 * in V1. i.e. in V2 we will handle this differently and so will not
 * have to duplicate code.
 * GEF Oct05 after asking Jeff.
 */

/* copied function (with appropriate renaming) starts here */


/*
 *      allreduce_intra
 *
 *      Function:       - allreduce using other MPI collectives
 *      Accepts:        - same as MPI_Allreduce()
 *      Returns:        - MPI_SUCCESS or error code
 */


void  ADCL_allreduce_linear( ADCL_request_t *req)
{ 
    /*  MPI_Comm comm = topo->t_comm;
      ADCL_vmap_t *rvmap = req->r_rvecs[0]->v_map;
      MPI_Op op = rvmap->m_op; */
    int rank, err;
    MPI_Aint true_lb, true_extent, lb, extent;
    ADCL_topology_t *topo = req->r_emethod->em_topo;
    void *sbuf = req->r_svecs[0]->v_data;
    void *rbuf = req->r_rvecs[0]->v_data;
    int bcount;

    /* use receive vector */
    MPI_Datatype dtype = req->r_rdats[0]; 
    int count = req->r_svecs[0]->v_dims[0]; 

    /*size = topo->t_size; */
    rank = topo->t_rank; 

    /* Reduce to 0 and broadcast. */
    /*if (MPI_IN_PLACE == sbuf) {
        if (0 == rank) {
            err = ADCL_reduce_linear(MPI_IN_PLACE, rbuf, count, dtype, op, 0, comm);
        } else {
            err = ADCL_reduce_linear(rbuf, NULL, count, dtype, op, 0, comm);
        }
    } else { 
        err = ADCL_reduce_linear(sbuf, rbuf, count, dtype, op, 0, comm);
    } */
    if(req->r_emethod->em_root == ADCL_NO_ROOT)
    {
    	req->r_emethod->em_root = 0;
    }

    ADCL_reduce_linear( req );

    if ( req->r_emethod->em_root == rank && MPI_IN_PLACE != sbuf ) {
       /* result of reduce operation is in rbuf, but we need it in sbuf for
	* broadcast */
       MPI_Type_get_extent(dtype, &lb, &extent);
       MPI_Type_get_true_extent(dtype, &true_lb, &true_extent);
       bcount = true_extent + (count - 1) * extent;
       err = ADCL_ddt_copy_content_same_ddt(dtype, bcount, (char*) sbuf, (char*) rbuf);
    }

    ADCL_bcast_linear( req );

    return; 
}

