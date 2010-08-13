/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef __ATF_H__
#define __ATF_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <ctype.h> 
#include <string.h>
#include <math.h>
#include "mpi.h"

#define ATF_SUCCESS 1
#define ATF_ERROR   0
#define patt_fcfs 102
#define solv_tfqmr 1
#define PATT_SPLIT 200

/* The global variables: */

extern int ATF_maxnproblem;
extern int ATF_maxnsolver;
/* extern int ATF_maxnpattern; */

extern int ATF_nhosts ;
extern int ATF_verbose;
extern int ATF_autos ;
extern float ATF_tol;
extern int ATF_nit;

extern int *ATF_firstranks;
extern int *ATF_problemsx;
extern int *ATF_problemsy;
extern int *ATF_problemsz;
extern int *ATF_solvarr;
/* extern int *ATF_patternarr;*/


/* For ATF_Init() */

extern int ATF_np[3];
extern int ATF_tid_iu, ATF_tid_io;
extern int ATF_tid_ju, ATF_tid_jo;
extern int ATF_tid_ku, ATF_tid_ko;

extern bool ATF_rand_ab;
extern bool ATF_rand_zu;
extern bool ATF_rand_sing;
extern bool ATF_rand_festk;
extern bool ATF_rand_symo;
extern bool ATF_rand_symu;


extern int *ATF_sendcount;
extern int *ATF_recvcount;
extern MPI_Datatype *ATF_sendtype;
extern MPI_Datatype *ATF_recvtype;
extern MPI_Aint *ATF_senddisps;
extern MPI_Aint *ATF_recvdisps;

extern MPI_Datatype ATF_rand_sing_stype;
extern MPI_Datatype ATF_rand_sing_rtype;
extern MPI_Datatype ATF_rand_ab_stype;
extern MPI_Datatype ATF_rand_ab_rtype;
extern MPI_Datatype ATF_rand_zu_stype;
extern MPI_Datatype ATF_rand_zu_rtype;
extern MPI_Datatype ATF_rand_festk_stype;
extern MPI_Datatype ATF_rand_festk_rtype;
extern MPI_Datatype ATF_rand_symo_stype;
extern MPI_Datatype ATF_rand_symo_rtype;
extern MPI_Datatype ATF_rand_symu_stype;
extern MPI_Datatype ATF_rand_symu_rtype;


/* For ATF_Init_comm; */

extern bool ATF_nb_rand_ab;
extern bool ATF_nb_rand_sing;
extern bool ATF_nb_rand_zu;
extern bool ATF_nb_rand_festk;
extern bool ATF_nb_rand_symo;
extern bool ATF_nb_rand_symu;

/* For ATF_comm */

extern MPI_Comm ATF_comm_rand_ab;
extern MPI_Comm ATF_comm_rand_sing;
extern MPI_Comm ATF_comm_rand_zu;
extern MPI_Comm ATF_comm_rand_festk;
extern MPI_Comm ATF_comm_rand_symo;
extern MPI_Comm ATF_comm_rand_symu;


/* For ATF_Init_matrx; */

extern int ATF_n1g;
extern int ATF_n2g;
extern int ATF_n3g;

extern int ATF_dim[3];
extern int ATF_coord[3];

extern double ATF_solv_ende, ATF_solv_anfang, ATF_solvtime;
extern double ATF_comm_ende, ATF_comm_anfang, ATF_commtime;  

/* Multi-dimension matrix allocation */

extern double *****ATF_rm000;
extern double *****ATF_rmb00;
extern double *****ATF_rmf00;
extern double *****ATF_rm0b0;
extern double *****ATF_rm0f0;
extern double *****ATF_rm00b;
extern double *****ATF_rm00f;

extern double ****ATF_rhs;

extern double ****ATF_dq;
extern double ****ATF_loes;


/*For matrix multiple*/


extern bool ATF_loesung_bekannt;

/* For ATF_Set.bsp2.c  */


extern double deltax;
extern double deltay;
extern double deltaz;
      


extern double ATF_skalar_1;
extern double ATF_skalar_2;
extern double ATF_skalar_3;
extern bool ATF_loesung_bekannt;


/*For ATF_Change.Matmul*/

extern MPI_Request ATF_recvhandle[6];
extern MPI_Request ATF_sendhandle[6];

/* For ATF_Change_init */
extern char ComPatternText[100];
extern char SolverText[100];

int ATF_Read_config(int *, int *);
/* ATF_Main.c calls ATF_Get_sover, ATF_Get_pattern, ATF_Reset_dq, */
/* ATF_Precon, ATF_Solver */
/* They are defined in ATF_Get.c */

int ATF_Get_solver(int , int *);
/* int ATF_Get_pattern(int, int *);*/
int ATF_Reset_dq (void);
int ATF_Precon (void);
int ATF_Precon_Matset (double ****);
/*int ATF_Precon_Matset (double ****);*/

/* ATF_Init.c includes Set_logic and Set_id */
/* ATF calls Set_logic, Set_id and Init_comm */
int ATF_Init(void);
int ATF_Set_logic(void);

/*int ATF_Set_id(void); */
int ATF_Init_comm(void);

int ATF_comm( int *);
int ATF_Get_problemsize (int, int *,int *,int *);


/* ATF_Init_matrix */
int ATF_Init_matrix(int, int, int );
int ATF_Calc(void);


int ATF_Set_Datatypes(double ****);

/*ATF_Set.c includes ATF_Set(), ATF_Set_rhs()*/
int ATF_Set(void);
int ATF_Reset();
int ATF_Set_rhs(int *);

/*ATF_Matmul.c, includes ATF_Matmul()*/
int ATF_Change_fcfs(double ****);

/*ATF_Solver.c*/

int ATF_Solver( int);
int ATF_Solver_tfqmr( int );




#endif

