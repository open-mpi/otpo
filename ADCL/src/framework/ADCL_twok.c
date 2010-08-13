/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "math.h"
#include "ADCL_internal.h"

/* Twok function set functions */
int get_twok_function_set(ADCL_twok_factorial_t *twok, ADCL_fnctset_t *org_fnctset, ADCL_fnctset_t **twok_fnctset );
void get_twok_functions( ADCL_twok_factorial_t *twok, ADCL_fnctset_t *org_fnctset, ADCL_function_t **fncts,
			 int level, int *current_function, int *attr_values);
void print_function_set(ADCL_fnctset_t *fnctset);
/* Generate Sign Table functions */
void generate_sign_table( ADCL_twok_factorial_t *twok );
void compute_sign_table(ADCL_twok_factorial_t *twok, int *flag, int start, int *comb_no);
void compute_sign_table_header(ADCL_twok_factorial_t *twok, int start, int *comb_no, int *temp);
void print_sign_table( ADCL_twok_factorial_t *twok );
/* Qs and SSTs */
void compute_Qs( ADCL_emethod_t *e );
void compute_SSTs( ADCL_twok_factorial_t *twok );
void print_SSTs( ADCL_twok_factorial_t *twok );
/* Reduce the function set according to the 2k results */
void reduce_fnctset( ADCL_emethod_t *e );
/* Get next function to be tested from remaining if there is one */
int get_from_remaining( ADCL_emethod_t *e );

/* Threshold to decide if an attribute is important for performance */
int ADCL_twok_threshold = 50;

int ADCL_twok_init ( ADCL_emethod_t *e )
{
    ADCL_twok_factorial_t *twok;
    ADCL_fnctset_t *org_fnctset = &(e->em_fnctset);

    /* Memory allocation for the data structure for 2k algo */
    e->em_twok = (ADCL_twok_factorial_t *)malloc(sizeof(ADCL_twok_factorial_t));
    /* Use twok as var name for easiness */
    twok = e->em_twok;
    /* Initialize the number 2k */
    twok->twok_num = pow(2, org_fnctset->fs_attrset->as_maxnum);
    /* Initialization of the 2k function set */
    get_twok_function_set(twok, org_fnctset, &twok->twok_fnctset);
    /* Initialize the first function to be tested as the first function
       in the twok function set */
    e->em_last = twok->twok_fncts_pos[0];
    twok->twok_next_pos = 1;
#ifdef TWOK_VERBOSE
    /* Print the 2k function set */
    print_function_set( twok->twok_fnctset );
#endif
    /* Generate the sign table */
    generate_sign_table( twok );
#ifdef TWOK_VERBOSE
    /* Print the sign table */
    print_sign_table( twok );
#endif
    return ADCL_SUCCESS;
}

/* Generates Two-k Function Set from an original function set */
int get_twok_function_set( ADCL_twok_factorial_t *twok, ADCL_fnctset_t *org_fnctset, ADCL_fnctset_t **twok_fnctset )
{
    int i,j, current_func;
    int *attr_values;
    char *name;
    ADCL_function_t **fncts;
    ADCL_attrset_t *attrset = org_fnctset->fs_attrset;

    /* 2k pointers to functions */
    fncts = (ADCL_function_t **)malloc(twok->twok_num * sizeof(ADCL_function_t *));
    /* 2k positions of the functions */
    twok->twok_fncts_pos = (int *)malloc(twok->twok_num * sizeof(int));
    /* 2k fs name */
    name=(char *)malloc(sizeof(char)*50);
    sprintf(name, "2k %s", org_fnctset->fs_name);
    current_func = 0;
    attr_values = (int *)malloc( twok->twok_num * sizeof(int) );
    get_twok_functions(twok, org_fnctset, fncts, attrset->as_maxnum-1, &current_func, attr_values);
    free(attr_values);
    /* We return the result of the 2k function set creation */
    return ADCL_fnctset_create( twok->twok_num, fncts, name, twok_fnctset );
}

void get_twok_functions( ADCL_twok_factorial_t *twok, ADCL_fnctset_t *org_fnctset, ADCL_function_t **fncts,
			 int level, int *current_function, int *attr_values)
{
    int i, j, pos, ret;
    int *attrvals;
    ADCL_attrset_t *attrset = org_fnctset->fs_attrset;

    for(i=0;i<2;i++) {
	if(i==0) {
	    attr_values[level] = -1;
	}
	else {
	    attr_values[level] = 1;
	}
	if(level>0) {
	    get_twok_functions(twok, org_fnctset, fncts, level-1, current_function, attr_values);
	}
	if(level==0) {
	    attrvals = (int *)malloc(attrset->as_maxnum * sizeof(int));
	    for(j=0; j<attrset->as_maxnum; j++)
	    {
		if( 1 == attr_values[j] ) {
		    attrvals[j] = attrset->as_attrs_maxval[j];
		}
		else { /* -1 */
		    attrvals[j] = attrset->as_attrs_baseval[j];
		}
	    }
	    ret = ADCL_fnctset_get_fnct_by_attrs ( org_fnctset, attrvals, &pos );
	    if ( ADCL_SUCCESS == ret ) {
		fncts[*current_function] = org_fnctset->fs_fptrs[pos];
#ifdef TWOK_VERBOSE
		printf("Pointer to function %d copied on position %d in 2k functionset\n", pos, *current_function);
#endif
		twok->twok_fncts_pos[*current_function] = pos;
	    }
	    else {
		printf("function not found\n");
	    }
	    free(attrvals);
	    (*current_function)++;
	}
    }
}

/* Generats Sign Table */
void generate_sign_table( ADCL_twok_factorial_t *twok )
{
    int i, j, k, m, comb_no;
    int *tmp; /* tmp buffer */
    ADCL_fnctset_t *fnctset = twok->twok_fnctset;
    /* Memeory allocation for the sign table and he labels */
    twok->twok_sign_table = (int **)malloc(twok->twok_num * sizeof(int *));
    for (i=0 ; i<twok->twok_num; i++) {
	twok->twok_sign_table[i] = (int *)malloc(twok->twok_num * sizeof(int));
    }
    twok->twok_labels = (int **) malloc(sizeof(int *) * twok->twok_num  );
    k = fnctset->fs_attrset->as_maxnum;
    for (i=0 ; i<twok->twok_num; i++) {
	twok->twok_labels[i] = (int *)calloc(k, sizeof(int));
    }
    tmp = (int *)malloc( k * sizeof(int) );
    /* Initialize the tmp buffer */
    for( i=0; i<k; i++ ) {
	tmp[i]=-1;
    }
    comb_no=1;
    compute_sign_table(twok, tmp, 0, &comb_no);
    comb_no=1;
    compute_sign_table_header(twok, 0, &comb_no, tmp );
    free(tmp);
}

void compute_sign_table( ADCL_twok_factorial_t *twok, int *flag, int start, int *comb_no )
{
    int i, j, k, val;
    ADCL_fnctset_t *fnctset =  twok->twok_fnctset;
    int length = fnctset->fs_attrset->as_maxnum;
    for( i=start; i<length; i++ ) {
	flag[i] = 1;
	for( j=0; j<twok->twok_num; j++ ) {
	    val = 1;
	    for( k=0; k<length; k++ ) {
		if( flag[k]==1 ) {
		    if(fnctset->fs_fptrs[j]->f_attrvals[k] == fnctset->fs_attrset->as_attrs_baseval[k]) {
			val*=-1;
		    }
		}
	    }
	    twok->twok_sign_table[j][0] = 1;
	    twok->twok_sign_table[j][*comb_no] = val;
	}
	(*comb_no)++;
	compute_sign_table(twok, flag, i+1, comb_no);
	flag[i]=-1;
    }
}

void compute_sign_table_header(ADCL_twok_factorial_t *twok, int start, int *comb_no, int *temp)
{
    int i, j;
    int length = twok->twok_fnctset->fs_attrset->as_maxnum;

    for( i=start; i<length; i++ ) {
	if(i>0) { 
	    for( j=0; j<i; j++ ) {
		twok->twok_labels[*comb_no][j] = temp[j];
	    }
	}
	twok->twok_labels[*comb_no][i] = 1;
	temp[i] = 1;
	(*comb_no)++;
	if(i<length-1) {
	    compute_sign_table_header(twok, i+1, comb_no, temp);
	}
	temp[i]=0;
    }
}

int ADCL_twok_get_next( ADCL_emethod_t *e )
{
    static int twokdone = 0;
    static int next = 0;
    ADCL_twok_factorial_t *twok = e->em_twok;
    int i, pos, rank;

    if(!twokdone) {
        /* A brute force like search on the 2k function set */
	next = twok->twok_fncts_pos[twok->twok_next_pos];
	twok->twok_next_pos++;
	/* All 2k functions were evaluated */
	if(twok->twok_num < twok->twok_next_pos) {
	    twokdone = 1;
#ifdef TWOK_VERBOSE
	    printf("Finished testing all 2k functions, proceed to twok logic\n");
#endif
	    if ( ADCL_TOPOLOGY_NULL == e->em_topo ) {
		rank = 0; 
	    }
	    else {
		rank = e->em_topo->t_rank;
	    }
            /* We don't need the winner but just filtering and globalizing the results so far */
	    twok->twok_best = ADCL_emethods_get_winner (e, e->em_topo->t_comm, e->em_fnctset.fs_maxnum);
#ifdef TWOK_VERBOSE
            /* Print performance results */
	    for (i=0; (i<twok->twok_num)&&(rank ==0); i++) {
		pos = twok->twok_fncts_pos[i];
		printf("Function %d perf: unfiltered=%f , filtered=%f, outliers=%f\n",
		       pos, e->em_stats[pos]->s_gpts[0],e->em_stats[pos]->s_gpts[1],e->em_stats[pos]->s_gpts[2] );
	    }
	    printf("Best function in twok is :%d\n", twok->twok_best );
#endif
	    /* Compute the Q's */
	    compute_Qs( e );
	    /* Compute SST's */
	    compute_SSTs( twok );
#ifdef TWOK_VERBOSE
	    /* Printing the results */
	    if ( 0 == rank ) {
		print_SSTs( twok );
	    } 
#endif
            /* Reduce the function set according to the 2k results */
	    reduce_fnctset( e );
	    /* Get next function to be tested from the remaining */
	    next = get_from_remaining( e );
	}
    }
    else {
	/* Get next function to be tested from the remaining */
	next = get_from_remaining( e );
    }

    return next;
}

/* We actually compute the Qs * 2k since it needs less computation */
void compute_Qs( ADCL_emethod_t *e )
{
    int i, j, f, pos;
    ADCL_twok_factorial_t *twok = e->em_twok;
    ADCL_fnctset_t *fnctset = twok->twok_fnctset;

    /* Memory allocation */
    twok->twok_q = (double *)calloc(twok->twok_num, sizeof(double));
    /* Computing the Qs by column multiplication */
    f = e->em_filtering;
    for ( i=0; i<twok->twok_num; i++ ) {
        for (j=0; j<twok->twok_num; j++) {
	    pos = twok->twok_fncts_pos[j];
            twok->twok_q[i] += twok->twok_sign_table[j][i] * e->em_stats[pos]->s_gpts[f];
        }
    }
    return;
}

void compute_SSTs( ADCL_twok_factorial_t *twok )
{
    int i;
    double sst_max;

    /* Memory allocation and Initializations */
    twok->twok_sst = (double *)calloc(twok->twok_num, sizeof(double));
    sst_max = twok->twok_sst[0];
    /* Computing SST's and the global SST */
    for(i=1; i<twok->twok_num; i++) {
        twok->twok_sst[i] = pow ( twok->twok_q[i], 2 );
	if(twok->twok_sst[i] > sst_max) {
	    sst_max = twok->twok_sst[i];
	}
    }
    /* Relative SST's in % */
    for(i=1; i<twok->twok_num; i++) {
        twok->twok_sst[i]=  (twok->twok_sst[i]/sst_max) * 100;
    }
    return;
}

void reduce_fnctset( ADCL_emethod_t *e )
{
    ADCL_twok_factorial_t *twok = e->em_twok;
    ADCL_fnctset_t *fnctset = twok->twok_fnctset;
    int i, j, sum, final_pos=0;
    int attr_index=0;

    for (i=1; i<twok->twok_num;i++) {
	sum = 0;
	for( j=0; j<fnctset->fs_attrset->as_maxnum; j++) {
	    if( 1 == twok->twok_labels[i][j] ) {
		final_pos = i;
		attr_index = j;
		sum++;
	    }
       	}
	if( 1 == sum ) { /* Meaning the impact of a single factor */
	    if(twok->twok_sst[final_pos] < ADCL_twok_threshold ) {
#ifdef TWOK_VERBOSE
                /* verbosity */
		printf("Attribute %d is not important for performance\n", attr_index);
#endif
		if(e->em_orgfnctset->fs_fptrs[twok->twok_best]->f_attrvals[attr_index] == fnctset->fs_attrset->as_attrs_baseval[attr_index]) {
		    ADCL_hypothesis_shrinklist_byattr ( e, attr_index, fnctset->fs_attrset->as_attrs_baseval[attr_index] );
		}
		else {
		    ADCL_hypothesis_shrinklist_byattr ( e, attr_index, fnctset->fs_attrset->as_attrs_maxval[attr_index] );
		}
	    }
	}
    }
#ifdef TWOK_VERBOSE
    printf("The reduced function set:\n");
    print_function_set(&e->em_fnctset);
#endif
}

/* Get next function to be tested from remaining if there is one */
int get_from_remaining( ADCL_emethod_t *e )
{
    int i, next;
    next = ADCL_EVAL_DONE;
    for(i=0; i<e->em_fnctset.fs_maxnum; i++) {
	if ( ADCL_STAT_IS_TESTED ( e->em_stats[i]) ) {
	    /* this function has already been evaluated */
	    continue;
	}
	else {
	    return i;
	}
    }
    return next;
}

void print_function(ADCL_function_t *func)
{
    int i;
    printf("\nFunction Name: %s\n", func->f_name);
    for(i=0;i<func->f_attrset->as_maxnum;i++) {
        printf("%d\t",func->f_attrvals[i]);
    }
}

void print_function_set(ADCL_fnctset_t *fnctset)
{
    int i;
    printf("\n");
    printf("Function Set Name: %s", fnctset->fs_name);
    for(i=0;i<(fnctset->fs_maxnum);i++) {
	printf("\n");
	print_function(fnctset->fs_fptrs[i]);
    }
    printf("\n");
}

void print_sign_table(ADCL_twok_factorial_t *twok )
{
    int i, j ,k;
    /* Initialization of k */
    k = twok->twok_fnctset->fs_attrset->as_maxnum;

    /* Print Header */
    printf("\n \t");
    for( i=1; i<twok->twok_num; i++) {
	for(j=0;j<k;j++) {
	    printf("%d",twok->twok_labels[i][j]);
	}
	printf("\t");
    }
    printf("\n");
    printf("------------------------------------------------------------------------------------");
    printf("\n");
    /* Printing the actual sign table */
    for(i=0;i<twok->twok_num;i++) {
	for(j=0; j<twok->twok_num; j++) {
	    printf("%d\t",twok->twok_sign_table[i][j]);
	}
	printf("\n");
    }
}

void print_SSTs(ADCL_twok_factorial_t *twok)
{
    int i;
    printf("\n");
    for(i=0; i<twok->twok_num; i++) {
        printf("\t%lf",twok->twok_sst[i]);
    }
    printf("\n");
}
