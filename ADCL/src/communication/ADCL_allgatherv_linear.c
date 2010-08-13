/* 
** Algorithms implementing ALLGATHERV operations. Taken from the Open MPI source code repository.
** (./ompi/mca/coll/tuned/coll_tuned_allgather.c)
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
 * Linear functions are copied from the BASIC coll module of Open MPI
 * they do not segment the message and are simple implementations
 * but for some small number of nodes and/or small data sizes they 
 * are just as fast as tuned/tree based segmenting operations 
 * and as such may be selected by the decision functions
 * These are copied into this module due to the way we select modules
 * in V1. i.e. in V2 we will handle this differently and so will not
 * have to duplicate code.
 * JPG following the examples from other coll_tuned implementations. Dec06.
 */

/* copied function (with appropriate renaming) starts here */

/*
 *	allgatherv_intra_basic
 *
 *	Function:	- allgatherv using other MPI collectives: 
 *                        gatherv + bcast (from basic module).
 *	Accepts:	- same as MPI_Allgatherv()
 *	Returns:	- MPI_SUCCESS or error code
 */
void ADCL_allgatherv_linear ( ADCL_request_t *req )
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

     /*
     * We don't have a root process defined. Arbitrarily assign root
     * to process with rank 0 (OMPI convention)
     */

    if (MPI_IN_PLACE == sbuf) {
        MPI_Type_get_extent(rdtype, &lb, &extent);
        send_type = rdtype;
        scount = rcounts[rank];
        send_buf = ((char*) rbuf) + rdispls[rank] * extent;
        /*send_buf = (char*) sbuf;
        for (i = 0; i < rank; ++i) {
            send_buf += (rcounts[i] * extent);
        }*/
    } else {
        send_buf = (char*) sbuf;
        scount = req->r_scnts[0];
        send_type = req->r_sdats[0];
    }

    err = MPI_Gatherv(send_buf, scount, send_type, 
                      rbuf, rcounts, rdispls, rdtype, 0,
		      comm);
    
    if (MPI_SUCCESS != err) {
        return;
    }
    /*
     * we now have all the data in the root's rbuf. Need to
     * broadcast the data out to the other processes
     *
     * Need to define a datatype that captures the different vectors
     * from each process. MPI_TYPE_INDEXED with params 
     *                    size,rcount,displs,rdtype,newtype
     * should do the trick.
     * Use underlying ddt functions to create, and commit the
     * new datatype on each process, then broadcast and destroy the
     * datatype.
     */

    err = MPI_Type_indexed(size,rcounts,rdispls,rdtype,&newtype);
    if (MPI_SUCCESS != err) {
        return ;
    }
    
    err = MPI_Type_commit(&newtype);
    if(MPI_SUCCESS != err) {
        return;
    }

    MPI_Bcast(rbuf, 1, newtype, 0, comm );
    MPI_Type_free (&newtype);

    return;
}
/* copied function (with appropriate renaming) ends here */

