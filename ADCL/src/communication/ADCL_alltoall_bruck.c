/* this routine is currently not used, since it is terribly slow for large communication volumes */

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

void ADCL_alltoall_bruck ( ADCL_request_t *req ) 
{
    ADCL_topology_t *topo = req->r_emethod->em_topo;
    MPI_Comm comm         = topo->t_comm;
    MPI_Datatype sdtype    = req->r_sdats[0]; 
    MPI_Datatype rdtype    = req->r_rdats[0]; 

    void *sbuf = req->r_svecs[0]->v_data;
    void *rbuf = req->r_rvecs[0]->v_data;

    ADCL_vmap_t *vmap = req->r_svecs[0]->v_map;
    int scount        = vmap->m_scnt;
    int rcount        = vmap->m_rcnt;

    int err = 0, line = -1;
    int rank, size;
    MPI_Aint sext, rext, lb, ddt_ext;
    int i, k;
    int sendto, recvfrom, distance, *displs = NULL, *blen = NULL;
    void *tmpbuf = NULL, *tmpbuf_free = NULL;
    int weallocated = 0;
    MPI_Datatype new_ddt;


    size = topo->t_size;
    rank = topo->t_rank;

    err = MPI_Type_get_extent(sdtype, &lb, &sext);
    if ( MPI_SUCCESS != err) { line = __LINE__; return; }

    err = MPI_Type_get_extent(rdtype, &lb, &rext);
    if ( MPI_SUCCESS != err) { line = __LINE__; return; }


    displs = (int *) malloc(size * sizeof(int));
    if (displs == NULL) { line = __LINE__; return; }
    blen = (int *) malloc(size * sizeof(int));
    if (blen == NULL) { line = __LINE__; return; }
    weallocated = 1;

    /* tmp buffer allocation for message data */
    tmpbuf = malloc ( scount*size*sext );

    /* Step 1 - local rotation - shift up by rank */
    err = ADCL_ddt_copy_content_same_ddt_generic (sdtype, (size-rank) * scount, 
            tmpbuf, (char*) sbuf + rank * scount * sext, 1);
    if (err<0) { line = __LINE__; goto err_hndl; }

    if (rank != 0) {
        ADCL_ddt_copy_content_same_ddt_generic (sdtype, rank * scount,
                (char*) tmpbuf + (size-rank) * scount* sext, sbuf, 1);
        if (err<0) { line = __LINE__; goto err_hndl; }
    }

    /* perform communication step */
    for (distance = 1; distance < size; distance<<=1) {
        sendto = (rank + distance) % size;
        recvfrom = (rank - distance + size) % size;
        k = 0;

        for (i = 1; i < size; i++) {
            if (( i & distance) == distance) {
                displs[k] = i * scount;
                blen[k] = scount;
                k++;
            }
        }

        /* Set indexes and displacements */
        err = MPI_Type_indexed(k, blen, displs, sdtype, &new_ddt);
        if (err != MPI_SUCCESS) { line = __LINE__; goto err_hndl;  }
        /* Commit the new datatype */
        err = MPI_Type_commit(&new_ddt);
        if (err != MPI_SUCCESS) { line = __LINE__; goto err_hndl;  }

        /* Sendreceive */
        err = MPI_Sendrecv ( tmpbuf, 1, new_ddt, sendto,
                ADCL_TAG_ALLTOALL,
                rbuf, 1, new_ddt, recvfrom,
                ADCL_TAG_ALLTOALL,
                comm, MPI_STATUS_IGNORE );
        if (err != MPI_SUCCESS) { line = __LINE__; goto err_hndl; }

        /* Copy back new data from recvbuf to tmpbuf */
        err = ADCL_ddt_copy_content_same_ddt_generic (new_ddt, 1, tmpbuf, rbuf, 0);
        if (err < 0) { line = __LINE__; goto err_hndl;  }

        /* free ddt */
        err = MPI_Type_free(&new_ddt);
        if (err != MPI_SUCCESS) { line = __LINE__; goto err_hndl;  }
    } /* end of for (distance = 1... */

    /* Step 3 - local rotation - */
    for (i = 0; i < size; i++) {
        err = ADCL_ddt_copy_content_same_ddt_generic (rdtype, rcount, 
                (char*) rbuf + (((rank-i+size) % size) * rcount * rext),
                (char*) tmpbuf + i * rcount * rext, 1);
        if (err < 0) { line = __LINE__; err = -1; goto err_hndl;  }
    }

    /* Step 4 - clean up */
err_hndl:
    if (tmpbuf != NULL) free(tmpbuf_free);
    if (weallocated) {
        if (displs != NULL) free(displs);
        if (blen != NULL) free(blen);
    }
    return;
}


/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * Copyright (c) 2004-2006 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2008 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2006 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2006 The Regents of the University of California.
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
 * Copyright (c) 2009      University of Houston. All rights reserved.
 * Copyright (c) 2009      High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"

int ADCL_ddt_copy_content_same_ddt_generic ( MPI_Datatype dtype, int count,
                                       void* destination_base, void* source_base, int contiguous ); 

int ADCL_ddt_copy_content_same_ddt_generic ( MPI_Datatype dtype, int count,
                                       void* destination_base, void* source_base, int contiguous )
{
    MPI_Aint lb, ext, true_lb, true_ext, old_ext; 
    int n_int, n_add, n_dt, combiner;
    int *int_arr; 
    MPI_Aint *add_arr;
    MPI_Datatype *dt_arr, old_dt; 
    MPI_Aint source, destination, src, dest;  
    int old_count, blocklength, displ; 
    int i, j, ret = 0; 

    /* empty data ? then do nothing. This should normally be trapped
     * at a higher level.
     */
    if( 0 == count ) return 1;

    /* If we have to copy a contiguous datatype then simply
     * do a memcpy.
     */
    MPI_Get_address ( source_base, &source ); 
    MPI_Get_address ( destination_base, &destination ); 
    MPI_Type_get_extent ( dtype, &lb, &ext ); 
    MPI_Type_get_true_extent ( dtype, &true_lb, &true_ext ); 

    if( contiguous ) {
        destination += true_lb;
        source      += true_lb;

        memcpy ( destination, source, ext * count);
        return 0;  /* completed */
    }

    /* decode data type dtype */
    MPI_Type_get_envelope ( dtype, &n_int, &n_add, &n_dt, &combiner ); 
    int_arr = (int*) malloc ( n_int * sizeof(int) );
    add_arr = (MPI_Aint*) malloc ( n_add * sizeof (MPI_Aint) ); 
    dt_arr  = (MPI_Datatype*) malloc ( n_dt * sizeof (MPI_Datatype) ); 
    MPI_Type_get_contents ( dtype, n_int, n_add, n_dt, int_arr, add_arr, dt_arr);

    switch (combiner) {
        case MPI_COMBINER_INDEXED: 
            for ( i=0; i<count; i++) {
                old_count  = int_arr[0]; 
                old_dt = dt_arr[0];
                MPI_Type_get_extent ( old_dt, &lb, &old_ext ); 
                for (j=1; j<=old_count; j++ ) {
                    blocklength = int_arr[j];
                    displ       = int_arr[old_count+j];
                    src  = source      + displ * old_ext;
                    dest = destination + displ * old_ext;

                    memcpy ( dest, src, blocklength*old_ext );  
                }
                source += ext; 
                destination += ext; 
            }
            break; 
        default: 
            ret = -1; 
    }

    free ( int_arr ); 
    free ( add_arr ); 
    free ( dt_arr  );  

    return ret; 

}


