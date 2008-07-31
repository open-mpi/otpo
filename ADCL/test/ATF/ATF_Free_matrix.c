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


#include "ATF_Adcl_global.h"

int ATF_Free_matrix()
{
  ATF_free_5D_double_matrix(&ATF_rm000);
  ATF_free_5D_double_matrix(&ATF_rmb00);
  ATF_free_5D_double_matrix(&ATF_rmf00);
  ATF_free_5D_double_matrix(&ATF_rm0b0);
  ATF_free_5D_double_matrix(&ATF_rm0f0);
  ATF_free_5D_double_matrix(&ATF_rm00b);
  ATF_free_5D_double_matrix(&ATF_rm00f);

  ATF_free_4D_double_matrix(&ATF_rhs);
  ATF_free_4D_double_matrix(&ATF_dq);
  ATF_free_4D_double_matrix(&ATF_loes);

  ADCL_Request_free( &adcl_Req_dq );
  ADCL_Request_free( &adcl_Req_loes );
  ADCL_Request_free( &adcl_Req_rhs);
  
  ADCL_Vector_deregister( &adcl_Vec_dq);
  ADCL_Vector_deregister( &adcl_Vec_loes);
  ADCL_Vector_deregister( &adcl_Vec_rhs);

  ADCL_Topology_free(&ADCL_topo);
  

  return ATF_SUCCESS;  
}

