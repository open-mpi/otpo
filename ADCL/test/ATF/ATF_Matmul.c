/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ATF.h"
#include "ATF_Matmul.h"

#include "ADCL.h"
#include "ATF_Adcl_global.h"

/* New added functions ADCL_Matmul */
int ATF_Matmul(ADCL_Request adcl_req, double ****vect, double ****res_vect)
{
    ADCL_Request_start (adcl_req);
  
    ATF_Matmul_blocking( vect, res_vect );
    return ATF_SUCCESS;
}


/*Change it later*/
/*
int ATF_Matmul(double ****vect, double ****result_vect, int npattern )
{
    int rank;
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    ADCL_Request_start(ADCL_request);

    switch ( npattern ) {
	case patt_fcfs: 
	    ATF_Change_fcfs(vect);
	    ATF_Matmul_blocking ( vect, result_vect);
	    break;
	default:
	    printf("No patterns matched!\n");
	    break;
    }


    
  return ATF_SUCCESS;
}

*/
/* This routine implements the matrix-vector multiply operation */
int ATF_Matmul_blocking(double ****vect, double ****result_vect)
{
    int i, j, k;
    int n1, n2, n3;
    int upperlimit;
  
    n1 = ATF_dim[0]+1;
    n2 = ATF_dim[1]+1;
    n3 = ATF_dim[2]+1;
  
    /* multiplication with the main diagonal*/
    for( i=1; i<n1; i++ ){
	for( j=1; j<n2; j++ ){
	    for( k=1; k<n3; k++){
		result_vect[i][j][k][0] = ATF_rm000[i][j][k][0][0]* vect[i][j][k][0];
	    }
	}
    }


    /*Multiplikation mit d2*/
    for( i=1; i<n1; i++ ){
	for( j=1; j<n2; j++ ){
	    for( k=1; k<n3; k++){
		result_vect[i][j][k][0] += ATF_rmb00[i][j][k][0][0] * 
		                        vect[i-1][j][k][0];
	    }
	}
    }

    /* Multiplikation mit d3 */
    if( ATF_rand_ab)
	upperlimit = n1-1;
    else
	upperlimit = n1;

    for( i=1; i< upperlimit; i++ ){
	for( j=1; j<n2; j++ ){
	    for( k=1; k<n3; k++){
		result_vect[i][j][k][0] += ATF_rmf00[i][j][k][0][0] * 
		                        vect[i+1][j][k][0];
	    }
	}
    }

    /* Multiplikation mit d4 */
    for( i=1; i<n1; i++ ){
	for( j=1; j<n2; j++ ){
	    for( k=1; k<n3; k++){
		result_vect[i][j][k][0] += ATF_rm0b0[i][j][k][0][0] * 
		                        vect[i][j-1][k][0];
	    }
	}
    }

    /*Multiplikation mit d5*/
    if( ATF_rand_zu)
	upperlimit = n2-1;
    else
   	upperlimit = n2;
    
    for( i=1; i < n1; i++ ){
	for( j=1; j < upperlimit; j++ ){
	    for( k=1; k<n3; k++){
		result_vect[i][j][k][0] += ATF_rm0f0[i][j][k][0][0] * 
		                        vect[i][j+1][k][0];
	    }
	}
    }

    /* Multiplikation mit d6*/
    for( i=1; i<n1; i++ ){
	for( j=1; j<n2; j++ ){
	    for( k=1; k<n3; k++){
		result_vect[i][j][k][0] += ATF_rm00b[i][j][k][0][0] * 
		                        vect[i][j][k-1][0];
	    }
	}
    }

    /*Multiplikation mit d7*/
    if( ATF_rand_symo)
	upperlimit = n3-1;
    else
	upperlimit = n3;
    
    for( i=1; i<n1; i++ ){
	for( j=1; j<n2; j++ ){
	    for( k=1; k<upperlimit; k++){
		result_vect[i][j][k][0] += ATF_rm00f[i][j][k][0][0] * 
		                        vect[i][j][k+1][0];
	    }
	}
    }

    return ATF_SUCCESS;
}
