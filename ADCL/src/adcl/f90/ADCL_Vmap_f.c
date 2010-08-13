/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2008           HLRS. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL.h"
#include "ADCL_internal.h"
#include "ADCL_fprototypes.h"

#ifndef _SX
#pragma weak adcl_vmap_halo_allocate_  = adcl_vmap_halo_allocate
#pragma weak adcl_vmap_halo_allocate__ = adcl_vmap_halo_allocate
#pragma weak ADCL_VMAP_HALO_ALLOCATE   = adcl_vmap_halo_allocate

#pragma weak adcl_vmap_list_allocate_  = adcl_vmap_list_allocate
#pragma weak adcl_vmap_list_allocate__ = adcl_vmap_list_allocate
#pragma weak ADCL_VMAP_LIST_ALLOCATE   = adcl_vmap_list_allocate

#pragma weak adcl_vmap_allreduce_allocate_  = adcl_vmap_allreduce_allocate
#pragma weak adcl_vmap_allreduce_allocate__ = adcl_vmap_allreduce_allocate
#pragma weak ADCL_VMAP_ALLREDUCE_ALLOCATE   = adcl_vmap_allreduce_allocate

#pragma weak adcl_vmap_alltoall_allocate_  = adcl_vmap_alltoall_allocate
#pragma weak adcl_vmap_alltoall_allocate__ = adcl_vmap_alltoall_allocate
#pragma weak ADCL_VMAP_ALLTOALL_ALLOCATE   = adcl_vmap_alltoall_allocate

#pragma weak adcl_vmap_all_allocate_  = adcl_vmap_all_allocate
#pragma weak adcl_vmap_all_allocate__ = adcl_vmap_all_allocate
#pragma weak ADCL_VMAP_ALL_ALLOCATE   = adcl_vmap_all_allocate

#pragma weak adcl_vmap_inplace_allocate_  = adcl_vmap_inplace_allocate
#pragma weak adcl_vmap_inplace_allocate__ = adcl_vmap_inplace_allocate
#pragma weak ADCL_VMAP_INPLACE_ALLOCATE   = adcl_vmap_inplace_allocate

#pragma weak adcl_vmap_free_  = adcl_vmap_free
#pragma weak adcl_vmap_free__ = adcl_vmap_free
#pragma weak ADCL_VMAP_FREE   = adcl_vmap_free
#endif

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
#ifdef _SX
void adcl_vmap_halo_allocate_ ( int* hwidth, int *vmap, int* ierror )
#else
void adcl_vmap_halo_allocate  ( int* hwidth, int *vmap, int* ierror )
#endif

{
    ADCL_vmap_t *tvmap=NULL;

    if ( NULL == hwidth ) {
       *ierror = ADCL_INVALID_ARG;
       return;
    }

    if ( 0 > hwidth){
        *ierror = ADCL_INVALID_HWIDTH;
        return;
    }

    *ierror = ADCL_Vmap_halo_allocate ( *hwidth, &tvmap );

    if ( ADCL_SUCCESS  == *ierror ) {
        *vmap = tvmap->m_findex;
    }

    return;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
#ifdef _SX
void adcl_vmap_list_allocate_ ( int *size, int *rcnts, int *displ, 
   int *vmap, int *ierror )
#else
void adcl_vmap_list_allocate  ( int *size, int *rcnts, int *displ, 
   int *vmap, int *ierror )
#endif
{
    ADCL_vmap_t *tvmap=NULL;

    if ( ( 0    >= *size )   ||
         ( NULL == rcnts )   ||
         ( NULL == displ )   ||
         ( NULL == vmap )    ){
         *ierror = ADCL_INVALID_ARG;
         return;
    }

    *ierror = ADCL_vmap_list_allocate ( *size, rcnts, displ, &tvmap );
    if ( ADCL_SUCCESS == *ierror) {
        *vmap = tvmap->m_findex;
    }

    return;
}


/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
#ifdef _SX
void adcl_vmap_allreduce_allocate_ ( int *op, int *vmap, int *ierror )
#else
void adcl_vmap_allreduce_allocate  ( int *op, int *vmap, int *ierror )
#endif

{
    MPI_Op cop;
    ADCL_vmap_t *tvmap=NULL;

    if ( NULL == op ) {
       *ierror = ADCL_INVALID_ARG;
       return;
    }

    cop = MPI_Op_f2c(*op);
    if ( cop != MPI_MIN && cop != MPI_MAX && cop != MPI_SUM){
       *ierror = ADCL_INVALID_ARG;
       return;
    }

    *ierror = ADCL_vmap_allreduce_allocate ( cop, &tvmap );
    if ( ADCL_SUCCESS == *ierror) {
        *vmap = tvmap->m_findex;
    }

    return;
}

/**********************************************************************/
#ifdef _SX
void adcl_vmap_alltoall_allocate_ ( int *scnt, int *rcnt, int *vmap, int *ierror )
#else
void adcl_vmap_alltoall_allocate  ( int *scnt, int *rcnt, int *vmap, int *ierror )
#endif
{
    ADCL_vmap_t *tvmap=NULL;

    if ( 0 >= *scnt || 0 >= *rcnt ) {
        *ierror = ADCL_INVALID_ARG;
        return;
    }

    *ierror = ADCL_vmap_alltoall_allocate ( *scnt, *rcnt, &tvmap );
    if ( ADCL_SUCCESS == *ierror) {
        *vmap = tvmap->m_findex;
    }

    return;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
#ifdef _SX
void adcl_vmap_all_allocate_ ( int *vmap, int* ierror )
#else
void adcl_vmap_all_allocate ( int *vmap, int* ierror )
#endif
{
    ADCL_vmap_t *tvmap=NULL;

    *ierror = ADCL_vmap_all_allocate ( &tvmap );
    if ( ADCL_SUCCESS == *ierror) {
        *vmap = tvmap->m_findex;
    }

    return;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
#ifdef _SX
void adcl_vmap_inplace_allocate_ ( int *vmap, int* ierror )
#else
void adcl_vmap_inplace_allocate  ( int *vmap, int* ierror )
#endif
{
    ADCL_vmap_t *tvmap=NULL;

    *ierror = ADCL_vmap_inplace_allocate ( &tvmap );
    if ( ADCL_SUCCESS == *ierror) {
        *vmap = tvmap->m_findex;
    }

    return;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
#ifdef _SX
void adcl_vmap_free_ ( int *vmap, int *ierror )
#else
void adcl_vmap_free  ( int *vmap, int *ierror )
#endif
{
    ADCL_vmap_t *cvmap=NULL;

    if ( NULL == vmap ) {
       *ierror = ADCL_INVALID_ARG;
       return;
    }

    cvmap = (ADCL_vmap_t *) ADCL_array_get_ptr_by_pos ( ADCL_vmap_farray,
                                                              *vmap );

    *ierror = ADCL_vmap_free ( &cvmap );

    *vmap = ADCL_FVMAP_NULL;

    return;
}


