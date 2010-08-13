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
#include <string.h>

ADCL_array_t *ADCL_function_farray;
static int ADCL_local_function_counter=0;

ADCL_array_t *ADCL_fnctset_farray;
static int ADCL_local_fnctset_counter=0;

static int get_next_attr_combination ( ADCL_attrset_t *attrset, int *attr_val_list );

static int check_excluded( int* , int , int**, int );
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
int ADCL_function_create_async ( ADCL_work_fnct_ptr *init_fnct,
                                 ADCL_work_fnct_ptr *wait_fnct,
                                 ADCL_attrset_t * attrset,
                                 int *array_of_attrvalues, char *name,
                                 ADCL_function_t **fnct)
{
    ADCL_function_t *newfunction;
    int ret=ADCL_SUCCESS;

    newfunction = ( ADCL_function_t *) calloc (1, sizeof (ADCL_function_t));
    if ( NULL == newfunction ) {
        return ADCL_NO_MEMORY;
    }

    newfunction->f_id = ADCL_local_function_counter++;
    ADCL_array_get_next_free_pos ( ADCL_function_farray, &(newfunction->f_findex));
    ADCL_array_set_element ( ADCL_function_farray, newfunction->f_findex,
                             newfunction->f_id, newfunction );

    newfunction->f_attrset = attrset;
    if (ADCL_ATTRSET_NULL != attrset ) {
        newfunction->f_attrvals = (int *) malloc ( sizeof(int) * attrset->as_maxnum );
        if ( NULL == newfunction->f_attrvals ) {
            ret = ADCL_NO_MEMORY;
            goto exit;
        }
        memcpy (newfunction->f_attrvals, array_of_attrvalues, attrset->as_maxnum*sizeof(int));
    }

    if ( NULL != name ) {
        newfunction->f_name = strdup ( name );
    }

    newfunction->f_iptr = init_fnct;
    newfunction->f_wptr = wait_fnct;
    if ( NULL != wait_fnct && ADCL_FUNCTION_NULL != wait_fnct ) {
        newfunction->f_db   = 1; /* true */
    }

 exit:
    if ( ret != ADCL_SUCCESS ) {
        ADCL_array_remove_element ( ADCL_function_farray, newfunction->f_findex );

        if ( NULL != newfunction->f_attrvals ) {
            free ( newfunction->f_attrvals );
        }
        if ( NULL != newfunction->f_name ) {
            free ( newfunction->f_name );
        }
        free ( newfunction );
    }

    *fnct = newfunction;
    return ret;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
int ADCL_function_free ( ADCL_function_t **fnct )
{
    ADCL_function_t *tfnct=*fnct;

    ADCL_array_remove_element ( ADCL_function_farray, tfnct->f_findex);

    if ( NULL != tfnct->f_attrvals ) {
        free ( tfnct->f_attrvals );
    }
    if ( NULL != tfnct->f_name ) {
        free ( tfnct->f_name );
    }
    free ( tfnct );

    *fnct = ADCL_FUNCTION_NULL;
    return ADCL_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
int ADCL_function_get_attrval ( ADCL_function_t *func, int attr_pos )
{
    return func->f_attrvals[attr_pos];
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

int ADCL_fnctset_create( int maxnum, ADCL_function_t **fncts, char *name, 
                         ADCL_fnctset_t **fnctset )
{
    int i, ret = ADCL_SUCCESS;
    ADCL_fnctset_t *newfnctset=NULL;

    newfnctset = ( ADCL_fnctset_t *) calloc (1, sizeof (ADCL_fnctset_t));
    if ( NULL == newfnctset ) {
        return ADCL_NO_MEMORY;
    }

    newfnctset->fs_id = ADCL_local_fnctset_counter++;
    ADCL_array_get_next_free_pos ( ADCL_fnctset_farray, &(newfnctset->fs_findex));
    ADCL_array_set_element ( ADCL_fnctset_farray, newfnctset->fs_findex,
                 newfnctset->fs_id, newfnctset );

    /* Make sure all functions have use the same attribute set */
    for (i=1; i<maxnum; i++ ) {
        if ( fncts[i]->f_attrset != fncts[0]->f_attrset ) {
            ADCL_printf( "Error Generating a function set: inconsistent attribute set across "
                         "multiple functions ");
            ret = ADCL_USER_ERROR;
            goto exit;
        }
    }
    newfnctset->fs_single_fnct = 0;
    newfnctset->fs_attrset = fncts[0]->f_attrset;
    newfnctset->fs_maxnum  = maxnum;
    newfnctset->fs_fptrs   = (ADCL_function_t **) calloc ( 1, maxnum*sizeof (ADCL_function_t *));
    if ( NULL == newfnctset->fs_fptrs ) {
        ret = ADCL_NO_MEMORY;
        goto exit;
    }
    memcpy ( newfnctset->fs_fptrs, fncts, maxnum * sizeof (ADCL_function_t *));
 
    if ( NULL != name ) {
        newfnctset->fs_name = strdup ( name );
    }

 exit:
    if ( ret != ADCL_SUCCESS ) {
        if ( NULL != newfnctset->fs_fptrs ) {
            free ( newfnctset->fs_fptrs );
        }

        if ( NULL != newfnctset->fs_name ) {
            free ( newfnctset->fs_name );
        }
        ADCL_array_remove_element ( ADCL_fnctset_farray, newfnctset->fs_findex);
        free ( newfnctset );
    }

    *fnctset = newfnctset;
    return ret;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/* Please note, memory for the copy argument is already allocated ! */
int ADCL_fnctset_dup ( ADCL_fnctset_t *org, ADCL_fnctset_t *copy )
{
    int ret = ADCL_SUCCESS;

    copy->fs_id      = ADCL_local_fnctset_counter++;
    copy->fs_findex  = -1; /* not set */

    if ( ADCL_ATTRSET_NULL == org->fs_attrset ) {
        copy->fs_attrset = ADCL_ATTRSET_NULL;
    }
    else {
        ADCL_attrset_dup ( org->fs_attrset, &copy->fs_attrset );
    }

    copy->fs_single_fnct = org->fs_single_fnct;
    copy->fs_maxnum  = org->fs_maxnum;
    copy->fs_fptrs   = (ADCL_function_t**)calloc(1,org->fs_maxnum*sizeof(ADCL_function_t*));
    if ( NULL == copy->fs_fptrs ) {
        ret = ADCL_NO_MEMORY;
        goto exit;
    }
    memcpy ( copy->fs_fptrs, org->fs_fptrs, org->fs_maxnum * sizeof (ADCL_function_t *));

    if ( NULL != org->fs_name ) {
        copy->fs_name = strdup ( org->fs_name );
    }

 exit:
    if ( ret != ADCL_SUCCESS ) {
        if ( NULL != copy->fs_fptrs ) {
            free ( copy->fs_fptrs );
        }
        
        if ( NULL != copy->fs_name ) {
            free ( copy->fs_name );
        }
    }

    return ret;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
int ADCL_fnctset_create_single ( ADCL_work_fnct_ptr *init_fnct,
                                 ADCL_work_fnct_ptr *wait_fnct,
                                 ADCL_attrset_t * attrset, char *name,
                                 int **without_attribute_combinations,
                                 int num_without_attribute_combinations,
                                 ADCL_fnctset_t **fnctset )
{
    int ret=ADCL_SUCCESS, new_attrs=ADCL_SUCCESS, i; 
    long long maxnum=1;
    int *attr_vals;
    ADCL_fnctset_t *newfnctset=NULL;
    int excluded;
    newfnctset = ( ADCL_fnctset_t *) calloc (1, sizeof (ADCL_fnctset_t));
    if ( NULL == newfnctset ) {
        return ADCL_NO_MEMORY;
    }

    newfnctset->fs_id = ADCL_local_fnctset_counter++;
    ADCL_array_get_next_free_pos ( ADCL_fnctset_farray, &(newfnctset->fs_findex));
    ADCL_array_set_element ( ADCL_fnctset_farray, newfnctset->fs_findex,
                             newfnctset->fs_id, newfnctset );
    newfnctset->fs_single_fnct = 1;
    newfnctset->fs_attrset = attrset;
    if ( NULL != name ) {
        newfnctset->fs_name = strdup ( name );
    }
    /* Calc the total number of functions */
    for ( i=0; i<attrset->as_maxnum; i++) {
        maxnum *= attrset->as_attrs_numval[i];
    }
    maxnum = maxnum - num_without_attribute_combinations;
    newfnctset->fs_maxnum  = maxnum;
    /* Memory allocation for the ADCL_functions */
    newfnctset->fs_fptrs   = (ADCL_function_t **) calloc ( 1, maxnum * sizeof (ADCL_function_t *));
    if ( NULL == newfnctset->fs_fptrs ) {
        ret = ADCL_NO_MEMORY;
        goto exit;
    }
    /* Memory allcation and Initialization of the attr values */
    attr_vals = (int *)malloc( attrset->as_maxnum*sizeof(int) );
    if ( NULL == attr_vals ) {
        ret = ADCL_NO_MEMORY;
        goto exit;
    }
    for ( i=0; i<attrset->as_maxnum; i++ ) {
        attr_vals[i] = attrset->as_attrs_baseval[i];
    }
    i = 0;
    excluded = 0;
    while ( ADCL_SUCCESS == new_attrs ) {
        if (0 != num_without_attribute_combinations) {
            excluded = check_excluded(attr_vals, attrset->as_maxnum, without_attribute_combinations,
                                      num_without_attribute_combinations);
        }
        if (!excluded) {
            ADCL_function_create_async ( init_fnct, wait_fnct , attrset,
                                         attr_vals, name, &newfnctset->fs_fptrs[i] );
            i++;
        }
        new_attrs = get_next_attr_combination ( attrset, attr_vals );        
    }
exit:
    if ( ret != ADCL_SUCCESS ) {
        if ( NULL != newfnctset->fs_fptrs ) {
            free ( newfnctset->fs_fptrs );
        }

        if ( NULL != newfnctset->fs_name ) {
            free ( newfnctset->fs_name );
        }
        ADCL_array_remove_element ( ADCL_fnctset_farray, newfnctset->fs_findex);
        free ( newfnctset );
    }

    *fnctset = newfnctset;
    return ret;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
int ADCL_fnctset_reg_hist_fnct ( ADCL_hist_functions_t *hist_functions, ADCL_fnctset_t *fnctset )
{

    /* Copy the pointer to the history functions st given by user */
    fnctset->fs_hist_functions = hist_functions;

}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
int ADCL_fnctset_free ( ADCL_fnctset_t **fnctset)
{
    int i;
    ADCL_fnctset_t *tfnctset=*fnctset;

    if ( NULL != tfnctset ) {
        if ( 1 == tfnctset->fs_single_fnct ) {
	    for ( i=0; i<tfnctset->fs_maxnum ;i++ ) {
                ADCL_function_free ( &tfnctset->fs_fptrs[i] );
            }
        }       
        if ( NULL != tfnctset->fs_fptrs ) {
            free ( tfnctset->fs_fptrs );
        }
        if ( -1 != tfnctset->fs_findex ) {
            ADCL_array_remove_element ( ADCL_fnctset_farray, tfnctset->fs_findex );
        }
        else {
            /* Free the attrset generated during the fnctset_dup operation */
            ADCL_attrset_free ( &tfnctset->fs_attrset );
        }

        if ( NULL != tfnctset->fs_name ) {
            free ( tfnctset->fs_name );
        }
        free ( tfnctset );
    }

    *fnctset = ADCL_FNCTSET_NULL;
    return ADCL_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
ADCL_function_t*  ADCL_fnctset_get_fnct_by_name ( ADCL_fnctset_t *fnctset, char *fname, int *fnum )
{
    ADCL_function_t* func;
    int i;
    for (i=0; i<fnctset->fs_maxnum; i++) {
        func = fnctset->fs_fptrs[i];
        if ( 0 == strncmp ( func->f_name, fname, strlen(fname))){
            *fnum = i;
            return func;
        }
    }
    return  ADCL_FUNCTION_NULL;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
int ADCL_fnctset_get_fnct_num ( ADCL_fnctset_t *fnctset, ADCL_function_t*fnct )
{
    int i=-1;
    for (i=0; i<fnctset->fs_maxnum; i++) {
        if(fnct == fnctset->fs_fptrs[i])
	{
	    return i;
	}
    }
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/* former ADCL_emethod_get_function_by_attrs */
int ADCL_fnctset_get_fnct_by_attrs ( ADCL_fnctset_t *fnctset,
                     int *attrval,
                     int *pos )
{
    int i, j, found;
    int ret=ADCL_NOT_FOUND;

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

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
int  ADCL_fnctset_shrink_by_attr ( ADCL_fnctset_t *fnctset, int attr_pos,
                                   int excluded_value)
{
    int i, j;

    for (i=0; i<fnctset->fs_maxnum; i++) {
        if ( fnctset->fs_fptrs[i]->f_attrvals[attr_pos] == excluded_value ) {
            ADCL_printf("#Removing function \n");
     	    fnctset->fs_maxnum--;
            for (j=i; j<(fnctset->fs_maxnum-1); j++) {
                /* move next functions from pos j+1 to pos j */
                fnctset->fs_fptrs[j] = fnctset->fs_fptrs[j+1];
            }
            i--;
        }
    }
    return  ADCL_SUCCESS;
}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static int get_next_attr_combination ( ADCL_attrset_t *attrset, int *attr_val_list )
{
    int i, ret = ADCL_SUCCESS;
    int thisval;

    for ( i = 0; i < attrset->as_maxnum; i++ ) {
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

static int check_excluded( int* attr_vals, int num_vals, 
                            int ** without_attribute_combinations,
                            int num_without_attribute_combinations )
{
    int i, j;
    for (i=0 ; i<num_without_attribute_combinations ; i++) {
        for (j=0 ; j<num_vals ; j++) {
            if (attr_vals[j] != without_attribute_combinations[i][j]) {
                break;
            }
        }
        if (j == num_vals) {
            return 1;
        }
    }
    return 0;
}
        
