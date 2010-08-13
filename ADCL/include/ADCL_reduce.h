#ifndef __ADCL_REDUCE__
#define __ADCL_REDUCE__

#include "ADCL.h"
#include "ADCL_internal.h"

#define ADCL_TAG_REDUCE 125
#define ADCL_METHOD_REDUCE_TOTAL_NUM 22
#define ADCL_ATTR_REDUCE_TOTAL_NUM  2
#define ADCL_ATTR_REDUCE_ALG_MAX 7
#define ADCL_ATTR_REDUCE_SEGSIZES 4

extern ADCL_function_t *ADCL_reduce_functions[ADCL_METHOD_REDUCE_TOTAL_NUM];

int ADCL_ddt_copy_content_same_ddt(MPI_Datatype dtype, int count, char* dest, char* src);
int ADCL_op_reduce( MPI_Op op, char *source, char *target,
                                  int count, MPI_Datatype dtype); 

int ADCL_basic_init ( MPI_Datatype btype, int cnt, MPI_Datatype **dats, int** cnts ); 
void ADCL_basic_free (MPI_Datatype **dats, int **cnts ); 

#define ADCL_COMPUTE_SEGCOUNT(SEGSIZE, TYPELNG, SEGCOUNT)        \
    if( ((SEGSIZE) >= (TYPELNG)) &&                                     \
        ((SEGSIZE) < ((TYPELNG) * (SEGCOUNT))) ) {                      \
        size_t residual;                                                \
        (SEGCOUNT) = (int)((SEGSIZE) / (TYPELNG));                      \
        residual = (SEGSIZE) - (SEGCOUNT) * (TYPELNG);                  \
        if( residual > ((TYPELNG) >> 1) )                               \
            (SEGCOUNT)++;                                               \
    }        

#if defined COMMMODE
#if COMMMODE == 1

  #define REDUCE_SEGSIZE 4096
  #define ADCL_REDUCE_BINARY ADCL_reduce_binary_segsize4k
  #define ADCL_REDUCE_BINOMIAL ADCL_reduce_binomial_segsize4k
  #define ADCL_REDUCE_CHAIN ADCL_reduce_chain_segsize4k
  #define ADCL_REDUCE_PIPELINE ADCL_reduce_pipeline_segsize4k
  #define ADCL_REDUCE_IN_ORDER_BINARY ADCL_reduce_in_order_binary_segsize4k

#elif COMMMODE == 2

  #define REDUCE_SEGSIZE 32768
  #define ADCL_REDUCE_BINARY ADCL_reduce_binary_segsize32k
  #define ADCL_REDUCE_BINOMIAL ADCL_reduce_binomial_segsize32k
  #define ADCL_REDUCE_CHAIN ADCL_reduce_chain_segsize32k
  #define ADCL_REDUCE_PIPELINE ADCL_reduce_pipeline_segsize32k
  #define ADCL_REDUCE_IN_ORDER_BINARY ADCL_reduce_in_order_binary_segsize32k

#elif COMMMODE == 3

  #define REDUCE_SEGSIZE 131072
  #define ADCL_REDUCE_BINARY ADCL_reduce_binary_segsize128k
  #define ADCL_REDUCE_BINOMIAL ADCL_reduce_binomial_segsize128k
  #define ADCL_REDUCE_CHAIN ADCL_reduce_chain_segsize128k
  #define ADCL_REDUCE_PIPELINE ADCL_reduce_pipeline_segsize128k
  #define ADCL_REDUCE_IN_ORDER_BINARY ADCL_reduce_in_order_binary_segsize128k
#elif COMMMODE == 4

  #define REDUCE_SEGSIZE 0
  #define ADCL_REDUCE_BINARY ADCL_reduce_binary_segsize0
  #define ADCL_REDUCE_BINOMIAL ADCL_reduce_binomial_segsize0
  #define ADCL_REDUCE_CHAIN ADCL_reduce_chain_segsize0
  #define ADCL_REDUCE_PIPELINE ADCL_reduce_pipeline_segsize0
  #define ADCL_REDUCE_IN_ORDER_BINARY ADCL_reduce_in_order_binary_segsize0
#endif
#endif

void ADCL_reduce_native( ADCL_request_t *req );

void ADCL_reduce_linear( ADCL_request_t *req );

void ADCL_reduce_binary_segsize0( ADCL_request_t *req );
void ADCL_reduce_binomial_segsize0( ADCL_request_t *req );
void ADCL_reduce_chain_segsize0( ADCL_request_t *req );
void ADCL_reduce_pipeline_segsize0( ADCL_request_t *req );
void ADCL_reduce_in_order_binary_segsize0( ADCL_request_t *req );

void ADCL_reduce_binary_segsize4k( ADCL_request_t *req );
void ADCL_reduce_binary_segsize32k( ADCL_request_t *req );
void ADCL_reduce_binary_segsize128k( ADCL_request_t *req );

void ADCL_reduce_binomial_segsize4k( ADCL_request_t *req );
void ADCL_reduce_binomial_segsize32k( ADCL_request_t *req );
void ADCL_reduce_binomial_segsize128k( ADCL_request_t *req );

void ADCL_reduce_chain_segsize4k( ADCL_request_t *req );
void ADCL_reduce_chain_segsize32k( ADCL_request_t *req );
void ADCL_reduce_chain_segsize128k( ADCL_request_t *req );

void ADCL_reduce_pipeline_segsize4k( ADCL_request_t *req );
void ADCL_reduce_pipeline_segsize32k( ADCL_request_t *req );
void ADCL_reduce_pipeline_segsize128k( ADCL_request_t *req );

void ADCL_reduce_in_order_binary_segsize4k( ADCL_request_t *req );
void ADCL_reduce_in_order_binary_segsize32k( ADCL_request_t *req );
void ADCL_reduce_in_order_binary_segsize128k( ADCL_request_t *req );
#endif
