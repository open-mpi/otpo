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

static int op_is_commute(MPI_Op op);

void ADCL_reduce_generic( ADCL_request_t *req,void *sendbuf,void *recvbuf,int root,ADCL_tree_t *tree,int count_by_segment,int max_outstanding_reqs )
{
/*not defined yet
root, tree, count_by_segment, max_outstanding_reqs*/
    ADCL_topology_t *topo = req->r_emethod->em_topo;
    MPI_Comm comm         = topo->t_comm;
    ADCL_vmap_t *rvmap = req->r_rvecs[0]->v_map;
    MPI_Op op = rvmap->m_op;
    void *sbuf = sendbuf;
    void *rbuf = recvbuf;    
    MPI_Datatype dtype   = req->r_rdats[0];
    int original_count = req->r_rvecs[0]->v_dims[0];
     // to be initialized
    
    int err;
    char *inbuf[2] = {NULL,NULL} , *inbuf_free[2] = {NULL,NULL};
    char *accumbuf = NULL, *accumbuf_free = NULL;
    char *local_op_buffer = NULL, *sendtmpbuf = NULL;
    MPI_Aint  rext,  lb, segment_increment;
    int typelng;
    MPI_Request reqs[2] = {MPI_REQUEST_NULL,MPI_REQUEST_NULL};//depending on where it is used it can be rreqs and sreqs
    int num_segments, line, ret, segindex, i, rank;
    int recvcount, prevcount, inbi;
    int bcount;

    if(sbuf == NULL)
    	sbuf = req->r_svecs[0]->v_data;
    if(rbuf == NULL)
    	rbuf = req->r_rvecs[0]->v_data;

    rank = topo->t_rank;

    err = MPI_Type_get_extent(dtype, &lb, &rext);
    MPI_Type_size(dtype, &typelng);
    num_segments = (original_count + count_by_segment - 1) / count_by_segment;
    segment_increment = count_by_segment * rext;

    sendtmpbuf = (char*) sbuf;
    if( sbuf == MPI_IN_PLACE ) {
        sendtmpbuf = (char *)rbuf;
    }
    if( tree->tree_nextsize > 0 ) {
        MPI_Aint true_lb, true_ext, real_segment_size;
        MPI_Type_get_true_extent ( dtype, &true_lb, &true_ext );
        bcount = true_ext + (original_count - 1) * rext;
        /* handle non existant recv buffer (i.e. its NULL) and
           protect the recv buffer on non-root nodes */
        accumbuf = (char*)rbuf;
        if( (NULL == accumbuf) || (root != rank) ) {
            /* Allocate temporary accumulator buffer. */
            accumbuf_free = (char*)malloc(true_ext +
                                          (original_count - 1) * rext)
;
            if (accumbuf_free == NULL) {
                line = __LINE__; ret = -1; goto error_hndl;
            }
            accumbuf = accumbuf_free - lb;
        }
/* If this is a non-commutative operation we must copy
           sendbuf to the accumbuf, in order to simplfy the loops */
        if (!op_is_commute(op))//no alternative available
	{
		 err = ADCL_ddt_copy_content_same_ddt_generic (dtype, bcount,
            (char*)accumbuf, (char*)sendtmpbuf, 1);  //generic or without

        }
/* Allocate two buffers for incoming segments */
        real_segment_size = true_ext + (count_by_segment - 1) * rext;
        inbuf_free[0] = (char*) malloc(real_segment_size);
        if( inbuf_free[0] == NULL ) {
            line = __LINE__; ret = -1; goto error_hndl;
        }
        inbuf[0] = inbuf_free[0] - lb;
        /* if there is chance to overlap communication -
           allocate second buffer */
        if( (num_segments > 1) || (tree->tree_nextsize > 1) )
	{
            inbuf_free[1] = (char*) malloc(real_segment_size);
            if( inbuf_free[1] == NULL ) {
                line = __LINE__; ret = -1; goto error_hndl;
            }
            inbuf[1] = inbuf_free[1] - lb;
        }
        inbi = 0;
        recvcount = 0;
        /* for each segment */
        for( segindex = 0; segindex <= num_segments; segindex++ ) {
            prevcount = recvcount;
            /* recvcount - number of elements in current segment */
            recvcount = count_by_segment;
            if( segindex == (num_segments-1) )
                recvcount = original_count - count_by_segment * segindex;
 /* for each child */
            for( i = 0; i < tree->tree_nextsize; i++ ) {
                /**
                 * We try to overlap communication:
                 * either with next segment or with the next child
                 */
                /* post irecv for current segindex on current child */
                if( segindex < num_segments ) {
                    void* local_recvbuf = inbuf[inbi];
                    if( 0 == i ) {
                        /* for the first step (1st child per segment) an
d
                         * commutative operations we might be able to ir
ecv
                         * directly into the accumulate buffer so that we can
                         * reduce(op) this with our sendbuf in one step as
                         * ompi_op_reduce only has two buffer pointers,
                         * this avoids an extra memory copy.
                         *
                         * BUT if the operation is non-commutative or
                         * we are root and are USING MPI_IN_PLACE this is wrong!
                         */
                        if( (op_is_commute(op)) &&
                            !((MPI_IN_PLACE == sbuf) && (rank == tree->tree_root)) ) {
                            local_recvbuf = accumbuf + segindex * segment_increment;
                        }
                    }

                    ret = MPI_Irecv(local_recvbuf, recvcount, dtype,
                                             tree->tree_next[i],
                                             ADCL_TAG_REDUCE, comm,
                                             &reqs[inbi]);//how to do??
                    if (ret != MPI_SUCCESS) { line = __LINE__; goto error_hndl;}
                }
                /* wait for previous req to complete, if any.
                   if there are no requests reqs[inbi ^1] will be
                   MPI_REQUEST_NULL. */
                /* wait on data from last child for previous segment */
                ret = MPI_Waitall( 1, &reqs[inbi ^ 1],
                                             MPI_STATUSES_IGNORE );
                if (ret != MPI_SUCCESS) { line = __LINE__; goto error_hndl;  }

local_op_buffer = inbuf[inbi ^ 1];
                if( i > 0 ) {
                    /* our first operation is to combine our own [sendbuf] data
                     * with the data we recvd from down stream (but only
                     * the operation is commutative and if we are not root and
                     * not using MPI_IN_PLACE)
                     */
                    if( 1 == i ) {
                        if( (op_is_commute(op)) &&
                            !((MPI_IN_PLACE == sbuf) && (rank == tree->tree_root)) ) {
                            local_op_buffer = sendtmpbuf + segindex * segment_increment;
                        }
                    }
                    /* apply operation */
                    ADCL_op_reduce(op, local_op_buffer,
                                   accumbuf + segindex * segment_increment,
                                   recvcount, dtype );
                } else if ( segindex > 0 ) {
                    void* accumulator = accumbuf + (segindex-1) * segment_increment;
                    if( tree->tree_nextsize <= 1 ) {
                        if( (op_is_commute(op)) &&
                            !((MPI_IN_PLACE == sbuf) && (rank == tree->tree_root)) ) {
                            local_op_buffer = sendtmpbuf + (segindex-1) * segment_increment;
                        }
                    }
                    ADCL_op_reduce(op, local_op_buffer, accumulator, prevcount,
                                   dtype );

                    /* all reduced on available data this step (i) complete,
                     * pass to the next process unless you are the root.
                     */
                    if (rank != tree->tree_root) {
                        /* send combined/accumulated data to parent */
                        ret = MPI_Send(accumulator, prevcount,
                                                  dtype, tree->tree_prev,
                                                  ADCL_TAG_REDUCE,
                                                  comm );
                        if (ret != MPI_SUCCESS) {
                            line = __LINE__; goto error_hndl;
                        }
                    }

/* we stop when segindex = number of segments
                       (i.e. we do num_segment+1 steps for pipelining */
                    if (segindex == num_segments) break;
                }

                /* update input buffer index */
                inbi = inbi ^ 1;
            } /* end of for each child */
        } /* end of for each segment */

        /* clean up */
        if( inbuf_free[0] != NULL) free(inbuf_free[0]);
        if( inbuf_free[1] != NULL) free(inbuf_free[1]);
        if( accumbuf_free != NULL ) free(accumbuf_free);
    }

    /* leaf nodes
       Depending on the value of max_outstanding_reqs and
       the number of segments we have two options:
       - send all segments using blocking send to the parent, or
       - avoid overflooding the parent nodes by limiting the number of
       outstanding requests to max_oustanding_reqs.
       TODO/POSSIBLE IMPROVEMENT: If there is a way to determine the eager size
       for the current communication, synchronization should be used only
       when the message/segment size is smaller than the eager size.
    */
    else {

        /* If the number of segments is less than a maximum number of oustanding
           requests or there is no limit on the maximum number of outstanding
           requests, we send data to the parent using blocking send */
        if ((0 == max_outstanding_reqs) ||
            (num_segments <= max_outstanding_reqs)) {

            segindex = 0;
            while ( original_count > 0) {
                if (original_count < count_by_segment) {
                    count_by_segment = original_count;
                }
                ret = MPI_Send((char*)sbuf +
                                         segindex * segment_increment,
                                         count_by_segment, dtype,
                                         tree->tree_prev,
                                         ADCL_TAG_REDUCE,
                                         comm );
                if (ret != MPI_SUCCESS) { line = __LINE__; goto error_hndl; }
                segindex++;
                original_count -= count_by_segment;
            }
        }
/* Otherwise, introduce flow control:
           - post max_outstanding_reqs non-blocking synchronous send,
           - for remaining segments
           - wait for a ssend to complete, and post the next one.
           - wait for all outstanding sends to complete.
        */
        else {

            int creq = 0;
            MPI_Request *sreq = NULL;

            sreq = (MPI_Request*) calloc( max_outstanding_reqs,
                                              sizeof(MPI_Request) );
            if (NULL == sreq) { line = __LINE__; ret = -1; goto error_hndl; }

           /* post first group of requests */
            for (segindex = 0; segindex < max_outstanding_reqs; segindex++) {
                ret = MPI_Isend((char*)sbuf +
                                          segindex * segment_increment,
                                          count_by_segment, dtype,
                                          tree->tree_prev,
                                          ADCL_TAG_REDUCE,
                                          comm,
                                          &sreq[segindex] );
                if (ret != MPI_SUCCESS) { line = __LINE__; goto error_hndl;  }
                original_count -= count_by_segment;
            }

            creq = 0;
            while ( original_count > 0 ) {
                /* wait on a posted request to complete */
                ret = MPI_Wait(&sreq[creq], MPI_STATUS_IGNORE);
                if (ret != MPI_SUCCESS) { line = __LINE__; goto error_hndl;  }
                sreq[creq] = MPI_REQUEST_NULL;

                if( original_count < count_by_segment ) {
                    count_by_segment = original_count;
                }
                ret = MPI_Isend((char*)sbuf +
                                          segindex * segment_increment,
                                          count_by_segment, dtype,
                                          tree->tree_prev,
                                          ADCL_TAG_REDUCE,
                                          comm,
                                          &sreq[creq] );
                if (ret != MPI_SUCCESS) { line = __LINE__; goto error_hndl;  }
                creq = (creq + 1) % max_outstanding_reqs;
                segindex++;
                original_count -= count_by_segment;
            }
/* Wait on the remaining request to complete */
            ret = MPI_Waitall( max_outstanding_reqs, sreq,
                                         MPI_STATUSES_IGNORE );
            if (ret != MPI_SUCCESS) { line = __LINE__; goto error_hndl;  }

            /* free requests */
            free(sreq);
        }
    }
    return;

 error_hndl:  /* error handler */
    if( inbuf_free[0] != NULL ) free(inbuf_free[0]);
    if( inbuf_free[1] != NULL ) free(inbuf_free[1]);
    if( accumbuf_free != NULL ) free(accumbuf);

    return;
}

static int op_is_commute(MPI_Op op)
{
	if(op == MPI_MAX || op == MPI_MIN || op == MPI_SUM || op == MPI_PROD)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
