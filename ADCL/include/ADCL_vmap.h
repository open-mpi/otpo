/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2008           HLRS. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef __ADCL_VMAP_H__
#define __ADCL_VMAP_H__

#include "ADCL_topology.h"

struct ADCL_vmap_s{
    int              m_id; /* unique identifier for this process */
    int          m_findex; /* index for the fortran interface */
    int           m_rfcnt; /* object reference counter */
    int         m_vectype; /* communication type */
    int          m_hwidth; /* how many halo-cells are being used */
//    int           *m_dims; /* extent of each of the dimensions */
//    void        *m_matrix; /* the matrix pointer */
//    void          *m_data; /* pointer to the data array */
//    MPI_Datatype    m_dat; /* basic datatype */
    int*          m_rcnts; /* receive counts for AllGatherV */ 
    int*          m_displ; /* displacements for AllGatherV */
    MPI_Op           m_op; /* MPI operator for AllReduce */
    int         m_inplace; /* MPI_IN_PLACE */
    int            m_scnt; /* send count for AlltoAll */
    int            m_rcnt; /* receive count for AlltoAll */
};
typedef struct ADCL_vmap_s ADCL_vmap_t;

extern ADCL_array_t *ADCL_vmap_farray;

/* Some internal routines to access elements of the vmap will be introduced
   later/up on demand by the other routines. */


/* ADCL_vmap_allocate:
   Description: This routine allocate a new data object including the memory
                are to hold the data

   @param   ndims:   number of dimensions
   @param    dims:   extent of each dimension
   @param      nc:   extent of each point of the vmap
   @param vectype:   type of the data to be communicated
   @param hdiwdth:   number of ghost cells. Note: this entry does not
                     modify dims or nc! It's stored for later usage
   @param     dat;   basic datatype of the array, described as an
                     MPI_Datatype

   @out       vec;   ADCL_vmap_t object

   @retval ADCL_SUCCESS          ok
   @retval ADCL_NO_MEMORY        memory allocation failed

   @retval ADCL_INVALID_NDIMS    invalid ndims input parameter
   @retval ADCL_INVALID_DIMS     invalid dims array input parameter
   @retval ADCL_INVALID_NC       invalid nc input parameter
   @retval ADCL_INVALID_HWIDTH   invalid hwidth input parameter
   @retval ADCL_INVALID_DAT      invalid dat input parameter
*/

int ADCL_vmap_halo_allocate ( int hwidth, ADCL_vmap_t **vec );
int ADCL_vmap_list_allocate ( int size, int* rcnts, int* displ, ADCL_vmap_t **vec );
int ADCL_vmap_allreduce_allocate ( MPI_Op op, ADCL_vmap_t **vec );
int ADCL_vmap_reduce_allocate ( MPI_Op op, ADCL_vmap_t **vec );
int ADCL_vmap_all_allocate ( ADCL_vmap_t **vec );
int ADCL_vmap_inplace_allocate ( ADCL_vmap_t **vec );
int ADCL_vmap_alltoall_allocate ( int scnt, int rcnt, ADCL_vmap_t **vmap );

/* ADCL_vmap_free
   Description: free an ADCL_vmap object including the allocated data field

   @param vec: ADCL_vmap object to be freed
   @out   vec: on successfull completion ADCL_VMAP_NULL, undefined else

   @retval ADCL_SUCCESS          ok
   @retval ADCL_INVALID_DATA     data pointer was not allocated with
                                 ADCL_vmap_allocate
*/
int ADCL_vmap_free  ( ADCL_vmap_t **vec );


/* ADCL_vmap_register:
   Description: This routine allocate a new data object. The routine
                assumes in contrary to ADCL_vmap_allocate, that the
        memory for the data has been allocated already

   @param   ndims:   number of dimensions
   @param    dims:   extent of each dimension
   @param      nc:   extent of each point of the vmap
   @param vectype:   type of the data to be communicated
   @param hdiwdth:   number of ghost cells. Note: this entry does not
                     modify dims or nc! It's stored for later usage
   @param     dat;   basic datatype of the array, described as an
                     MPI_Datatype
   @param    data;   pointer to the data field

   @out       vec;   ADCL_vmap object

   @retval ADCL_SUCCESS          ok
   @retval ADCL_NO_MEMORY        memory allocation failed

   @retval ADCL_INVALID_NDIMS    invalid ndims input parameter
   @retval ADCL_INVALID_DIMS     invalid dims array input parameter
   @retval ADCL_INVALID_NC       invalid nc input parameter
   @retval ADCL_INVALID_HWIDTH   invalid hwidth input parameter
   @retval ADCL_INVALID_DAT      invalid dat input parameter
   @retval ADCL_INVALID_DATA     invalid data pointer input parameter
*/
int ADCL_vmap_register ( int ndims, int *dims, int nc, int vectype, int hwidth,
                           MPI_Datatype dat, void *data, ADCL_vmap_t **vec );




/* ADCL_vmap_deregister
   Description: free an ADCL_vmap object without touching the data
                fields, since this has been allocate by the user and not
                by the ADCL_vmap routine

   @param vec: ADCL_vmap object to be freed
   @out   vec: on successfull completion ADCL_VMAP_NULL, undefined else

   @retval ADCL_SUCCESS          ok
   @retval ADCL_INVALID_DATA     data array was allocated with
                                 ADCL_vmap_allocate, should be freed with
                 ADCL_vmap_free instead of
                 ADCL_vmap_deregister
*/
int ADCL_vmap_deregister  ( ADCL_vmap_t **vec );


void* ADCL_vmap_get_data_ptr ( ADCL_vmap_t *vec );


#endif /* __ADCL_VMAP_H__ */
