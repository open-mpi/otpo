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

ADCL_array_t *ADCL_hist_array = NULL;
/* Prediction algorithm to be used : WMV, CLOSEST, NBC, SVM */
int ADCL_hist_predictor = ADCL_PRED_ALGO;

/* Create a new hist entry consisting on the problem characteristics */
static int hist_create_new ( ADCL_emethod_t *e, ADCL_hist_t *hist );
/* Re-initialization of the history object */
static void hist_reinit(ADCL_hist_t *hist);
/* Add a history entry to the history file */
static void hist_add_to_file( ADCL_hist_t* hist, ADCL_emethod_t *e );
/* Check if a given history entry is identical to a given emethod */
static int hist_check_for_ident(ADCL_emethod_t *e, ADCL_hist_t *hist);
/* Classify the implementation according to a given acceptable performance window */
static int hist_classify_implementations( double *elapsed_time, int *impl_class, int nb_of_impl, int perf_win );
/* Find the maximum distance */
static double hist_find_dmax(double *distance, int *relation, int num_sizes );

/* Function to create a history entry after solving a problem */
int ADCL_hist_create ( ADCL_emethod_t *e )
{
    ADCL_hist_t *hist;
    ADCL_topology_t *topo = e->em_topo;
    ADCL_vector_t   *vec  = e->em_vec;
    int i, size, topo_status;
    int *dims, *coords;

    /* For now we support only cartesian topology */
    MPI_Topo_test ( topo->t_comm, &topo_status );
    if ( ( MPI_CART != topo_status ) || ( ADCL_VECTOR_NULL == vec ) ) {
        return ADCL_INVALID_ARG;
    }
    size = topo->t_size;
    hist = (ADCL_hist_t *) calloc (1, sizeof(ADCL_hist_t));
    hist->h_rcnts = (int *)calloc (size, sizeof(int));
    hist->h_displ = (int *)calloc (size, sizeof(int));
    if ( (NULL == hist) || (NULL == hist->h_rcnts) || (NULL == hist->h_displ) ) {
        return ADCL_NO_MEMORY;
    }
    /* Internal info for object management */
    hist->h_id = ADCL_local_id_counter++;
    ADCL_array_get_next_free_pos ( ADCL_hist_array, &hist->h_findex );
    ADCL_array_set_element ( ADCL_hist_array, hist->h_findex, hist->h_id, hist );
    hist->h_refcnt = 1;
    /* Network Topology information */
    hist->h_np = size;
    /* Logical Topology information */
    hist->h_tndims = topo->t_ndims;
    hist->h_tperiods = (int *)malloc( hist->h_tndims*sizeof(int) );
    dims = (int *)malloc( hist->h_tndims*sizeof(int) );
    coords = (int *)malloc( hist->h_tndims*sizeof(int) );
    MPI_Cart_get ( topo->t_comm, topo->t_ndims, dims, hist->h_tperiods, coords );
    free( dims );
    free( coords );
    /* Vector information */
    hist->h_vndims = vec->v_ndims;
    hist->h_vdims = (int *)malloc(hist->h_vndims*sizeof(int) );
    for ( i=0; i<hist->h_vndims ;i++ ) {
        hist->h_vdims[i] = vec->v_dims[i];
    }
    hist->h_nc = vec->v_nc;
    /* Vector map information */
    hist->h_vectype = vec->v_map->m_vectype;
    hist->h_hwidth = vec->v_map->m_hwidth;
    if ((NULL != vec->v_map->m_rcnts) && (NULL != vec->v_map->m_displ)) {
        for ( i=0; i<size; i++ ) {
            hist->h_rcnts[i] = vec->v_map->m_rcnts[i];
            hist->h_displ[i] = vec->v_map->m_displ[i];
        }
    }
    else {
        for ( i=0; i<size; i++ ) {
            hist->h_rcnts[i] = 0;
            hist->h_displ[i] = 0;
        }
    }
    hist->h_op = vec->v_map->m_op;
    hist->h_inplace = vec->v_map->m_inplace;
    /* Attribute information */
    if (NULL != e->em_orgfnctset->fs_attrset && ADCL_ATTRSET_NULL !=  e->em_orgfnctset->fs_attrset){
       hist->h_asmaxnum = e->em_orgfnctset->fs_attrset->as_maxnum;
       hist->h_attrvals = (int *)malloc( hist->h_asmaxnum * sizeof(int) );
       /* Initialization */
       for (i=0; i<hist->h_asmaxnum ; i++){
           hist->h_attrvals[i] = -1;
       }
       if ( ADCL_PERF_HYPO == e->em_search_algo ) {
           for (i=0; i<hist->h_asmaxnum ; i++){
               if( e->em_hypo->h_attr_confidence[i] > 1 ) {
                   hist->h_attrvals[i] = e->em_fnctset.fs_fptrs[0]->f_attrvals[i];
               }
           }
       }
    }
    /* Function set and winner function */
    hist->h_fsname = strdup ( e->em_orgfnctset->fs_name );
    hist->h_fsnum = e->em_orgfnctset->fs_maxnum;
    hist->h_wfname = strdup ( e->em_wfunction->f_name );
    hist->h_wfnum = ADCL_fnctset_get_fnct_num ( e->em_orgfnctset, e->em_wfunction );
    /* Performance hist for H.L. */
    /* Execution times */
    hist->h_perf = (double *)malloc(hist->h_fsnum*sizeof(double));
    for(i=0; i<hist->h_fsnum ; i++) {
        hist->h_perf[i] = e->em_stats[i]->s_gpts[ e->em_filtering];
    }
    /* The acceptable performance window */
    hist->h_perf_win = ADCL_PERF_WIN;
    hist->h_class = (int *)malloc(hist->h_fsnum*sizeof(int));
    /* Classifying the implementations */
    hist_classify_implementations( hist->h_perf, hist->h_class, hist->h_fsnum, ADCL_PERF_WIN );
    /* Set to Invalid dmax */
    hist->h_dmax = -1.00;

#ifdef ADCL_KNOWLEDGE_TOFILE
    /* Update the hist file */
    hist_add_to_file( hist, e );
#endif
    return ADCL_SUCCESS;
}

/* Function to create a new history entry for a problem without a solution */
static int hist_create_new ( ADCL_emethod_t *e, ADCL_hist_t *hist )
{
    ADCL_topology_t *topo = e->em_topo;
    ADCL_vector_t   *vec  = e->em_vec;
    int i, size, topo_status;
    int *dims, *coords;

    /* For now we support only cartesian topology */
    MPI_Topo_test ( topo->t_comm, &topo_status );
    if ( ( MPI_CART != topo_status ) || ( ADCL_VECTOR_NULL == vec ) ) {
        return ADCL_INVALID_ARG;
    }
    size = topo->t_size;
    hist->h_rcnts = (int *)calloc (size, sizeof(int));
    hist->h_displ = (int *)calloc (size, sizeof(int));
    if ( (NULL == hist) || (NULL == hist->h_rcnts) || (NULL == hist->h_displ) ) {
        return ADCL_NO_MEMORY;
    }
    /* Internal info for object management */
    hist->h_id = ADCL_local_id_counter++;
    ADCL_array_get_next_free_pos ( ADCL_hist_array, &hist->h_findex );
    ADCL_array_set_element ( ADCL_hist_array, hist->h_findex, hist->h_id, hist );
    hist->h_refcnt = 1;
    /* Network Topology information */
    hist->h_np = size;
    /* Logical Topology information */
    hist->h_tndims = topo->t_ndims;
    hist->h_tperiods = (int *)malloc( hist->h_tndims*sizeof(int) );
    dims = (int *)malloc( hist->h_tndims*sizeof(int) );
    coords = (int *)malloc( hist->h_tndims*sizeof(int) );
    MPI_Cart_get ( topo->t_comm, topo->t_ndims, dims, hist->h_tperiods, coords );
    free( dims );
    free( coords );
    /* Vector information */
    hist->h_vndims = vec->v_ndims;
    hist->h_vdims = (int *)malloc(hist->h_vndims*sizeof(int) );
    for ( i=0; i<hist->h_vndims ;i++ ) {
        hist->h_vdims[i] = vec->v_dims[i];
    }
    hist->h_nc = vec->v_nc;
    /* Vector map information */
    hist->h_vectype = vec->v_map->m_vectype;
    hist->h_hwidth = vec->v_map->m_hwidth;
    if ((NULL != vec->v_map->m_rcnts) && (NULL != vec->v_map->m_displ)) {
        for ( i=0; i<size; i++ ) {
            hist->h_rcnts[i] = vec->v_map->m_rcnts[i];
            hist->h_displ[i] = vec->v_map->m_displ[i];
        }
    }
    else {
        for ( i=0; i<size; i++ ) {
            hist->h_rcnts[i] = 0;
            hist->h_displ[i] = 0;
        }
    }
    hist->h_op = vec->v_map->m_op;
    hist->h_inplace = vec->v_map->m_inplace;

    return ADCL_SUCCESS;
}

void ADCL_hist_free ( void )
{
    int i, last;
    ADCL_hist_t *hist;

    last = ADCL_array_get_last ( ADCL_hist_array );
    /* Free all the hist objects */
    for ( i=0; i<= last; i++ ) {
        hist = ( ADCL_hist_t * ) ADCL_array_get_ptr_by_pos( ADCL_hist_array, i );
        if ( NULL != hist  ) {
            if ( NULL != hist->h_tperiods ) {
                free ( hist->h_tperiods );
            }
            if ( NULL != hist->h_vdims ) {
                free ( hist->h_vdims );
            }
            if ( NULL != hist->h_rcnts ) {
                free ( hist->h_rcnts );
            }
            if ( NULL != hist->h_displ ) {
                free ( hist->h_displ );
            }
            if ( NULL != hist->h_attrvals ) {
                free ( hist->h_attrvals );
            }           
            if ( NULL != hist->h_fsname ) {
                free ( hist->h_fsname );
            }
            if ( NULL != hist->h_wfname ) {
                free ( hist->h_wfname );
            }
            if ( NULL != hist->h_perf ) {
                free ( hist->h_perf );
            }
            ADCL_array_remove_element ( ADCL_hist_array, hist->h_findex );
            free ( hist );
        }
    }
    return;
}

/* Function to reinitialize an useless history object to be reused */
static void hist_reinit(ADCL_hist_t *hist)
{
    if ( NULL != hist ) {
        if ( NULL != hist->h_tperiods ) {
            free ( hist->h_tperiods );
	    hist->h_tperiods = NULL;
        }
        if ( NULL != hist->h_vdims ) {
            free ( hist->h_vdims );
	    hist->h_vdims = NULL;
        }
        if ( NULL != hist->h_rcnts ) {
            free ( hist->h_rcnts );
	    hist->h_rcnts = NULL;
        }
        if ( NULL != hist->h_displ ) {
            free ( hist->h_displ );
	    hist->h_displ = NULL;
        }
        if ( NULL != hist->h_attrvals ) {
            free ( hist->h_attrvals );
	    hist->h_attrvals = NULL;
        }           
        if ( NULL != hist->h_fsname ) {
            free ( hist->h_fsname );
	    hist->h_fsname = NULL;
        }
        if ( NULL != hist->h_wfname ) {
            free ( hist->h_wfname );
	    hist->h_wfname = NULL;
        }
        if ( NULL != hist->h_perf ) {
            free ( hist->h_perf );
	    hist->h_perf = NULL;
        }
        /* may be more stuff should be done */
    }
}

/* Function to find a (identical or similar) solution in the history entries */
int ADCL_hist_find ( ADCL_emethod_t *e, ADCL_hist_t **found_hist )
{

    ADCL_fnctset_t *fnctset = e->em_orgfnctset;
    ADCL_hist_list_t *hist_list = e->em_hist_list;
    ADCL_hist_list_t *hist_list1;
    ADCL_hist_list_t *hist_list2;
    ADCL_hist_t *hist;
    ADCL_hist_t *hist1;
    ADCL_hist_t *hist2;
    int ret = ADCL_UNEQUAL;
    FILE *fp;
    int nchar = 80;
    char *line = NULL;
    int nhist = 0;
    int filtering = 0;
    char *perf = NULL;
    int ident, explored_hist, pass_filter;
    int i, j, k;
    /* For closest algo */
    int init = 0;
    double min_dist;
    /* For WMV algo */
    double dist, max_weight;
    double *prediction_weight;
    int predicted_winner;


    if( -2 == e->em_explored_hist ) {

#ifdef ADCL_KNOWLEDGE_TOFILE
        /* Check if a reading function of the according function set do exist */
        if( NULL != fnctset->fs_hist_functions ) {
            if( NULL == fnctset->fs_hist_functions->hf_reader ) {
                ADCL_printf("No history reader function registered \n");
                goto exit;    
            }
            if( NULL == fnctset->fs_hist_functions->hf_distance ) {
                ADCL_printf("No history distance function registered \n");
                goto exit;    
            }
        }
        else {
            ADCL_printf("No history functions registered \n");
            goto exit;
        }
        /* Check if a filtering mechanism is set up */
        if( (NULL == fnctset->fs_hist_functions->hf_filter)||(NULL == e->em_hist_criteria) ) {
            ADCL_printf("No filtering function/criteria st registered \n");
        }
	else {
	    filtering =1;
	}

        /*** TBD: READ HEADER, To be a seperate function later on ***/

        /* Open history file and read the header */
        fp = fopen ("hist.adcl", "r");
        if ( NULL == fp ) {
            goto exit;
        }
        /* Read the history file header */
        line = (char *)malloc( nchar * sizeof(char) );
        fgets( line, nchar, fp ); /* XML header lines */
        fgets( line, nchar, fp );
        fgets( line, nchar, fp ); /* ADCL Tag */
        fgets( line, nchar, fp );
        get_int_data_from_xml ( line, &nhist );

        /************/

        /* If no filtering mechanism, we check only for Identical history */
        /* Then only a single history object is enough */
        hist = (ADCL_hist_t *) calloc (1, sizeof(ADCL_hist_t));
        if ( NULL == hist ) {
            goto exit;
        }
        /* Internal info for object management */
        hist->h_id = ADCL_local_id_counter++;
        ADCL_array_get_next_free_pos ( ADCL_hist_array, &hist->h_findex );
        ADCL_array_set_element ( ADCL_hist_array, hist->h_findex, hist->h_id, hist );
        hist->h_refcnt = 1;

        /*** Reading of the history entries and checking for indentical solved problems ***/
        for ( i=0; i<nhist; i++ ) {
            /* Use the user defined functions of the according function set for reading and filtering */
            /* Read the entry using the user predefined reading function */
            fnctset->fs_hist_functions->hf_reader( fp, hist );
            /* Check if it is an identical history entry */
            ident = hist_check_for_ident( e, hist);
            /*** An identical solution has been found ***/
            if( ADCL_IDENT==ident ) {
                *found_hist = hist;
		ret = ADCL_IDENT;
                goto exit;
	    }
	    else if( ADCL_UNEQUAL == ident ){/* Not an identical problem */
                if( 0 == filtering){
                    hist_reinit(hist);
		}
                else {
                    /* Filter the history entry */
                    pass_filter = fnctset->fs_hist_functions->hf_filter(hist,
                                  e->em_hist_criteria->hc_filter_criteria);
		    if( 0 == pass_filter) { /* did not pass */
                        hist_reinit(hist);
                    }
                    else {
                        /* Increment the count of history entries */
                        e->em_hist_cnt++;
                        /* Copy the history object handle in current */
                        hist_list->hl_curr = hist;
                        /* Update the classes if the performance window has been changed */
                        if(ADCL_PERF_WIN != hist->h_perf_win) {
                            /* Re-classify the implementations */
                            hist_classify_implementations( hist->h_perf, hist->h_class, hist->h_fsnum, ADCL_PERF_WIN );
                            /* Set to Invalid dmax */
                            hist->h_dmax = -1;
		        }
                        /* Next */
	        	hist_list->hl_next = (ADCL_hist_list_t *)calloc(1, sizeof(ADCL_hist_list_t));
                        hist_list = hist_list->hl_next;
                        /* Allocate a new history object */
                        hist = (ADCL_hist_t *) calloc (1, sizeof(ADCL_hist_t));
                        if ( NULL == hist ) {
                            goto exit;
                        }
                        /* Internal info for object management */
                        hist->h_id = ADCL_local_id_counter++;
                        ADCL_array_get_next_free_pos ( ADCL_hist_array, &hist->h_findex );
                        ADCL_array_set_element ( ADCL_hist_array, hist->h_findex, hist->h_id, hist );
                        hist->h_refcnt = 1;
	            }
	        }
	    }
	}
        fclose ( fp );
        if( NULL != perf ) {
            free ( perf );
        }
        if ( NULL != line ) {
            free ( line );
        }

#endif
	e->em_explored_hist = -1;
    }

    /*** Prediction of a solution from similar solved problems ***/
    if ( (-1 == e->em_explored_hist) && (1 == filtering) ) {
        /* Check if a sufficient number of history entries 
           is available to try to make a prediction */
        if( ADCL_MIN_HIST > e->em_hist_cnt ) {
	    ADCL_printf("The number of history entries is unsifficient to make a prediction\n");
            goto exit;
	}
        /* Create a hist entry of the current problem without a solution */
        e->em_hist = (ADCL_hist_t *)calloc(1, sizeof(ADCL_hist_t));
        hist_create_new ( e, e->em_hist );
	if (ADCL_WMV == ADCL_hist_predictor) {
	    prediction_weight = (double *)calloc(e->em_orgfnctset->fs_maxnum, sizeof(double));
	}
        /* Memory allocation for distances and relations */
        e->em_relations = (int **)malloc(e->em_hist_cnt*sizeof(int *));
        e->em_distances = (double **)malloc(e->em_hist_cnt*sizeof(double *));
	for(i=0;i<e->em_hist_cnt;i++){
            e->em_relations[i] = (int *)malloc(e->em_hist_cnt*sizeof(int));
            e->em_distances[i] = (double *)malloc(e->em_hist_cnt*sizeof(double));
	}
        /* Computing distances and relations */
        /* Outer loop on hist1 */
        hist_list1 = e->em_hist_list;
	i=0;
        while ( NULL != hist_list1 ) {
            /* Get history entry handle */
            hist1 = hist_list1->hl_curr;
            /* Check if it is not NULL */
            if(NULL == hist1) {
                break;
	    }
            /* Inner loop on hist2 */
            hist_list2 = e->em_hist_list;
            j = 0;
            while ( NULL != hist_list2 ) {
                /* Get history entry handle */
                hist2 = hist_list2->hl_curr;
                /* Check if it is not NULL */
                if( NULL == hist2 ) {
                    break;
		}
                /* Compute distance */
                //can be optimized since it is a symmetric matrix
                e->em_distances[i][j] = fnctset->fs_hist_functions->hf_distance(hist1, hist2);
                /* Compute relation */
                e->em_relations[i][j] = 0;
                /* Is the winner of pb size i among the best impl of pb size j ? */
                if ( ADCL_BEST_CLASS == hist2->h_class[hist1->h_wfnum] ) {
                    e->em_relations[i][j] = 1;
                }
                /* Go to the next */
	        hist_list2 = hist_list2->hl_next;
                j++;
            }
            /* Compute dmax of hist 1 */
	    hist1->h_dmax = hist_find_dmax( e->em_distances[i], e->em_relations[i], e->em_hist_cnt );
            /* Compute the distance of the emethod hist and hist1 */
            dist = fnctset->fs_hist_functions->hf_distance(e->em_hist, hist1);

            if( dist <= hist1->h_dmax ) {
		if (ADCL_WMV == ADCL_hist_predictor) {
		    /* Add the weight to the predicted winner by hist1 */
		    prediction_weight[hist1->h_wfnum]+=1/dist;
		} else if (ADCL_CLOSEST == ADCL_hist_predictor) {
		    if( 0 == init ) {
			/* set minimum distance */
			min_dist = dist;
			/* set the estimated winner */
			e->em_hist->h_wfnum = hist1->h_wfnum;
			/* return the pointer to the similar hist */
			*found_hist = hist1;
			/* set initted */
			init = 1;
			/* Here we are sure at least we have one prediction */
			ret = ADCL_SIMILAR;
		    }
		    else if( dist < min_dist ){
			/* set minimum distance */
			min_dist = dist;
			/* set the estimated winner */
			e->em_hist->h_wfnum = hist1->h_wfnum;
			/* return the pointer to the similar hist */
			*found_hist = hist1;
		    }
                }
	    }
	    /* Go to the next */
	    hist_list1 = hist_list1->hl_next;
	    i++;
	}

	if (ADCL_WMV == ADCL_hist_predictor) {
	    max_weight = 0;
	    predicted_winner = -1;
	    for(i=0; i<e->em_orgfnctset->fs_maxnum; i++) {
		if ( prediction_weight[i] > max_weight ) {
		    max_weight = prediction_weight[i];
		    predicted_winner = i;
		}
	    }
	    if( 0 <= predicted_winner ) {
		e->em_hist->h_wfnum = predicted_winner;
		ret = ADCL_SIMILAR;
	    }
	    free(prediction_weight);
	}
	if ( ADCL_SIMILAR == ret ) {
	    if (ADCL_WMV == ADCL_hist_predictor) {
		ADCL_printf("#%d A solution to the problem is predicted from similar problem(s) in history, winner is %d\n",
			    e->em_topo->t_rank, e->em_hist->h_wfnum);
	    }
	    else if (ADCL_CLOSEST == ADCL_hist_predictor) {
		ADCL_printf("#%d A solution to the problem is predicted from the closest history entry, winner is %d\n",
			    e->em_topo->t_rank, e->em_hist->h_wfnum);
	    }
	}
	else if ( ADCL_UNEQUAL == ret ) {
	    ADCL_printf("No prediction can be done for this PS with the current history file\n");
	}
    }
    
exit:

    e->em_explored_hist = 0;
    return ret;
}

/* Function to add a hist object to the history file */
static void hist_add_to_file( ADCL_hist_t* hist, ADCL_emethod_t *e )
{
    int rank, nhist;
    FILE *fp;
    int nchar = 80, nch;
    char *line = NULL;

    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    /* Only rank 0 process will be in charge of writing in the history file */
    if ( 0 == rank ) {
        /* Make sure that history functions are registered */
        if( NULL == e->em_orgfnctset->fs_hist_functions ) {
	    return;
	}
	else {
             /* Make sure that history writer function is registered */
            if( NULL == e->em_orgfnctset->fs_hist_functions->hf_writer ) {
		return;
	    }
        }
        /* Check if the file exists */
        fp = fopen ("hist.adcl", "r");
        if(NULL == fp) {
            /* First entry ever */
            fp = fopen ("hist.adcl", "w");
            fprintf ( fp, "<?xml version=\"1.0\" ?>\n<?xml-stylesheet"
                      " type=\"text/xsl\" href=\"ADCL.xsl\"?>\n<ADCL>\n" );
            fprintf ( fp, "  <NUM>1</NUM>\n" );
	}
	else {
            /* Close in read mode */
            fclose(fp);
            /* Open in read write mode */
            fp = fopen ("hist.adcl", "r+");
            line = (char *)malloc( nchar * sizeof(char) );
            /* Read the XML file line by line */
            fgets( line, nchar, fp ); /* XML header lines */
            fgets( line, nchar, fp );
            fgets( line, nchar, fp ); /* ADCL Tag */
            fgets( line, nchar, fp );
            nch = strlen(line);
            get_int_data_from_xml ( line, &nhist );
            fseek(fp, -nch, SEEK_CUR);
            fprintf ( fp, "  <NUM>%d</NUM>\n", nhist+1 );
            fseek(fp, -7, SEEK_END);
	}
        /* Use the user defined writing function of the according function set */
        e->em_orgfnctset->fs_hist_functions->hf_writer(fp, hist);
        /* End of file */
        fprintf ( fp, "</ADCL>" );
        /* Close the file */
        fclose ( fp );
    }
    return;
}

static int hist_check_for_ident(ADCL_emethod_t *e, ADCL_hist_t *hist)
{
    ADCL_topology_t *topo = e->em_topo;
    ADCL_vector_t   *vec  = e->em_vec;
    int size = topo->t_size;
    int *dims, *periods, *coords;
    int i, found = -1;
    int ret = ADCL_UNEQUAL;

    /* Case of a NULL vector object */
    if ( (ADCL_VECTOR_NULL == vec) && (0 != hist->h_vndims)) {
        return ret;
    }
    /* Case of a NULL topology object */
    if ( (ADCL_VECTOR_NULL == topo) && (0 != hist->h_tndims)) {
        return ret;
    }

    /* Check for number of dimensions */
    if ( ( topo->t_ndims    == hist->h_tndims  ) &&
         ( vec->v_ndims     == hist->h_vndims  ) &&
         ( vec->v_nc        == hist->h_nc      ) &&
         ( vec->v_map->m_vectype == hist->h_vectype ) &&
         ( vec->v_map->m_hwidth  == hist->h_hwidth ) &&
         ( vec->v_map->m_op      == hist->h_op ) &&
         ( vec->v_map->m_inplace == hist->h_inplace ) &&
         ( 0 == strncmp (hist->h_fsname,
                         e->em_orgfnctset->fs_name,
                         strlen(e->em_orgfnctset->fs_name))) ) {
        found = 0;
        periods = (int *)malloc( topo->t_ndims * sizeof(int) );
        dims = (int *)malloc( topo->t_ndims * sizeof(int) );
        coords = (int *)malloc( topo->t_ndims * sizeof(int) );
        /* Check for identical logical topology dimensions */
        MPI_Cart_get ( topo->t_comm, topo->t_ndims, dims, periods, coords );
        for ( i=0; i<topo->t_ndims; i++ ) {
            if ( periods[i] != hist->h_tperiods[i] ) {
                found = -1;
                break;
            }
        }
        free( periods );
        free( dims );
        free( coords );
        if(found == -1) {
	    return ret;
	}
        /* Check for identical vector/data dimensions */
        for ( i=0 ; i<vec->v_ndims; i++ ){
            if ( vec->v_dims[i] != hist->h_vdims[i] ) {
                return ret;
            }
        }
        /* Check for identical vmap */
        if ((NULL != vec->v_map->m_rcnts) && (NULL != vec->v_map->m_displ)) {
            if ( hist->h_np != size ) {
	    /* So far we don't worry about np, to be studied later on in details */
               // return ret;
            }
            /* Check for identical rcnts and displ */
            for ( i=0 ; i<topo->t_size; i++ ){
                if ( ( vec->v_map->m_rcnts[i] != hist->h_rcnts[i] ) ||
                     ( vec->v_map->m_displ[i] != hist->h_displ[i] ) ) {

		    return ret;
                }
            }
        }
	else{
            /* Perhaps we should check that hist vmap data are zeros */
	}
    }

    if ( found != -1 ) {
        ret = ADCL_IDENT;
        ADCL_printf("#%d An identical problem/solution is found, winner is %s \n",
                    topo->t_rank, hist->h_wfname);
    }
    /* Return */
    return ret;
}

/* Function to classify the implementation to best and worst according to perf res and perf_win */
static int hist_classify_implementations( double *elapsed_time, int *impl_class, int nb_of_impl, int perf_win )
{
    int i, cnt, best_impl;
    double best_threshold;

    /* Initialization */
    cnt = 0;
    best_impl = 0;
    best_threshold = 1 + (perf_win/100.00);
    /* Searching for the best performing implementation */
    /* Might be done more efficiently later on */
    for(i=1; i<nb_of_impl; i++) {
        if(elapsed_time[i] < elapsed_time[best_impl]) {
            best_impl = i;
	}
    }
    /* Classification of the implementations */
    for(i=0; i<nb_of_impl; i++) {
        if(elapsed_time[i]/elapsed_time[best_impl] <= best_threshold) {
            impl_class[i] = ADCL_BEST_CLASS;
            cnt ++;
	}  
        else {
            impl_class[i] = ADCL_WORST_CLASS;
	}
    }

    return cnt;// TBD check if we really need that
}

/* A function that filter an array of zeros and ones according to a window size */
static void filter_array( int *relation, int num_sizes )
{
    int i, j, cnt;
    int p = ADCL_SMOOTH_WIN/2;
    int *tmp;

    /* Allocate a temporary buffer for a copy of the original array */
    tmp = (int *)malloc(num_sizes*sizeof(int));
    memcpy(tmp, relation, num_sizes*sizeof(int));
    /* Filtering operation */
    for(i=p+1; i<num_sizes-p;i++) {
        cnt = 0;
        for (j=(i-p); j<=(i+p); j++) {
            if (1 == tmp[j]){
                cnt ++;
            }
        }
        if(cnt > p) {
            relation[i] = 1;
        }
    }
    /* Free allocated memory */
    free(tmp);
    return;
}

/* Function to find dmax given an array of distances and relations */
static double hist_find_dmax(double *distance, int *relation, int num_sizes )
{
    int i, r, k, extendable, bound, last_swap;
    double d, dmax;

    bound = num_sizes-1;
    dmax = 0;

    /* Sort distance array and move relation array with it */
    while (bound) {
      last_swap = 0;
      for ( k=0; k<bound; k++ ) {
         d = distance[k]; /* t is a maximum of A[0]..A[k] */
         r = relation[k];
         if ( d > distance[k+1] ) {
           distance[k] = distance[k+1];
           distance[k+1] = d; /*swap*/
           relation[k] = relation[k+1];
           relation[k+1] = r; /*swap*/           
           last_swap = k; /* mark the last swap position */
         }
      }
      bound=last_swap;
    }
#ifdef ADCL_SMOOTH_HIST
    /* Filtering of the relation array */
    filter_array( relation, num_sizes );
#endif
    /* searching the first 0 in the sorted and filtered array */
    extendable = 1;
    for (i=0; i<num_sizes; i++) {
        if ((relation[i]==1) && (extendable==1)) {
            dmax = distance[i];
        }
        if(relation[i] == 0) {
            extendable = 0;
	}
    }
    return dmax;
}

/* Get an integer data from XML line */
int get_int_data_from_xml (char *str, int *res)
{
    char *n, *p;
    char *ext;
    int num;
    int ret = ADCL_ERROR_INTERNAL;
    n = strstr (str,">");
    p = strstr (str,"/");
    num = p-n-2;
    if ( NULL != n && NULL != p && num>0 ) {
        ext = (char *)calloc( num, sizeof(char) );
        strncpy ( ext, n+1, num );
        *res = atoi( ext );
        free( ext );
        ret = ADCL_SUCCESS;
    }
    return ret;
}

/* Get a string data from XML line */
int get_str_data_from_xml (char *str, char **dest)
{
    char *n, *p;
    int num;
    int ret = ADCL_ERROR_INTERNAL;
    n = strstr (str,">");
    p = strstr (str,"/");
    if ( NULL != n && NULL != p ) {
        num = p-n-2;
        (*dest) = (char *)calloc( num, sizeof(char) );
        strncpy ((*dest), n+1, num);
        ret = ADCL_SUCCESS;
    }
    return ret;
}
