/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"
#include "ADCL_papi.h"

ADCL_array_t *ADCL_papi_farray;
static int ADCL_local_id_counter=0;

int ADCL_papi_init(void)
{
    int code;
    /* Initialization of the PAPI library */
    if (!PAPI_is_initialized()) {
        if ( (code = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT)
        ADCL_handle_error(code,"library_init");
    }
    /* Initialization of the timer */

}

int ADCL_papi_create ( ADCL_papi_t **papi )
{
    ADCL_papi_t *newpapi=NULL;
    int code, i;

    i = 0;
    newpapi = (ADCL_papi_t *) calloc ( 1, sizeof ( ADCL_papi_t ));
    if ( NULL == newpapi ) {
    return ADCL_NO_MEMORY;
    }

    newpapi->p_id = ADCL_local_id_counter++;
    ADCL_array_get_next_free_pos ( ADCL_papi_farray, &(newpapi->p_findex));
    ADCL_array_set_element ( ADCL_papi_farray,
                 newpapi->p_findex,
                 newpapi->p_id,
                 newpapi );

    newpapi->p_num_events = 0;
    newpapi->p_hwinfo = NULL;
    if (!PAPI_is_initialized()) {
        if ( (code = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT) {
            ADCL_handle_error(code,"library_init");
	}
    }
    /* Query events to check availability and determine size of event set */
    if (PAPI_query_event(PAPI_L2_TCH) == PAPI_OK) /* Level 2 Total Cache Hits */
    newpapi->p_num_events ++;

    if (PAPI_query_event(PAPI_L2_TCA) == PAPI_OK) /* Level 2 Total Cache Accesses */
    newpapi->p_num_events ++;

    /* allocate events and valuesarrays */
    newpapi->p_events = (unsigned int *) malloc (sizeof(int) * newpapi->p_num_events);
    if ( NULL == newpapi->p_events ) {
    return ADCL_NO_MEMORY;
    }
    newpapi->p_values = (long_long *) malloc (sizeof(long_long) * newpapi->p_num_events);
    if ( NULL == newpapi->p_values ) {
    return ADCL_NO_MEMORY;
    }

    /* Add available events to the event set */
    if (PAPI_query_event(PAPI_L2_TCH) == PAPI_OK) /* Level 2 Total Cache Hits */
    newpapi->p_events[i++] = PAPI_L2_TCH;

    if (PAPI_query_event(PAPI_L2_TCA) == PAPI_OK) /* Level 2 Total Cache Accesses */
    newpapi->p_events[i++] = PAPI_L2_TCA;

    *papi = newpapi;
    return ADCL_SUCCESS;
}

int ADCL_papi_enter ( ADCL_papi_t *papi )
{
    int code;

    if ( (code = PAPI_start_counters(papi->p_events ,
                     papi->p_num_events)) != PAPI_OK)
    ADCL_handle_error(code,"start_counters");

    papi->p_time_stamp = PAPI_get_real_usec();
    papi->p_hwinfo = PAPI_get_hardware_info();

    return ADCL_SUCCESS;
}

int ADCL_papi_leave ( ADCL_papi_t *papi )
{
    int code;
    long_long end;

    if ( (code = PAPI_stop_counters(papi->p_values , papi->p_num_events)) != PAPI_OK)
    ADCL_handle_error(code,"stop_counters");

    end = PAPI_get_real_usec();
    papi->p_exec_time = end - papi->p_time_stamp;

    return ADCL_SUCCESS;
}

int ADCL_papi_free ( ADCL_papi_t **papi)
{
    ADCL_papi_t *tpapi = *papi;

    free (tpapi->p_events);
    free (tpapi->p_values);
    ADCL_array_remove_element ( ADCL_papi_farray, tpapi->p_findex);

    *papi = ADCL_PAPI_NULL;
    return ADCL_SUCCESS;
}

int ADCL_papi_print ( ADCL_papi_t *papi)
{
    char event_name [PAPI_MAX_STR_LEN];
    int i;

    printf("Execution time: %lld\n", papi->p_exec_time);
    printf("%d CPUS with %f MHz\n",papi->p_hwinfo->totalcpus, papi->p_hwinfo->mhz);
    for (i=0 ; i<papi->p_num_events ; i++) {
    PAPI_event_code_to_name(papi->p_events[i],event_name);
    printf("Event %s : %lld\n" , event_name , papi->p_values[i]);
    }

    return ADCL_SUCCESS;
}


















