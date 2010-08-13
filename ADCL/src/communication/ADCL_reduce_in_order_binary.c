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

/* copied function (with appropriate renaming) starts here */

/*
 *  reduce_lin_intra
 *
 *  Function:   - reduction using O(N) algorithm
 *  Accepts:    - same as MPI_Reduce()
 *  Returns:    - MPI_SUCCESS or error code
 */

void ADCL_REDUCE_IN_ORDER_BINARY( ADCL_request_t *req )
{
   int root = 0, segsize /*= 0*/, max_outstanding_reqs = 0;
   ADCL_topology_t *topo = req->r_emethod->em_topo;
   MPI_Comm comm = topo->t_comm;
   MPI_Datatype datatype = req->r_rdats[0];
   int  count = req->r_rvecs[0]->v_dims[0];
   void *sendbuf = req->r_svecs[0]->v_data;
   void *recvbuf = req->r_rvecs[0]->v_data;
   ADCL_vmap_t *rvmap = req->r_rvecs[0]->v_map;
   MPI_Op op = rvmap->m_op;
   int ret;
   int io_root;
   int rank = topo->t_rank;
   int size = topo->t_size;
   void *use_this_sendbuf= NULL, *use_this_recvbuf = NULL;
   int segcount = count;
   int typelng;

    ADCL_tree_t *tree;
    segsize = REDUCE_SEGSIZE;
    root = req->r_emethod->em_root;
    tree = ADCL_build_in_order_bintree( req );
    /**
     * Determine number of segments and number of elements
     * sent per operation
     */
    MPI_Type_size( datatype, &typelng );
    ADCL_COMPUTE_SEGCOUNT( segsize, typelng, segcount );

    /* An in-order binary tree must use root (size-1) to preserve the order of
       operations.  Thus, if root is not rank (size - 1), then we must handle
       1. MPI_IN_PLACE option on real root, and
       2. we must allocate temporary recvbuf on rank (size - 1).
       Note that generic function must be careful not to switch order of
       operations for non-commutative ops.
    */
    io_root = size - 1;
    use_this_sendbuf = sendbuf;
    use_this_recvbuf = recvbuf;
    if (io_root != root) {
        MPI_Aint tlb, text, lb, ext;
        char *tmpbuf = NULL;

        MPI_Type_get_extent(datatype, &lb, &ext);
        MPI_Type_get_true_extent(datatype, &tlb, &text);

        if ((root == rank) && (MPI_IN_PLACE == sendbuf)) {
            tmpbuf = (char *) malloc(text + (count - 1) * ext);
            ADCL_ddt_copy_content_same_ddt(datatype, count,
                                                (char*)tmpbuf,
                                                (char*)recvbuf);
            use_this_sendbuf = tmpbuf;
        } else if (io_root == rank) {
            tmpbuf = (char *) malloc(text + (count - 1) * ext);
            use_this_recvbuf = tmpbuf;
        }
    }
/*here the send and recv buffers have been changed and are not same as the ones in request object. what to do? - sarat
*/
    /* Use generic reduce with in-order binary tree topology and io_root */
    ret = ADCL_reduce_generic( req,use_this_sendbuf, use_this_recvbuf, io_root, 
                               tree, segcount, max_outstanding_reqs );
/* Clean up */
    if (io_root != root) {
        if (root == rank) {
            /* Receive result from rank io_root to recvbuf */
            ret = MPI_Recv(recvbuf, count, datatype, io_root,
                                    ADCL_TAG_REDUCE, comm,
                                    MPI_STATUS_IGNORE);
            if (MPI_IN_PLACE == sendbuf) {
                free(use_this_sendbuf);
            }

        } else if (io_root == rank) {
            /* Send result from use_this_recvbuf to root */
            ret = MPI_Send(use_this_recvbuf, count, datatype, root,
                                    ADCL_TAG_REDUCE,
                                    comm);
            free(use_this_recvbuf);
        }
    }
    return;
}

