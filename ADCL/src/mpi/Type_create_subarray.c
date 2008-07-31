/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
/* Implementation of the MPI_Type_create_subarray in case 
   the library does not provide this. Based on the implementation of Open MPI.
   Modified by Edgar Gabriel in order to work on arbitrary MPI libraries.
   Copyright (C) 2006 University of Houston. All rights reserved.
*/


/*
 * The original Open MPI copyrights.
 *
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2005 The University of Tennessee and The University
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

#include "mpi.h"


int MPI_Type_create_subarray(int ndims,
                             int size_array[],
                             int subsize_array[],
                             int start_array[],
                             int order,
                             MPI_Datatype oldtype,
                             MPI_Datatype *newtype)

{
    MPI_Datatype last_type; 
    int i, step, start_loop, end_loop;
    MPI_Aint size, displ, extent;

    MPI_Type_extent( oldtype, &extent );

    /* If the ndims is zero then return the NULL datatype */
    if( ndims < 2 ) {
        if( 0 == ndims ) {
            *newtype = MPI_DATATYPE_NULL;
            return MPI_SUCCESS;
        }
        MPI_Type_contiguous( subsize_array[0], oldtype, &last_type );
        size = size_array[0];
        displ = start_array[0];
        goto replace_subarray_type;
    }

    if( MPI_ORDER_C == order ) {
        start_loop = i = ndims - 1;
        step = -1;
        end_loop = -1;
        if( end_loop > (start_loop + 2 * step) )
            end_loop = start_loop + 2 * step;
    } else {
        start_loop = i = 0;
        step = 1;
        end_loop = ndims;
        if( end_loop < (start_loop + 2 * step) )
            end_loop = start_loop + 2 * step;
    }

    /* As we know that the ndims is at least 1 we can start by creating the first dimension data
     * outside the loop, such that we dont have to create a duplicate of the oldtype just to be able
     * to free it.
     */
    MPI_Type_vector( subsize_array[i+step], subsize_array[i], size_array[i],
		     oldtype, newtype );

    last_type = *newtype;
    size = size_array[i] * size_array[i+step];
    displ = start_array[i] + start_array[i+step] * size_array[i];
    for( i += 2 * step; i != end_loop; i += step ) {
        MPI_Type_hvector( subsize_array[i], 1, size * extent,
			  last_type, newtype );
        MPI_Type_free ( &last_type );
        displ += size * start_array[i];
        size *= size_array[i];
        last_type = *newtype;
    }

  replace_subarray_type:    
    /**
     * We cannot use resized here. Resized will only set the soft lb and ub markers
     * without moving the real data inside. What we need is to force the displacement
     * of the data create upward to the right position AND set the LB and UB. A type
     * struct is the function we need.
     */
    {
        MPI_Aint displs[3];
        MPI_Datatype types[3];
        int blength[3] = { 1, 1, 1 };

        displs[0] = 0; displs[1] = displ * extent; displs[2] = size * extent;
        types[0] = MPI_LB; types[1] = last_type; types[2] = MPI_UB;
        MPI_Type_struct( 3, blength, displs, types, newtype );
    }
    MPI_Type_free( &last_type );
    

    return MPI_SUCCESS;
}
