/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ATF.h"

/*Get solvers from data base*/
int ATF_Get_solver(int n, int *solv)
{
    *solv = ATF_solvarr[n];
    return ATF_SUCCESS;
}

/*Get solvers from data base*/
/*
int ATF_Get_pattern(int n, int *patt)
{
    *patt = ATF_patternarr[n];
    return ATF_SUCCESS;
}
*/
/*Get solvers from data base*/
int ATF_Reset_dq ( void )
{
    int i, j, k, l;
    int nc =1;
    
    /* Set the whole vector to zero! */
    for( i=0; i<=ATF_dim[0]+1; i++ ){
        for( j=0; j<=ATF_dim[1]+1; j++ ){
    	    for( k=0; k<=ATF_dim[2]+1; k++ ){
                for( l=0; l<nc; l++ ){
		    ATF_dq[i][j][k][l] = 0.0;
		}
	    }
	}
    }

    return ATF_SUCCESS;
}

int ATF_Get_problemsize (int n , int *px,int *py,int *pz)
{
    *px = * (ATF_problemsx + n);
    *py = * (ATF_problemsy + n);
    *pz = * (ATF_problemsz + n);
	
    return ATF_SUCCESS;
}


