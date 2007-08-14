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

int ADCL_Papi_create ( ADCL_Papi *papi )
{
    if ( NULL == papi ) {
    return ADCL_INVALID_PAPI;
    }
    return ADCL_papi_create (papi);
}

int ADCL_Papi_enter ( ADCL_Papi papi )
{
    if ( NULL == papi ) {
    return ADCL_INVALID_PAPI;
    }
    return ADCL_papi_enter (papi);
}

int ADCL_Papi_leave ( ADCL_Papi papi )
{
    if ( NULL == papi ) {
    return ADCL_INVALID_PAPI;
    }
    return ADCL_papi_leave (papi);
}

int ADCL_Papi_free ( ADCL_Papi *papi)
{
    ADCL_papi_t *ppapi = *papi;

    if ( NULL == papi  ) {
    return ADCL_INVALID_ARG;
    }
    if ( ppapi->p_id < 0 ) {
    return ADCL_INVALID_PAPI;
    }

    return ADCL_papi_free (papi);
}

int ADCL_Papi_print ( ADCL_Papi papi)
{
    if ( NULL == papi ) {
    return ADCL_INVALID_PAPI;
    }
    return ADCL_papi_print (papi);
}
