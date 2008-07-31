/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef _ATF_TFQMR_CAL_H_
#define _ATF_TFQMR_CAL_H_

int ATF_tfqmr_Cal_A_EQ_B(double ****, double ****);
int ATF_tfqmr_Cal_A_EQ_B_plus_C(double ****, double ****, double ****);
int ATF_tfqmr_Cal_A_EQ_B_sub_C( double ****, double ****, double ****);
int ATF_tfqmr_Cal_A_EQ_c( double ****, double );
int ATF_tfqmr_Cal_c_EQ_c_plus_A_mul_B(double ****, double ****, double *);
int ATF_tfqmr_Cal_A_EQ_B_sub_z_mul_C(double ****, double ****, double ****, double);
int ATF_tfqmr_Cal_A_EQ_B_plus_z_mul_C(double ****, double ****, double ****, double);
int ATF_tfqmr_Cal_A_EQ_B(double ****, double ****);
int ATF_tfqmr_Cal_A_EQ_c_mul_B(double ****, double ****, double);
int ATF_tfqmr_Cal_dq_EQ_dq_plus_z_mul_A(double ****, double);

#endif

