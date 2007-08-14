/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef ATF_MEMORY_H
#define ATF_MEMORY_H

int ATF_allocate_2D_double_matrix(double ***matrix,int dims[2]);
int ATF_allocate_3D_double_matrix(double ****matrix,int dims[3]);
int ATF_allocate_4D_double_matrix(double *****matrix,int dims[4]);
int ATF_allocate_5D_double_matrix(double ******matrix, int dims[5]);

int ATF_free_2D_double_matrix(double ***matrix);
int ATF_free_3D_double_matrix(double ****matrix);
int ATF_free_4D_double_matrix(double *****matrix);
int ATF_free_5D_double_matrix(double ******matrix);

int ATF_Free_matrix();
#endif
