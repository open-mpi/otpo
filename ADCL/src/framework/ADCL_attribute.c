/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"

ADCL_array_t *ADCL_attribute_farray;
static int ADCL_attribute_local_counter=0;

ADCL_array_t *ADCL_attrset_farray;
static int ADCL_attrset_local_counter=0;

int ADCL_attribute_create ( int maxnvalues, int *array_of_values, char **values_names,
                            char *attr_name, ADCL_attribute_t **attribute)
{
    int i, ret = ADCL_SUCCESS;
    ADCL_attribute_t *newattribute=NULL;

    newattribute = ( ADCL_attribute_t *) calloc (1, sizeof (ADCL_attribute_t));
    if ( NULL == newattribute ) {
        return ADCL_NO_MEMORY;
    }

    newattribute->a_id = ADCL_attribute_local_counter++;
    ADCL_array_get_next_free_pos ( ADCL_attribute_farray, &(newattribute->a_findex));
    ADCL_array_set_element ( ADCL_attribute_farray, newattribute->a_findex,
                 newattribute->a_id, newattribute );

    newattribute->a_refcnt     = 1;
    newattribute->a_maxnvalues = maxnvalues;
    newattribute->a_values = (int *) malloc ( maxnvalues * sizeof (int));
    if ( NULL == newattribute->a_values ) {
        ret = ADCL_NO_MEMORY;
        goto exit;
    }

    memcpy ( newattribute->a_values, array_of_values, maxnvalues*sizeof(int));
    if ( NULL != values_names ) {
        newattribute->a_values_names = (char **)malloc( maxnvalues * sizeof(char*) );
        if ( NULL == newattribute->a_values_names ) {
            ret = ADCL_NO_MEMORY;
            goto exit;
        }
        for ( i=0; i<maxnvalues; i++ ) {
            if ( NULL != values_names[i] ) {
                newattribute->a_values_names[i] = strdup ( values_names[i] );
            }
        }
    }
    if ( NULL != attr_name ) {
        newattribute->a_attr_name = strdup ( attr_name );
    }
exit:
    if ( ret != ADCL_SUCCESS ) {
        if ( NULL != newattribute->a_values ) {
            free ( newattribute->a_values );
        }
        ADCL_array_remove_element ( ADCL_attribute_farray, newattribute->a_findex );
        free ( newattribute );
    }

    *attribute = newattribute;
    return ret;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
int ADCL_attribute_dup ( ADCL_attribute_t *org, ADCL_attribute_t **copy )
{
    int i, ret = ADCL_SUCCESS;
    ADCL_attribute_t *newattribute=NULL;

    newattribute = ( ADCL_attribute_t *) calloc (1, sizeof (ADCL_attribute_t));
    if ( NULL == newattribute ) {
        return ADCL_NO_MEMORY;
    }

    newattribute->a_id         = ADCL_attribute_local_counter++;
    newattribute->a_findex     = -1;
    newattribute->a_refcnt     = 1;
    newattribute->a_maxnvalues = org->a_maxnvalues;
    newattribute->a_values     = (int *) malloc ( org->a_maxnvalues * sizeof (int));
    if ( NULL == newattribute->a_values ) {
        ret = ADCL_NO_MEMORY;
        goto exit;
    }

    memcpy ( newattribute->a_values, org->a_values, org->a_maxnvalues*sizeof(int));

    if ( NULL != org->a_values_names ) {
        newattribute->a_values_names = (char **)malloc( org->a_maxnvalues * sizeof(char*) );
        if ( NULL == newattribute->a_values_names ) {
            ret = ADCL_NO_MEMORY;
            goto exit;
        }
        for ( i=0; i<org->a_maxnvalues; i++ ) {
            if ( NULL != org->a_values_names[i] ) {
                newattribute->a_values_names[i] = strdup ( org->a_values_names[i] );
            }
        }
    }
    if ( NULL != org->a_attr_name ) {
        newattribute->a_attr_name = strdup ( org->a_attr_name );
    }
 exit:
    if ( ret != ADCL_SUCCESS ) {
        if ( NULL != newattribute->a_values ) {
            free ( newattribute->a_values );
        }
        free ( newattribute );
    }   
    *copy = newattribute;
    return ADCL_SUCCESS;
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
int ADCL_attribute_free ( ADCL_attribute_t **attribute)
{
    ADCL_attribute_t *tattribute=*attribute;
    int i;

    if ( NULL != tattribute ) {
        if ( NULL != tattribute->a_values ) {
            free (tattribute->a_values );
        }
        if ( tattribute->a_findex != -1 ) {
            ADCL_array_remove_element ( ADCL_attribute_farray, tattribute->a_findex );
        }
        if ( NULL != tattribute->a_values_names ) {
            for ( i=0; i<tattribute->a_maxnvalues; i++ ) {
                if ( NULL != tattribute->a_values_names[i] ) {
                    free ( tattribute->a_values_names[i] );
                }
            }
            free ( tattribute->a_values_names );
        }
        if ( NULL != tattribute->a_attr_name ) {
            free ( tattribute->a_attr_name );
        }
        free ( tattribute );
    }

    *attribute = ADCL_ATTRIBUTE_NULL;
    return ADCL_SUCCESS;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
int ADCL_attribute_get_nextval ( ADCL_attribute_t *attr, int val )
{
    int i, nextval=-1;
    for ( i=0; i< attr->a_maxnvalues; i++ ) {
        if ( attr->a_values[i] == val ) {
            nextval = i;
            break;
        }
    }

    if ( nextval != -1 && nextval != (attr->a_maxnvalues-1) ) {
        nextval = attr->a_values[i+1];
    }

    return nextval;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
int ADCL_attribute_get_val ( ADCL_attribute_t *attr, int attrval_pos )
{
    if ( attrval_pos < 0 || attrval_pos >= attr->a_maxnvalues ) {
        return ADCL_ERROR_INTERNAL;
    }

    return attr->a_values[attrval_pos];
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
int ADCL_attribute_get_pos ( ADCL_attribute_t *attr, int attrval )
{
    int i, pos=ADCL_ERROR_INTERNAL;

    for ( i=0; i< attr->a_maxnvalues; i++ ) {
        if ( attr->a_values[i] == attrval ) {
            pos = i;
            break;
        }
    }

    return pos;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
int ADCL_attrset_create ( int maxnum, ADCL_attribute_t **array_of_attributes,
                          ADCL_attrset_t **attrset)
{
    ADCL_attrset_t *newattrset=NULL;
    int i;

    newattrset = ( ADCL_attrset_t *) calloc (1, sizeof (ADCL_attrset_t));
    if ( NULL == newattrset ) {
        return ADCL_NO_MEMORY;
    }

    newattrset->as_id = ADCL_attrset_local_counter++;
    ADCL_array_get_next_free_pos ( ADCL_attrset_farray, &(newattrset->as_findex));
    ADCL_array_set_element ( ADCL_attrset_farray, newattrset->as_findex,
                 newattrset->as_id, newattrset );

    newattrset->as_refcnt     = 1;
    newattrset->as_maxnum = maxnum;
    newattrset->as_attrs = (ADCL_attribute_t **) malloc ( maxnum * sizeof (ADCL_attribute_t *));
    if ( NULL == newattrset->as_attrs ) {
        free ( newattrset );
        return ADCL_NO_MEMORY;
    }

    memcpy ( newattrset->as_attrs, array_of_attributes, maxnum*sizeof(ADCL_attribute_t *));

    /* Determine the base and the last values for all attributes. That's required for
       some of the loops within the performance hypothesis code */
    newattrset->as_attrs_baseval = (int *) malloc ( maxnum * sizeof(int) );
    newattrset->as_attrs_maxval  = (int *) malloc ( maxnum * sizeof(int) );
    newattrset->as_attrs_numval  = (int *) malloc ( maxnum * sizeof(int) );
    if ( NULL == newattrset->as_attrs_baseval ||
         NULL == newattrset->as_attrs_maxval ||
         NULL == newattrset->as_attrs_numval ) {
        free ( newattrset->as_attrs);
        free ( newattrset );
        return ADCL_NO_MEMORY;
    }

    for ( i=0; i< maxnum; i++ ) {
        newattrset->as_attrs_baseval[i] = array_of_attributes[i]->a_values[0];
        newattrset->as_attrs_maxval[i]  = 
            array_of_attributes[i]->a_values[array_of_attributes[i]->a_maxnvalues-1];
        newattrset->as_attrs_numval[i]  = array_of_attributes[i]->a_maxnvalues;
    }

    *attrset = newattrset;
    return ADCL_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
int ADCL_attrset_dup ( ADCL_attrset_t *org, ADCL_attrset_t **copy )
{
    ADCL_attrset_t *newattrset=NULL;
    int i;

    newattrset = ( ADCL_attrset_t *) calloc (1, sizeof (ADCL_attrset_t));
    if ( NULL == newattrset ) {
        return ADCL_NO_MEMORY;
    }

    newattrset->as_id     = ADCL_attrset_local_counter++;
    newattrset->as_findex = -1;
    newattrset->as_refcnt = 1;
    newattrset->as_maxnum = org->as_maxnum;
    newattrset->as_attrs = (ADCL_attribute_t **) malloc ( org->as_maxnum * sizeof (ADCL_attribute_t *));
    if ( NULL == newattrset->as_attrs ) {
        free ( newattrset );
        return ADCL_NO_MEMORY;
    }

    /* Determine the base and the last values for all attributes. That's required for
       some of the loops within the performance hypothesis code */
    newattrset->as_attrs_baseval = (int *) malloc ( org->as_maxnum * sizeof(int) );
    newattrset->as_attrs_maxval  = (int *) malloc ( org->as_maxnum * sizeof(int) );
    newattrset->as_attrs_numval  = (int *) malloc ( org->as_maxnum * sizeof(int) );
    if ( NULL == newattrset->as_attrs_baseval ||
         NULL == newattrset->as_attrs_maxval  ||
         NULL == newattrset->as_attrs_numval ) {
        free ( newattrset->as_attrs);
        free ( newattrset );
        return ADCL_NO_MEMORY;
    }

    memcpy ( newattrset->as_attrs_baseval, org->as_attrs_baseval, org->as_maxnum * sizeof(int));
    memcpy ( newattrset->as_attrs_maxval, org->as_attrs_maxval, org->as_maxnum * sizeof(int));
    memcpy ( newattrset->as_attrs_numval, org->as_attrs_numval, org->as_maxnum * sizeof(int));

    for ( i=0; i< org->as_maxnum; i++ ) {
        ADCL_attribute_dup ( org->as_attrs[i], &newattrset->as_attrs[i]);
    }

    *copy = newattrset;
    return ADCL_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
int ADCL_attrset_free ( ADCL_attrset_t **attrset)
{
    ADCL_attrset_t *tattrset=*attrset;
    int i;

    if ( NULL != tattrset ) {

        if ( NULL != tattrset->as_attrs_baseval ) {
            free ( tattrset->as_attrs_baseval) ;
        }
        
        if ( NULL != tattrset->as_attrs_maxval ) {
            free ( tattrset->as_attrs_maxval) ;
        }
        
        if ( NULL != tattrset->as_attrs_numval ) {
            free ( tattrset->as_attrs_numval) ;
        }
        
        if ( tattrset->as_findex != -1 ) {
            ADCL_array_remove_element ( ADCL_attrset_farray, tattrset->as_findex);
            if ( NULL != tattrset->as_attrs ) {
                free ( tattrset->as_attrs );
            }
        }
        else {
            /* These attributes are the result of an ADCL_attrset_dup
               operation and have to be freed together with the duplicated
               attrset */
            if ( NULL != tattrset->as_attrs ) {
                for ( i=0; i< tattrset->as_maxnum; i++ ) {
                    ADCL_attribute_free ( &tattrset->as_attrs[i] );
                }
                free ( tattrset->as_attrs );
            }
        }
        free ( tattrset );
    }

    *attrset = ADCL_ATTRSET_NULL;
    return ADCL_SUCCESS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
int ADCL_attrset_get_pos ( ADCL_attrset_t *attrset, ADCL_attribute_t *attr )
{
    int i, found =0;
    for ( i=0; i< attrset->as_maxnum; i++ ) {
        if ( attrset->as_attrs[i] == attr ) {
            found = 1;
            break;
        }
    }

    if ( 0 == found ) {
        i = -1;
    }

    return i;
}
