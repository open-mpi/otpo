/*
 * Copyright (c) 2006-2009      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include <math.h>
#include "ADCL_alltoallv.h"
#include "ADCL_config.h"

const int ADCL_attr_alltoallv_alg_native=1000;
const int ADCL_attr_alltoallv_alg_pairwise=1001;
const int ADCL_attr_alltoallv_alg_linear=1002; 

/* Not yet implemented but foreseen.

const int ADCL_attr_alltoallv_transfer_IsendIrecv=1100;
const int ADCL_attr_alltoallv_transfer_SendIrecv=1101;
const int ADCL_attr_alltoallv_transfer_Sendrecv=1102;
const int ADCL_attr_alltoallv_transfer_Send_Recv=1103;
#ifdef MPI_WIN
const int ADCL_attr_alltoallv_transfer_FenceGet=1104;
const int ADCL_attr_alltoallv_transfer_FencePut=1105;
const int ADCL_attr_alltoallv_transfer_PostStartGet=1106;
const int ADCL_attr_alltoallv_transfer_PostStartPut=1107;
#endif
*/

ADCL_attribute_t *ADCL_alltoallv_attrs[ADCL_ATTR_ALLTOALLV_TOTAL_NUM];
ADCL_attrset_t *ADCL_alltoallv_attrset;
ADCL_function_t *ADCL_alltoallv_functions[ADCL_METHOD_ALLTOALLV_TOTAL_NUM];
ADCL_fnctset_t *ADCL_alltoallv_fnctset;


int ADCL_predefined_alltoallv ( void )
{
    int count;
    int m_alltoallv_attrs[2];
    int ADCL_attr_alltoallv_alg[ADCL_ATTR_ALLTOALLV_ALG_MAX];
    int ADCL_attr_alltoallv_transfer[ADCL_ATTR_ALLTOALLV_TRANSFER_MAX];
    

    char * ADCL_attr_alltoallv_alg_names[ADCL_ATTR_ALLTOALLV_ALG_MAX] = {"Alltoallv_native",
									 "Alltoallv_pair",
									 "Alltoallv_linear" };

#ifdef MPI_WIN
    char *ADCL_attr_alltoallv_transfer_names[ADCL_ATTR_ALLTOALLV_TRANSFER_MAX] = { "IsendIrecv", "SendIrecv",
                                                               "Send_Recv", "Sendrecv",
                                                               "FenceGet", "FencePut",
                                                               "StartPostGet","StartPostPut" };
#else
    char *ADCL_attr_alltoallv_transfer__names[ADCL_ATTR_ALLTOALLV_TRANSFER_MAX] = { "IsendIrecv", "SendIrecv",
									  "Send_Recv", "Sendrecv" };
#endif


    ADCL_attr_alltoallv_alg[0] = ADCL_attr_alltoallv_alg_native;
    ADCL_attr_alltoallv_alg[1] = ADCL_attr_alltoallv_alg_pairwise;
    ADCL_attr_alltoallv_alg[2] = ADCL_attr_alltoallv_alg_linear;


/* Not yet implemented;
   ADCL_attr_alltoallv_transfer[0] = ADCL_attr_alltoallv_transfer_IsendIrecv;
   ADCL_attr_alltoallv_transfer[1] = ADCL_attr_alltoallv_transfer_SendIrecv;
   ADCL_attr_alltoallv_transfer[2] = ADCL_attr_alltoallv_transfer_Sendrecv;
   ADCL_attr_alltoallv_transfer[3] = ADCL_attr_alltoallv_transfer_Send_Recv;
#ifdef MPI_WIN
   ADCL_attr_alltoallv_transfer[4] = ADCL_attr_alltoallv_transfer_FenceGet;
   ADCL_attr_alltoallv_transfer[5] = ADCL_attr_alltoallv_transfer_FencePut;
   ADCL_attr_alltoallv_transfer[6] = ADCL_attr_alltoallv_transfer_StartPostGet;
   ADCL_attr_alltoallv_transfer[7] = ADCL_attr_alltoallv_transfer_StartPostPut;
#endif
*/



    ADCL_attribute_create ( ADCL_ATTR_ALLTOALLV_ALG_MAX, ADCL_attr_alltoallv_alg, 
			    ADCL_attr_alltoallv_alg_names, "alltoallv_alg", 
			    &ADCL_alltoallv_attrs[0] );
/* Not yet implemented 
   ADCL_attribute_create ( ADCL_ATTR_ALLTOALLV_TRANSFER_MAX, ADCL_attr_alltoallv_transfer,
                           ADCL_attr_alltoallv_transfer_names, "alltoallv_transfer primitives",
			   &ADCL_alltoallv_attrs[1] );
*/
    
    ADCL_attrset_create ( ADCL_ATTR_ALLTOALLV_TOTAL_NUM, ADCL_alltoallv_attrs, &ADCL_alltoallv_attrset );

    count=0;
    /* Register function native, no transfer primitives yet set */
    m_alltoallv_attrs[0] = ADCL_attr_alltoallv_alg_native;
    /* m_alltoallv_attr[1] = ADCL_attr_alltoallv_transfer_Sendrecv */
    ADCL_function_create_async ( ADCL_alltoallv_native, NULL, ADCL_alltoallv_attrset,
				 m_alltoallv_attrs, "Alltoallv_native_SR", 
				 &ADCL_alltoallv_functions[count]);
    count++;


    /* Register function pairwise, no transfer primitives yet set */
    m_alltoallv_attrs[0] = ADCL_attr_alltoallv_alg_pairwise;
    /* m_alltoallv_attr[1] = ADCL_attr_alltoallv_transfer_Sendrecv */
    ADCL_function_create_async ( ADCL_alltoallv_pairwise, NULL, ADCL_alltoallv_attrset,
				 m_alltoallv_attrs, "Alltoallv_pair_SR", 
				 &ADCL_alltoallv_functions[count]);
    count++;


    /* Register function linear, no transfer primitives yet set */
    m_alltoallv_attrs[0] = ADCL_attr_alltoallv_alg_linear;
    /* m_alltoallv_attr[1] = ADCL_attr_alltoallv_transfer_Sendrecv */
    ADCL_function_create_async ( ADCL_alltoallv_linear, NULL, ADCL_alltoallv_attrset,
				 m_alltoallv_attrs, "Alltoallv_linear_SR", 
				 &ADCL_alltoallv_functions[count]);
    count++;


    ADCL_fnctset_create ( ADCL_METHOD_ALLTOALLV_TOTAL_NUM, 
			  ADCL_alltoallv_functions, 
			  "Alltoallv",
			  &ADCL_alltoallv_fnctset );

    if ( count != ADCL_METHOD_ALLTOALLV_TOTAL_NUM ) {
	ADCL_printf ("Alltoallv: Total Number wrong\n");
	return ADCL_ERROR_INTERNAL;
    }

    return ADCL_SUCCESS;
}
