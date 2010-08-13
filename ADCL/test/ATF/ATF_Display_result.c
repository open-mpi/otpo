/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ATF.h"
#include "ATF_Display_Result.h"
/*#include "ATF_Matmul.h"*/
#include "ATF_Memory.h"

#include "ADCL.h"
#include "ATF_Adcl_global.h"

int ATF_Display_Result()
{

  int i,j,k;
  int n1, n2, n3;
  int rank, size;

  double res_max, zwischenspeicher, betrag;
  double l0, l2, res;
  /*
  double ****zwischen_vekt;
  double ****zwischen_vekt_2;
  */
  double ****tmp_vect;
  double ****tmp_vect_2;
  
  int dim[4];	

  MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
  MPI_Comm_size ( MPI_COMM_WORLD, &size );
  
  l0  = 0.0;
  l2  = 0.0;
  res = 0.0;

  n1 = ATF_dim[0]+1;
  n2 = ATF_dim[1]+1;
  n3 = ATF_dim[2]+1;

  dim[0] = ATF_dim[0]+2;
  dim[1] = ATF_dim[1]+2;
  dim[2] = ATF_dim[2]+2;
  dim[3] = 1; 

  ATF_allocate_4D_double_matrix(&tmp_vect,dim);
  ATF_allocate_4D_double_matrix(&tmp_vect_2,dim);

/*Berechnen des Residuums, sowie Speichern des groessten
   Elements*/
/*  ATF_Matmul(ATF_dq, zwischen_vekt, patt_fcfs);*/
  ATF_Matmul( adcl_Req_dq,ATF_dq, tmp_vect);

  res_max = 0;

  for(i=1; i<n1; i++ ){
	for(j=1; j<n2; j++){
	  for(k=1; k<n3; k++){
		tmp_vect_2 [i][j][k][0] = ATF_rhs[i][j][k][0] - tmp_vect[i][j][k][0];
		betrag = abs ( tmp_vect_2 [i][j][k][0] );

		if ( betrag > res_max){
			res_max = betrag;
		}
		l2 = l2 + tmp_vect_2 [i][j][k][0] * tmp_vect_2 [i][j][k][0];
	  }
	}
  }

  MPI_Reduce ( &l2, &zwischenspeicher, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  l2 = sqrt ( zwischenspeicher );  

  MPI_Reduce ( &res_max, &l0, 1, MPI_DOUBLE,  MPI_MAX, 0, MPI_COMM_WORLD );

  /*loesung_bekannt Not defined*/
  if ( ATF_loesung_bekannt ){

    for(i=1; i<n1; i++ ){
	  for(j=1; j<n2; j++){
	    for(k=1; k<n3; k++){
          res = res + ( ATF_dq[i][j][k][0] - ATF_loes[i][j][k][0] )* ( ATF_dq[i][j][k][0] - ATF_loes[i][j][k][0]);
	    }
	  }
    }

    MPI_Allreduce ( &res, &zwischenspeicher, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

    res = sqrt ( zwischenspeicher);

  }

/*...Knoten Null macht nun die Bildschirmausgabe*/
  if ( rank == 0 ){

	ATF_solvtime  = ATF_solv_ende - ATF_solv_anfang;

	printf("==================================================\n");
/*	printf("%s\n",set_text);*/
	printf("Problem size : %d X %d X %d\n", ATF_n1g, ATF_n2g, ATF_n3g);
/*	printf("%s\n",solver_text); */
/*	printf ("%s\n",precon_text);*/
/*	printf("%s\n",compattern_text);
*/	printf("==================================================\n");
	printf("Metacomputing Parameters\n");

	printf("Number of procs: %d\n", size);
	printf("Number of hosts: %d\n", ATF_nhosts);

	if ( ATF_nhosts > 1 ){
	  printf("Process distribution:\n");

	  for( i = 1;i <= (ATF_nhosts-1); i++){
	    printf(" Host: %d, ranks: %d-%d\n",i, ATF_firstranks[i], ATF_firstranks[i+1]-1);
	  }

		printf(" Host: %d, ranks: %d-%d\n", ATF_nhosts, ATF_firstranks[ATF_nhosts] ,size-1);
	}

	printf("==================================================\n");
		
	/*	printf("Number of iterations          : %d\n", r_nit);*/
	printf("Required residuum             : %f\n", ATF_tol);
	printf("L2 - Residuum                 : %f\n", l2);
	printf("Largest single  proc. residuum: %f\n", l0);

	if ( ATF_loesung_bekannt ){
	  printf("Deviation from correct result : %f\n", res);
	}
		
	printf("==================================================\n");
	printf("Timing\n");
	printf("Time for solving the equations : %f\n",ATF_solvtime );
	
	if ( ATF_commtime != 0 )
	  printf("Time spent in communication  :%f\n", ATF_commtime);
	else
	  printf("Time spent in communication not measured\n");
		
	printf("==================================================\n");

  }

  ATF_free_4D_double_matrix(&tmp_vect_2);
  ATF_free_4D_double_matrix(&tmp_vect);


  return ATF_SUCCESS;
}

