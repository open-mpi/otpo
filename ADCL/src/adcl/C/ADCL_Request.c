/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2007           Cisco, Inc. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL.h"
#include "ADCL_internal.h"

int ADCL_Request_create ( ADCL_Vector vec, ADCL_Topology topo,
                          ADCL_Fnctset fnctset, ADCL_Request *req )
{
    int i, ret;
    ADCL_vector_t **svecs, **rvecs;

    if ( ( NULL == vec ) ||
         ( NULL == topo )    ||
         ( NULL == fnctset ) ||
         ( NULL == req ) ){
        return ADCL_INVALID_ARG;
    }
    if ( ADCL_VECTOR_NULL != vec ) {
        if ( 0 > vec->v_id ) {
            return ADCL_INVALID_VECTOR;
        }
    }
    if ( ADCL_TOPOLOGY_NULL != topo ) {
        if ( 0 > topo->t_id ) {
            return ADCL_INVALID_TOPOLOGY;
        }
    }
    if ( ADCL_FNCTSET_NULL != fnctset ) {
        if ( 0 > fnctset->fs_id ) {
            return ADCL_INVALID_FNCTSET;
        }
    }
    if ( ADCL_VECTOR_NULL != vec ) {
        svecs = (ADCL_vector_t **) malloc ( 4 * topo->t_ndims * sizeof(ADCL_vector_t *));
        if ( NULL == svecs ) {
            return ADCL_NO_MEMORY;
        }
        rvecs = &svecs[2*topo->t_ndims];

        for ( i=0; i<2*topo->t_ndims; i++ ) {
            svecs[i] = vec;
            rvecs[i] = vec;
        }
        ret = ADCL_request_create_generic ( svecs, rvecs, topo, fnctset,
                                            req, MPI_ORDER_C );
        free ( svecs );
    }
    else {
        ret = ADCL_request_create_generic ( NULL, NULL, topo,
                                            fnctset, req, MPI_ORDER_C);
    }

    return ret;
}


int ADCL_Request_create_generic ( ADCL_Vectset vectset,
                                  ADCL_Topology topo, ADCL_Fnctset fnctset,
                                  ADCL_Request *req )
{
    int i;

    if ( ( NULL == vectset ) ||
         ( NULL == topo )    ||
         ( NULL == fnctset ) ||
         ( NULL == req ) ){
        return ADCL_INVALID_ARG;
    }
    if ( ADCL_VECTSET_NULL == vectset ) {
        if ( 0 > vectset->vs_id ) {
            return ADCL_INVALID_VECTSET;
        }
    }
    for ( i=0; i< 2*topo->t_ndims; i++ ) {
        if ( 0 > vectset->vs_svecs[i]->v_id ||
             0 > vectset->vs_svecs[i]->v_id ) {
            return ADCL_INVALID_VECTOR;
        }
    }
    if ( ADCL_TOPOLOGY_NULL != topo ) {
        if ( 0 > topo->t_id ) {
            return ADCL_INVALID_TOPOLOGY;
        }
    }
    if ( ADCL_FNCTSET_NULL != fnctset ) {
        if ( 0 > fnctset->fs_id ) {
            return ADCL_INVALID_FNCTSET;
        }
    }
    return ADCL_request_create_generic ( vectset->vs_svecs, vectset->vs_rvecs, 
                                         topo, fnctset, req, MPI_ORDER_C );
}


int ADCL_Request_free ( ADCL_Request *req )
{
    ADCL_request_t *preq = *req;

    if ( NULL == req ) {
        return ADCL_INVALID_REQUEST;
    }
    if ( 0 > preq->r_id ) {
        return ADCL_INVALID_REQUEST;
    }
    if ( ADCL_COMM_AVAIL != preq->r_comm_state ) {
        return ADCL_INVALID_REQUEST;
    }

    return ADCL_request_free ( req );
}

int ADCL_Request_start ( ADCL_Request req )
{
    int ret=ADCL_SUCCESS;
    int db;
    TIME_TYPE t1, t2;
    MPI_Comm comm =  req->r_emethod->em_topo->t_comm;
#ifdef PERF_DETAILS
    TIME_TYPE start_time, end_time;
    static TIME_TYPE elapsed_time = 0;

    start_time = MPI_Wtime();
#endif /* PERF_DETAILS */

#ifdef ADCL_USE_BARRIER
    if ( req->r_emethod->em_state == ADCL_STATE_TESTING ) {
        MPI_Barrier ( comm );
    }
#endif /* ADCL_USE_BARRIER */

#ifdef PERF_DETAILS
    end_time = MPI_Wtime();
    elapsed_time += (end_time - start_time);
#endif /* PERF_DETAILS */

    t1 = TIME;
    ret = ADCL_request_init ( req, &db );
    if ( ADCL_SUCCESS != ret ) {
        return ret;
    }

    if ( db ) {
        ret = ADCL_request_wait ( req );
    }

#ifdef PERF_DETAILS
    start_time = MPI_Wtime();
#endif /* PERF_DETAILS */

#ifdef ADCL_USE_BARRIER
    if ( req->r_emethod->em_state == ADCL_STATE_TESTING ) {
        MPI_Barrier ( comm );
    }
#endif /* ADCL_USE_BARRIER */

#ifdef PERF_DETAILS
    end_time = MPI_Wtime();
    elapsed_time += (end_time - start_time);
    ADCL_printf("Total elapsed time in Barriers = %f\n",elapsed_time);    
#endif /* PERF_DETAILS */

    t2 = TIME;
#ifndef ADCL_USERLEVEL_TIMINGS
    ADCL_request_update ( req, t1, t2 );
#endif /* ADCL_USERLEVEL_TIMINGS */
    return ret;
}

int ADCL_Request_init ( ADCL_Request req )
{
    TIME_TYPE t1, t2;
    int ret=ADCL_SUCCESS;
    int db;
    MPI_Comm comm =  req->r_emethod->em_topo->t_comm;

#ifdef ADCL_USE_BARRIER
    if ( req->r_emethod->em_state == ADCL_STATE_TESTING ) {
        MPI_Barrier ( comm );
    }
#endif /* ADCL_USE_BARRIER */
    t1 = TIME;
    ret = ADCL_request_init ( req, &db );
    t2 = TIME;
    req->r_time = t2-t1;

    return ret;
}


int ADCL_Request_wait ( ADCL_Request req )
{
    int ret=ADCL_SUCCESS;
    TIME_TYPE t1, t2;
    MPI_Comm comm =  req->r_emethod->em_topo->t_comm;

    /* Check validity of the request  */
    t1 = TIME;
    ret = ADCL_request_wait ( req );
#ifdef ADCL_USE_BARRIER
    if ( req->r_emethod->em_state == ADCL_STATE_TESTING )   {
        MPI_Barrier ( comm );
    }
#endif /* ADCL_USE_BARRIER */
    t2 = TIME;
#ifndef ADCL_USERLEVEL_TIMINGS
    ADCL_request_update ( req, t1, (t2+req->r_time));
#endif /* ADCL_USERLEVEL_TIMINGS */
    return ret;
}

int ADCL_Request_start_overlap ( ADCL_Request req, ADCL_work_fnct_ptr* midfctn,
                                 ADCL_work_fnct_ptr *endfctn,
                                 ADCL_work_fnct_ptr *totalfctn )

{
    TIME_TYPE t1, t2;
    int ret=ADCL_SUCCESS;
    int db;
    MPI_Comm comm =  req->r_emethod->em_topo->t_comm;

#ifdef ADCL_USE_BARRIER
    if ( req->r_emethod->em_state == ADCL_STATE_TESTING )   {
        MPI_Barrier ( comm );
    }
#endif /* ADCL_USE_BARRIER */
    t1 = TIME;
    ret = ADCL_request_init ( req, &db );
    if ( ADCL_SUCCESS != ret ) {
        return ret;
    }

    if ( db ) {
        if ( ADCL_NULL_FNCT_PTR != midfctn ) {
            midfctn ( req );
        }
        ret = ADCL_request_wait ( req );
        if ( ADCL_NULL_FNCT_PTR != endfctn ) {
            endfctn ( req );
        }
    }
    else {
        if ( ADCL_NULL_FNCT_PTR != endfctn ) {
            totalfctn ( req );
        }
    }
#ifdef ADCL_USE_BARRIER
    if ( req->r_emethod->em_state == ADCL_STATE_TESTING ) {
        MPI_Barrier ( comm );
    }
#endif /* ADCL_USE_BARRIER */
    t2 = TIME;
#ifndef ADCL_USERLEVEL_TIMINGS
    ADCL_request_update ( req, t1, t2 );
#endif /* ADCL_USERLEVEL_TIMINGS */
    return ret;
}

int ADCL_Request_get_comm ( ADCL_Request req, MPI_Comm *comm, int *rank, int *size )
{
    if ( req->r_id < 0 ) {
        return ADCL_INVALID_REQUEST;
    }
    if ( NULL == comm || NULL == rank || NULL == size ) {
        return ADCL_INVALID_ARG;
    }

    return ADCL_request_get_comm ( req, comm, rank, size );
}

int ADCL_Request_get_curr_function ( ADCL_Request req, char **function_name,
                                     char ***attrs_names, int *attrs_num ,
                                     char ***attrs_values_names, int **attrs_values_num )
{
    if ( req->r_id < 0 ) {
        return ADCL_INVALID_REQUEST;
    }

    return ADCL_request_get_curr_function ( req, function_name, attrs_names, attrs_num,
                                            attrs_values_names, attrs_values_num );
}

int ADCL_Request_update ( ADCL_Request req, TIME_TYPE time )
{
    if ( NULL == req ) {
        return ADCL_INVALID_REQUEST;
    }
    if ( 0 > req->r_id ) {
        return ADCL_INVALID_REQUEST;
    }

    return ADCL_request_update ( req, 0, time );
}

int ADCL_Request_get_winner_stat ( ADCL_Request req, double *filtered_avg,
                                   double *unfiltered_avg, double *outliers_num )
{
    if ( req->r_id < 0 ) {
        return ADCL_INVALID_REQUEST;
    }
    return ADCL_request_get_winner_stat ( req, filtered_avg, unfiltered_avg,
                                          outliers_num );
}

int ADCL_Request_get_functions_with_average ( ADCL_Request req, 
                                              double filtered_avg, 
                                              int *num_functions,
                                              char ***function_name, 
                                              char ***attrs_names, 
                                              int *attrs_num , 
                                              char ****attrs_values_names, 
                                              int ***attrs_values_num )
{
    if ( req->r_id < 0 ) {
        return ADCL_INVALID_REQUEST;
    }

    return ADCL_request_get_functions_with_average ( req, filtered_avg, num_functions,
                                                     function_name, attrs_names, attrs_num,
                                                     attrs_values_names, attrs_values_num );
}

int ADCL_Request_save_status ( ADCL_request_t *req, int *tested_num,
                               double **unfiltered_avg,
                               double **filtered_avg,
                               double **outliers, int *winner_so_far )
{
    if ( req->r_id < 0 ) {
        return ADCL_INVALID_REQUEST;
    }
    if ( (NULL == tested_num) ||
         (NULL == unfiltered_avg) ||
         (NULL == filtered_avg) ||
         (NULL == outliers) ) {
        return ADCL_INVALID_ARG;
    }
    return ADCL_request_save_status ( req, tested_num, unfiltered_avg,
                                      filtered_avg, outliers, winner_so_far );
}

int ADCL_Request_restore_status ( ADCL_Request req, int tested_num,
                                  double *unfiltered_avg,
                                  double *filtered_avg,
                                  double *outliers )
{
    if ( req->r_id < 0 ) {
        return ADCL_INVALID_REQUEST;
    }
    if ( (NULL == unfiltered_avg) ||
         (NULL == filtered_avg) ||
         (NULL == outliers) ) {
        return ADCL_INVALID_ARG;
    }
    return ADCL_request_restore_status ( req, tested_num, unfiltered_avg,
                                         filtered_avg, outliers );
}
