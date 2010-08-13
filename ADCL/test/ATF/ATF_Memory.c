/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ATF.h"
#include "ATF_Memory.h"

int ATF_allocate_2D_double_matrix(double ***matrix,int dims[2])
{
  int i;
  double ** tmp_field0;
  double *data;
  
  data = (double *) calloc(1, dims[0]*dims[1]*sizeof(double));
  if (data == NULL ) {
      printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ATF_ERROR;
  }
  
  tmp_field0 = (double **) malloc (dims[0]*sizeof(double *));
  if (tmp_field0 == NULL ) {
      printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ATF_ERROR;
  }
  for (i=0; i<(dims[0]); i++) {
    tmp_field0[i] = &(data[i*dims[1]]);
  }

  *matrix=tmp_field0;
  return ATF_SUCCESS;
}


int ATF_allocate_3D_double_matrix(double ****matrix,int dims[3])
{
  int i;
  double ***tmp_field1;
  double **tmp_field0;
  double *data;
  
  data = (double *) calloc(1, dims[0]*dims[1]*dims[2]*sizeof(double));
  if (data == NULL ) {
      printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ATF_ERROR;
  }
  
  tmp_field0 = (double **) malloc (dims[0]*dims[1]*sizeof(double *));
  if (tmp_field0 == NULL ) {
      printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ATF_ERROR;
  }
  for (i=0; i<(dims[0]*dims[1]); i++) {
    tmp_field0[i] = &(data[i*dims[2]]);
  }
  tmp_field1 = (double ***) malloc (dims[0]*sizeof(double **));
  if (tmp_field1 == NULL ) {
      printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ATF_ERROR;
  }
  for (i=0; i<dims[0]; i++) {
    tmp_field1[i] = &(tmp_field0[i*dims[1]]);
  }
  
  *matrix=tmp_field1;
  return ATF_SUCCESS;
}

int ATF_allocate_4D_double_matrix(double *****matrix,int dims[4])
{
  int i;
  double ****tmp_field2;
  double ***tmp_field1;
  double **tmp_field0;
  double *data;
  
  
  data = (double *) calloc(1, dims[0]*dims[1]*dims[2]*dims[3]*sizeof(double));
  if (data == NULL ) {
      printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ATF_ERROR;
  }
  
  tmp_field0 = (double **) malloc (dims[0]*dims[1]*dims[2]*sizeof(double *));
  if (tmp_field0 == NULL ) {
      printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ATF_ERROR;
  }
  for (i=0; i<(dims[0]*dims[1]*dims[2]); i++) {
    tmp_field0[i] = &(data[i*dims[3]]);
  }
  tmp_field1 = (double ***) malloc (dims[0]*dims[1]*sizeof(double **));
  if (tmp_field1 == NULL ) {
      printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ATF_ERROR;
  }
  for (i=0; i<dims[0]*dims[1]; i++) {
    tmp_field1[i] = &(tmp_field0[i*dims[2]]);
  }
  tmp_field2 = (double ****) malloc (dims[0]*sizeof(double **));
  if (tmp_field2 == NULL ) {
      printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ATF_ERROR;
  }
  for (i=0; i<dims[0]; i++) {
    tmp_field2[i] = &(tmp_field1[i*dims[1]]);
  }
  
  *matrix=tmp_field2;
  return ATF_SUCCESS;
}

int ATF_allocate_5D_double_matrix(double ******matrix,int dims[5])
{
  int i;
  double *****tmp_field3;
  double ****tmp_field2;
  double ***tmp_field1;
  double **tmp_field0;
  double *data;
  
  
  data = (double *) calloc(1, dims[0]*dims[1]*dims[2]*dims[3]*dims[4]*sizeof(double));
  if (data == NULL ) {
      printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ATF_ERROR;
  }
  
  tmp_field0 = (double **) malloc (dims[0]*dims[1]*dims[2]*dims[3]*sizeof(double *));
  if (tmp_field0 == NULL ) {
      printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ATF_ERROR;
  }
  for (i=0; i<(dims[0]*dims[1]*dims[2]*dims[3]); i++) {
    tmp_field0[i] = &(data[i*dims[4]]);
  }
  tmp_field1 = (double ***) malloc (dims[0]*dims[1]*dims[2]*sizeof(double **));
  if (tmp_field1 == NULL ) {
      printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ATF_ERROR;
  }
  for (i=0; i<dims[0]*dims[1]*dims[2]; i++) {
    tmp_field1[i] = &(tmp_field0[i*dims[3]]);
  }
  tmp_field2 = (double ****) malloc (dims[0]*dims[1]*sizeof(double ***));
  if (tmp_field2 == NULL ) {
      printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ATF_ERROR;
  }
  for (i=0; i<dims[0]*dims[1]; i++) {
    tmp_field2[i] = &(tmp_field1[i*dims[2]]);
  }
  tmp_field3 = (double *****) malloc (dims[0]*sizeof(double ****));
  if (tmp_field3 == NULL ) {
      printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
      return ATF_ERROR;
  }
  for (i=0; i<dims[0]; i++) {
    tmp_field3[i] = &(tmp_field2[i*dims[1]]);
  }
  
  *matrix=tmp_field3;
  return ATF_SUCCESS;
}

int ATF_free_2D_double_matrix(double ***matrix)
{
  double *matrix_tmp0;
  double **matrix_tmp1;

  matrix_tmp0=**matrix;
  matrix_tmp1=*matrix;

  free(matrix_tmp0);
  free(matrix_tmp1);

  *matrix=NULL;
  return ATF_SUCCESS;
}

int ATF_free_3D_double_matrix(double ****matrix)
{
  double *matrix_tmp0;
  double **matrix_tmp1;
  double ***matrix_tmp2;

  matrix_tmp0=***matrix;
  matrix_tmp1=**matrix;
  matrix_tmp2=*matrix;

  free(matrix_tmp0);
  free(matrix_tmp1);
  free(matrix_tmp2);

  *matrix=NULL;
  return ATF_SUCCESS;
}

int ATF_free_4D_double_matrix(double *****matrix)
{
  double *matrix_tmp0;
  double **matrix_tmp1;
  double ***matrix_tmp2;
  double ****matrix_tmp3;

  matrix_tmp0=****matrix;
  matrix_tmp1=***matrix;
  matrix_tmp2=**matrix;
  matrix_tmp3=*matrix;

  free(matrix_tmp0);
  free(matrix_tmp1);
  free(matrix_tmp2);
  free(matrix_tmp3);

  *matrix = NULL;
  return ATF_SUCCESS;
}

int ATF_free_5D_double_matrix(double ******matrix)
{
  double *matrix_tmp0;
  double **matrix_tmp1;
  double ***matrix_tmp2;
  double ****matrix_tmp3;
  double *****matrix_tmp4;

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
  return ATF_SUCCESS;
}

