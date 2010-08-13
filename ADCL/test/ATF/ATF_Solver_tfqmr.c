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
#include "ATF_tfqmr.Cal.h"
/*#include "ATF_Matmul.h"*/

#include "ADCL.h"
#include "ATF_Adcl_global.h"

/* 
** Implementation of the TFQMR solver based on the original paper
** written by Nachtigall. 
*/

static int dump_matrix ( int id);
static int dump_vect  ( double ****vec, int id );
static int dump_vect_short ( double ****vec, int id );

int ATF_Solver_tfqmr( int nreal)
{ 
    int m;
    int rank, size;
    int dim[4];
    
    int dim3[3];
    int nc;
    
    double tfqmr_limit, one, zero, temp;
    double tau, theta, theta_old, eta, eta_old;
    double tfqmr_rho, tfqmr_rho_old, tfqmr_sigma;
    double alpha, tfqmr_c, beta;
    double skalar_1, skalar_2, skalar_3;
    double all_vekt[2], all_erg[2];
    
    double ****tfqmr_y, ****tfqmr_y_old, ****tfqmr_y_old_1;
    
    double ****tfqmr_r,	****tfqmr_v, ****tfqmr_r_tilde;
    double ****tfqmr_d, ****tfqmr_w;
    double ****res;
   
    /*New added for ADCL*/
    double ****tmp_vect_2, ****tmp_vect;
      
    /*Define ADCL objects*/
    ADCL_Vmap adcl_vmap;
    ADCL_Vector  adcl_Vec_tfqmr_y, adcl_Vec_tfqmr_y_old, adcl_Vec_tfqmr_y_old_1;
    ADCL_Request adcl_Req_tfqmr_y, adcl_Req_tfqmr_y_old, adcl_Req_tfqmr_y_old_1;
    
    dim[0] = ATF_dim[0]+2;
    dim[1] = ATF_dim[1]+2;
    dim[2] = ATF_dim[2]+2;
    dim[3] = 1;

    dim3[0] = dim[0];
    dim3[1] = dim[1];
    dim3[2] = dim[2];

    nc = 0;

    ATF_allocate_4D_double_matrix(&tfqmr_y, dim);
    ATF_allocate_4D_double_matrix(&tfqmr_y_old, dim);
    ATF_allocate_4D_double_matrix(&tfqmr_y_old_1, dim);

    ATF_allocate_4D_double_matrix(&tfqmr_r, dim);
    ATF_allocate_4D_double_matrix(&tfqmr_v, dim);
    ATF_allocate_4D_double_matrix(&tfqmr_r_tilde, dim);
    ATF_allocate_4D_double_matrix(&tfqmr_d,dim);
    ATF_allocate_4D_double_matrix(&tfqmr_w,dim);

    ATF_allocate_4D_double_matrix(&res,dim);

    ATF_allocate_4D_double_matrix(&tmp_vect_2, dim);
    ATF_allocate_4D_double_matrix(&tmp_vect,dim);

    /*For adcl library*/
    ADCL_Vmap_halo_allocate ( 1, &adcl_vmap );
    ADCL_Vector_register_generic( 3, dim3, nc, adcl_vmap, MPI_DOUBLE, &(tfqmr_y[0][0][0][0]), &adcl_Vec_tfqmr_y );
    ADCL_Request_create( adcl_Vec_tfqmr_y, ADCL_topo, ADCL_FNCTSET_NEIGHBORHOOD, &adcl_Req_tfqmr_y );

    ADCL_Vector_register_generic( 3, dim3, nc, adcl_vmap, MPI_DOUBLE, &(tfqmr_y_old[0][0][0][0]), &adcl_Vec_tfqmr_y_old );
    ADCL_Request_create( adcl_Vec_tfqmr_y_old, ADCL_topo, ADCL_FNCTSET_NEIGHBORHOOD, &adcl_Req_tfqmr_y_old );

    ADCL_Vector_register_generic( 3, dim3, nc, adcl_vmap, MPI_DOUBLE, &(tfqmr_y_old_1[0][0][0][0]), &adcl_Vec_tfqmr_y_old_1 );
    ADCL_Request_create( adcl_Vec_tfqmr_y_old_1, ADCL_topo, ADCL_FNCTSET_NEIGHBORHOOD, &adcl_Req_tfqmr_y_old_1 );

    tfqmr_limit = 1.0e-09;

    /* Initialize all variables*/
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );

    /*The second one is pointer*/

    /*In ADCL, the function is substituted by ATF_Matmul 
    ATF_Matmul ( ATF_dq, zwischen_vekt_2, pattern);
    */
    ATF_Matmul( adcl_Req_dq, ATF_dq, tmp_vect_2 );

    /*	A=B-C ,where A, B and C have the same dimensions*/

    /*ATF_tfqmr_Cal_A_EQ_B_sub_C(tfqmr_r,  ATF_rhs, zwischen_vekt_2);*/
    ATF_tfqmr_Cal_A_EQ_B_sub_C(tfqmr_r, ATF_rhs, tmp_vect_2);

    zero = 0.0;
    one = 1.0;
    
    ATF_tfqmr_Cal_A_EQ_B( tfqmr_w, tfqmr_r);
    ATF_tfqmr_Cal_A_EQ_B( tfqmr_y, tfqmr_r);
    ATF_tfqmr_Cal_A_EQ_B( tfqmr_y_old, tfqmr_r);
    ATF_tfqmr_Cal_A_EQ_B( tfqmr_y_old_1, tfqmr_r);
    ATF_tfqmr_Cal_A_EQ_c( tfqmr_d, zero);
    ATF_tfqmr_Cal_A_EQ_c( tfqmr_r_tilde, one);
    
    /* ATF_Matmul ( tfqmr_y, tfqmr_v, pattern);*/
    ATF_Matmul(adcl_Req_tfqmr_y, tfqmr_y, tfqmr_v);

    /* Calculate the norm of the vector tfqmr_r */
    skalar_1 = 0.0;
    ATF_tfqmr_Cal_c_EQ_c_plus_A_mul_B( tfqmr_r, tfqmr_r, &skalar_1);
    all_vekt[0] = skalar_1;
    
    theta     = 0.0;
    theta_old = 0.0;
    eta       = 0.0;
    eta_old   = 0.0;
    
    /* scalar produkt: tfqmr_r_tilde^T * tfqmr_r */
    skalar_1 = 0.0;
    ATF_tfqmr_Cal_c_EQ_c_plus_A_mul_B ( tfqmr_r_tilde, tfqmr_r, &skalar_1 );
    all_vekt[1] = skalar_1;
    
    MPI_Allreduce ( all_vekt, all_erg, 2, MPI_DOUBLE, MPI_SUM,MPI_COMM_WORLD);
    
    tau	      = sqrt( all_erg[0] );
    tfqmr_rho     = all_erg[1];
    tfqmr_rho_old = tfqmr_rho;
    
    
/*========================================================================*/
    /*  Start of the main loop */
    for( nreal=1; nreal <= ATF_nit; nreal++){	
/*========================================================================*/
	/* ...tfqmr_sigma = tfqmr_r_tilde^T * tfqmr_v */
	
	skalar_1 = 0.0;
	ATF_tfqmr_Cal_c_EQ_c_plus_A_mul_B( tfqmr_r_tilde,tfqmr_v, &skalar_1);
	MPI_Allreduce ( &skalar_1, &tfqmr_sigma, 1, MPI_DOUBLE,
			MPI_SUM, MPI_COMM_WORLD);
	alpha = tfqmr_rho_old / tfqmr_sigma;
	
	/* A=B-z*C, where A, B and C are matrix, z is variable*/
	ATF_tfqmr_Cal_A_EQ_B_sub_z_mul_C( tfqmr_y_old, tfqmr_y_old_1, 
					  tfqmr_v, alpha );
	
/*************************************************************************/
	/* ...Inner loop of the  CGS-Algorithm*/
	for(m=(2*nreal-1); m<= 2*nreal; m++){
/************************************************************************/
	    if ( m == (2*nreal-1) ){
		/*ATF_Matmul( tfqmr_y_old_1,zwischen_vekt_2, pattern);*/
                
                ATF_Matmul( adcl_Req_tfqmr_y_old_1, tfqmr_y_old_1, tmp_vect_2);
	    }
	    else{
		/*ATF_Matmul( tfqmr_y_old,zwischen_vekt_2, pattern);*/
                ATF_Matmul( adcl_Req_tfqmr_y_old, tfqmr_y_old, tmp_vect_2);
	    }
	    
	    /* A=B-Z*C, where A B and C are all matrix */
	    /*changed ATF_tfqmr_Cal_A_EQ_B_sub_z_mul_C(tfqmr_w, tfqmr_w,
					     zwischen_vekt_2, alpha); */
            ATF_tfqmr_Cal_A_EQ_B_sub_z_mul_C( tfqmr_w, tfqmr_w, tmp_vect_2, alpha);
	    
	    skalar_1 = 0.0;
	    ATF_tfqmr_Cal_c_EQ_c_plus_A_mul_B( tfqmr_w, tfqmr_w, &skalar_1);
	    
	    MPI_Allreduce ( &skalar_1, &skalar_2, 1, MPI_DOUBLE, 
			    MPI_SUM, MPI_COMM_WORLD);
	    skalar_3 = sqrt ( skalar_2 );
	    
	    theta = skalar_3/ tau;
	    tfqmr_c = 1 / sqrt ( 1 + theta * theta );
	    tau = tau * theta * tfqmr_c;
	    eta = tfqmr_c * tfqmr_c * alpha;
	    temp = theta_old * theta_old * eta_old/alpha;
	    
	    /* A=z*B, where A and B are all matrix, make program faster */
	    ATF_tfqmr_Cal_A_EQ_c_mul_B( tmp_vect_2, tfqmr_d, temp);
	    
	    if ( m == (2*nreal-1)){
		/* A=B+C, B is vector, A and C are matrix */
		ATF_tfqmr_Cal_A_EQ_B_plus_C( tfqmr_d, tfqmr_y_old_1, 
					     tmp_vect_2);
	    }
	    else{
		ATF_tfqmr_Cal_A_EQ_B_plus_C( tfqmr_d, tfqmr_y_old, 
					     tmp_vect_2);
	    }
	    
	    /* Calculate the new estimate of the solution dq */
	    ATF_tfqmr_Cal_dq_EQ_dq_plus_z_mul_A( tfqmr_d, eta);
	    
/*  HIER MUSS DIE UEBERPRUEFUNG DER ABBRUCHBEDINGUNG NOCH HINEIN!!!
    NACHTIGALL SCHLAEGT VOR:
    || tfqmr_r || <= SQRT ( m +  1 ) * tau
    MOEGLICH WAERE ABER AUCH
    res = A*rhs -dq
    || res || <= qmr_tol
    
    skalar_1 = m + 1.0
    skalar_3 = SQRT ( skalar_1) * tau
    write (*,*) rank, ' : res = ', skalar_3
    if ( skalar_3.lt.tol ) then
    goto 1001
    end if
*/
	    /*	calculate the residuum */
	    
	    /* ATF_Matmul ( ATF_dq, zwischen_vekt_2, pattern); */
            ATF_Matmul( adcl_Req_dq, ATF_dq, tmp_vect_2);
	    ATF_tfqmr_Cal_A_EQ_B_sub_C( res,  ATF_rhs, tmp_vect_2);
	    
	    skalar_1 = 0.0;
	    ATF_tfqmr_Cal_c_EQ_c_plus_A_mul_B( res, res, &skalar_1);
	    
	    MPI_Allreduce ( &skalar_1, &skalar_2, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	    skalar_3 = sqrt ( skalar_2 );
	    
	    if ( skalar_3 < ATF_tol ){
		goto OUT;
	    }
	    /*Kopieren einiger Parameter, von denen schon in der inneren
	     *Schleife die alten Werte gebraucht werden.*/
	    
	    eta_old       = eta;
	    theta_old     = theta;
	}	
/***********************************************************************
...Ende der inneren Schleife*/
	/* 500       continue;*/
/***********************************************************************/
	
	if ( ATF_verbose == 1 ){
	    if ( rank == 0 ){
		printf("%d, %16.14E\n",nreal, skalar_3);
	    }
	}
	
	skalar_1 = 0.0;
	
	ATF_tfqmr_Cal_c_EQ_c_plus_A_mul_B( tfqmr_r_tilde,tfqmr_w, &skalar_1);
	
	MPI_Allreduce ( &skalar_1, &tfqmr_rho, 1, MPI_DOUBLE,MPI_SUM, 
			MPI_COMM_WORLD);
	beta = tfqmr_rho / tfqmr_rho_old;
	
	/*A = B+z*C*/
	ATF_tfqmr_Cal_A_EQ_B_plus_z_mul_C( tfqmr_y,tfqmr_w, tfqmr_y_old, beta);
	
	/*ATF_Matmul ( tfqmr_y_old, zwischen_vekt_2, pattern);*/
        ATF_Matmul(adcl_Req_tfqmr_y_old, tfqmr_y_old, tmp_vect_2);
	
	ATF_tfqmr_Cal_A_EQ_B_plus_z_mul_C( tmp_vect, tmp_vect_2,
					  tfqmr_v, beta);
	
	ATF_Matmul ( adcl_Req_tfqmr_y, tfqmr_y, tmp_vect_2);
	
	ATF_tfqmr_Cal_A_EQ_B_plus_z_mul_C( tfqmr_v, tmp_vect_2,
					   tmp_vect, beta);
	
	/* Si!hern der Variablen, von denen spaeter auch der alte Wert noch
	   benoetigt wird*/
	
	tfqmr_rho_old = tfqmr_rho;
	ATF_tfqmr_Cal_A_EQ_B(tfqmr_y_old_1, tfqmr_y);
	

	
    } /* end of main loop */

  

    /* exit point in case the algorithm breaks down */    
 OUT:
    
    ATF_free_4D_double_matrix(&tfqmr_y);
    ATF_free_4D_double_matrix(&tfqmr_y_old);
    ATF_free_4D_double_matrix(&tfqmr_y_old_1);
    ATF_free_4D_double_matrix(&tfqmr_r);
    ATF_free_4D_double_matrix(&tfqmr_v);
    ATF_free_4D_double_matrix(&tfqmr_r_tilde);
    ATF_free_4D_double_matrix(&tfqmr_d);
    ATF_free_4D_double_matrix(&tfqmr_w);
    ATF_free_4D_double_matrix(&tmp_vect);
    ATF_free_4D_double_matrix(&tmp_vect_2);
    ATF_free_4D_double_matrix(&res);
    
    /*Free the ADCL objects*/
    ADCL_Request_free( &adcl_Req_tfqmr_y);
    ADCL_Request_free( &adcl_Req_tfqmr_y_old);
    ADCL_Request_free( &adcl_Req_tfqmr_y_old_1);

    ADCL_Vector_deregister( &adcl_Vec_tfqmr_y);
    ADCL_Vector_deregister( &adcl_Vec_tfqmr_y_old);
    ADCL_Vector_deregister( &adcl_Vec_tfqmr_y_old_1);
    ADCL_Vmap_free ( &adcl_vmap);
    
    return ATF_SUCCESS;
}


static int dump_matrix ( int id)
{
    char name[32];
    int i, j, k;
    int rank;
    FILE *fd;

    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    sprintf (name, "%d_%d.out", rank, id );
    
    fd = fopen ( name, "w" );
    if ( fd == NULL ) {
	MPI_Abort ( MPI_COMM_WORLD, 1 );
    }

    fprintf (fd, " rm000\n");
    fprintf (fd, " ============================\n");

    for (k=1; k< ATF_dim[2]+1; k++ ) {
	for (j=1; j< ATF_dim[1]+1; j++ ) {
	    for (i=1; i< ATF_dim[0]+1; i++ ) {
		fprintf(fd,"%12d :%12d %11d %11d   %16.14lf     \n", 
			rank, i, j, k, ATF_rm000[i][j][k][0][0] );
	    }
	}
    }


    fprintf (fd, " rmb00\n");
    fprintf (fd, " ============================\n");
    
    for (k=1; k< ATF_dim[2]+1; k++ ) {
	for (j=1; j< ATF_dim[1]+1; j++ ) {
	    for (i=1; i< ATF_dim[0]+1; i++ ) {
		fprintf(fd,"%12d :%12d %11d %11d   %16.14lf     \n", 
			rank, i, j, k, ATF_rmb00[i][j][k][0][0] );
	    }
	}
    }

    fprintf (fd, " rm0b0\n");
    fprintf (fd, " ============================\n");

    for (k=1; k< ATF_dim[2]+1; k++ ) {
	for (j=1; j< ATF_dim[1]+1; j++ ) {
	    for (i=1; i< ATF_dim[0]+1; i++ ) {
		fprintf(fd,"%12d :%12d %11d %11d   %16.14lf     \n", 
			rank, i, j, k, ATF_rm0b0[i][j][k][0][0] );
	    }
	}
    }

    fprintf (fd, " rm00b\n");
    fprintf (fd, " ============================\n");

    for (k=1; k< ATF_dim[2]+1; k++ ) {
	for (j=1; j< ATF_dim[1]+1; j++ ) {
	    for (i=1; i< ATF_dim[0]+1; i++ ) {
		fprintf(fd,"%12d :%12d %11d %11d   %16.14lf     \n", 
			rank, i, j, k, ATF_rm00b[i][j][k][0][0] );
	    }
	}
    }


    fprintf (fd, " rmf00\n");
    fprintf (fd, " ============================\n");

    for (k=1; k< ATF_dim[2]+1; k++ ) {
	for (j=1; j< ATF_dim[1]+1; j++ ) {
	    for (i=1; i< ATF_dim[0]+1; i++ ) {
		fprintf(fd,"%12d :%12d %11d %11d   %16.14lf     \n", 
			rank, i, j, k, ATF_rmf00[i][j][k][0][0] );
	    }
	}
    }

    fprintf (fd, " rm0f0\n");
    fprintf (fd, " ============================\n");

    for (k=1; k< ATF_dim[2]+1; k++ ) {
	for (j=1; j< ATF_dim[1]+1; j++ ) {
	    for (i=1; i< ATF_dim[0]+1; i++ ) {
		fprintf(fd,"%12d :%12d %11d %11d   %16.14lf     \n", 
			rank, i, j, k, ATF_rm0f0[i][j][k][0][0] );
	    }
	}
    }

    fprintf (fd, " rm00f\n");
    fprintf (fd, " ============================\n");

    for (k=1; k< ATF_dim[2]+1; k++ ) {
	for (j=1; j< ATF_dim[1]+1; j++ ) {
	    for (i=1; i< ATF_dim[0]+1; i++ ) {
		fprintf(fd,"%12d :%12d %11d %11d   %16.14lf     \n", 
			rank, i, j, k, ATF_rm00f[i][j][k][0][0] );
	    }
	}
    }

    fclose ( fd );
    return ATF_SUCCESS;
}

static int dump_vect  ( double ****vec, int id )
{
    char name[32];
    int i, j, k;
    int rank;
    FILE *fd;

    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    sprintf (name, "%d_%d.out", rank, id );
    
    fd = fopen ( name, "w" );
    if ( fd == NULL ) {
	MPI_Abort ( MPI_COMM_WORLD, 1 );
    }

    fprintf (fd, " New entry\n");
    fprintf (fd, " ============================\n");

    for (k=0; k< ATF_dim[2]+2; k++ ) {
	for (j=0; j< ATF_dim[1]+2; j++ ) {
	    for (i=0; i< ATF_dim[0]+2; i++ ) {
		fprintf(fd,"%12d :%12d %11d %11d   %16.14lf     \n", 
			rank, i, j, k, vec[i][j][k][0] );
	    }
	}
    }

    fclose ( fd );
    return ATF_SUCCESS;
}

static int dump_vect_short ( double ****vec, int id )
{
    char name[32];
    int i, j, k;
    int rank;
    FILE *fd;

    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    sprintf (name, "%d_%d.out", rank, id );
    
    fd = fopen ( name, "w" );
    if ( fd == NULL ) {
	MPI_Abort ( MPI_COMM_WORLD, 1 );
    }

    fprintf (fd, " New entry\n");
    fprintf (fd, " ============================\n");

    for (k=1; k< ATF_dim[2]+1; k++ ) {
	for (j=1; j< ATF_dim[1]+1; j++ ) {
	    for (i=1; i< ATF_dim[0]+1; i++ ) {
		fprintf(fd,"%12d :%12d %11d %11d   %16.14lf     \n", 
			rank, i, j, k, vec[i][j][k][0] );
	    }
	}
    }

    fclose ( fd );
    return ATF_SUCCESS;
}
