/*
 * Copyright (c) 2008-2009      HLRS. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#ifndef __ADCL_ALLTOALL_H__
#define __ADCL_ALLTOALL_H__

#include "ADCL.h"
#include "ADCL_internal.h" 

#define ADCL_ATTR_ALLTOALL_TOTAL_NUM    1 /* will be 2 later */
#define ADCL_METHOD_ALLTOALL_TOTAL_NUM  9 /* will be larger later */
#define ADCL_ATTR_ALLTOALL_ALG_MAX      3 /* Number of underlaying algorithms available */
#define ADCL_ATTR_ALLTOALL_TRANSFER_MAX 8 /* not used right now */

#if COMMMODE == 1
  #define ADCL_ALLTOALL_LADD  ADCL_alltoall_ladd_block2 
  #define BBLOCK 2
#elif COMMMODE == 2
  #define ADCL_ALLTOALL_LADD  ADCL_alltoall_ladd_block4 
  #define BBLOCK 4
#elif COMMMODE == 3
  #define ADCL_ALLTOALL_LADD  ADCL_alltoall_ladd_block8 
  #define BBLOCK 8
#endif

void ADCL_alltoall_ladd_block2 ( ADCL_request_t *req );
void ADCL_alltoall_ladd_block4 ( ADCL_request_t *req );
void ADCL_alltoall_ladd_block8 ( ADCL_request_t *req );

void ADCL_alltoall_pairwise_excl ( ADCL_request_t *req );

#endif /* __ADCL_ALLTOALL_H__ */
