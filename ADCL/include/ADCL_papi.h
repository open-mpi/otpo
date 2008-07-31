/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef __ADCL_PAPI_H__
#define __ADCL_PAPI_H__

#include <stdio.h>
#include <stdlib.h>
#include "papi.h"

struct ADCL_papi_s{
    int p_id;                /* id of the object */
    int p_findex;            /* index of this object in the fortran array */
    int p_num_events;        /* number of events in the event set */
    long_long p_time_stamp;  /* starting stamp */
    long_long p_exec_time;   /* total execution time */
    unsigned int *p_events;  /* the Event set */
    long_long *p_values;     /* values of each event in the event set */
    const PAPI_hw_info_t *p_hwinfo; /* Hardware Information */
};
typedef struct ADCL_papi_s ADCL_papi_t;
extern ADCL_array_t *ADCL_papi_farray;

int ADCL_papi_create ( ADCL_papi_t **papi );
int ADCL_papi_free   ( ADCL_papi_t **papi );
int ADCL_papi_enter  ( ADCL_papi_t *papi );
int ADCL_papi_leave  ( ADCL_papi_t *papi );
int ADCL_papi_print  ( ADCL_papi_t *papi );

static int ADCL_handle_error (int code , char *code_name) {
    char message[PAPI_MAX_STR_LEN];
    printf("ERROR IN PAPI FUNCTION %s\n" , code_name);
    PAPI_perror(code, message , PAPI_MAX_STR_LEN);
    printf("PAPI ERROR %i: %s\n", code , message);
    exit(1);
}

#endif
