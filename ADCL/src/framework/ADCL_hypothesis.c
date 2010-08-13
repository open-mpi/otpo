/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"

int ADCL_hypo_req_confidence = 2;

static int next_attr_combination ( ADCL_attrset_t *attrset,
                   ADCL_attribute_t  *active_attr,
                   int *attr_val_list );

static int next_attr_combination_excluding_active_attr ( ADCL_attrset_t *attrset,
                             int *attr_val_list,
                             int active_attr_pos );
static int get_max_attr_vals ( ADCL_attrset_t *attrset );
extern int ADCL_emethod_selection;

int ADCL_hypothesis_init ( ADCL_emethod_t *e  )
{
    ADCL_hypothesis_t *hypo;
    ADCL_fnctset_t *f = &(e->em_fnctset);
    int i;
    /* Memory allocation for the hypo structure */
    e->em_hypo = (ADCL_hypothesis_t *)malloc(sizeof(ADCL_hypothesis_t));
    /* Use hypo variable easier */
    hypo = e->em_hypo;
    hypo->h_num_avail_meas = 0;
    if ( ADCL_ATTRSET_NULL != f->fs_attrset ) {
        hypo->h_attr_hypothesis = (int *) malloc (f->fs_attrset->as_maxnum * sizeof(int));
        hypo->h_attr_confidence = (int *) calloc (1, f->fs_attrset->as_maxnum * sizeof(int));
        hypo->h_curr_attrvals   = (int *) malloc (f->fs_attrset->as_maxnum * sizeof(int));
        if ( NULL == hypo->h_attr_hypothesis ||
             NULL == hypo->h_attr_confidence ||
             NULL == hypo->h_curr_attrvals   ) {
            return ADCL_NO_MEMORY;
        }

        for ( i=0; i< e->em_fnctset.fs_attrset->as_maxnum; i++ ) {
            hypo->h_attr_hypothesis[i] = ADCL_ATTR_NOT_SET;
            hypo->h_curr_attrvals[i]   = f->fs_attrset->as_attrs_baseval[i];
        }

        /* Initialize the attribute list  */
        hypo->h_active_attr = f->fs_attrset->as_attrs[0];
        hypo->h_active_attrpos = 0;
    }

    return ADCL_SUCCESS;
}


/**********************************************************************/
/**********************************************************************/
/**********************************************************************/

int ADCL_hypothesis_eval_v3 ( ADCL_emethod_t *e )
{
    int i, loop, winner_attr_val_pos, winner_attr_val;
    int done=0, ret=ADCL_SUCCESS;
    int maxdim, *attrval_list=NULL;
    ADCL_hypothesis_t *hypo=e->em_hypo;
    ADCL_attrset_t *attrset=e->em_fnctset.fs_attrset;

    ADCL_statistics_t **tmp_stats;
    ADCL_function_t **tmp_funcs;

    maxdim = get_max_attr_vals ( attrset );
    tmp_stats = (ADCL_statistics_t **) malloc ( maxdim * sizeof (ADCL_statistics_t *));
    tmp_funcs = (ADCL_function_t **) malloc ( maxdim * sizeof (ADCL_function_t *));
    attrval_list = (int *) malloc ( attrset->as_maxnum * sizeof(int));
    if ( NULL == tmp_stats || NULL == tmp_funcs || NULL == attrval_list ) {
        return ADCL_NO_MEMORY;
    }

    for ( i=0; i< attrset->as_maxnum; i++ ) {
        /* Reset the confidence values to 0 and all performance
           hypothesis to NOT SET for the attributes which have not
           yet reached the critical values */
        if ( hypo->h_attr_confidence[i] < ADCL_hypo_req_confidence ) {
            hypo->h_attr_confidence[i] = 0 ;
            hypo->h_attr_hypothesis[i] = ADCL_ATTR_NOT_SET;
        }
    }

    for ( loop=0; loop<attrset->as_maxnum; loop++ ){
        if ( hypo->h_attr_hypothesis[loop] != ADCL_ATTR_NOT_SET ) {
            /* skip this attribute, its already done */
            continue;
        }

        /* Initialize the attribute value list to the base values */
        for ( i = 0; i < attrset->as_maxnum; i++ ) {
            attrval_list[i] = attrset->as_attrs_baseval[i];
        }

        while (!done) {
            ADCL_hypothesis_eval_one_attr ( e, attrset->as_maxnum,
                            attrval_list, attrset->as_attrs[loop], loop,
                            attrset->as_attrs[loop]->a_maxnvalues,
                            &winner_attr_val_pos, &winner_attr_val,
                            tmp_stats, tmp_funcs );
	    if ( winner_attr_val_pos != ADCL_ATTR_NOT_SET ) {
                ADCL_hypothesis_set ( e, loop, winner_attr_val );
            }
            ret = next_attr_combination_excluding_active_attr ( attrset,
                                                                attrval_list,
                                                                loop );
            if ( ADCL_SUCCESS != ret ) {
                break;
            }
        }
    }

    /* Now the shrink the emethods list if we reached a threshold */
    for ( loop=0; loop<attrset->as_maxnum; loop++ ){
        if ( hypo->h_attr_confidence[loop] >= ADCL_hypo_req_confidence &&
             hypo->h_attr_hypothesis[loop] != ADCL_ATTR_NOT_SET        &&
             attrset->as_attrs[loop]->a_maxnvalues > 1 ) {
            ret = ADCL_CHANGE_OCCURED;
            DISPLAY ((ADCL_DISPLAY_MESSAGE, e->em_id, "Function set:%d %s, attribute %d %s reached threshhold, ptimal value:%d", 
	              e->em_orgfnctset->fs_id, e->em_orgfnctset->fs_name, attrset->as_attrs[loop]->a_id, 
		      attrset->as_attrs[loop]->a_attr_name, hypo->h_attr_hypothesis[loop] ));
            ADCL_hypothesis_shrinklist_byattr( e, loop,
                                               hypo->h_attr_hypothesis[loop]);
        }
    }

    free ( tmp_stats );
    free ( tmp_funcs );
    free ( attrval_list );

    return ret;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_hypothesis_shrinklist_byattr ( ADCL_emethod_t *e,
                                        int attr_pos, int required_value )
{
    int i, count=0;
    ADCL_fnctset_t *fnctset=&(e->em_fnctset);
    int org_count=fnctset->fs_maxnum;
    ADCL_attrset_t *attrset=fnctset->fs_attrset;
    ADCL_function_t *tfunc=NULL;


    /* This following loop is only for debugging purposes and should be removed
       from later production runs */
    for ( i=0; i < org_count; i++ ) {
        if ( fnctset->fs_fptrs[i]->f_attrvals[attr_pos] != required_value ) {
            ADCL_printf("#Removing function %d for attr %d required %d is %d\n",
                        fnctset->fs_fptrs[i]->f_id, attrset->as_attrs[attr_pos]->a_id,
                        required_value, fnctset->fs_fptrs[i]->f_attrvals[attr_pos]);
        }
    }

    for ( i=0; i < org_count; i++ ) {
        tfunc = fnctset->fs_fptrs[i];
        if ( tfunc->f_attrvals[attr_pos] == required_value ) {
            if ( count != i ) {
                /* move this emethod from pos i to pos count */
                fnctset->fs_fptrs[count] = fnctset->fs_fptrs[i];

                /* do the same with the statistics objects */
                e->em_stats[count] = e->em_stats[i];
            }
            count++;
        }
    }
    fnctset->fs_maxnum = count;
    ADCL_printf("#Fnctset has been shrinked from %d to %d entries\n",
        org_count, count );


    /* Finally, adjust the attribute list and the attrset to the new values */
    attrset->as_attrs[attr_pos]->a_maxnvalues =  1;
    attrset->as_attrs[attr_pos]->a_values[0]  =  required_value;

    attrset->as_attrs_baseval[attr_pos] = required_value;
    attrset->as_attrs_maxval[attr_pos]  = required_value;
    attrset->as_attrs_numval[attr_pos]  = 1;

    return ADCL_SUCCESS;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_hypothesis_set ( ADCL_emethod_t *e, int attrpos, int attrval )
{
    ADCL_hypothesis_t *hypo = e->em_hypo;

    if ( ADCL_ATTR_NOT_SET == hypo->h_attr_hypothesis[attrpos] ) {
        hypo->h_attr_hypothesis[attrpos] = attrval;
        hypo->h_attr_confidence[attrpos] = 1;
        ADCL_printf("#Hypothesis for attr %d set to %d, confidence"
                    " %d\n", attrpos, attrval, hypo->h_attr_confidence[attrpos]);
    }
    else if ( attrval == hypo->h_attr_hypothesis[attrpos] ) {
        hypo->h_attr_confidence[attrpos]++;
        ADCL_printf("#Hypothesis for attr %d is %d, confidence "
        "incr to %d\n", attrpos, attrval, hypo->h_attr_confidence[attrpos]);
    }
    else {
        hypo->h_attr_confidence[attrpos]--;
        ADCL_printf("#Hypothesis for attr %d is %d, confidence "
                    "decr to %d\n", attrpos, hypo->h_attr_hypothesis[attrpos],
                    hypo->h_attr_confidence[attrpos]);
        if ( hypo->h_attr_confidence[attrpos] == 0 ) {
            /* we don't have a performance hypthesis for this attribute anymore */
            hypo->h_attr_hypothesis[attrpos] = ADCL_ATTR_NOT_SET;
        }
    }

    return ADCL_SUCCESS;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
int ADCL_hypothesis_eval_one_attr ( ADCL_emethod_t *e, int num_attrs,  int *attr_values,
                    ADCL_attribute_t * attr, int attr_pos, int max_attr_vals,
                    int *winner_attr_val_pos, int *winner_attr_val,
                    ADCL_statistics_t **tmp_stats, ADCL_function_t **tmp_funcs )
{
    int ret, i, winner, count;

    *winner_attr_val_pos  = ADCL_ATTR_NOT_SET;

    for ( count=0, i=0; i<max_attr_vals; i++ ) {
        attr_values[attr_pos] = ADCL_attribute_get_val ( attr, i ) ;
        ret = ADCL_emethod_get_stats_by_attrs ( e, attr_values,
                            &tmp_stats[count],
                            &tmp_funcs[count]);

        if ( ADCL_SUCCESS != ret ) {
            /* There is no function with this particular combination of attribute values */
            continue;
        }
        if ( !(ADCL_STAT_IS_FILTERED(tmp_stats[count])) ) {
            /* this function does exist, has however not yet been evaluated */
            goto exit;
        }
        count++;
    }

    if ( count < 2 ) {
        goto exit;
    }
    /*
    ** Assuming that the data is filtered at this point already,
    ** and the global max per series has been determined
    */
    ADCL_statistics_get_winner_v3 ( tmp_stats, count, &winner );
    *winner_attr_val     = ADCL_function_get_attrval ( tmp_funcs[winner], attr_pos );
    *winner_attr_val_pos = ADCL_attribute_get_pos ( attr, *winner_attr_val );

 exit:
    /* Reset the value for the attribute to be analysed to its base value,
       in order to avoid confusion */
    attr_values[attr_pos] = ADCL_attribute_get_val ( attr, 0 );
    return count;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_hypothesis_get_next ( ADCL_emethod_t *e )
{
    int done=0; /* false */
    int i, ret, ret2, next=ADCL_EVAL_DONE;
    int just_evaluated=0; /* false */
    ADCL_hypothesis_t *hypo=e->em_hypo;
    ADCL_attrset_t *attrset= (&(e->em_fnctset))->fs_attrset;

    while ( !done ) {
        ret = next_attr_combination ( e->em_fnctset.fs_attrset,
                                      hypo->h_active_attr,
                                      hypo->h_curr_attrvals);
        if ( ret == ADCL_EVAL_DONE ) {
            /* All possible attribute value combinations have been tested */
            break;
        }
        ret2 = ADCL_fnctset_get_fnct_by_attrs ( &(e->em_fnctset),
                                                    hypo->h_curr_attrvals,
                                                    &next );
        if ( ADCL_NOT_FOUND == ret2 ) {
            /*
            ** there is no function with this particular
            ** combination of attribute values
            */
            continue;
        }
        if ( ADCL_STAT_IS_TESTED ( e->em_stats[next]) ) {
            /* this function has already been evaluated */
            continue;
        }

        if ( ADCL_ATTR_NEW_BLOCK == ret && !just_evaluated ) {
            ret = ADCL_hypothesis_eval_v3 ( e );
            just_evaluated = 1;
            /*
            ** check whether the optimal value for the currently investigated
            ** attribute has been determined, by looking at whether it
            ** has more than one possible value
            */
            if ( ret == ADCL_CHANGE_OCCURED ) {
                if ( 1 == hypo->h_active_attr->a_maxnvalues ) {
                    /* yes it, has. Determine the next active attribute  */

                    for ( ; hypo->h_active_attrpos<attrset->as_maxnum; hypo->h_active_attrpos++){
                        hypo->h_active_attr = attrset->as_attrs[hypo->h_active_attrpos];
                        if ( hypo->h_active_attr->a_maxnvalues != 1 ) {
                            /* this attribute has not been optimized yet */
                            break;
                        }
                    }
                    if ( hypo->h_active_attrpos == attrset->as_maxnum ) {
                        /* the system has determined the optimal values for all
                           attributes. So we are done and can return */
                        next = ADCL_EVAL_DONE;
                        break;
                    }
                }
                /* Reset all attributes to its base values */
                for ( i=0; i< attrset->as_maxnum; i++  ) {
                    hypo->h_curr_attrvals[i] = attrset->as_attrs_baseval[i];
                }

                /* Get the according function pointer */
                ret2 = ADCL_fnctset_get_fnct_by_attrs ( &(e->em_fnctset),
                                        hypo->h_curr_attrvals,
                                        &next );
                if ( ADCL_NOT_FOUND == ret2 ) {
                    /*
                    ** there is no function with this particular
                    ** combination of attribute values
                    */
                    continue;
                }
                if ( ADCL_STAT_IS_TESTED ( e->em_stats[next]) ) {
                    /* this function has already been evaluated */
                    continue;
                }
                return ADCL_CHANGE_OCCURED;
            }
            else {
                /*
                ** This path should be executed, if we have a new attribute combination
                ** we had to do an evaluation but the evaluation did not lead to an
                ** an optimal value for the currently optimized attribute. So we just
                ** keep going on.
                */
                break;
            }
        }
        done=1;
    }

    return next;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
 * attrset:       attribute set which we are working with.
 * active_attr: position of the attribute currently being investigated
 * attr_val_list: array of dimension ADCL_ATTR_TOTAL_NUM containg the last used
 *                combination of attributes
 */

int next_attr_combination ( ADCL_attrset_t *attrset,
                ADCL_attribute_t  *active_attr,
                int *attr_val_list )
{
    int ret=ADCL_SUCCESS;
    int thisval, thispos;
    ADCL_attribute_t *thisattr;

    thisattr = active_attr;
    thispos = ADCL_attrset_get_pos ( attrset, thisattr );
    thisval = attr_val_list[thispos];

    if ( thisval < attrset->as_attrs_maxval[thispos] ) {
        attr_val_list[thispos] = ADCL_attribute_get_nextval (thisattr, thisval);
        return ret;
    }
    else if ( thisval == attrset->as_attrs_maxval[thispos] ) {
        attr_val_list[thispos] = attrset->as_attrs_baseval[thispos];
        ret = next_attr_combination_excluding_active_attr ( attrset, attr_val_list,
                                  thispos );
        if ( ret != ADCL_EVAL_DONE ) {
            return ADCL_ATTR_NEW_BLOCK;
        }
    }
    else {
        /* Bug, should not happen */
    }

    return ADCL_EVAL_DONE;
}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/

static int next_attr_combination_excluding_active_attr ( ADCL_attrset_t *attrset,
                             int *attr_val_list,
                             int active_attr_pos )
{
    int i, ret=ADCL_SUCCESS;
    int thisval;

    for ( i = 0; i < attrset->as_maxnum; i++ ) {
        if ( i == active_attr_pos ) {
            continue;
        }

        thisval = attr_val_list[i];

        if ( thisval < attrset->as_attrs_maxval[i] ) {
            attr_val_list[i] = ADCL_attribute_get_nextval (attrset->as_attrs[i],
                                   attr_val_list[i]);
            return ret;
        }
        else if ( thisval == attrset->as_attrs_maxval[i] ){
            attr_val_list[i] = attrset->as_attrs_baseval[i];
        }
        else {
            /* Bug, should not happen */
        }
    }

    return ADCL_EVAL_DONE;
}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/

static int get_max_attr_vals ( ADCL_attrset_t *attrset )
{
    int i, max=attrset->as_attrs_numval[0];

    for ( i=1; i < attrset->as_maxnum; i++ ) {
        if ( attrset->as_attrs_numval[i] > max ) {
            max = attrset->as_attrs_numval[i];
        }
    }

    return max;
}


