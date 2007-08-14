/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef ADCL_GLOBAL_H
#define ADCL_GLOBAL_H
#include "ADCL.h"


extern ADCL_Vector adcl_Vec_dq;
extern ADCL_Vector adcl_Vec_loes;
extern ADCL_Vector adcl_Vec_rhs;

extern ADCL_Topology ADCL_topo;

extern ADCL_Request adcl_Req_dq;
extern ADCL_Request adcl_Req_loes;
extern ADCL_Request adcl_Req_rhs;

extern MPI_Comm ADCL_Cart_comm;


int ATF_Matmul(ADCL_Request , double ****, double ****);

#endif

