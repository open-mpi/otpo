/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2007           Cisco, Inc. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"
#include "math.h"

static int ADCL_local_id_counter=0;
static ADCL_function_t*  ADCL_request_get_function ( ADCL_request_t *req, int mode);

ADCL_array_t *ADCL_request_farray;

extern ADCL_fnctset_t *ADCL_neighborhood_fnctset;

#define CHECK_COMM_STATE(state1,state2) if (state1!=state2) return ADCL_SUCCESS


int ADCL_request_create_generic ( ADCL_vector_t **svecs,
                                  ADCL_vector_t **rvecs,
                                  ADCL_topology_t *topo,
                                  ADCL_fnctset_t *fnctset,
                                  ADCL_request_t **req, int order )
{
    int i, ret = ADCL_SUCCESS;
    ADCL_request_t *newreq;


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
        vec =  svecs[0];

        newreq->r_svecs = (ADCL_vector_t **) malloc ( 2* topo->t_ndims * sizeof (ADCL_vector_t *));
        newreq->r_rvecs = (ADCL_vector_t **) malloc ( 2* topo->t_ndims * sizeof (ADCL_vector_t *));
        if ( NULL == newreq->r_rvecs || NULL == newreq->r_svecs ) {
            ret = ADCL_NO_MEMORY;
            goto exit;
        }
        for ( i=0; i < 2* topo->t_ndims; i++ ) {
            newreq->r_svecs[i] = svecs[i];
            newreq->r_rvecs[i] = rvecs[i];
        }

        /*
        ** Generate the derived datatypes describing which parts of this
        ** vector are going to be sent/received from which process
        */
        if ( vec->v_ndims == 1 || (vec->v_ndims == 2 && vec->v_nc > 0 )) {
            ret = ADCL_indexed_1D_init ( vec->v_dims[0],
                                         vec->v_hwidth,
                                         vec->v_nc,
                                         order,
                                         vec->v_dat,
                                         &(newreq->r_sdats),
                                         &(newreq->r_rdats) );
        }
        else if ( (vec->v_ndims == 2 && vec->v_nc == 0) ||
                  (vec->v_ndims == 3 && vec->v_nc > 0) ) {
            ret = ADCL_indexed_2D_init ( vec->v_dims,
                                         vec->v_hwidth,
                                         vec->v_nc,
                                         order,
                                         vec->v_dat,
                                         &(newreq->r_sdats),
                                         &(newreq->r_rdats) );
        }
        else if ( (vec->v_ndims == 3 && vec->v_nc == 0) ||
                  (vec->v_ndims == 4 && vec->v_nc > 0 )){
            ret = ADCL_indexed_3D_init ( vec->v_dims,
                                         vec->v_hwidth,
                                         vec->v_nc,
                                         order,
                                         vec->v_dat,
                                         &(newreq->r_sdats),
                                         &(newreq->r_rdats) );
        }
        else if ( ADCL_TOPOLOGY_NULL != topo ) {
            ret = ADCL_subarray_init   ( topo->t_ndims,
                                         vec->v_ndims,
                                         vec->v_dims,
                                         vec->v_hwidth,
                                         vec->v_nc,
                                         order,
                                         vec->v_dat,
                                         &(newreq->r_sdats),
                                         &(newreq->r_rdats) );
        }
        if ( ret != ADCL_SUCCESS ) {
            goto exit;
        }

        /* Initialize temporary buffer(s) for Pack/Unpack operations */
        ret = ADCL_packunpack_init ( topo->t_ndims * 2,
                                     topo->t_neighbors,
                                     topo->t_comm,
                                     &(newreq->r_sbuf),
                                     newreq->r_sdats,
                                     &(newreq->r_spsize),
                                     &(newreq->r_rbuf),
                                     newreq->r_rdats,
                                     &(newreq->r_rpsize) );
        if ( ADCL_SUCCESS != ret ) {
            goto exit;
        }
    }

    /* Initialize the emethod data structures required for evaluating
       the different communication methods.
    */
    newreq->r_function = NULL;
    newreq->r_emethod  = ADCL_emethod_init ( topo, vec, fnctset );
    if ( NULL == newreq->r_emethod ) {
        ret = ADCL_NO_MEMORY;
        goto exit;
    }
    /*
    ** Increase the reference count on the topology object
    */

    /* Set now elements of the structure required for the communication itself */

#ifdef MPI_WIN
    /*  Doesn't work in this concept anymore!!!

    MPI_Type_size (array_of_send_vecs[0]->v_dat, &tsize);
    if ( vec->v_nc > 0 ) {
    tcount = vec->v_nc;
    }
    for (i=0; i <vec->v_ndims; i++){
        tcount *= vec->v_dims[i];
    }
    MPI_Win_create ( newreq->r_vec->v_data, tcount , tsize, MPI_INFO_NULL,
             topo->t_comm, &(newreq->r_win));
    MPI_Comm_group ( topo->t_comm, &(newreq->r_group) );
    */
#else
    newreq->r_win        = MPI_WIN_NULL; /* TBD: unused for right now! */
    newreq->r_group      = MPI_GROUP_NULL; /* TBD: unused for right now! */
#endif


    /* Allocate the request arrays, 2 for each neighboring process */
    if ( 0 != topo->t_ndims ) {
        newreq->r_sreqs=(MPI_Request *)malloc(2 * topo->t_ndims*
                         sizeof(MPI_Request));
        newreq->r_rreqs=(MPI_Request *)malloc(2 * topo->t_ndims*
                         sizeof(MPI_Request));
        if ( NULL == newreq->r_rreqs || NULL == newreq->r_sreqs ) {
            ret = ADCL_NO_MEMORY;
            goto exit;
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
            ADCL_subarray_free ( 2 * topo->t_ndims, &(newreq->r_sdats),
                                 &(newreq->r_rdats) );
            ADCL_packunpack_free ( 2 * topo->t_ndims, &(newreq->r_rbuf),
                                   &(newreq->r_sbuf), &(newreq->r_spsize),
                                   &(newreq->r_rpsize) );
        }

        if ( MPI_WIN_NULL != newreq->r_win ) {
            MPI_Win_free ( &(newreq->r_win) );
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

    if ( NULL != preq->r_svecs ) {
        free ( preq->r_svecs );
    }
    if ( NULL != preq->r_rvecs ) {
        free ( preq->r_rvecs );
    }

    if ( NULL != preq->r_sreqs ) {
        free ( preq->r_sreqs );
    }
    if ( NULL != preq->r_rreqs ) {
        free ( preq->r_rreqs );
    }
    ADCL_array_remove_element ( ADCL_request_farray, preq->r_findex);

    if ( NULL != preq->r_sdats  && NULL != preq->r_rdats ) {
        ADCL_subarray_free ( 2 * preq->r_emethod->em_topo->t_ndims,
                     &(preq->r_sdats),
                     &(preq->r_rdats) );
    }
    if ( NULL != preq->r_sbuf  && NULL != preq->r_rbuf ) {
        ADCL_packunpack_free ( 2 * preq->r_emethod->em_topo->t_ndims,
                       &(preq->r_rbuf),
                       &(preq->r_sbuf),
                       &(preq->r_spsize),
                       &(preq->r_rpsize) );
    }

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

    CHECK_COMM_STATE ( req->r_comm_state, ADCL_COMM_AVAIL );

    req->r_function = ADCL_request_get_function ( req, ADCL_COMM_AVAIL );
    req->r_function->f_iptr ( req );

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

    ADCL_printf("%d: request %d method %d (%s) %8.4f \n",
        req->r_emethod->em_topo->t_rank, req->r_id, req->r_function->f_id,
        req->r_function->f_name, t2>t1 ? (t2-t1):(1000000-t1+t2));
    switch ( req->r_emethod->em_state ) {
    case ADCL_STATE_TESTING:
        ADCL_emethods_update (req->r_emethod, req->r_erlast,
                  req->r_erflag, t1, t2);
        break;
    case ADCL_STATE_REGULAR:
        req->r_emethod->em_state = ADCL_emethod_monitor (req->r_emethod,
                                 req->r_emethod->em_last,
                                 t2, t1 );
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
static ADCL_function_t*  ADCL_request_get_function ( ADCL_request_t *req,
                                                     int mode )
{
    int tmp, flag;
    ADCL_function_t *tfunc=NULL;
    MPI_Comm comm = req->r_emethod->em_topo->t_comm;
    int rank = req->r_emethod->em_topo->t_rank;
#ifdef PERF_DETAILS
    static TIME_TYPE elapsed_time = 0;
    TIME_TYPE start_time, end_time;
#endif /* PERF_DETAILS */
    switch ( req->r_emethod->em_state ) {
    case ADCL_STATE_TESTING:
#ifdef PERF_DETAILS
        start_time = MPI_Wtime();
#endif /* PERF_DETAILS */
        tmp = ADCL_emethods_get_next ( req->r_emethod, mode, &flag );
#ifdef PERF_DETAILS
        end_time = MPI_Wtime();
        elapsed_time += end_time - start_time;
#endif /* PERF_DETAILS */

        if ( ADCL_EVAL_DONE == tmp ) {
            req->r_emethod->em_state = ADCL_STATE_DECISION;
        }
        else if ( ADCL_SOL_FOUND == tmp ) {
            tfunc = req->r_emethod->em_wfunction;
            break;
	}
        else if ( (ADCL_ERROR_INTERNAL == tmp)||( 0 > tmp )) {
            return NULL;
        }
        else {
            req->r_erlast = tmp;
            req->r_erflag = flag;
            tfunc = ADCL_emethod_get_function (req->r_emethod, tmp );
            break;
        }
        /* no break; statement here on purpose! */
    case ADCL_STATE_DECISION:
#if 0
        ADCL_printf("#%d: Initiating decision procedure for req %d\n",
            rank, req->r_id);
#endif
#ifdef PERF_DETAILS
        ADCL_printf("Total elapsed time of emethod_get_function = %f\n",elapsed_time);
#endif /* PERF_DETAILS */
        tmp = ADCL_emethods_get_winner ( req->r_emethod, comm,
                                         req->r_emethod->em_fnctset.fs_maxnum);
        req->r_emethod->em_last    = tmp;
        req->r_emethod->em_wfunction = ADCL_emethod_get_function (req->r_emethod, tmp);
        ADCL_printf("#%d:  req %d winner is %d %s\n",
            rank, req->r_id, req->r_emethod->em_wfunction->f_id,
            req->r_emethod->em_wfunction->f_name);
	ADCL_data_create ( req->r_emethod );
        req->r_emethod->em_state = ADCL_STATE_REGULAR;
        /* no break; statement here on purpose! */
    case ADCL_STATE_REGULAR:
        tfunc = req->r_emethod->em_wfunction;
        break;
    default:
        ADCL_printf("#%s: Unknown object status for req %d, status %d\n",
            __FILE__, req->r_id, req->r_emethod->em_state );
        break;
    }

    return tfunc;
}


/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_request_get_comm ( ADCL_request_t *req, MPI_Comm *comm, int *rank, int *size)
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
        if ( (fabs(filtered_average) * 0.001) >= 
             fabs(filtered_average - req->r_emethod->em_stats[i]->s_gpts[1]) ) {
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
        if ( (fabs(filtered_average) * 0.001) >= 
             fabs(filtered_average - req->r_emethod->em_stats[n]->s_gpts[1]) ) {
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
    if ( req->r_emethod->em_stats[(*tested_num)]->s_count == ADCL_EMETHOD_NUMTESTS ) {
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
    if ( 0 == req->r_emethod->em_perfhypothesis ) {
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
