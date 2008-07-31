/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"

static int ADCL_local_id_counter = 0;
ADCL_array_t *ADCL_emethod_array = NULL;

int ADCL_emethod_selection = -1;
int ADCL_merge_requests = 1;
int ADCL_emethod_numtests = ADCL_EMETHOD_NUMTESTS;
int ADCL_emethod_use_perfhypothesis = 0; /* false */

#define ADCL_ATTR_TOTAL_NUM 3
extern ADCL_attribute_t *ADCL_neighborhood_attrs[ADCL_ATTR_TOTAL_NUM];

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
ADCL_emethod_t *ADCL_emethod_init (ADCL_topology_t *t, ADCL_vector_t *v,
                   ADCL_fnctset_t *f )

{
    ADCL_emethod_t *e=NULL;
    ADCL_hypothesis_t *hypo=NULL;
    int i, ret=ADCL_SUCCESS;

    if ( ADCL_merge_requests && v != ADCL_VECTOR_NULL ) {
        int j, last, found=-1;
        int result;
        ADCL_topology_t *topo;
        ADCL_vector_t *vec;

        /* Check first, whether we have an entry in the ADCL_emethods_array,
           which fulfills already our requirements;
        */
        last = ADCL_array_get_last ( ADCL_emethod_array );
        for ( i=0; i<= last; i++ ) {
            e = ( ADCL_emethod_t * ) ADCL_array_get_ptr_by_pos (
            ADCL_emethod_array, i );
            topo = e->em_topo;
            vec  = e->em_vec;
            if ( ADCL_VECTOR_NULL == vec  ) {
                continue;
            }

            MPI_Comm_compare ( topo->t_comm, t->t_comm, &result );
            if ( ( result != MPI_IDENT) && (result != MPI_CONGRUENT) ) {
                continue;
            }

            found = i;
            if ( ( e->em_orgfnctset == f )          &&
                 ( topo->t_ndims   == t->t_ndims  ) &&
                 ( vec->v_ndims    == v->v_ndims  ) &&
                 ( vec->v_nc       == v->v_nc     ) &&
                 ( vec->v_hwidth   == v->v_hwidth ) ) {

                for ( j=0; j< (2*topo->t_ndims); j++ ) {
                    if ( topo->t_neighbors[i] != t->t_neighbors [i] ) {
                        found = -1;
                        break;
                    }
                }
                if ( found == -1 ) {
                    continue;
                }
                for ( j=0 ; j< vec->v_ndims; j++ ){
                    if ( vec->v_dims[i] != v->v_dims[i] ) {
                        found = -1;
                        break;
                    }
                }
                if ( found != -1 ) {
                    break;
                }
            }
        }

        if ( found > -1 ) {
            e->em_rfcnt++;
            return e;
        }
    }

    /* we did not find this configuraion yet, so we have to add it */
    e = ( ADCL_emethod_t *) calloc (1, sizeof(ADCL_emethod_t));
    if ( NULL == e ) {
        return NULL;
    }

    ADCL_array_get_next_free_pos ( ADCL_emethod_array, &e->em_findex );
    ADCL_array_set_element ( ADCL_emethod_array, e->em_findex,
                 e->em_id, e );

    e->em_id          = ADCL_local_id_counter++;
    e->em_rfcnt       = 1;
    e->em_state       = ADCL_STATE_TESTING;
    e->em_explored_data = -1;
    e->em_topo        = t;
    e->em_vec         = v;
    e->em_orgfnctset  = f;

    /*
    ** Set the algorithm for the selection logic. Set it to the default value,
    ** if we have an attribute set. However, if no attributes are assigned to
    ** functionset, we have to use the brute force algorithm
    */
    if ( f->fs_attrset != NULL && f->fs_attrset != ADCL_ATTRSET_NULL ) {
        e->em_perfhypothesis = ADCL_emethod_use_perfhypothesis;
    }
    else {
        e->em_perfhypothesis = 0;
    }

    /*
    ** Generate a duplicate of the functions which we will work with.
    ** The reason is, that the list of functions etc. might be modified
    ** during the runtime optimization. We do not want to delete the original
    ** set, since it might be use by multiple requests/emehods *and*
    ** we might need the original list again when a re-evaluation has been
    ** initiated.
    */
    ADCL_fnctset_dup ( f, &(e->em_fnctset));

    /* Allocate the according number of statistics objects */
    e->em_stats = (ADCL_statistics_t **) calloc ( 1, f->fs_maxnum*sizeof(ADCL_statistics_t *));
    if ( NULL == e->em_stats ) {
        ret = ADCL_NO_MEMORY;
        goto exit;
    }
    for ( i=0; i< f->fs_maxnum; i++ ) {
        e->em_stats[i] = (ADCL_statistics_t *) calloc (1, sizeof(ADCL_statistics_t ));
        if ( NULL == e->em_stats[i] ) {
            ret = ADCL_NO_MEMORY;
            goto exit;
        }

        /* Allocate the measurements arrays */
        e->em_stats[i]->s_time = (TIME_TYPE *)calloc (1, sizeof(TIME_TYPE)*ADCL_emethod_numtests);
        if ( NULL == e->em_stats[i]->s_time ) {
            ret = ADCL_NO_MEMORY;
            goto exit;
        }
    }

    /* initiate the performance hypothesis structure */

    if ( e->em_perfhypothesis ) {
        ADCL_hypothesis_init ( e );
    }

    if ( 0 == strcmp ( f->fs_name , "Neighborhood communication") ) {
        if ( -1 != ADCL_emethod_selection ) {
            e->em_state = ADCL_STATE_REGULAR;
            e->em_wfunction = ADCL_emethod_get_function ( e, ADCL_emethod_selection );
        }
    }

 exit:
    if ( ret != ADCL_SUCCESS  ) {
        if ( NULL != e->em_stats  ) {
            for ( i=0; i< f->fs_maxnum; i++ ) {
                if ( NULL != e->em_stats[i]) {
                    if ( NULL != e->em_stats[i]->s_time ) {
                        free ( e->em_stats[i]->s_time );
                    }
                    free ( e->em_stats[i] );
                }
            }
            free ( e->em_stats );
        }
        if ( NULL != hypo->h_attr_hypothesis ) {
            free ( hypo->h_attr_hypothesis );
        }
        if ( NULL != hypo->h_attr_confidence ) {
            free ( hypo->h_attr_confidence );
        }
        if ( NULL != hypo->h_curr_attrvals ) {
            free ( hypo->h_curr_attrvals );
        }

        ADCL_array_remove_element ( ADCL_emethod_array, e->em_findex );
        free ( e );
        e = NULL;
    }

    return e;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
void ADCL_emethod_free ( ADCL_emethod_t * e )
{
    int i;
    e->em_rfcnt--;
    if ( e->em_rfcnt == 0 ) {
        ADCL_hypothesis_t *hypo = &(e->em_hypo);

        if ( NULL != e->em_stats  ) {
            for ( i=0; i< e->em_fnctset.fs_maxnum; i++ ) {
                if ( NULL != e->em_stats[i] ) {
                    if ( NULL != e->em_stats[i]->s_time ) {
                        free ( e->em_stats[i]->s_time );
                    }
                    free ( e->em_stats[i] );
                }
            }
            free ( e->em_stats );
        }

        if ( NULL != hypo->h_attr_hypothesis ) {
            free ( hypo->h_attr_hypothesis );
        }
        if ( NULL != hypo->h_attr_confidence ) {
            free ( hypo->h_attr_confidence );
        }
        if ( NULL != hypo->h_curr_attrvals ) {
            free ( hypo->h_curr_attrvals );
        }

        ADCL_array_remove_element ( ADCL_emethod_array, e->em_findex );
        free ( e );
    }

    return;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
ADCL_function_t* ADCL_emethod_get_function ( ADCL_emethod_t *e, int pos)
{
    return e->em_fnctset.fs_fptrs[pos];
}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_emethod_get_function_by_attrs ( ADCL_emethod_t *em,
                     int *attrval,
                     int *pos )
{
    int i, j, found;
    ADCL_fnctset_t* fnctset;
    int ret=ADCL_NOT_FOUND;

    fnctset = &(em->em_fnctset);
    for ( i=0; i< fnctset->fs_maxnum; i++ ) {
        for ( found=1, j=0; j<fnctset->fs_attrset->as_maxnum; j++ ){
            if ( fnctset->fs_fptrs[i]->f_attrvals[j] != attrval[j] ) {
                found = 0; /* false */
                break;
            }
        }
        if ( found ) {
            *pos = i;
            ret = ADCL_SUCCESS;
            break;
        }
    }
    return ret;
}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_emethod_get_stats_by_attrs ( ADCL_emethod_t *em, int *attrval,
                      ADCL_statistics_t **stat,
                      ADCL_function_t **func )
{
  int i, j, found=0;
  ADCL_fnctset_t* fnctset;

  *stat = NULL;
  *func = NULL;

  fnctset = &(em->em_fnctset);
  for ( i=0; i< fnctset->fs_maxnum; i++ ) {
      for ( found=1, j=0; j<fnctset->fs_attrset->as_maxnum; j++ ) {
        if ( fnctset->fs_fptrs[i]->f_attrvals[j] != attrval[j] ) {
            found = 0; /* false */
            break;
        }
      }
      if ( found ) {
        *stat = em->em_stats[i];
        *func = fnctset->fs_fptrs[i];
        break;
      }
  }

  if ( !found ) {
      return ADCL_NOT_FOUND;
  }

  return ADCL_SUCCESS;
}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_emethod_monitor ( ADCL_emethod_t *emethod, int pos,
               TIME_TYPE tstart, TIME_TYPE tend )
{
    /* to be done later */
    return ADCL_STATE_REGULAR;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_emethods_get_winner (ADCL_emethod_t *emethod, MPI_Comm comm, int count)
{
    int winner;

    /*
    ** Filter the input data, i.e. remove outliers which
    ** would falsify the results
    */
    ADCL_statistics_filter_timings ( emethod->em_stats, count,
                                     emethod->em_topo->t_rank );

    /*
    ** Determine now how many point each method achieved globally. The
    ** method with the largest number of points will be the chosen one.
    */
    if ( 0 == emethod->em_perfhypothesis ) {
        ADCL_statistics_global_max_v3 ( emethod->em_stats, count,
                                        emethod->em_topo->t_comm,
                                        emethod->em_topo->t_rank);
    }

    ADCL_statistics_get_winner_v3 ( emethod->em_stats, count, &winner );

    return winner;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_emethods_get_next ( ADCL_emethod_t *e, int mode, int *flag )
{
    int next = ADCL_EVAL_DONE;
    int last = e->em_last;
    int data_search_res;
    ADCL_data_t *data;
    ADCL_function_t *func;

    /* Search for solution/hints in the data stored from previous runs */
    data_search_res = ADCL_data_find( e, &data );
    switch (data_search_res) {
    case ADCL_IDENT:
        func = ADCL_fnctset_get_fnct_by_name ( e->em_orgfnctset,
                                               data->d_wfname );
        if ( ADCL_FUNCTION_NULL != func ) {
            e->em_state = ADCL_STATE_REGULAR;
            e->em_wfunction = func;
            return ADCL_SOL_FOUND;
        }
        break;
    case ADCL_SIMILAR:
        /* one idea is to increment the confidance number for 
           attributes values of the winning function */
        break;
    case ADCL_UNEQUAL:
    default:
        break;
    }

    if ( e->em_stats[last]->s_count < ADCL_emethod_numtests ) {
        *flag = ADCL_FLAG_PERF;
        e->em_stats[last]->s_count++;
        return last;
    }

    ADCL_STAT_SET_TESTED ( e->em_stats[last]);
    ADCL_statistics_filter_timings ( &(e->em_stats[last]), 1,
                                     e->em_topo->t_rank );

    if ( e->em_stats[last]->s_rescount < ADCL_emethod_numtests ) {
        /*
        ** ok, some data is still outstanding. So we
        ** do not switch yet to the evaluation mode,
        ** we return the last method with the noperf flag
        ** (= performance data not relevant for evaluation)
        */
        *flag = ADCL_FLAG_NOPERF;
        return last;
    }

    if ( e->em_perfhypothesis ) {
        ADCL_statistics_global_max_v3 ( &(e->em_stats[last]), 1,
                                        e->em_topo->t_comm,
                                        e->em_topo->t_rank );
        next = ADCL_hypothesis_get_next ( e );
        if ( next != ADCL_EVAL_DONE ) {
            e->em_last=next;
            e->em_stats[next]->s_count++;
        }
    }
    else {
        if ( last < ( e->em_fnctset.fs_maxnum -1 ) ) {
            next = last+1;
            e->em_last = next;
            e->em_stats[next]->s_count++;
        }
    }

    *flag = ADCL_FLAG_PERF;
    return next;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
void ADCL_emethods_update ( ADCL_emethod_t *emethod, int pos, int flag,
                TIME_TYPE tstart, TIME_TYPE tend )
{
    ADCL_statistics_t *stat;
    TIME_TYPE exectime;

    ADCL_STAT_TIMEDIFF ( tstart, tend, exectime );
    if ( flag == ADCL_FLAG_PERF ) {
        stat = emethod->em_stats[pos];
        stat->s_time[stat->s_rescount] = exectime;
        stat->s_rescount++;
    }

    return;
}
