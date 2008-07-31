/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef __ADCL_VECTOR_H__
#define __ADCL_VECTOR_H__


struct ADCL_vector_s{
    int              v_id; /* unique identifier for this process */
    int          v_findex; /* index for the fortran interface */
    int           v_rfcnt; /* object reference counter */
    int           v_alloc; /* TRUE(1) if allocated by ADCL, FALSE(0) if
                              done by user */
    int           v_ndims; /* number of dimensions */
    int              v_nc; /* extent of each point */
    int         v_comtype; /* communication type */
    int          v_hwidth; /* how many halo-cells are being used */
    int           *v_dims; /* extent of each of the dimensions */
    void        *v_matrix; /* the matrix pointer */
    void          *v_data; /* pointer to the data array */
    MPI_Datatype    v_dat; /* basic datatype */
};
typedef struct ADCL_vector_s ADCL_vector_t;

extern ADCL_array_t *ADCL_vector_farray;

/* Some internal routines to access elements of the vector will be introduced
   later/up on demand by the other routines. */


/* ADCL_vector_allocate:
   Description: This routine allocate a new data object including the memory
                are to hold the data

   @param   ndims:   number of dimensions
   @param    dims:   extent of each dimension
   @param      nc:   extent of each point of the vector
   @param comtype:   type of the data to be communicated
   @param hdiwdth:   number of ghost cells. Note: this entry does not
                     modify dims or nc! It's stored for later usage
   @param     dat;   basic datatype of the array, described as an
                     MPI_Datatype

   @out       vec;   ADCL_vector_t object

   @retval ADCL_SUCCESS          ok
   @retval ADCL_NO_MEMORY        memory allocation failed

   @retval ADCL_INVALID_NDIMS    invalid ndims input parameter
   @retval ADCL_INVALID_DIMS     invalid dims array input parameter
   @retval ADCL_INVALID_NC       invalid nc input parameter
   @retval ADCL_INVALID_HWIDTH   invalid hwidth input parameter
   @retval ADCL_INVALID_DAT      invalid dat input parameter
*/
int ADCL_vector_allocate ( int ndims, int *dims, int nc, int comtype, int hwidth,
                           MPI_Datatype dat, ADCL_vector_t **vec );




/* ADCL_vector_free
   Description: free an ADCL_vector object including the allocated data field

   @param vec: ADCL_vector object to be freed
   @out   vec: on successfull completion ADCL_VECTOR_NULL, undefined else

   @retval ADCL_SUCCESS          ok
   @retval ADCL_INVALID_DATA     data pointer was not allocated with
                                 ADCL_vector_allocate
*/
int ADCL_vector_free  ( ADCL_vector_t **vec );





/* ADCL_vector_register:
   Description: This routine allocate a new data object. The routine
                assumes in contrary to ADCL_vector_allocate, that the
        memory for the data has been allocated already

   @param   ndims:   number of dimensions
   @param    dims:   extent of each dimension
   @param      nc:   extent of each point of the vector
   @param comtype:   type of the data to be communicated
   @param hdiwdth:   number of ghost cells. Note: this entry does not
                     modify dims or nc! It's stored for later usage
   @param     dat;   basic datatype of the array, described as an
                     MPI_Datatype
   @param    data;   pointer to the data field

   @out       vec;   ADCL_vector object

   @retval ADCL_SUCCESS          ok
   @retval ADCL_NO_MEMORY        memory allocation failed

   @retval ADCL_INVALID_NDIMS    invalid ndims input parameter
   @retval ADCL_INVALID_DIMS     invalid dims array input parameter
   @retval ADCL_INVALID_NC       invalid nc input parameter
   @retval ADCL_INVALID_HWIDTH   invalid hwidth input parameter
   @retval ADCL_INVALID_DAT      invalid dat input parameter
   @retval ADCL_INVALID_DATA     invalid data pointer input parameter
*/
int ADCL_vector_register ( int ndims, int *dims, int nc, int comtype, int hwidth,
                           MPI_Datatype dat, void *data, ADCL_vector_t **vec );




/* ADCL_vector_deregister
   Description: free an ADCL_vector object without touching the data
                fields, since this has been allocate by the user and not
                by the ADCL_vector routine

   @param vec: ADCL_vector object to be freed
   @out   vec: on successfull completion ADCL_VECTOR_NULL, undefined else

   @retval ADCL_SUCCESS          ok
   @retval ADCL_INVALID_DATA     data array was allocated with
                                 ADCL_vector_allocate, should be freed with
                 ADCL_vector_free instead of
                 ADCL_vector_deregister
*/
int ADCL_vector_deregister  ( ADCL_vector_t **vec );


void* ADCL_vector_get_data_ptr ( ADCL_vector_t *vec );

struct ADCL_vectset_s{
    int                    vs_id; /* id of the object */
    int                vs_findex; /* index of this object in the fortran array */
    int                vs_maxnum; /* no. of vector objects in this vector-group */
    ADCL_vector_t     **vs_svecs; /* ptr to the vectors describing send data items */
    ADCL_vector_t     **vs_rvecs; /* ptr to the vectors describing recv data items */
};
typedef struct ADCL_vectset_s ADCL_vectset_t;
extern ADCL_array_t *ADCL_vectset_farray;

int ADCL_vectset_create ( int maxnum,
                          ADCL_vector_t **svecs,
                          ADCL_vector_t **rvecs,
                          ADCL_vectset_t **vectset );

int ADCL_vectset_free   ( ADCL_vectset_t **vectset );

#endif /* __ADCL_VECTOR_H__ */
