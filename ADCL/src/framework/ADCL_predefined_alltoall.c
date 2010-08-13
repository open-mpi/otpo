/*
 * Copyright (c) 2007-2009      HLRS. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include <math.h>
#include "ADCL_alltoall.h"
#include "ADCL_config.h"

const int ADCL_attr_alltoall_alg_native=1000;
const int ADCL_attr_alltoall_alg_pairwise=1001;
const int ADCL_attr_alltoall_alg_linear=1002; 

/* Not yet implemented but foreseen.

const int ADCL_attr_alltoall_transfer_IsendIrecv=1100;
const int ADCL_attr_alltoall_transfer_SendIrecv=1101;
const int ADCL_attr_alltoall_transfer_Sendrecv=1102;
const int ADCL_attr_alltoall_transfer_Send_Recv=1103;
#ifdef MPI_WIN
const int ADCL_attr_alltoall_transfer_FenceGet=1104;
const int ADCL_attr_alltoall_transfer_FencePut=1105;
const int ADCL_attr_alltoall_transfer_PostStartGet=1106;
const int ADCL_attr_alltoall_transfer_PostStartPut=1107;
#endif
*/

ADCL_attribute_t *ADCL_alltoall_attrs[ADCL_ATTR_ALLTOALL_TOTAL_NUM];
ADCL_attrset_t *ADCL_alltoall_attrset;
ADCL_function_t *ADCL_alltoall_functions[ADCL_METHOD_ALLTOALL_TOTAL_NUM];
ADCL_fnctset_t *ADCL_alltoall_fnctset;


int ADCL_predefined_alltoall ( void )
{
    int count;
    int m_alltoall_attrs[2];
    int ADCL_attr_alltoall_alg[ADCL_ATTR_ALLTOALL_ALG_MAX];
    //int ADCL_attr_alltoall_transfer[ADCL_ATTR_ALLTOALL_TRANSFER_MAX];
    

    char * ADCL_attr_alltoall_alg_names[ADCL_ATTR_ALLTOALL_ALG_MAX] = 
        {"Alltoall_native",  "Alltoall_pair", "Alltoall_linear"}; //, "Alltoall_bruck" };

#ifdef MPI_WIN
    char *ADCL_attr_alltoall_transfer_names[ADCL_ATTR_ALLTOALL_TRANSFER_MAX] = { "IsendIrecv", "SendIrecv",
                                                               "Send_Recv", "Sendrecv",
                                                               "FenceGet", "FencePut",
                                                               "StartPostGet","StartPostPut" };
#else
    char *ADCL_attr_alltoall_transfer__names[ADCL_ATTR_ALLTOALL_TRANSFER_MAX] = { "IsendIrecv", "SendIrecv",
									  "Send_Recv", "Sendrecv" };
#endif


    ADCL_attr_alltoall_alg[0] = ADCL_attr_alltoall_alg_native;
    ADCL_attr_alltoall_alg[1] = ADCL_attr_alltoall_alg_pairwise;
    ADCL_attr_alltoall_alg[2] = ADCL_attr_alltoall_alg_linear;


/* Not yet implemented;
   ADCL_attr_alltoall_transfer[0] = ADCL_attr_alltoall_transfer_IsendIrecv;
   ADCL_attr_alltoall_transfer[1] = ADCL_attr_alltoall_transfer_SendIrecv;
   ADCL_attr_alltoall_transfer[2] = ADCL_attr_alltoall_transfer_Sendrecv;
   ADCL_attr_alltoall_transfer[3] = ADCL_attr_alltoall_transfer_Send_Recv;
#ifdef MPI_WIN
   ADCL_attr_alltoall_transfer[4] = ADCL_attr_alltoall_transfer_FenceGet;
   ADCL_attr_alltoall_transfer[5] = ADCL_attr_alltoall_transfer_FencePut;
   ADCL_attr_alltoall_transfer[6] = ADCL_attr_alltoall_transfer_StartPostGet;
   ADCL_attr_alltoall_transfer[7] = ADCL_attr_alltoall_transfer_StartPostPut;
#endif
*/



    ADCL_attribute_create ( ADCL_ATTR_ALLTOALL_ALG_MAX, ADCL_attr_alltoall_alg, 
			    ADCL_attr_alltoall_alg_names, "alltoall_alg", 
			    &ADCL_alltoall_attrs[0] );
/* Not yet implemented 
   ADCL_attribute_create ( ADCL_ATTR_ALLTOALL_TRANSFER_MAX, ADCL_attr_alltoall_transfer,
                           ADCL_attr_alltoall_transfer_names, "alltoall_transfer primitives",
			   &ADCL_alltoall_attrs[1] );
*/
    
    ADCL_attrset_create ( ADCL_ATTR_ALLTOALL_TOTAL_NUM, ADCL_alltoall_attrs, &ADCL_alltoall_attrset );

    count=0;
    /* Register function native, no transfer primitives yet set */
    m_alltoall_attrs[0] = ADCL_attr_alltoall_alg_native;
    /* m_alltoall_attr[1] = ADCL_attr_alltoall_transfer_Sendrecv */
    ADCL_function_create_async ( ADCL_alltoall_native, NULL, ADCL_alltoall_attrset,
        			 m_alltoall_attrs, "Alltoall_native_SR", 
        			 &ADCL_alltoall_functions[count]);
    count++;

    /* Register function pairwise, no transfer primitives yet set */
    m_alltoall_attrs[0] = ADCL_attr_alltoall_alg_pairwise;
    /* m_alltoall_attr[1] = ADCL_attr_alltoall_transfer_Sendrecv */
    ADCL_function_create_async ( ADCL_alltoall_linear_sync, NULL, ADCL_alltoall_attrset,
        			 m_alltoall_attrs, "Alltoall_linear_sync_SR", 
        			 &ADCL_alltoall_functions[count]);
    count++;

    /* Register function pairwise, no transfer primitives yet set */
    m_alltoall_attrs[0] = ADCL_attr_alltoall_alg_pairwise;
    /* m_alltoall_attr[1] = ADCL_attr_alltoall_transfer_Sendrecv */
    ADCL_function_create_async ( ADCL_alltoall_bruck, NULL, ADCL_alltoall_attrset,
        			 m_alltoall_attrs, "Alltoall_bruck_SR", 
        			 &ADCL_alltoall_functions[count]);
    count++;

    /* Register function pairwise, no transfer primitives yet set */
    m_alltoall_attrs[0] = ADCL_attr_alltoall_alg_pairwise;
    /* m_alltoall_attr[1] = ADCL_attr_alltoall_transfer_Sendrecv */
    ADCL_function_create_async ( ADCL_alltoall_pairwise, NULL, ADCL_alltoall_attrset,
        			 m_alltoall_attrs, "Alltoall_pair_SR", 
        			 &ADCL_alltoall_functions[count]);
    count++;

    /* Register function pairwise, no transfer primitives yet set */
    m_alltoall_attrs[0] = ADCL_attr_alltoall_alg_pairwise;
    /* m_alltoall_attr[1] = ADCL_attr_alltoall_transfer_Sendrecv */
    ADCL_function_create_async ( ADCL_alltoall_pairwise_excl, NULL, ADCL_alltoall_attrset,
        			 m_alltoall_attrs, "Alltoall_pair_excl_SR", 
        			 &ADCL_alltoall_functions[count]);
    count++;

    /* Register function linear, no transfer primitives yet set */
    m_alltoall_attrs[0] = ADCL_attr_alltoall_alg_linear;
    /* m_alltoall_attr[1] = ADCL_attr_alltoall_transfer_Sendrecv */
    ADCL_function_create_async ( ADCL_alltoall_linear, NULL, ADCL_alltoall_attrset,
        			 m_alltoall_attrs, "Alltoall_linear_SR", 
        			 &ADCL_alltoall_functions[count]);
    count++;

    /* Register function linear, no transfer primitives yet set */
    m_alltoall_attrs[0] = ADCL_attr_alltoall_alg_linear;
    /* m_alltoall_attr[1] = ADCL_attr_alltoall_transfer_Sendrecv */
    ADCL_function_create_async ( ADCL_alltoall_ladd_block2, NULL, ADCL_alltoall_attrset,
        			 m_alltoall_attrs, "Alltoall_ladd_block2", 
        			 &ADCL_alltoall_functions[count]);
    count++;

    /* Register function linear, no transfer primitives yet set */
    m_alltoall_attrs[0] = ADCL_attr_alltoall_alg_linear;
    /* m_alltoall_attr[1] = ADCL_attr_alltoall_transfer_Sendrecv */
    ADCL_function_create_async ( ADCL_alltoall_ladd_block4, NULL, ADCL_alltoall_attrset,
        			 m_alltoall_attrs, "Alltoall_ladd_block4", 
        			 &ADCL_alltoall_functions[count]);
    count++;

    /* Register function linear, no transfer primitives yet set */
    m_alltoall_attrs[0] = ADCL_attr_alltoall_alg_linear;
    /* m_alltoall_attr[1] = ADCL_attr_alltoall_transfer_Sendrecv */
    ADCL_function_create_async ( ADCL_alltoall_ladd_block8, NULL, ADCL_alltoall_attrset,
        			 m_alltoall_attrs, "Alltoall_ladd_block8", 
        			 &ADCL_alltoall_functions[count]);
    count++;

    if ( count != ADCL_METHOD_ALLTOALL_TOTAL_NUM ) {
	ADCL_printf ("Alltoall: Total Number wrong\n");
	return ADCL_ERROR_INTERNAL;
    }

    ADCL_fnctset_create ( ADCL_METHOD_ALLTOALL_TOTAL_NUM, 
			  ADCL_alltoall_functions, 
			  "Alltoall",
			  &ADCL_alltoall_fnctset );

    return ADCL_SUCCESS;
}
