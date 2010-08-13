/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ATF.h"

char SolverText[100];

/*Get solvers from data base*/
int ATF_Solver (int solv)
{
	int rank;
	int nreal;
	
	MPI_Comm_rank( MPI_COMM_WORLD, &rank);
	
	/*What's this?*/
      /*
	if(!ATF_Change_init( pattern )){
            printf("Change Init error!\n");
            return ATF_ERROR;
        }	
      */
	switch( solv ){
            case(solv_tfqmr):
            {
       /*         if( pattern > PATT_SPLIT){*/
                    strcpy(SolverText, "Solver : TFQMR blocking");
                    MPI_Barrier( MPI_COMM_WORLD);
                    ATF_solv_anfang = MPI_Wtime();

                    if(!ATF_Solver_tfqmr(nreal)){
                        printf("Solver_tfqmr1 error!\n");
                    }
				
                     ATF_solv_ende = MPI_Wtime();

         /*       }
		else{
			}
			break;
		}
	*/	
                }
		default:
			break;

	}

	return ATF_SUCCESS;
}

