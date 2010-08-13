/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2006 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

/*
** Codes have been adapted to be used in ADCL
** Following are the ADCL copyrights
*/

/*
 * Copyright (c) 2008      University of Houston. All rights reserved.
 * Copyright (c) 2008      High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"

int
ADCL_ddt_copy_content_same_ddt(MPI_Datatype dtype, int count, char* dest, char* src)
{
    int ret=ADCL_SUCCESS; 

    /* this is not a good idea, but should work for contiguous data types */
    memcpy( dest, src, count );

error_hndl:
    return ret;

}



/**
 * Perform a reduction operation.
 *
 * @param op The operation (IN)
 * @param source Source (input) buffer (IN)
 * @param target Target (output) buffer (IN/OUT)
 * @param count Number of elements (IN)
 * @param dtype MPI datatype (IN)
 *
 * @returns void As with MPI user-defined reduction functions, there
 * is no return code from this function.
 *
 * Perform a reduction operation with count elements of type dtype in
 * the buffers source and target.  The target buffer obtains the
 * result (i.e., the original values in the target buffer are reduced
 * with the values in the source buffer and the result is stored in
 * the target buffer).
 *
 * This function figures out which reduction operation function to
 * invoke and whether to invoke it with C- or Fortran-style invocation
 * methods.  If the op is intrinsic and has the operation defined for
 * dtype, the appropriate back-end function will be invoked.
 * Otherwise, the op is assumed to be a user op and the first function
 * pointer in the op array will be used.
 *
 * NOTE: This function assumes that a correct combination will be
 * given to it; it makes no provision for errors (in the name of
 * optimization).  If you give it an intrinsic op with a datatype that
 * is not defined to have that operation, it is likely to seg fault.
 */

int ADCL_op_reduce( MPI_Op op, char *source, char *target,
                                  int count, MPI_Datatype dtype)
{
   double *d_src, *d_tar;
   int    *i_src, *i_tar;
   int i; 

   if (MPI_DOUBLE == dtype) {
      d_src = (double*) source;
      d_tar = (double*) target;
      if (MPI_MIN == op){
         for (i=0; i<count; i++){
            d_tar[i] = (d_src[i] < d_tar[i]) ? d_src[i] : d_tar[i];
         }
      }
      else if (MPI_MAX == op) { 
         for (i=0; i<count; i++){
            d_tar[i] = (d_src[i] < d_tar[i]) ? d_tar[i] : d_src[i];
         }
      }
      else if (MPI_SUM == op) {
         for (i=0; i<count; i++){
            d_tar[i] = d_src[i] + d_tar[i]; 
         }
      }
      else {
         return ADCL_INVALID_OP; 
      }
   }
   else if (MPI_INT == dtype){
      i_src = (int*) source;
      i_tar = (int*) target;
      if (MPI_MIN == op){
         for (i=0; i<count; i++){
            i_tar[i] = (i_src[i] < i_tar[i]) ? i_src[i] : i_tar[i];
         }
      }
      else if (MPI_MAX == op) { 
         for (i=0; i<count; i++){
            i_tar[i] = (i_src[i] < i_tar[i]) ? i_tar[i] : i_src[i];
         }
      }
      else if (MPI_SUM == op) {
         for (i=0; i<count; i++){
            i_tar[i] = i_src[i] + i_tar[i]; 
         }
      }
      else {
         return ADCL_INVALID_OP; 
      }
   }
   else {
      return ADCL_INVALID_DAT; 
   } 


   return ADCL_SUCCESS;
}


int ADCL_basic_init ( MPI_Datatype btype, int cnt, MPI_Datatype **dats, int** cnts )
{
    int err = ADCL_SUCCESS;
    MPI_Datatype *tdats=NULL;
    int *tcnts = NULL; 

    tdats = ( MPI_Datatype *) malloc (sizeof(MPI_Datatype));
    if ( NULL == tdats  ) {
       return ADCL_NO_MEMORY;
    }
    tcnts = ( int *) malloc (sizeof(int));
    if ( NULL == tcnts  ) {
       return ADCL_NO_MEMORY;
    }

    tdats[0] = btype;
    tcnts[0] = cnt;

    *dats = tdats;
    *cnts = tcnts;

    return err;
}

void ADCL_basic_free (MPI_Datatype **dats, int **cnts )
{
    MPI_Datatype *tdats = *dats;
    int *tcnts = *cnts;

    if ( NULL != tdats ) {
       free ( tdats );
    }
    if ( NULL != tcnts ) {
       free ( tcnts );
    }

    *dats = NULL;
    *cnts = NULL;

    return;
}

