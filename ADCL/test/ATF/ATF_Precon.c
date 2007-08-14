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
/*  Vorkonditionierung der Matrix durch ein einfaches Jakobi-
 **   Verfahren. Dabei wird die Hauptdiagonale als einfachste
 **   Naeherung der Matrix genommen und invertiert.
*/

int ATF_Precon()
{
    int i, j, k;
    int dim4[4];

    double ****precon_m;

    dim4[0] = ATF_dim[0]+2;
    dim4[1] = ATF_dim[1]+2;
    dim4[2] = ATF_dim[2]+2;
    dim4[3] = 1;

    ATF_allocate_4D_double_matrix(&precon_m,dim4);


    /* Set up preconditioning matrix. Since we are using a simple Jacobi
       preconditioner, the preconditioning matrix is the inverse of the main
       diagonal */

    for(i=1; i <= ATF_dim[0]; i++){
      for(j=1; j <= ATF_dim[1]; j++){
        for(k=1; k <= ATF_dim[2]; k++){

          if (ATF_rm000[i][j][k][0][0] != 0 )
		precon_m[i][j][k][0] = 1.0 / ATF_rm000[i][j][k][0][0];
	    else
		precon_m[i][j][k][0]= 0.0;
        }
      }
    }
   
/* Multiplizieren der rechten Seite und der Matrixdiagonalen
   mit der Preconditionierungsmatrix
*/      
    ATF_Precon_Matset (precon_m);
    ATF_free_4D_double_matrix (&precon_m);

    return ATF_SUCCESS;
}


/* Nach dem berechnen der Vorkonditionierungsmatrix, muss die Matrix
 ** mit der inversen multipliziert werden. Aufgrund der einfachen
 ** Struktur der Matrix und der Vorkonditionierungsmatrix
 ** wird das Diagonale fuer Diagonale erledigt
*/


int ATF_Precon_Matset (double **** precon_m)
{
    /* Multiply the matrix with the preconditioning matrix */
    int i, j, k;

    for(i=1; i <= ATF_dim[0]; i++){
	for(j=1; j <= ATF_dim[1]; j++){
	    for(k=1; k <= ATF_dim[2]; k++){

              ATF_rhs[i][j][k][0]=ATF_rhs[i][j][k][0] * precon_m[i][j][k][0];
	    }
	}
    }

    for(i=1; i <= ATF_dim[0]; i++){
	for(j=1; j <= ATF_dim[1]; j++){
	    for(k=1; k <= ATF_dim[2];k++){
		ATF_rm000[i][j][k][0][0] = ATF_rm000[i][j][k][0][0]*precon_m[i][j][k][0];
		ATF_rmb00[i][j][k][0][0] = ATF_rmb00[i][j][k][0][0]*precon_m[i][j][k][0];
		ATF_rmf00[i][j][k][0][0] = ATF_rmf00[i][j][k][0][0]*precon_m[i][j][k][0];
		ATF_rm0b0[i][j][k][0][0] = ATF_rm0b0[i][j][k][0][0]*precon_m[i][j][k][0];
		ATF_rm0f0[i][j][k][0][0] = ATF_rm0f0[i][j][k][0][0]*precon_m[i][j][k][0];
		ATF_rm00b[i][j][k][0][0] = ATF_rm00b[i][j][k][0][0]*precon_m[i][j][k][0];
		ATF_rm00f[i][j][k][0][0] = ATF_rm00f[i][j][k][0][0]*precon_m[i][j][k][0];
	    }
	}
    }
    
    return ATF_SUCCESS;
}


