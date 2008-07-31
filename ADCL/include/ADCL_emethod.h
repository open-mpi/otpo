/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef __ADCL_EMETHOD_H__
#define __ADCL_EMETHOD_H__

#include "ADCL_sysconfig.h"
#include "ADCL_topology.h"
#include "ADCL_vector.h"
#include "ADCL_statistics.h"
#include "ADCL_function.h"


struct ADCL_emethod_s {
    int                     em_id; /* unique identifier */
    int                  em_rfcnt; /* reference counter */
    int                 em_findex; /* index of this object in the fortran array */
    int                  em_state; /* state of the object */
    int                   em_last; /* last element given out */
    int         em_perfhypothesis; /* use performance hypothesis for the runtime
                                      optimization? */
    int          em_explored_data; /* number of explored data objects for identical
                                      and similar problems */
    ADCL_topology_t      *em_topo; /* pointer to topology object */
    ADCL_vector_t         *em_vec; /* pointer to vector object. Only the size of the
                                      data array is really required, not the buffer
                                      pointers. */
    ADCL_statistics_t  **em_stats; /* array of statistics objects containing the
                                      measurements etc. Length of the array is equal
                                      to the no. of fncts registered in the fnctset */
    ADCL_fnctset_t *em_orgfnctset; /* pointer to the original function set. Only
                                      required when a re-evaluation has been requested.
                                      Not to be modified during optimization */
    ADCL_fnctset_t     em_fnctset; /* copy of the function group associated with
                                      this object */
    ADCL_hypothesis_t     em_hypo; /* Performance hypothesis object of to this
                                      emethod */
    ADCL_function_t  *em_wfunction; /* winner function */
};
typedef struct ADCL_emethod_s ADCL_emethod_t;


ADCL_emethod_t* ADCL_emethod_init ( ADCL_topology_t *topo, ADCL_vector_t *vec,
                    ADCL_fnctset_t *fnctset);

void ADCL_emethod_free ( ADCL_emethod_t * er );


int ADCL_emethod_monitor ( ADCL_emethod_t *emethod, int pos,
               TIME_TYPE tstart, TIME_TYPE tend );

int  ADCL_emethods_get_winner ( ADCL_emethod_t *ermethods, MPI_Comm comm, int count );
void ADCL_emethods_update ( ADCL_emethod_t *ermethods, int pos,
                int flag, TIME_TYPE tstart, TIME_TYPE tend );

int  ADCL_emethods_get_next ( ADCL_emethod_t *emethods, int mode, int *flag);
ADCL_function_t *  ADCL_emethod_get_function ( ADCL_emethod_t *emethod, int pos);
int ADCL_emethod_get_function_by_attrs ( ADCL_emethod_t *erm, int *attr, int *pos);
int ADCL_emethod_get_stats_by_attrs ( ADCL_emethod_t *em, int *attrval,
                      ADCL_statistics_t **stat, ADCL_function_t **func );



#endif /* __ADCL_EMETHOD_H__ */
