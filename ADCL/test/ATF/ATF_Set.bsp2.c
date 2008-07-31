/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
/*
Function:		Initiate the matrix, rhs and dq				
Aurthor: 		Eric Huang							
Time:			15/April/2006						
Modified history:								*/
#include "ATF.h"
#include "ATF_Matmul.h"

#include "ATF_Memory.h"
#include "ATF_Adcl_global.h"

double deltax;
double deltay;
double deltaz;

/*...Diskretisieren der Gleichung, belegen der Matrix und der
 **   rechten Seite. Dieses File diskretisiert eine Laplacegleichung,
 **   wobei ein Einheitsquader in n1g,n2g,n3g Punkte
 **   unterteilt wird. Diskretisiert wird mit einer Zentralen-Differenz.
*/  

int ATF_Set()
{
    int i,j,k;
    double hauptdiag, xdiag_1, xdiag_2, ydiag, zdiag;
    
    double deltaquadx ;
    double deltaquady;
    double deltaquadz;
    
    
    deltax = 1/(ATF_n1g - 1.0 );
    deltay = 1/(ATF_n2g - 1.0 );
    deltaz = 1/(ATF_n3g - 1.0 );
    
    
    deltaquadx = deltax * deltax;
    deltaquady = deltay * deltay;
    deltaquadz = deltaz * deltaz;
    
    hauptdiag = (1.0/deltaquadx + 1.0/deltaquady + 1.0/deltaquadz)*2.0;
    
    xdiag_1   = -(1.0/deltaquadx + 1000.0/(2.0*deltax) );
    xdiag_2   = -(1.0/deltaquadx - 1000.0/(2.0*deltax) );
    ydiag = -1.0/deltaquady;
    zdiag = -1.0/deltaquadz;


/*...Kurze Darstellung der Gleichung:
 **  - d^2 u / dx^2 - d^2 u / dy^2 - d^u / dz^2 + 1000 * du/dx = rhs
 **
 **    Aus der Zentralen Differenz folgt, dass auf der Hauptdiagonalen
 **    immer 6/deltaquad steht, und auf den Nebendiagonalen -1/deltaquad,
*/      
    /*ATF_rm000[0][0][0][0][0] and ATF_rm000[last][last][last][0][0]
    ** are not set values, why?*/
    for( i=1; i <= ATF_dim[0]; i++){
      for( j=1; j <= ATF_dim[1]; j++){
    	for( k=1; k<= ATF_dim[2]; k++){
	
            ATF_rm000[i][j][k][0][0] = hauptdiag;
            ATF_rmb00[i][j][k][0][0] = xdiag_1;
            ATF_rmf00[i][j][k][0][0] = xdiag_2;
            ATF_rm0b0[i][j][k][0][0] = ydiag;
            ATF_rm0f0[i][j][k][0][0] = ydiag;
            ATF_rm00b[i][j][k][0][0] = zdiag;
            ATF_rm00f[i][j][k][0][0] = zdiag;
	    }
	}
    }
    
    /* ...Korriegieren der Raender  */
    if(ATF_rand_sing){
      
        for(j=1; j <= ATF_dim[1]; j++){
            for(k=1; k <= ATF_dim[2]; k++){

		ATF_rm000[1][j][k][0][0] = 1.0;
		ATF_rmb00[1][j][k][0][0] = 0.0;
		ATF_rmf00[1][j][k][0][0] = 0.0;
		ATF_rm0b0[1][j][k][0][0] = 0.0;
		ATF_rm0f0[1][j][k][0][0] = 0.0;
		ATF_rm00b[1][j][k][0][0] = 0.0;
		ATF_rm00f[1][j][k][0][0] = 0.0;
	    }
	}
    }
    
    /* Rand_ab */
    if(ATF_rand_ab){
      
	for(j=1; j <= ATF_dim[1]; j++){
	    for(k=1; k <= ATF_dim[2]; k++){
              
		ATF_rm000[ATF_dim[0]][j][k][0][0] = 1.0;
		ATF_rmb00[ATF_dim[0]][j][k][0][0] = 0.0;
		ATF_rmf00[ATF_dim[0]][j][k][0][0] = 0.0;
		ATF_rm0b0[ATF_dim[0]][j][k][0][0] = 0.0;
		ATF_rm0f0[ATF_dim[0]][j][k][0][0] = 0.0;
		ATF_rm00b[ATF_dim[0]][j][k][0][0] = 0.0;
		ATF_rm00f[ATF_dim[0]][j][k][0][0] = 0.0;
	    }
	}
    }
    
    /* Rand_festk*/
    
    if ( ATF_rand_festk ){
      
	for(i=1; i<=ATF_dim[0]; i++){
	    for(k=1; k<=ATF_dim[2]; k++){
              
		ATF_rm000[i][1][k][0][0] = 1.0;
		ATF_rmb00[i][1][k][0][0] = 0.0;
		ATF_rmf00[i][1][k][0][0] = 0.0;
		ATF_rm0b0[i][1][k][0][0] = 0.0;
		ATF_rm0f0[i][1][k][0][0] = 0.0;
		ATF_rm00b[i][1][k][0][0] = 0.0;
		ATF_rm00f[i][1][k][0][0] = 0.0;
	    }
	}
    }
    
    /* Rand_zu*/
    if ( ATF_rand_zu ){
	for( i=1; i<=ATF_dim[0]; i++){
	    for( k=1; k<=ATF_dim[2]; k++){

		ATF_rm000[i][ATF_dim[1]][k][0][0] = 1.0;
                ATF_rmb00[i][ATF_dim[1]][k][0][0] = 0.0;
		ATF_rmf00[i][ATF_dim[1]][k][0][0] = 0.0;
		ATF_rm0b0[i][ATF_dim[1]][k][0][0] = 0.0;
		ATF_rm0f0[i][ATF_dim[1]][k][0][0] = 0.0;
		ATF_rm00b[i][ATF_dim[1]][k][0][0] = 0.0;
		ATF_rm00f[i][ATF_dim[1]][k][0][0] = 0.0;
	    }
	}
    }
    
    /*Rand_symu*/
    
    if ( ATF_rand_symu ){
      
	for(i=1; i<= ATF_dim[0]; i++){
	    for(j=1; j <= ATF_dim[1]; j++){

		ATF_rm000[i][j][1][0][0] = 1.0;
		ATF_rmb00[i][j][1][0][0] = 0.0;
		ATF_rmf00[i][j][1][0][0] = 0.0;
		ATF_rm0b0[i][j][1][0][0] = 0.0;
		ATF_rm0f0[i][j][1][0][0] = 0.0;
		ATF_rm00b[i][j][1][0][0] = 0.0;
		ATF_rm00f[i][j][1][0][0] = 0.0;
	    }
	}
    }
    
    /* Rand_symo */
    if ( ATF_rand_symo ){
      
	for(i=1;i <= ATF_dim[0]; i++){
	    for(j=1; j <= ATF_dim[1]; j++){

		ATF_rm000[i][j][ATF_dim[2]][0][0] = 1.0;
		ATF_rmb00[i][j][ATF_dim[2]][0][0] = 0.0;
		ATF_rmf00[i][j][ATF_dim[2]][0][0] = 0.0;
		ATF_rm0b0[i][j][ATF_dim[2]][0][0] = 0.0;
		ATF_rm0f0[i][j][ATF_dim[2]][0][0] = 0.0;
		ATF_rm00b[i][j][ATF_dim[2]][0][0] = 0.0;
		ATF_rm00f[i][j][ATF_dim[2]][0][0] = 0.0;
	    }
	}
    }
    
    return ATF_SUCCESS;
}

double	ATF_skalar_1 ;
double	ATF_skalar_2 ;
double	ATF_skalar_3 ;
bool ATF_have_solution;
bool ATF_loesung_bekannt;
/*...Setzen der rechten Seite so, dass man die zum Schluss die
 **   erwuenschte Loesung kennt.
 *
 */
   
int ATF_Set_rhs ( int *ATF_dim)
{
    int i, j, k;
    int size, rank;
    int dim[4];
    
    double my_pi = 3.1415926535897932384;
    double x, y, z, xoffset, yoffset, zoffset;
    
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank);
    MPI_Comm_size ( MPI_COMM_WORLD, &size);
    
    ATF_have_solution = true;
    
    /* Determine the global position of this process in the overall
       grid */
    xoffset = ATF_coord[0]*ATF_dim[0];
    yoffset = ATF_coord[1]*ATF_dim[1];
    zoffset = ATF_coord[2]*ATF_dim[2];
    
    dim[0] = ATF_dim[0]+2;
    dim[1] = ATF_dim[1]+2;
    dim[2] = ATF_dim[2]+2;
    dim[3] = 1;
      
    ATF_loesung_bekannt = true;
    
    /*  Die Loesung soll spaeter folgender Funktion entsprechen
     *  u(x, y, z ) = exp (xyz) * sin (pi*x) * sin(pi*y) * sin(pi*z)
    */ 
      
    for(k=1; k<=ATF_dim[2]; k++){
	z = ( zoffset + k - 1) * deltaz;
	if ( z >1.0){
	    z = 1.0;
	}
	
	for(j=1; j <= ATF_dim[1]; j++){
	    y = ( yoffset + j - 1) * deltay;
	    if ( y >1.0){
		y = 1.0;
	    }
	    
	    for(i=1;i <= ATF_dim[0]; i++){
		x = ( xoffset + i - 1) * deltax;
		if ( x >1.0){
		    x = 1.0;
		}
		
		ATF_loes[i][j][k][0] = exp( x*y*z ) * sin( my_pi * x) * 
		    sin( my_pi * y ) * sin ( my_pi * z);
	    }
	}
    }

    ATF_Matmul ( adcl_Req_loes, ATF_loes, ATF_rhs );

    if ( ATF_rand_sing ){
	for(j=1; j<=ATF_dim[1]; j++){
	    for(k=1; k<=ATF_dim[2]; k++){
		ATF_rhs [1][j][k][0] = 0.0;
		ATF_loes[1][j][k][0] = 0.0;
	    }
	}
    }
    
    if ( ATF_rand_ab ){
	
	for(j=1; j<=ATF_dim[1]; j++){
		for(k=1; k<=ATF_dim[2]; k++){

		ATF_rhs [ATF_dim[0]][j][k][0] = 0.0;
		ATF_loes[ATF_dim[0]][j][k][0] = 0.0;
	    }
	}
    }
    
    if ( ATF_rand_festk ){
	for(i=1; i<=ATF_dim[0]; i++){
		for(k=1; k<=ATF_dim[2]; k++){

		ATF_rhs [i][1][k][0] = 0.0;
		ATF_loes[i][1][k][0] = 0.0;
	    }
		}
    }
    
    if ( ATF_rand_zu ){
	
	for(i=1; i<=ATF_dim[0]; i++){
		for(k=1; k<=ATF_dim[2]; k++){
		ATF_rhs [i][ATF_dim[1]][k][0] = 0.0;
		ATF_loes[i][ATF_dim[1]][k][0] = 0.0;
	    }
	}
    }
    
    if ( ATF_rand_symu ){
	for(i=1; i <= ATF_dim[0]; i++){
		for(j=1; j <= ATF_dim[1]; j++){

		ATF_rhs [i][j][1][0] = 0.0;
		ATF_loes[i][j][1][0] = 0.0;
	    }
	}
    }
    
    if ( ATF_rand_symo ){
	
	for(i=1; i<=ATF_dim[0]; i++){
            for(j=1; j<=ATF_dim[1]; j++){

                ATF_rhs [i][j][ATF_dim[2]][0] = 0.0;
		ATF_loes[i][j][ATF_dim[2]][0] = 0.0;
	    }
	}
    }
            
    return ATF_SUCCESS;
}

