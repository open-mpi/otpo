/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL.h"
#include "ADCL_internal.h"

int ADCL_Attribute_create ( int maxnvalues, int *array_of_values, char **values_names,
                            char *attr_name, ADCL_Attribute *attr )
{
    if ( 0 >= maxnvalues ) {
        return ADCL_INVALID_ARG;
    }
    if ( NULL == array_of_values ) {
        return ADCL_INVALID_ARG;
    }
    if ( NULL == attr ) {
        return ADCL_INVALID_ARG;
    }

    return ADCL_attribute_create ( maxnvalues, array_of_values, values_names,
                                   attr_name, attr );
}

int ADCL_Attribute_free   ( ADCL_Attribute *attr )
{

    if ( NULL == attr ) {
        return ADCL_INVALID_ARG;
    }
    if ( (*attr)->a_id < 0 ) {
        return ADCL_INVALID_ATTRIBUTE;
    }

    return ADCL_attribute_free ( attr );
}

int ADCL_Attrset_create ( int maxnum, ADCL_Attribute *array_of_attributes,
                          ADCL_Attrset *attrset )
{
    int i;

    if ( 0 >=  maxnum ) {
        return ADCL_INVALID_ARG;
    }

    if ( NULL == array_of_attributes ) {
        return ADCL_INVALID_ARG;
    }

    for (i=0; i< maxnum; i++ ) {
        if ( array_of_attributes[i]->a_id < 0 ) {
            return ADCL_INVALID_ATTRIBUTE;
        }
    }

    return ADCL_attrset_create ( maxnum, array_of_attributes, attrset);
}

int ADCL_Attrset_free ( ADCL_Attrset *attrset )
{

    if ( NULL == attrset ) {
        return ADCL_INVALID_ARG;
    }
    if ( (*attrset)->as_id < 0 ) {
        return ADCL_INVALID_ATTRSET;
    }

    return ADCL_attrset_free ( attrset );
}

