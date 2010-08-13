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
 * ** Codes have been adapted to be used in ADCL 
 * ** Following are the ADCL copyrights
 * */

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
 *  reduce_lin_intra
 *
 *  Function:   - reduction using O(N) algorithm
 *  Accepts:    - same as MPI_Reduce()
 *  Returns:    - MPI_SUCCESS or error code
 */

void ADCL_reduce_linear( ADCL_request_t *req )
{
   int i, rank, err, size;
   MPI_Aint true_lb, true_extent, lb, extent;
   char *free_buffer = NULL;
   char *pml_buffer = NULL;
   char *inplace_temp = NULL;
   char *inbuf;
   int root = req->r_emethod->em_root; 

   ADCL_topology_t *topo = req->r_emethod->em_topo;
   MPI_Comm comm = topo->t_comm;
   ADCL_vmap_t *svmap = req->r_svecs[0]->v_map;
   ADCL_vmap_t *rvmap = req->r_rvecs[0]->v_map;
   /* careful, pointers to sbuf are modified! */
   void *req_sbuf = req->r_svecs[0]->v_data;
   void *req_rbuf = req->r_rvecs[0]->v_data;
   void *sbuf, *rbuf;
   int bcount;




   /* use receive vector */
   MPI_Datatype dtype = req->r_rdats[0]; 
   int count = req->r_rvecs[0]->v_dims[0];
   MPI_Op op = rvmap->m_op; 

    /* Initialize */
   size = topo->t_size;
   rank = topo->t_rank;
   sbuf = req_sbuf;
   rbuf = req_rbuf;

    if (MPI_IN_PLACE == sbuf) {
        sbuf = rbuf;
    }

    /* If not root, send data to the root. */
    if (rank != root) {
        err = MPI_Send(sbuf, count, dtype, root,
                                ADCL_TAG_REDUCE, comm);
        return;
    }

    /* so this is root */

    /* see discussion in ompi_coll_basic_reduce_lin_intra about extent and true extend */
    /* for reducing buffer allocation lengths.... */

    MPI_Type_get_extent(dtype, &lb, &extent);
    MPI_Type_get_true_extent(dtype, &true_lb, &true_extent);
    bcount = true_extent + (count - 1) * extent;

    if (MPI_IN_PLACE == req_sbuf) {
        inplace_temp = (char*)malloc(true_extent + (count - 1) * extent);
        if (NULL == inplace_temp) {
            return;
        }
        rbuf = inplace_temp - lb;
    }

    if (size > 1) {
        free_buffer = (char*)malloc(true_extent + (count - 1) * extent);
        if (NULL == free_buffer) {
            return;
        }
        pml_buffer = free_buffer - lb;
    }

    /* Initialize the receive buffer. */

    if (root == (size - 1)) {
        err = ADCL_ddt_copy_content_same_ddt(dtype, bcount, (char*)rbuf, (char*)sbuf);
    } else {
        err = MPI_Recv(rbuf, count, dtype, size - 1,
                                ADCL_TAG_REDUCE, comm, MPI_STATUS_IGNORE);
    }
    if (MPI_SUCCESS != err) {
        if (NULL != free_buffer) {
            free(free_buffer);
        }
        return;
    }

    /* Loop receiving and calling reduction function (C or Fortran). */

    for (i = size - 2; i >= 0; --i) {
        if (root == i) {
            inbuf = (char*)sbuf;
        } else {
            err = MPI_Recv(pml_buffer, count, dtype, i,
                                    ADCL_TAG_REDUCE, comm, MPI_STATUS_IGNORE);
            if (MPI_SUCCESS != err) {
                if (NULL != free_buffer) {
                    free(free_buffer);
                }
                return;
            }

            inbuf = pml_buffer;
        }

        /* Perform the reduction */
        ADCL_op_reduce(op, inbuf, rbuf, count, dtype);
    }

    if (NULL != inplace_temp) {
        err = ADCL_ddt_copy_content_same_ddt(dtype, bcount, (char*)req_rbuf, inplace_temp);
        free(inplace_temp);
    }
    if (NULL != free_buffer) {
        free(free_buffer);
    }

    /* All done */
    return;
}


