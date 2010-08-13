/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef __ADCL_MEMORY_H__
#define __ADCL_MEMORY_H__

#ifdef ADCL_DUMMY_MPI
#include "ADCL_dummy_mpi.h"
#else
#include "mpi.h"
#endif

void* ADCL_allocate_matrix ( int ndims, int *dims, MPI_Datatype dat, void *pt);
void ADCL_free_matrix ( int ndims, MPI_Datatype dat, void *mat);

void* ADCL_allocate_double_matrix ( int ndims, int *dims, void *pt);
void ADCL_free_double_matrix ( int ndims, void *mat);

void* ADCL_allocate_float_matrix ( int ndims, int *dims, void *pt );
void ADCL_free_float_matrix ( int ndims, void *mat);

void* ADCL_allocate_int_matrix ( int ndims, int *dims, void *pt);
void ADCL_free_int_matrix ( int ndims, void *mat);

#if defined TYPEMODE
#if TYPEMODE == 1
#define TYPE double
#define ADCL_allocate_TYPE_matrix    ADCL_allocate_double_matrix
#define ADCL_allocate_2D_TYPE_matrix ADCL_allocate_2D_double_matrix
#define ADCL_allocate_3D_TYPE_matrix ADCL_allocate_3D_double_matrix
#define ADCL_allocate_4D_TYPE_matrix ADCL_allocate_4D_double_matrix
#define ADCL_allocate_5D_TYPE_matrix ADCL_allocate_5D_double_matrix

#define ADCL_free_TYPE_matrix        ADCL_free_double_matrix
#define ADCL_free_2D_TYPE_matrix     ADCL_free_2D_double_matrix
#define ADCL_free_3D_TYPE_matrix     ADCL_free_3D_double_matrix
#define ADCL_free_4D_TYPE_matrix     ADCL_free_4D_double_matrix
#define ADCL_free_5D_TYPE_matrix     ADCL_free_5D_double_matrix

#elif TYPEMODE == 2
#define TYPE float
#define ADCL_allocate_TYPE_matrix    ADCL_allocate_float_matrix
#define ADCL_allocate_2D_TYPE_matrix ADCL_allocate_2D_float_matrix
#define ADCL_allocate_3D_TYPE_matrix ADCL_allocate_3D_float_matrix
#define ADCL_allocate_4D_TYPE_matrix ADCL_allocate_4D_float_matrix
#define ADCL_allocate_5D_TYPE_matrix ADCL_allocate_5D_float_matrix

#define ADCL_free_TYPE_matrix        ADCL_free_float_matrix
#define ADCL_free_2D_TYPE_matrix     ADCL_free_2D_float_matrix
#define ADCL_free_3D_TYPE_matrix     ADCL_free_3D_float_matrix
#define ADCL_free_4D_TYPE_matrix     ADCL_free_4D_float_matrix
#define ADCL_free_5D_TYPE_matrix     ADCL_free_5D_float_matrix

#elif TYPEMODE == 3
#define TYPE int
#define ADCL_allocate_TYPE_matrix    ADCL_allocate_int_matrix
#define ADCL_allocate_2D_TYPE_matrix ADCL_allocate_2D_int_matrix
#define ADCL_allocate_3D_TYPE_matrix ADCL_allocate_3D_int_matrix
#define ADCL_allocate_4D_TYPE_matrix ADCL_allocate_4D_int_matrix
#define ADCL_allocate_5D_TYPE_matrix ADCL_allocate_5D_int_matrix

#define ADCL_free_TYPE_matrix        ADCL_free_int_matrix
#define ADCL_free_2D_TYPE_matrix     ADCL_free_2D_int_matrix
#define ADCL_free_3D_TYPE_matrix     ADCL_free_3D_int_matrix
#define ADCL_free_4D_TYPE_matrix     ADCL_free_4D_int_matrix
#define ADCL_free_5D_TYPE_matrix     ADCL_free_5D_int_matrix

#endif
#endif

/*
** prototypes for the private functions. I have to admit, that
** multi-dimensional arrays in C are a nightmare :-)
*/
int ADCL_allocate_2D_double_matrix(double ***matrix,int dims[2]);
int ADCL_allocate_3D_double_matrix(double ****matrix,int dims[3]);
int ADCL_allocate_4D_double_matrix(double *****matrix,int dims[4]);
int ADCL_allocate_5D_double_matrix(double ******matrix, int dims[5]);

int ADCL_free_2D_double_matrix(double ***matrix);
int ADCL_free_3D_double_matrix(double ****matrix);
int ADCL_free_4D_double_matrix(double *****matrix);
int ADCL_free_5D_double_matrix(double ******matrix);

int ADCL_allocate_2D_float_matrix(float ***matrix,int dims[2]);
int ADCL_allocate_3D_float_matrix(float ****matrix,int dims[3]);
int ADCL_allocate_4D_float_matrix(float *****matrix,int dims[4]);
int ADCL_allocate_5D_float_matrix(float ******matrix, int dims[5]);

int ADCL_free_2D_float_matrix(float ***matrix);
int ADCL_free_3D_float_matrix(float ****matrix);
int ADCL_free_4D_float_matrix(float *****matrix);
int ADCL_free_5D_float_matrix(float ******matrix);

int ADCL_allocate_2D_int_matrix(int ***matrix,int dims[2]);
int ADCL_allocate_3D_int_matrix(int ****matrix,int dims[3]);
int ADCL_allocate_4D_int_matrix(int *****matrix,int dims[4]);
int ADCL_allocate_5D_int_matrix(int ******matrix, int dims[5]);

int ADCL_free_2D_int_matrix(int ***matrix);
int ADCL_free_3D_int_matrix(int ****matrix);
int ADCL_free_4D_int_matrix(int *****matrix);
int ADCL_free_5D_int_matrix(int ******matrix);


#endif /* __ADCL_MEMORY_H__ */
