/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2007           Cisco, Inc. All rights reserved.
 * Copyright (c) 2009           HLRS. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"
#include "math.h"

static int ADCL_local_id_counter=0;
ADCL_array_t *ADCL_request_farray;

extern int ADCL_emethod_numtests;
extern ADCL_fnctset_t *ADCL_neighborhood_fnctset;

#define CHECK_COMM_STATE(state1,state2) if (state1!=state2) return ADCL_SUCCESS


int ADCL_request_create_generic_rooted ( ADCL_vector_t **svecs,
                                         ADCL_vector_t **rvecs,
                                         ADCL_topology_t *topo,
                                         ADCL_fnctset_t *fnctset,
                                         ADCL_request_t **req, int root, int order )
{
    int i, ret = ADCL_SUCCESS, dims;
    ADCL_request_t *newreq;
    /* ADCL criteria structure for neighborhood com fnctset */
    ADCL_neighborhood_criteria_t *ADCL_neighborhood_criteria = NULL;
    int ident_vecs=1;
#ifdef MPI_WIN
    MPI_Aint lb, extent;
    //int tsize;
    MPI_Aint tcount=1;
#endif
    //int vectype;
    /*
    ** Since the dimension of all vector objects has to be the same,
    ** we can pick any of them  for the shortcut purposes, e.g. for the
    ** emethod_init function, since these functions are only interested in
    ** the  number of dimensions and the extent of each dimension
    */
    ADCL_vector_t *vec=ADCL_VECTOR_NULL;

    /* Alloacte the general ADCL_request structure */
    newreq = (ADCL_request_t *) calloc ( 1, sizeof(ADCL_request_t));
    if ( NULL == newreq ) {
        return ADCL_NO_MEMORY;
    }

    /* Fill in the according elements, start with the simple ones */
    newreq->r_id = ADCL_local_id_counter++;
    ADCL_array_get_next_free_pos ( ADCL_request_farray, &(newreq->r_findex) );
    ADCL_array_set_element ( ADCL_request_farray,
                 newreq->r_findex,
                 newreq->r_id,
                 newreq );

    newreq->r_comm_state = ADCL_COMM_AVAIL;
    if ( NULL != svecs && NULL != rvecs ) {
        /* copy data arrays */
        if ( ADCL_VECTOR_HALO == svecs[0]->v_map->m_vectype ) {
	   /* assume, someone uses old settings with ADCL_VECTOR_HALO */
           vec =  svecs[0];

           newreq->r_svecs = (ADCL_vector_t **) malloc ( 2* topo->t_nneigh * sizeof (ADCL_vector_t *));
           newreq->r_rvecs = (ADCL_vector_t **) malloc ( 2* topo->t_nneigh * sizeof (ADCL_vector_t *));
           if ( NULL == newreq->r_rvecs || NULL == newreq->r_svecs ) {
               ret = ADCL_NO_MEMORY;
               goto exit;
           }
           for ( i=0; i < 2*topo->t_nneigh; i++ ) {
               newreq->r_svecs[i] = svecs[i];
               ADCL_vector_add_reference( svecs[i] );
	       newreq->r_rvecs[i] = rvecs[i];
               ADCL_vector_add_reference( rvecs[i] );
               if ( (svecs[0]->v_id != svecs[i]->v_id) ||
                    (svecs[0]->v_id != rvecs[i]->v_id)) {
                   ident_vecs = 0;
               }
           }
           /*
           ** Generate the derived datatypes describing which parts of this
           ** vector are going to be sent/received from which process
           */
           /* dimension of sdats and rdats, resp. 
              neighborhood communication: 2 * nneigh = 2 * ntopodim 
              ext. neighborhood comm.   : 2 * nneigh (non-blocking) + 2 * ntopodim (blocking) */
           newreq->r_ndats = ( topo->t_nneigh == topo->t_ndims ) ? 2 * topo->t_nneigh : 2 * topo->t_nneigh + 2 * topo->t_ndims;

           if ( vec->v_ndims == 1 || (vec->v_ndims == 2 && vec->v_nc > 0 )) {
               ret = ADCL_indexed_1D_init ( vec->v_dims[0], vec->v_map->m_hwidth, vec->v_nc, order,
                                            vec->v_dat, &(newreq->r_sdats), &(newreq->r_rdats) );
           }
           else if ( (vec->v_ndims == 2 && vec->v_nc == 0) ||
                     (vec->v_ndims == 3 && vec->v_nc > 0) ) {
               //ret = ADCL_indexed_2D_init ( vec->v_dims, vec->v_map->m_hwidth, vec->v_nc, order, topo->t_nneigh,
               //                            vec->v_dat, &(newreq->r_sdats), &(newreq->r_rdats) );
               ret = ADCL_subarray_init   ( topo->t_ndims, vec->v_ndims, vec->v_dims, vec->v_map->m_hwidth,
                       vec->v_nc, order, topo->t_nneigh, vec->v_dat, newreq->r_ndats, &(newreq->r_sdats), &(newreq->r_rdats) );
           }
           else if ( (vec->v_ndims == 3 && vec->v_nc == 0) ||
                   (vec->v_ndims == 4 && vec->v_nc > 0 )){
               //ret = ADCL_indexed_3D_init ( vec->v_dims, vec->v_map->m_hwidth, vec->v_nc, order, topo->t_nneigh,
               //        vec->v_dat, &(newreq->r_sdats), &(newreq->r_rdats) );
               ret = ADCL_subarray_init   ( topo->t_ndims, vec->v_ndims, vec->v_dims, vec->v_map->m_hwidth,
                       vec->v_nc, order, topo->t_nneigh, vec->v_dat, newreq->r_ndats, &(newreq->r_sdats), &(newreq->r_rdats) );
           }
           else if ( ADCL_TOPOLOGY_NULL != topo ) {
               if ( topo->t_nneigh > topo->t_ndims ) { printf("not implemented\n"); exit(-1); }
               ret = ADCL_subarray_init   ( topo->t_ndims, vec->v_ndims, vec->v_dims, vec->v_map->m_hwidth,
                       vec->v_nc, order, topo->t_nneigh, vec->v_dat, newreq->r_ndats, &(newreq->r_sdats), &(newreq->r_rdats) );
           }

           /* Initialize temporary buffer(s) for Pack/Unpack operations */
           ret = ADCL_packunpack_init ( newreq->r_ndats, topo->t_nneigh, topo->t_neighbors, topo->t_comm, &(newreq->r_sbuf),
                                        newreq->r_sdats, &(newreq->r_spsize), &(newreq->r_rbuf), newreq->r_rdats,
                                        &(newreq->r_rpsize) );
           if ( ADCL_SUCCESS != ret ) {
                goto exit;
           }

        }
        else { 
           // ADCL_VECTOR_ALL  / ADCL_VECTOR_LIST 
           ident_vecs = 0; 
           
           newreq->r_svecs = (ADCL_vector_t **) malloc ( sizeof (ADCL_vector_t *));
           newreq->r_rvecs = (ADCL_vector_t **) malloc ( sizeof (ADCL_vector_t *));
           if ( NULL == newreq->r_rvecs || NULL == newreq->r_svecs ) {
               ret = ADCL_NO_MEMORY;
               goto exit;
           }
           
	   newreq->r_svecs[0] = svecs[0];
	   ADCL_vector_add_reference( svecs[0] );
           newreq->r_rvecs[0] = rvecs[0];
	   ADCL_vector_add_reference( rvecs[0] );

	   // !!! this is a temporary solution and not OK!
           vec =  svecs[0];
           // ! only temorarily: disables state machine
           //vec = ADCL_VECTOR_NULL;

           dims = 1;
           for ( i=0; i<svecs[0]->v_ndims; i++ ) { 
	      dims = dims * svecs[0]->v_dims[i];
	   }
           switch (svecs[0]->v_map->m_vectype) {
               case ADCL_VECTOR_ALL:
               case ADCL_VECTOR_LIST:
               case ADCL_VECTOR_ALLREDUCE:
               case ADCL_VECTOR_REDUCE:
                   ret = ADCL_basic_init (svecs[0]->v_dat, dims, &(newreq->r_sdats), &(newreq->r_scnts));
                   break;
               case ADCL_VECTOR_ALLTOALL:
                   ret = ADCL_basic_init (svecs[0]->v_dat, 1, &(newreq->r_sdats), &(newreq->r_scnts));
                   break;
               case ADCL_VECTOR_INPLACE:
                   vec = rvecs[0];
                   break;
               default:
                   ret = ADCL_ERROR_INTERNAL; 
                   goto exit;
           }

           dims = 1;
           for ( i=0; i<rvecs[0]->v_ndims; i++ ) { 
               dims = dims * rvecs[0]->v_dims[i];
           }
           switch (rvecs[0]->v_map->m_vectype) {
               case ADCL_VECTOR_ALL:
               case ADCL_VECTOR_LIST:
               case ADCL_VECTOR_ALLREDUCE:
               case ADCL_VECTOR_REDUCE:
                   ret = ADCL_basic_init (rvecs[0]->v_dat, dims, &(newreq->r_rdats), &(newreq->r_rcnts));
                   break;
               case ADCL_VECTOR_ALLTOALL:
                   ret = ADCL_basic_init (rvecs[0]->v_dat, 1, &(newreq->r_rdats), &(newreq->r_rcnts));
                   break;
               default:
                   ret = ADCL_ERROR_INTERNAL; 
                   goto exit;
           }
	}

        if ( ret != ADCL_SUCCESS ) {
            goto exit;
        }

     }

    /* Initialize the emethod data structures required for evaluating
       the different communication methods.
    */
    newreq->r_function = NULL;
    newreq->r_emethod  = ADCL_emethod_init ( topo, vec, fnctset, root );
    if ( NULL == newreq->r_emethod ) {
        ret = ADCL_NO_MEMORY;
        goto exit;
    }
    /*
    ** Increase the reference count on the topology object
    */

    /* Set now elements of the structure required for the communication itself */

#ifdef MPI_WIN
    /*  Doesn't work in this concept anymore!!! */
    /*  But only if all vectors are identical   */
    if ( ident_vecs && (NULL != vec) && (ADCL_VECTOR_NULL != vec) ) {
        MPI_Type_get_extent (vec->v_dat, &lb, &extent);
        if ( vec->v_nc > 0 ) {
            tcount = vec->v_nc;
        }
        for (i=0; i <vec->v_ndims; i++){
            tcount *= vec->v_dims[i];
        }
        ret = MPI_Win_create ( vec->v_data, tcount*extent, extent, MPI_INFO_NULL,
                         topo->t_comm, &(newreq->r_win));
        ret = MPI_Comm_group ( topo->t_comm, &(newreq->r_group) );
    }
    else {
        /* For now, we will not support one sided comm in that case.
           Then we shrink the function set by eliminating them */
        if ( 0 == strcmp ( newreq->r_emethod->em_fnctset.fs_name,
                           "Neighborhood communication") ) {
            ADCL_fnctset_shrink_by_attr( &(newreq->r_emethod->em_fnctset), 2,
                                         ADCL_attr_transfer_nn_FenceGet );
            ADCL_fnctset_shrink_by_attr( &(newreq->r_emethod->em_fnctset), 2,
                                         ADCL_attr_transfer_nn_FencePut );
            ADCL_fnctset_shrink_by_attr( &(newreq->r_emethod->em_fnctset), 2,
                                         ADCL_attr_transfer_nn_PostStartGet );
            ADCL_fnctset_shrink_by_attr( &(newreq->r_emethod->em_fnctset), 2,
                                         ADCL_attr_transfer_nn_PostStartPut );
        }
        newreq->r_win        = MPI_WIN_NULL;
        newreq->r_group      = MPI_GROUP_NULL;
    }
#else
    newreq->r_win        = MPI_WIN_NULL;   /* TBD: unused for right now! */
    newreq->r_group      = MPI_GROUP_NULL; /* TBD: unused for right now! */
#endif

    /* Allocate the request arrays */
    if ( NULL != newreq->r_svecs ) {
       switch ( newreq->r_svecs[0]->v_map->m_vectype) {
       case ADCL_VECTOR_HALO:
           if ( 0 != topo->t_nneigh ) {
              newreq->r_sreqs=(MPI_Request *)malloc(2 * topo->t_nneigh*
                        sizeof(MPI_Request));
           }
           break;
       case ADCL_VECTOR_LIST:
       case ADCL_VECTOR_ALLTOALL:
            newreq->r_sreqs=(MPI_Request *)malloc(topo->t_size*
                                                  sizeof(MPI_Request));
            break;
       case ADCL_VECTOR_ALL: 
       case ADCL_VECTOR_ALLREDUCE:
       case ADCL_VECTOR_REDUCE:
       case ADCL_VECTOR_INPLACE:
           newreq->r_sreqs=(MPI_Request *)malloc(topo->t_ndims*
                        sizeof(MPI_Request));
           break;
       default:
           ret = ADCL_ERROR_INTERNAL;
           goto exit;
       }
       if ( NULL == newreq->r_sreqs ) {
            ret = ADCL_NO_MEMORY;
            goto exit;
       }
    }

    if ( NULL != newreq->r_rvecs ) {
        switch ( newreq->r_rvecs[0]->v_map->m_vectype ) {
        case ADCL_VECTOR_HALO:
            if ( 0 != topo->t_nneigh ) {
                newreq->r_rreqs=(MPI_Request *)malloc(2 * topo->t_nneigh*
                                                      sizeof(MPI_Request));
            }
            break;
        case ADCL_VECTOR_ALL:
        case ADCL_VECTOR_ALLREDUCE:
        case ADCL_VECTOR_REDUCE:
            newreq->r_rreqs=(MPI_Request *)malloc(topo->t_nneigh*
                                                  sizeof(MPI_Request));
            break;
        case ADCL_VECTOR_ALLTOALL:
        case ADCL_VECTOR_LIST:
            newreq->r_rreqs=(MPI_Request *)malloc(topo->t_size*
                                                  sizeof(MPI_Request));
            break;
        default:
            ret = ADCL_ERROR_INTERNAL;
            goto exit;
        }
        if ( NULL == newreq->r_rreqs ) { 
            ret = ADCL_NO_MEMORY;
            goto exit;
        }
    }

    /* Registering a filtering criteria st for neighborhood communication */
    if ( 0 == strcmp ( fnctset->fs_name , "Neighborhood communication") ) {
        if( NULL == newreq->r_emethod->em_hist_criteria ) {
            /* Filtering criteria */
            ADCL_neighborhood_criteria = (ADCL_neighborhood_criteria_t *)calloc(1, sizeof(ADCL_neighborhood_criteria_t));
            newreq->r_emethod->em_hist_criteria = (ADCL_Hist_criteria)calloc(1, sizeof(ADCL_hist_criteria_t));
            if ( (NULL == ADCL_neighborhood_criteria)||
                 ( NULL == newreq->r_emethod->em_hist_criteria ) ) {
                ret = ADCL_NO_MEMORY;
                goto exit;
            }
            newreq->r_emethod->em_hist_criteria->hc_filter_criteria = (void *)ADCL_neighborhood_criteria;
            newreq->r_emethod->em_hist_criteria->hc_set_criteria = (ADCL_hist_set_criteria *)ADCL_neighborhood_set_criteria;
            ADCL_neighborhood_set_criteria( newreq, ADCL_neighborhood_criteria );
            newreq->r_emethod->em_hist_criteria->hc_criteria_set = 1;
	}
    }   
exit:
    if ( ret != ADCL_SUCCESS ) {
        if ( NULL != newreq->r_svecs ) {
            free ( newreq->r_svecs );
        }
        if ( NULL != newreq->r_rvecs ) {
            free ( newreq->r_rvecs );
        }

        if ( NULL != newreq->r_sreqs ) {
            free ( newreq->r_sreqs );
        }
        if ( NULL != newreq->r_rreqs ) {
            free ( newreq->r_rreqs );
        }
        ADCL_array_remove_element ( ADCL_request_farray, newreq->r_findex );

        if ( NULL != vec ) {
           if (NULL == svecs[0]->v_map || ADCL_VECTOR_HALO == svecs[0]->v_map->m_vectype ) {
              ADCL_subarray_free ( newreq->r_ndats, &(newreq->r_sdats), &(newreq->r_rdats) ); 
              ADCL_packunpack_free ( newreq->r_ndats, &(newreq->r_rbuf),
                                   &(newreq->r_sbuf), &(newreq->r_spsize),
                                   &(newreq->r_rpsize) );
	   }
	   else
	   {
              switch (svecs[0]->v_map->m_vectype) {
              case ADCL_VECTOR_ALL:
                 //ADCL_contiguous_free_generic (1, &(newreq->r_sdats), &(newreq->r_scnts));
                 ADCL_basic_free (&(newreq->r_sdats), &(newreq->r_scnts));
                 break;
	      case ADCL_VECTOR_LIST:
	      case ADCL_VECTOR_ALLREDUCE:
              case ADCL_VECTOR_REDUCE:
	      case ADCL_VECTOR_INPLACE:
              case ADCL_VECTOR_ALLTOALL:
	         ADCL_basic_free (&(newreq->r_sdats), &(newreq->r_scnts));
		 break;
              }

              switch (rvecs[0]->v_map->m_vectype) {
              case ADCL_VECTOR_ALL:
                 ADCL_contiguous_free_generic (1, &(newreq->r_rdats), &(newreq->r_rcnts));
                 break;
	      case ADCL_VECTOR_LIST:
	      case ADCL_VECTOR_ALLREDUCE:
       case ADCL_VECTOR_REDUCE:
              case ADCL_VECTOR_ALLTOALL:
                 ADCL_basic_free (&(newreq->r_rdats), &(newreq->r_rcnts));
		 break;
	      }
	   }

        }

        if ( NULL != (void*) newreq->r_win || MPI_WIN_NULL != newreq->r_win ) {
            MPI_Win_free ( &(newreq->r_win) );
        }

        if ( NULL != newreq->r_emethod->em_hist_criteria ) {
            free(newreq->r_emethod->em_hist_criteria);
        }

        if ( NULL != ADCL_neighborhood_criteria ){
            free(ADCL_neighborhood_criteria);
	}

        if ( NULL != newreq ) {
            free ( newreq );
            newreq = ADCL_REQUEST_NULL;
        }
    }

    *req = newreq;
    return ret;
}


/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_request_free ( ADCL_request_t **req )
{
   ADCL_request_t *preq=*req;
   int i, ret = ADCL_SUCCESS;

   /* free r_svecs, r_sdats, r_rdats, r_sbuf, r_rbuf */
   if ( NULL != preq->r_svecs ) {
      switch (preq->r_svecs[0]->v_map->m_vectype) {
         case ADCL_VECTOR_HALO: 
            /* r_svecs[i] and r_rvecs[i] have also to be "freed", which normally ends up in 
             * decrementing the reference counter */	    
	    for ( i=0; i < 2* preq->r_emethod->em_topo->t_nneigh; i++ ) {
	       ADCL_vector_free( &(preq->r_svecs[i]) ); 
            }
            if ( NULL != preq->r_sdats  && NULL != preq->r_rdats ) {
               ADCL_subarray_free ( preq->r_ndats, &(preq->r_sdats), &(preq->r_rdats) );
            }
            if ( NULL != preq->r_sbuf  && NULL != preq->r_rbuf ) {
                 ADCL_packunpack_free ( preq->r_ndats,
                          &(preq->r_rbuf),
                          &(preq->r_sbuf),
                          &(preq->r_spsize),
                          &(preq->r_rpsize) );
            }
            break;  
         case ADCL_VECTOR_ALL:
         case ADCL_VECTOR_LIST:
         case ADCL_VECTOR_ALLTOALL:
         case ADCL_VECTOR_ALLREDUCE:
         case ADCL_VECTOR_REDUCE:
            ADCL_vector_free( &(preq->r_svecs[0]) ); 
            if ( NULL != preq->r_sdats  && NULL != preq->r_rdats ) {
               ADCL_basic_free (&(preq->r_sdats), &(preq->r_scnts));
            }
	    break;
	 case ADCL_VECTOR_INPLACE:
	    break;
         default:
             ret = ADCL_ERROR_INTERNAL;
      }
      free ( preq->r_svecs );
   }

   /* free r_rvecs */
   if ( NULL != preq->r_rvecs ) {
      switch (preq->r_rvecs[0]->v_map->m_vectype) {
         case ADCL_VECTOR_HALO: 
	    for ( i=0; i < 2* preq->r_emethod->em_topo->t_nneigh; i++ ) {
	       ADCL_vector_free( &(preq->r_rvecs[i]) ); 
            }
	    break;
         case ADCL_VECTOR_ALL:
            ADCL_vector_free( &(preq->r_rvecs[0]) ); 
            ADCL_contiguous_free_generic (1, &(preq->r_rdats), &(preq->r_rcnts));
            break;
         case ADCL_VECTOR_LIST:
         case ADCL_VECTOR_ALLTOALL:
         case ADCL_VECTOR_ALLREDUCE:
         case ADCL_VECTOR_REDUCE:
            ADCL_vector_free( &(preq->r_rvecs[0]) ); 
            ADCL_basic_free (&(preq->r_rdats), &(preq->r_rcnts));
            break;
         default:
             ret = ADCL_ERROR_INTERNAL;
      }
      free ( preq->r_rvecs );
   }

   if ( NULL != preq->r_sreqs ) {
       free ( preq->r_sreqs );
   }
   if ( NULL != preq->r_rreqs ) {
       free ( preq->r_rreqs );
   }
   ADCL_array_remove_element ( ADCL_request_farray, preq->r_findex);

   ADCL_emethod_free ( preq->r_emethod );

   if ( MPI_WIN_NULL != preq->r_win ) {
       MPI_Win_free ( &(preq->r_win) );
   }
   if ( MPI_GROUP_NULL != preq->r_group ) {
       MPI_Group_free ( &(preq->r_group ));
   }

   if ( NULL != preq ) {
       free ( preq );
   }

   *req = ADCL_REQUEST_NULL;
   return ADCL_SUCCESS;
}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_request_init ( ADCL_request_t *req, int *db )
{
    TIME_TYPE t1, t2;
    MPI_Comm comm;
    TIME_TYPE start_time, end_time;
    static TIME_TYPE elapsed_time = 0;

    if ( ADCL_TOPOLOGY_NULL != req->r_emethod->em_topo ){
        comm = req->r_emethod->em_topo->t_comm;
    }

    CHECK_COMM_STATE ( req->r_comm_state, ADCL_COMM_AVAIL );

#ifdef ADCL_NEW_OUTPUT_FORMAT
    req->r_function = ADCL_emethod_get_function_by_state ( req->r_emethod,
          &(req->r_erlast), &(req->r_erflag), "emethod", req->r_emethod->em_id, ADCL_COMM_AVAIL);
#else
    req->r_function = ADCL_emethod_get_function_by_state ( req->r_emethod,  
       &(req->r_erlast), &(req->r_erflag), "req", req->r_id, ADCL_COMM_AVAIL);  
#endif

#ifdef PERF_DETAILS
    start_time = TIME;
#endif /* PERF_DETAILS */
#ifdef ADCL_USE_BARRIER
    if ( req->r_emethod->em_state == ADCL_STATE_TESTING ) {
        MPI_Barrier ( comm );
    }
#endif /* ADCL_USE_BARRIER */
#ifdef PERF_DETAILS
    end_time = TIME;
    elapsed_time += (end_time - start_time);
#endif /* PERF_DETAILS */
    /* Get starting point in time after barrier */
    t1 = TIME;
    /* Execute the function */
    req->r_function->f_iptr ( req );

#ifdef PERF_DETAILS
    start_time = TIME;
#endif /* PERF_DETAILS */
#ifdef ADCL_USE_BARRIER
    if ( req->r_emethod->em_state == ADCL_STATE_TESTING ) {
        MPI_Barrier ( comm );
    }
#endif /* ADCL_USE_BARRIER */
#ifdef PERF_DETAILS
    end_time = TIME;
    elapsed_time += (end_time - start_time);
    ADCL_printf("Total elapsed time in Barriers = %f\n",elapsed_time);    
#endif /* PERF_DETAILS */
    /* Get ending point in time after barrier */
    t2 = TIME;
#ifndef ADCL_USERLEVEL_TIMINGS
    /* Update the request with the timings */
    ADCL_request_update ( req, t1, t2 );
#endif /* ADCL_USERLEVEL_TIMINGS */

    *db = req->r_function->f_db;
    if ( req->r_function->f_db ) {
        req->r_comm_state = ADCL_COMM_ACTIVE;
    }
    return ADCL_SUCCESS;
}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_request_wait ( ADCL_request_t *req )
{

    CHECK_COMM_STATE ( req->r_comm_state, ADCL_COMM_ACTIVE );

    if ( NULL != req->r_function->f_wptr ) {
        req->r_function->f_wptr ( req );
    }

    req->r_comm_state = ADCL_COMM_AVAIL;
    return ADCL_SUCCESS;
}


/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_request_update ( ADCL_request_t *req,
                          TIME_TYPE t1, TIME_TYPE t2 )
{
    if ( (t1 == -1 ) && ( t2 == -1 ) ) {
        return ADCL_SUCCESS;
    }
/*
    printf("method %d (%s) %8.4f \n",
        req->r_function->f_id,
        req->r_function->f_name, 
        t2>t1 ? (t2-t1):(1000000-t1+t2));
*/

    if ( ADCL_TOPOLOGY_NULL != req->r_emethod->em_topo ) {
#ifdef ADCL_NEW_OUTPUT_FORMAT
        ADCL_printf("%d: emethod %d method %d (%s) %8.4f \n",
            req->r_emethod->em_topo->t_rank, req->r_emethod->em_id, req->r_function->f_id,
            req->r_function->f_name, t2>t1 ? (t2-t1):(1000000-t1+t2));
#else
        ADCL_printf("%d: request %d method %d (%s) %8.4f \n",
            req->r_emethod->em_topo->t_rank, req->r_id, req->r_function->f_id,
            req->r_function->f_name, t2>t1 ? (t2-t1):(1000000-t1+t2));
        // DISPLAY((ADCL_DISPLAY_MESSAGE,req->r_emethod->em_id,"%d: request %d method %d (%s) %8.4f \n",
        //          req->r_emethod->em_topo->t_rank, req->r_id, req->r_function->f_id,
        //	    req->r_function->f_name, t2>t1 ? (t2-t1):(1000000-t1+t2)));
#endif
    }
    else {
#ifdef ADCL_NEW_OUTPUT_FORMAT
        ADCL_printf("emethod %d method %d (%s) %8.4f \n", req->r_emethod->em_id, 
            req->r_function->f_id, req->r_function->f_name, t2>t1 ? (t2-t1):(1000000-t1+t2));
#else
        ADCL_printf("request %d method %d (%s) %8.4f \n", req->r_id, 
            req->r_function->f_id, req->r_function->f_name, t2>t1 ? (t2-t1):(1000000-t1+t2));
        // DISPLAY((ADCL_DISPLAY_MESSAGE,req->r_emethod->em_id,"request %d method %d (%s) %8.4f \n", req->r_id,
        //            req->r_function->f_id, req->r_function->f_name, t2>t1 ? (t2-t1):(1000000-t1+t2)));
#endif
    }
    switch ( req->r_emethod->em_state ) {
    case ADCL_STATE_TESTING:
        ADCL_emethods_update (req->r_emethod, req->r_erlast,
                  req->r_erflag, t1, t2);
        break;
    case ADCL_STATE_REGULAR:
        req->r_emethod->em_state = ADCL_emethod_monitor (req->r_emethod,
                                 req->r_emethod->em_last,
                                 t1, t2 );
        break;
    case ADCL_STATE_DECISION:
    default:
        ADCL_printf("%s: Unknown object status for req %d, status %d \n",
            __FILE__, req->r_id, req->r_emethod->em_state );
        break;
    }

    return ADCL_SUCCESS;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_request_reg_hist_criteria ( ADCL_request_t *req, ADCL_hist_criteria_t *hist_criteria )
{

    /* Check if NULL function or structure */
    if ((NULL != hist_criteria->hc_set_criteria ) &&
        (NULL != hist_criteria->hc_filter_criteria ) &&
	(0 == hist_criteria->hc_criteria_set )) {
        /* Execute the function to set the filter criteria st */
        hist_criteria->hc_set_criteria(req, hist_criteria->hc_filter_criteria);
        hist_criteria->hc_criteria_set = 1;
    }
    /* Attach the whole structure to the emethod */
    req->r_emethod->em_hist_criteria = hist_criteria;

    return ADCL_SUCCESS;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/* static ADCL_function_t*  ADCL_request_get_function ( ADCL_request_t *req,
                                                     int mode )
   moved to ADCL_emethod.c as ADCL_emethod_get_function_by_state  (with
   different paramters!)    */

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_request_get_comm ( ADCL_request_t *req, MPI_Comm *comm, int *rank, int *size )
{
    ADCL_topology_t *topo=req->r_emethod->em_topo;

    *comm = topo->t_comm;
    *rank = topo->t_rank;
    *size = topo->t_size;

    return ADCL_SUCCESS;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_request_get_curr_function ( ADCL_request_t *req, char **function_name,
                                     char ***attrs_names, int *attrs_num,
                                     char ***attrs_values_names, int **attrs_values_num )
{
    int i, k, ret = ADCL_SUCCESS;
    ADCL_function_t *curr_function = req->r_function;

    /* Name of the function */
    if ( NULL != function_name ) {
        *function_name = strdup ( curr_function->f_name );
    }
    /* Memory allocation for attributes names and number */
    if ( NULL != attrs_names ) {
        (*attrs_names) = (char**)malloc(curr_function->f_attrset->as_maxnum*sizeof(char*));
        if ( NULL == (*attrs_names) ) {
            return ADCL_NO_MEMORY;
        }

    }
    if ( NULL != attrs_num ) {
        *attrs_num = curr_function->f_attrset->as_maxnum;
    }
    /* Memory allocation for values names and numbers */
    if ( NULL != attrs_values_names ) {
        (*attrs_values_names) = (char**)malloc(curr_function->f_attrset->as_maxnum*sizeof(char*));
        if ( NULL == (*attrs_values_names) ) {
            ret = ADCL_NO_MEMORY;
            goto exit;
        }
    }
    if ( NULL != attrs_values_num ) {
        *attrs_values_num = (int *)malloc(curr_function->f_attrset->as_maxnum*sizeof(int));
        if ( NULL == (*attrs_values_num) ) {
            ret = ADCL_NO_MEMORY;
            goto exit;
        }
    }
    for ( i=0; i<curr_function->f_attrset->as_maxnum; i++ ) {
        /* Name of the i th attribute */
        if ( NULL != attrs_names ) {
            (*attrs_names)[i] =
                strdup ( curr_function->f_attrset->as_attrs[i]->a_attr_name );
        }
        if ( NULL !=  attrs_values_num ) {
            (*attrs_values_num)[i] =  curr_function->f_attrvals[i];
        }
        if ( NULL != attrs_values_names ) {
            for ( k=0; k<curr_function->f_attrset->as_attrs_numval[i]; k++ ) {
                if ( curr_function->f_attrvals[i] ==
                     curr_function->f_attrset->as_attrs[i]->a_values[k] ) {
                    /* Name of the function attribute value in the i th attribute */
                    (*attrs_values_names)[i] =
                        strdup ( curr_function->f_attrset->as_attrs[i]->a_values_names[k] );
                    break;
                }
            }
        }
    }
 exit:
    if ( ADCL_SUCCESS != ret ) {
        if ( NULL != (*attrs_names) ) {
            free(*attrs_names);
        }
        if ( NULL != (*attrs_values_names) ) {
            free(*attrs_values_names);
        }
    }

    return ret;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_request_get_winner_stat ( ADCL_request_t *req, double *filtered_avg,
                                   double *unfiltered_avg, double *outliers_num )
{
    ADCL_fnctset_t *fnctset = req->r_emethod->em_orgfnctset;
    ADCL_function_t *func = req->r_emethod->em_wfunction;
    int i;

    if (NULL == func ) {
        return ADCL_INVALID_ARG;
    }
    for ( i=0; i<fnctset->fs_maxnum; i++ ) {
        if ( func == fnctset->fs_fptrs[i] ) {
            if ( 0 != req->r_emethod->em_stats[i]->s_gpts[0] ) {
                *unfiltered_avg = req->r_emethod->em_stats[i]->s_gpts[0];
                *filtered_avg = req->r_emethod->em_stats[i]->s_gpts[1];
                *outliers_num = req->r_emethod->em_stats[i]->s_gpts[2] *
                                req->r_emethod->em_stats[i]->s_rescount;
                return ADCL_SUCCESS;
             }
        }
    }
    return ADCL_INVALID_ARG;
}

int ADCL_request_get_functions_with_average ( ADCL_request_t *req, 
                                              double filtered_average,
                                              int *number_functions,
                                              char ***function_name,
                                              char ***attrs_names, 
                                              int *attrs_num,
                                              char ****attrs_values_names, 
                                              int ***attrs_values_num )
{
    int i, j, k, n, ret = ADCL_SUCCESS;
    ADCL_fnctset_t *fnctset = req->r_emethod->em_orgfnctset;
    
    for ( i=0; i<fnctset->fs_maxnum; i++ ) {
        if ( (req->r_emethod->em_stats[i]->s_gpts[1]>= filtered_average) &&
             (req->r_emethod->em_stats[i]->s_gpts[1]< filtered_average+0.01) ) {
            (*number_functions) ++;
        }
    }
    if (0 == *number_functions) {
        return ret;
    }
    if ( NULL != function_name ) {
        (*function_name) = (char **)malloc(sizeof(char *) * (*number_functions));
    }
    if ( NULL != attrs_values_names) {
        (*attrs_values_names) = (char ***)malloc(sizeof(char **) * (*number_functions));
    }
    if ( NULL != attrs_values_num) {
        (*attrs_values_num) = (int **)malloc(sizeof(int *) * (*number_functions));
    }
    (*attrs_num) = fnctset->fs_fptrs[0]->f_attrset->as_maxnum;
    if (NULL != attrs_names) {
        (*attrs_names) = (char **)malloc(sizeof(char *) * (*attrs_num));
    }

    for (j=0 ; j<(*attrs_num) ; j++) {
        if (NULL != attrs_names) {
            (*attrs_names)[j] = strdup (fnctset->fs_fptrs[0]->f_attrset->
                                        as_attrs[j]->a_attr_name);
        }
    }
    i = 0;
    for ( n=0; n<fnctset->fs_maxnum; n++ ) {
        if ( (req->r_emethod->em_stats[n]->s_gpts[1]>= filtered_average) &&
             (req->r_emethod->em_stats[n]->s_gpts[1]< filtered_average+0.01) ) {
            if (NULL != function_name) {
                (*function_name)[i] = strdup(fnctset->fs_fptrs[n]->f_name);
            }
            if (NULL != attrs_values_names) {
                (*attrs_values_names)[i] = (char **)malloc(sizeof(char *) * 
                                                           fnctset->fs_fptrs[n]->f_attrset->as_maxnum);
            }
            if (NULL != attrs_values_num) {
                (*attrs_values_num)[i] = (int *)malloc(sizeof(int) * 
                                                       fnctset->fs_fptrs[n]->f_attrset->as_maxnum);
            }
            for (j=0 ; j<fnctset->fs_fptrs[n]->f_attrset->as_maxnum ; j++) {
                for (k=0 ; k<fnctset->fs_fptrs[n]->f_attrset->as_attrs_numval[j]; k++ ) {
                    if ( fnctset->fs_fptrs[n]->f_attrvals[j] ==
                         fnctset->fs_fptrs[n]->f_attrset->as_attrs[j]->a_values[k] ) {
                        /* Name of the function attribute value in the i th attribute */
                        if (NULL != attrs_values_names) {
                            (*attrs_values_names)[i][j] =
                                strdup ( fnctset->fs_fptrs[n]->f_attrset->as_attrs[j]->a_values_names[k] );
                        }
                        break;
                    }
                }
                if (NULL != attrs_values_num) {
                    (*attrs_values_num)[i][j] = fnctset->fs_fptrs[n]->f_attrvals[j];
                }
            }
            i++;
        }
    }
    return ret;
}

int ADCL_request_save_status ( ADCL_request_t *req, int *tested_num,
                               double **unfiltered_avg,
                               double **filtered_avg,
                               double **outliers, int *winner_so_far )
{
    int i;
    /* Number of executed functions */
    (*tested_num) = req->r_erlast;
    /* Boundary conditions */
    if ( req->r_emethod->em_stats[(*tested_num)]->s_count == ADCL_emethod_numtests ) {
        (*tested_num)++;
        ADCL_STAT_SET_TESTED ( req->r_emethod->em_stats[(*tested_num)-1] );
        ADCL_statistics_filter_timings ( &(req->r_emethod->em_stats[(*tested_num)-1]), 1,
                                         req->r_emethod->em_topo->t_rank );
    }
    *filtered_avg = (double *)malloc( (*tested_num)*sizeof(double) );
    *unfiltered_avg = (double *)malloc( (*tested_num)*sizeof(double) );
    *outliers = (double *)malloc( (*tested_num)*sizeof(double) );
    *winner_so_far = ADCL_emethods_get_winner ( req->r_emethod, 
                                                req->r_emethod->em_topo->t_comm,
                                                (*tested_num) );
    for (i=0; i<(*tested_num); i++ ) {
        (*unfiltered_avg)[i] =  req->r_emethod->em_stats[i]->s_gpts[0];
        (*filtered_avg)[i] =  req->r_emethod->em_stats[i]->s_gpts[1];
        (*outliers)[i] =  req->r_emethod->em_stats[i]->s_gpts[2];
    }
    return ADCL_SUCCESS;
}

int ADCL_request_restore_status ( ADCL_request_t *req, int tested_num,
                                  double *unfiltered_avg,
                                  double *filtered_avg,
                                  double *outliers )
{
    int i;

    if ( ADCL_BRUTE_FORCE == req->r_emethod->em_search_algo ) {
        for (i=0; i<tested_num; i++) {
            req->r_emethod->em_stats[i]->s_lpts[0] = unfiltered_avg[i];
            req->r_emethod->em_stats[i]->s_lpts[1] = filtered_avg[i];
            req->r_emethod->em_stats[i]->s_lpts[2] = outliers[i];
            ADCL_STAT_SET_FILTERED ( req->r_emethod->em_stats[i] );
        }

        req->r_emethod->em_last = tested_num;
    }
    return ADCL_SUCCESS;
}

int ADCL_request_get_fsname ( ADCL_request_t *req, char **fsname )
{
    ADCL_fnctset_t *fnctset = req->r_emethod->em_orgfnctset;

    *fsname = strdup( fnctset->fs_name );
    
    return ADCL_SUCCESS;
}

int ADCL_request_get_tndims ( ADCL_request_t *req, int *tndims )
{
    ADCL_topology_t *topo = req->r_emethod->em_topo;

    if( NULL != topo) {
        *tndims = topo->t_ndims;
    }
    else {
        return ADCL_INVALID_TOPOLOGY;
    }
    return ADCL_SUCCESS;
}

/* Returns 1 if still testing and 0 else */
int ADCL_request_get_state ( ADCL_request_t *req, int *state )
{
    if(ADCL_STATE_TESTING == req->r_emethod->em_state) {
	*state = 1;
    }
    else {
	*state = 0;
    }
    return ADCL_SUCCESS;
}
