/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef __ADCL_ATTRIBUTE_H__
#define __ADCL_ATTRIBUTE_H__

#if ADCL_DUMMY_MPI
#include "ADCL_dummy_mpi.h"
#else
#include "mpi.h"
#endif

#define ADCL_ATTR_NOT_SET   -1  /* attributes not set*/
#define ADCL_ATTR_NEW_BLOCK -2  /* signal that we start a new block.
                                   Used in the performance hypothesis v2 */


struct ADCL_attribute_s{
    int              a_id; /* id of the object */
    int          a_findex; /* index of this object in the fortran array */
    int          a_refcnt; /* reference counter of this object */
    int      a_maxnvalues; /* no. of different values this attribute can have */
    int         *a_values; /* list of potential values for this attribute */
    char **a_values_names; /* list of names of the attribute values */
    char     *a_attr_name; /* name of the attribute */
};
typedef struct ADCL_attribute_s ADCL_attribute_t;
extern ADCL_array_t *ADCL_attribute_farray;

int ADCL_attribute_create ( int maxnvalues, int *array_of_values, char **values_names,
                            char *attr_name, ADCL_attribute_t **attribute);
int ADCL_attribute_free   ( ADCL_attribute_t **attribute);
int ADCL_attribute_dup ( ADCL_attribute_t *org, ADCL_attribute_t **copy );
int ADCL_attribute_get_nextval ( ADCL_attribute_t *attr, int val );
int ADCL_attribute_get_val ( ADCL_attribute_t *attr, int attrval_pos );
int ADCL_attribute_get_pos ( ADCL_attribute_t *attr, int attrval );


struct ADCL_attrset_s{
    int                   as_id; /* id of the object */
    int               as_findex; /* index of this object in the fortran array */
    int               as_refcnt; /* reference counter of this object */
    int               as_maxnum; /* no. of attributes contained in this set */
    int       *as_attrs_baseval; /* array containing the first values of each attribute
                                    in this attribute set */
    int        *as_attrs_maxval; /* array containing the last values of each attribute
                                    in this attribute set */
    int        *as_attrs_numval; /* array containing the num of different values for each attribute
                                    in this attribute set */
    ADCL_attribute_t **as_attrs; /* array of ADCL_attributes in this set */
};
typedef struct ADCL_attrset_s ADCL_attrset_t;
extern ADCL_array_t *ADCL_attrset_farray;

int ADCL_attrset_create ( int maxnum, ADCL_attribute_t **array_of_attributes,
                          ADCL_attrset_t **attrset );
int ADCL_attrset_free ( ADCL_attrset_t **attrset );
int ADCL_attrset_dup ( ADCL_attrset_t *org, ADCL_attrset_t **copy );
int ADCL_attrset_get_pos ( ADCL_attrset_t *attrset, ADCL_attribute_t *attr );

#endif /* __ADCL_ATTRIBUTE_H__ */
