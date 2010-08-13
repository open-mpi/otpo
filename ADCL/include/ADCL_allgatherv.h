#ifndef __ADCL_ALLGATHERV__
#define __ADCL_ALLGATHERV__

#ifdef ADCL_DUMMY_MPI
#include "ADCL_dummy_mpi.h"
#else
#include "mpi.h"
#endif

#define ADCL_TAG_ALLGATHERV 123

/* in change.h

void ADCL_allgatherv_neighborexchange(ADCL_request_t *req);
void ADCL_allgatherv_ring( ADCL_request_t *req );
void ADCL_allgatherv_bruck( ADCL_request_t *req );
void ADCL_allgatherv_linear( ADCL_request_t *req );
void ADCL_allgatherv_two_procs(ADCL_request_t *req );*/



#endif
