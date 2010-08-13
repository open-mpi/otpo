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

void ADCL_REDUCE_BINARY( ADCL_request_t *req )
{
   int segsize /*= 32*1024*/,max_outstanding_reqs = 0;
   int root = req->r_emethod->em_root;
   ADCL_topology_t *topo = req->r_emethod->em_topo;
   MPI_Comm comm = topo->t_comm;
   MPI_Datatype datatype = req->r_rdats[0];
   int  count = req->r_rvecs[0]->v_dims[0];
   int size = topo->t_size;
  
   int lastsegcount; //to be initialized
   int segcount = count;
   int typelng;
   ADCL_tree_t *tree;
   tree = ADCL_build_tree(2, req, root);
   segsize = REDUCE_SEGSIZE;
    /**
     * Determine number of segments and number of elements
     * sent per operation
     */
   MPI_Type_size( datatype, &typelng );

   ADCL_COMPUTE_SEGCOUNT( segsize, typelng, segcount );
   ADCL_reduce_generic(req,NULL,NULL,root,tree,segcount,max_outstanding_reqs);
   return;
}

