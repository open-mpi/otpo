/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ATF.h"
#include "ATF_tfqmr.Cal.h"


/*Local const variables for all calculation routines*/

/* A=B+C
** A,B and C are matrix wit the same upper&lower bound
*/
int ATF_tfqmr_Cal_A_EQ_B_plus_C(double ****A, double ****B, double ****C)
{
	int i, j, k, l;
	int n1, n2, n3, nc;

 
	n1 = ATF_dim[0]+1;
	n2 = ATF_dim[1]+1;
	n3 = ATF_dim[2]+1;

	nc = 1;

	for(i=1; i < n1;i++){
		for(j=1; j < n2; j++){
			for(k=1; k<n3; k++){
				for(l=0; l<nc; l++){
				
					A[i][j][k][l] = B[i][j][k][l] + C[i][j][k][l]; 
				}
			}
		}
	}
	
	return ATF_SUCCESS;
}

/* A=B
** A,B and C are matrix wit the same upper&lower bound
*/
int ATF_tfqmr_Cal_A_EQ_B(double ****A, double ****B)
{
	int i, j, k, l;
	int n1, n2, n3, nc;

	n1 = ATF_dim[0]+1;
	n2 = ATF_dim[1]+1;
	n3 = ATF_dim[2]+1;

	nc = 1;

	for(i=1; i < n1;i++){
		for(j=1; j < n2; j++){
			for(k=1; k<n3; k++){
				for(l=0; l<nc; l++){
				
					A[i][j][k][l] = B[i][j][k][l];
				}
			}
		}
	}

	return ATF_SUCCESS;
}

int ATF_tfqmr_Cal_A_EQ_c(double ****A, double constant)
{
	int i, j, k, l;
	int n1, n2, n3, nc;

	n1 = ATF_dim[0]+1;
	n2 = ATF_dim[1]+1;
	n3 = ATF_dim[2]+1;
	nc = 1;

	for(i=1; i < n1;i++){
		for(j=1; j < n2; j++){
			for(k=1; k<n3; k++){
				for(l=0; l<nc; l++){
				
					A[i][j][k][l] = constant;
				}
			}
		}
	}
	
	return ATF_SUCCESS;
}

/* c=c+A*B
 * c is variable, A and B are Matrix
*/

int ATF_tfqmr_Cal_c_EQ_c_plus_A_mul_B(double ****A, double ****B, double *variable)
{
	int i, j, k, l;
	int n1, n2, n3, nc;
	
	n1 = ATF_dim[0]+1;
	n2 = ATF_dim[1]+1;
	n3 = ATF_dim[2]+1;
	nc = 1;

	*variable = 0.0;

	for(i=1; i < n1;i++){
		for(j=1; j < n2; j++){
			for(k=1; k<n3; k++){
				for(l=0; l<nc; l++){
				
					*variable = *variable+ A[i][j][k][l]*B[i][j][k][l];
				}
			}
		}
	}

	return ATF_SUCCESS;
}

/*A = B-z*C*/
int ATF_tfqmr_Cal_A_EQ_B_sub_z_mul_C(double ****A, double ****B, double ****C, double z)
{

	int i, j, k, l;
	int n1, n2, n3, nc;

	n1 = ATF_dim[0]+1;
	n2 = ATF_dim[1]+1;
	n3 = ATF_dim[2]+1;
	nc = 1;

	for(i=1; i < n1;i++){
		for(j=1; j < n2; j++){
			for(k=1; k<n3; k++){
				for(l=0; l<nc; l++){
				
					A[i][j][k][l]= B[i][j][k][l]- z* C[i][j][k][l];
				}
			}
		}
	}

	return ATF_SUCCESS;

}

/*A =z*B */

int ATF_tfqmr_Cal_A_EQ_c_mul_B(double ****A, double ****B, double c)
{

	int i, j, k, l;
	int n1, n2, n3, nc;

	n1 = ATF_dim[0]+1;
	n2 = ATF_dim[1]+1;
	n3 = ATF_dim[2]+1;
	nc = 1;

	for(i=1; i < n1;i++){
		for(j=1; j < n2; j++){
			for(k=1; k<n3; k++){
				for(l=0; l<nc; l++){
				
					A[i][j][k][l]= c* B[i][j][k][l];
				}
			}
		}
	}

	return ATF_SUCCESS;
}

/*dq = dq+ c*B*/

int ATF_tfqmr_Cal_dq_EQ_dq_plus_z_mul_A(double ****A, double c)
{
	int i, j, k, l;
	int n1, n2, n3, nc;

	n1 = ATF_dim[0]+1;
	n2 = ATF_dim[1]+1;
	n3 = ATF_dim[2]+1;
	nc = 1;

	for(i=1; i < n1;i++){
		for(j=1; j < n2; j++){
			for(k=1; k<n3; k++){
				for(l=0; l<nc; l++){
				
					ATF_dq[i][j][k][l]= ATF_dq[i][j][k][l] + c* A[i][j][k][l];
				}
			}
		}
	}

	return ATF_SUCCESS;
}

/*A=B-C*/
int ATF_tfqmr_Cal_A_EQ_B_sub_C( double ****A, double ****B, double ****C)
{
	int i, j, k, l;
	int n1, n2, n3, nc;

	n1 = ATF_dim[0]+1;
	n2 = ATF_dim[1]+1;
	n3 = ATF_dim[2]+1;
	nc = 1;

	for(i=1; i < n1;i++){
		for(j=1; j < n2; j++){
			for(k=1; k<n3; k++){
				for(l=0; l<nc; l++){
				
					A[i][j][k][l]= B[i][j][k][l] - C[i][j][k][l];
				}
			}
		}
	}

	return ATF_SUCCESS;
}

/* A=B+z*C */
int ATF_tfqmr_Cal_A_EQ_B_plus_z_mul_C(double ****A, double ****B, double ****C, double z)
{
	
	int i, j, k, l;
	int n1, n2, n3, nc;

	n1 = ATF_dim[0]+1;
	n2 = ATF_dim[1]+1;
	n3 = ATF_dim[2]+1;
	nc = 1;

	for(i=1; i < n1;i++){
		for(j=1; j < n2; j++){
			for(k=1; k<n3; k++){
				for(l=0; l<nc; l++){
				
					A[i][j][k][l]= B[i][j][k][l] + z* C[i][j][k][l];
				}
			}
		}
	}

	return ATF_SUCCESS;
}

