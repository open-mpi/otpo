/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"


void* ADCL_allocate_TYPE_matrix ( int ndims, int *dims, void *matpt )
{
    switch ( ndims ) {
    case 1: {
        TYPE *matrix;
        matrix = ( TYPE *) malloc ( dims[0] * sizeof( TYPE ));
        if ( NULL == matrix ) {
            return NULL;
        }
        *((void **)matpt) = matrix;
        return &(matrix[0]);
    }
    case 2: {
        TYPE **matrix;
        ADCL_allocate_2D_TYPE_matrix ( &matrix, dims );
        *((void **)matpt) = matrix;
        return &(matrix[0][0]);
    }
    case 3: {
        TYPE ***matrix;
        ADCL_allocate_3D_TYPE_matrix ( &matrix, dims );
        *((void **)matpt) = matrix;
        return &(matrix[0][0][0]);
    }
    case 4: {
        TYPE ****matrix;
        ADCL_allocate_4D_TYPE_matrix ( &matrix, dims );
        *((void **)matpt) = matrix;
        return &(matrix[0][0][0][0]);
    }
    case 5: {
        TYPE *****matrix;
        ADCL_allocate_5D_TYPE_matrix ( &matrix, dims );
        *((void **)matpt) = matrix;
        return &(matrix[0][0][0][0][0]);
    }
    default:
        ADCL_printf("This dimension %d currently not supported\n",
            ndims );
        break;
    }

    return NULL;
}

void ADCL_free_TYPE_matrix ( int ndims, void *mat)
{
    switch ( ndims ) {
    case 1: {
        TYPE *matrix = ( TYPE *) mat;
        if ( NULL != matrix ) {
            free ( matrix );
            matrix = NULL;
        }
        break;
    }
    case 2: {
        TYPE **matrix = ( TYPE **) mat;
        ADCL_free_2D_TYPE_matrix ( &matrix );
        break;
    }
    case 3: {
        TYPE ***matrix = ( TYPE ***) mat;
        ADCL_free_3D_TYPE_matrix ( &matrix );
        break;
    }
    case 4: {
        TYPE ****matrix = ( TYPE ****) mat;
        ADCL_free_4D_TYPE_matrix ( &matrix );
        break;
    }
    case 5: {
        TYPE *****matrix = ( TYPE *****) mat;
        ADCL_free_5D_TYPE_matrix ( &matrix );
        break;
    }
    default:
        ADCL_printf("This dimension %d currently not supported\n",
            ndims );
        break;
    }

    return;
}




int ADCL_allocate_2D_TYPE_matrix (TYPE ***matrix,int dims[2])
{
  int i;
  TYPE ** tmp_field0;
  TYPE *data;

  data = (TYPE *) malloc(dims[0]*dims[1]*sizeof(TYPE));
  if (data == NULL ) {
      ADCL_printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ADCL_NO_MEMORY;
  }

  tmp_field0 = (TYPE **) malloc (dims[0]*sizeof(TYPE *));
  if (tmp_field0 == NULL ) {
      ADCL_printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ADCL_NO_MEMORY;
  }
  for (i=0; i<(dims[0]); i++) {
    tmp_field0[i] = &(data[i*dims[1]]);
  }

  *matrix=tmp_field0;
  return ADCL_SUCCESS;
}


int ADCL_allocate_3D_TYPE_matrix (TYPE ****matrix,int dims[3])
{
  int i;
  TYPE ***tmp_field1;
  TYPE **tmp_field0;
  TYPE *data;

  data = (TYPE *) malloc(dims[0]*dims[1]*dims[2]*sizeof(TYPE));
  if (data == NULL ) {
      ADCL_printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ADCL_NO_MEMORY;
  }

  tmp_field0 = (TYPE **) malloc (dims[0]*dims[1]*sizeof(TYPE *));
  if (tmp_field0 == NULL ) {
      ADCL_printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ADCL_NO_MEMORY;
  }
  for (i=0; i<(dims[0]*dims[1]); i++) {
    tmp_field0[i] = &(data[i*dims[2]]);
  }
  tmp_field1 = (TYPE ***) malloc (dims[0]*sizeof(TYPE **));
  if (tmp_field1 == NULL ) {
      ADCL_printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ADCL_NO_MEMORY;
  }
  for (i=0; i<dims[0]; i++) {
    tmp_field1[i] = &(tmp_field0[i*dims[1]]);
  }

  *matrix=tmp_field1;
  return ADCL_SUCCESS;
}

int ADCL_allocate_4D_TYPE_matrix (TYPE *****matrix,int dims[4])
{
  int i;
  TYPE ****tmp_field2;
  TYPE ***tmp_field1;
  TYPE **tmp_field0;
  TYPE *data;


  data = (TYPE *) malloc(dims[0]*dims[1]*dims[2]*dims[3]*sizeof(TYPE));
  if (data == NULL ) {
      ADCL_printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ADCL_NO_MEMORY;
  }

  tmp_field0 = (TYPE **) malloc (dims[0]*dims[1]*dims[2]*sizeof(TYPE *));
  if (tmp_field0 == NULL ) {
      ADCL_printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ADCL_NO_MEMORY;
  }
  for (i=0; i<(dims[0]*dims[1]*dims[2]); i++) {
    tmp_field0[i] = &(data[i*dims[3]]);
  }
  tmp_field1 = (TYPE ***) malloc (dims[0]*dims[1]*sizeof(TYPE **));
  if (tmp_field1 == NULL ) {
      ADCL_printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ADCL_NO_MEMORY;
  }
  for (i=0; i<dims[0]*dims[1]; i++) {
    tmp_field1[i] = &(tmp_field0[i*dims[2]]);
  }
  tmp_field2 = (TYPE ****) malloc (dims[0]*sizeof(TYPE **));
  if (tmp_field2 == NULL ) {
      ADCL_printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ADCL_NO_MEMORY;
  }
  for (i=0; i<dims[0]; i++) {
    tmp_field2[i] = &(tmp_field1[i*dims[1]]);
  }

  *matrix=tmp_field2;
  return ADCL_SUCCESS;
}

int ADCL_allocate_5D_TYPE_matrix (TYPE ******matrix,int dims[5])
{
  int i;
  TYPE *****tmp_field3;
  TYPE ****tmp_field2;
  TYPE ***tmp_field1;
  TYPE **tmp_field0;
  TYPE *data;


  data = (TYPE *) malloc(dims[0]*dims[1]*dims[2]*dims[3]*dims[4]*sizeof(TYPE));
  if (data == NULL ) {
      ADCL_printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ADCL_NO_MEMORY;
  }

  tmp_field0 = (TYPE **) malloc (dims[0]*dims[1]*dims[2]*dims[3]*sizeof(TYPE *));
  if (tmp_field0 == NULL ) {
      ADCL_printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ADCL_NO_MEMORY;
  }
  for (i=0; i<(dims[0]*dims[1]*dims[2]*dims[3]); i++) {
    tmp_field0[i] = &(data[i*dims[4]]);
  }
  tmp_field1 = (TYPE ***) malloc (dims[0]*dims[1]*dims[2]*sizeof(TYPE **));
  if (tmp_field1 == NULL ) {
      ADCL_printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ADCL_NO_MEMORY;
  }
  for (i=0; i<dims[0]*dims[1]*dims[2]; i++) {
    tmp_field1[i] = &(tmp_field0[i*dims[3]]);
  }
  tmp_field2 = (TYPE ****) malloc (dims[0]*dims[1]*sizeof(TYPE ***));
  if (tmp_field2 == NULL ) {
      ADCL_printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ADCL_NO_MEMORY;
  }
  for (i=0; i<dims[0]*dims[1]; i++) {
    tmp_field2[i] = &(tmp_field1[i*dims[2]]);
  }
  tmp_field3 = (TYPE *****) malloc (dims[0]*sizeof(TYPE ****));
  if (tmp_field3 == NULL ) {
      ADCL_printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ADCL_NO_MEMORY;
  }
  for (i=0; i<dims[0]; i++) {
    tmp_field3[i] = &(tmp_field2[i*dims[1]]);
  }

  *matrix=tmp_field3;
  return ADCL_SUCCESS;
}

int ADCL_free_2D_TYPE_matrix (TYPE ***matrix)
{
  TYPE *matrix_tmp0;
  TYPE **matrix_tmp1;

  matrix_tmp0=**matrix;
  matrix_tmp1=*matrix;

  free(matrix_tmp0);
  free(matrix_tmp1);

  *matrix=NULL;
  return ADCL_SUCCESS;
}

int ADCL_free_3D_TYPE_matrix (TYPE ****matrix)
{
  TYPE *matrix_tmp0;
  TYPE **matrix_tmp1;
  TYPE ***matrix_tmp2;

  matrix_tmp0=***matrix;
  matrix_tmp1=**matrix;
  matrix_tmp2=*matrix;

  free(matrix_tmp0);
  free(matrix_tmp1);
  free(matrix_tmp2);

  *matrix=NULL;
  return ADCL_SUCCESS;
}

int ADCL_free_4D_TYPE_matrix (TYPE *****matrix)
{
  TYPE *matrix_tmp0;
  TYPE **matrix_tmp1;
  TYPE ***matrix_tmp2;
  TYPE ****matrix_tmp3;

  matrix_tmp0=****matrix;
  matrix_tmp1=***matrix;
  matrix_tmp2=**matrix;
  matrix_tmp3=*matrix;

  free(matrix_tmp0);
  free(matrix_tmp1);
  free(matrix_tmp2);
  free(matrix_tmp3);

  *matrix = NULL;
  return ADCL_SUCCESS;
}

int ADCL_free_5D_TYPE_matrix (TYPE ******matrix)
{
  TYPE *matrix_tmp0;
  TYPE **matrix_tmp1;
  TYPE ***matrix_tmp2;
  TYPE ****matrix_tmp3;
  TYPE *****matrix_tmp4;

  matrix_tmp0=*****matrix;
  matrix_tmp1=****matrix;
  matrix_tmp2=***matrix;
  matrix_tmp3=**matrix;
  matrix_tmp4=*matrix;

  free(matrix_tmp0);
  free(matrix_tmp1);
  free(matrix_tmp2);
  free(matrix_tmp3);
  free(matrix_tmp4);

  *matrix=NULL;
  return ADCL_SUCCESS;
}

