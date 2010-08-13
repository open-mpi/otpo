#include <math.h>
#include "ADCL_reduce.h"
#include "ADCL_config.h"

const int ADCL_attr_reduce_alg_binary=1000;
const int ADCL_attr_reduce_alg_binomial=1001;
const int ADCL_attr_reduce_alg_chain=1002;
const int ADCL_attr_reduce_alg_pipeline=1003;
const int ADCL_attr_reduce_alg_in_order_binary=1004;
const int ADCL_attr_reduce_alg_native=1005;
const int ADCL_attr_reduce_alg_linear=1006;

const int ADCL_attr_reduce_segsize_4k = 100;
const int ADCL_attr_reduce_segsize_32k = 101;
const int ADCL_attr_reduce_segsize_128k = 102;
const int ADCL_attr_reduce_segsize_0= 103;

ADCL_attribute_t *ADCL_reduce_attrs[ADCL_ATTR_REDUCE_TOTAL_NUM];
ADCL_attrset_t *ADCL_reduce_attrset;
ADCL_function_t *ADCL_reduce_functions[ADCL_METHOD_REDUCE_TOTAL_NUM];
ADCL_fnctset_t *ADCL_reduce_fnctset;

int ADCL_predefined_reduce( void )
{
    int count=0;
    int m_reduce_attr[2];
    int ADCL_attr_reduce_alg[ADCL_ATTR_REDUCE_ALG_MAX];
    int ADCL_attr_reduce_segsize[ADCL_ATTR_REDUCE_SEGSIZES];


    char * ADCL_attr_reduce_alg_names[ADCL_ATTR_REDUCE_ALG_MAX] =
        { "Reduce_binary","Reduce_binomial","Reduce_chain","Reduce_pipeline","Reduce_in_order_binary","Reduce_native","Reduce_linear" }; 

    char *ADCL_attr_reduce_segsize_names[ADCL_ATTR_REDUCE_SEGSIZES] = { "4k","32k","128k","0" };

    ADCL_attr_reduce_alg[0] = ADCL_attr_reduce_alg_binary;
    ADCL_attr_reduce_alg[1] = ADCL_attr_reduce_alg_binomial;
    ADCL_attr_reduce_alg[2] = ADCL_attr_reduce_alg_chain;
    ADCL_attr_reduce_alg[3] = ADCL_attr_reduce_alg_pipeline;
    ADCL_attr_reduce_alg[4] = ADCL_attr_reduce_alg_in_order_binary;
    ADCL_attr_reduce_alg[5] = ADCL_attr_reduce_alg_native;
    ADCL_attr_reduce_alg[6] = ADCL_attr_reduce_alg_linear;

    ADCL_attr_reduce_segsize[0] = ADCL_attr_reduce_segsize_4k;
    ADCL_attr_reduce_segsize[1] = ADCL_attr_reduce_segsize_32k;
    ADCL_attr_reduce_segsize[2] = ADCL_attr_reduce_segsize_128k;
    ADCL_attr_reduce_segsize[3] = ADCL_attr_reduce_segsize_0;

    ADCL_attribute_create ( ADCL_ATTR_REDUCE_ALG_MAX, ADCL_attr_reduce_alg,
                            ADCL_attr_reduce_alg_names, "reduce algorithms",
                            &ADCL_reduce_attrs[0] );
    ADCL_attribute_create ( ADCL_ATTR_REDUCE_SEGSIZES, ADCL_attr_reduce_segsize,
                            ADCL_attr_reduce_segsize_names, "reduce segment sizes",
                            &ADCL_reduce_attrs[1] );

    ADCL_attrset_create ( ADCL_ATTR_REDUCE_TOTAL_NUM, ADCL_reduce_attrs, &ADCL_reduce_attrset );

/* ******************************************************************** */
/* REDUCE - Fortran function set 5                                   */
/* ******************************************************************** */

    count = 0;


    m_reduce_attr[0] = ADCL_attr_reduce_alg_binary;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_4k;

    ADCL_function_create_async ( ADCL_reduce_binary_segsize4k, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_binary_4k",
                                 & ADCL_reduce_functions[count]);
    count++;

    m_reduce_attr[0] = ADCL_attr_reduce_alg_binomial;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_4k;

    ADCL_function_create_async ( ADCL_reduce_binomial_segsize4k, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_binomial_4k",
                                 & ADCL_reduce_functions[count]);
    count++;

    m_reduce_attr[0] = ADCL_attr_reduce_alg_chain;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_4k;

    ADCL_function_create_async ( ADCL_reduce_chain_segsize4k, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_chain_4k",
                                 & ADCL_reduce_functions[count]);
    count++;

    m_reduce_attr[0] = ADCL_attr_reduce_alg_pipeline;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_4k;

    ADCL_function_create_async ( ADCL_reduce_pipeline_segsize4k, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_pipeline_4k",
                                 & ADCL_reduce_functions[count]);
    count++;

    m_reduce_attr[0] = ADCL_attr_reduce_alg_in_order_binary;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_4k;
   
    ADCL_function_create_async ( ADCL_reduce_in_order_binary_segsize4k, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_in_order_binary_4k",
                                 & ADCL_reduce_functions[count]);
    count++;


    m_reduce_attr[0] = ADCL_attr_reduce_alg_binary;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_32k;

    ADCL_function_create_async ( ADCL_reduce_binary_segsize32k, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_binary_32k",
                                 & ADCL_reduce_functions[count]);
    count++;

    m_reduce_attr[0] = ADCL_attr_reduce_alg_binomial;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_32k;

    ADCL_function_create_async ( ADCL_reduce_binomial_segsize32k, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_binomial_32k",
                                 & ADCL_reduce_functions[count]);
    count++;

    m_reduce_attr[0] = ADCL_attr_reduce_alg_chain;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_32k;

    ADCL_function_create_async ( ADCL_reduce_chain_segsize32k, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_chain_32k",
                                 & ADCL_reduce_functions[count]);
    count++;

    m_reduce_attr[0] = ADCL_attr_reduce_alg_pipeline;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_32k;

    ADCL_function_create_async ( ADCL_reduce_pipeline_segsize32k, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_pipeline_32k",
                                 & ADCL_reduce_functions[count]);
    count++;

    m_reduce_attr[0] = ADCL_attr_reduce_alg_in_order_binary;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_32k;
   
    ADCL_function_create_async ( ADCL_reduce_in_order_binary_segsize32k, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_in_order_binary_32k",
                                 & ADCL_reduce_functions[count]);
    count++;


    m_reduce_attr[0] = ADCL_attr_reduce_alg_binary;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_128k;

    ADCL_function_create_async ( ADCL_reduce_binary_segsize128k, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_binary_128k",
                                 & ADCL_reduce_functions[count]);
    count++;
   


    m_reduce_attr[0] = ADCL_attr_reduce_alg_binomial;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_128k;

    ADCL_function_create_async ( ADCL_reduce_binomial_segsize128k, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_binomial_128k",
                                 & ADCL_reduce_functions[count]);
    count++;
    


    m_reduce_attr[0] = ADCL_attr_reduce_alg_chain;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_128k;

    ADCL_function_create_async ( ADCL_reduce_chain_segsize128k, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_chain_128k",
                                 & ADCL_reduce_functions[count]);
    count++;



    m_reduce_attr[0] = ADCL_attr_reduce_alg_pipeline;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_128k;

    ADCL_function_create_async ( ADCL_reduce_pipeline_segsize128k, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_pipeline_128k",
                                 & ADCL_reduce_functions[count]);
    count++;
   


    m_reduce_attr[0] = ADCL_attr_reduce_alg_in_order_binary;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_128k;
   
    ADCL_function_create_async ( ADCL_reduce_in_order_binary_segsize128k, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_in_order_binary_128k",
                                 & ADCL_reduce_functions[count]);
    count++;


    m_reduce_attr[0] = ADCL_attr_reduce_alg_binary;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_0;

    ADCL_function_create_async ( ADCL_reduce_binary_segsize0, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_binary",
                                 & ADCL_reduce_functions[count]);
    count++;

    m_reduce_attr[0] = ADCL_attr_reduce_alg_binomial;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_0;

    ADCL_function_create_async ( ADCL_reduce_binomial_segsize0, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_binomial",
                                 & ADCL_reduce_functions[count]);
    count++;

    m_reduce_attr[0] = ADCL_attr_reduce_alg_chain;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_0;

    ADCL_function_create_async ( ADCL_reduce_chain_segsize0, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_chain",
                                 & ADCL_reduce_functions[count]);
    count++;

    m_reduce_attr[0] = ADCL_attr_reduce_alg_pipeline;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_0;

    ADCL_function_create_async ( ADCL_reduce_pipeline_segsize0, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_pipeline",
                                 & ADCL_reduce_functions[count]);
    count++;

    m_reduce_attr[0] = ADCL_attr_reduce_alg_in_order_binary;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_0;
   
    ADCL_function_create_async ( ADCL_reduce_in_order_binary_segsize0, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_in_order_binary",
                                 & ADCL_reduce_functions[count]);
    count++;

    m_reduce_attr[0] = ADCL_attr_reduce_alg_native;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_0;

    ADCL_function_create_async ( ADCL_reduce_native, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_native",
                                 & ADCL_reduce_functions[count]);
    count++;

    m_reduce_attr[0] = ADCL_attr_reduce_alg_linear;
    m_reduce_attr[1] = ADCL_attr_reduce_segsize_0;

    ADCL_function_create_async ( ADCL_reduce_linear, NULL,
                                 ADCL_reduce_attrset,
                                 m_reduce_attr, "Reduce_linear",
                                 & ADCL_reduce_functions[count]);
    count++;



    ADCL_fnctset_create ( ADCL_METHOD_REDUCE_TOTAL_NUM,
                          ADCL_reduce_functions,
                          "Reduce",
                          &ADCL_reduce_fnctset );


    if ( count != ADCL_METHOD_REDUCE_TOTAL_NUM) {
        ADCL_printf("Total Number wrong\n");
        return ADCL_ERROR_INTERNAL;
    }

    return ADCL_SUCCESS;
}
