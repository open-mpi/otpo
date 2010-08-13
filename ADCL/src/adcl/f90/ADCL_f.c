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
#include "ADCL_fprototypes.h"

#pragma weak ADCL_INIT   = adcl_init
#pragma weak adcl_init_  = adcl_init
#pragma weak adcl_init__ = adcl_init

#pragma weak ADCL_Finalize   = adcl_finalize
#pragma weak adcl_finalize_  = adcl_finalize
#pragma weak adcl_finalize__ = adcl_finalize

#ifdef _SX
void adcl_init_ ( int *ierror)
#else
void adcl_init ( int *ierror)
#endif
{
    *ierror = ADCL_Init ();
    return;
}

#ifdef _SX
void adcl_finalize_ ( int *ierror )
#else
void adcl_finalize ( int *ierror )
#endif
{
    *ierror = ADCL_Finalize();
    return;
}
