/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"

//int ADCL_subarray_init ( int topodims, int vecndims, int *vecdims,
//             int hwidth, int nc, int order, MPI_Datatype btype,
//             MPI_Datatype **senddats, MPI_Datatype **recvdats)
//{
//    int i, j, k;
//    int ret = ADCL_SUCCESS;
//    int *subdims=NULL, *sstarts=NULL, *rstarts=NULL;
//    MPI_Datatype *sdats=NULL, *rdats=NULL;
//
//    subdims  = ( int*) malloc ( vecndims * sizeof(int) );
//    if ( NULL == subdims ) {
//    return ADCL_NO_MEMORY;
//    }
//
//    sstarts  = ( int*) malloc ( vecndims * sizeof(int) );
//    rstarts  = ( int*) malloc ( vecndims * sizeof(int) );
//    if ( NULL == sstarts || NULL == rstarts  ) {
//    ret = ADCL_NO_MEMORY;
//    goto exit;
//    }
//
//    sdats = ( MPI_Datatype *) malloc ( topodims * 2 * sizeof(MPI_Datatype));
//    rdats = ( MPI_Datatype *) malloc ( topodims * 2 * sizeof(MPI_Datatype));
//    if ( NULL == sdats || NULL == rdats  ) {
//    ret = ADCL_NO_MEMORY;
//    goto exit;
//    }
//
//    /* Loop over all topology dimensions */
//    for ( i = 0; i < topodims; i++ ) {
//
//    /* handle left and right neighbor separatly */
//    for ( j=2*i; j<= 2*i+1; j++ ) {
//
//        /* Set subdims and starts arrays. Basically,
//           subdims is in each direction the total extent of the
//           according dimension of the data array without the halo-cells
//           except for the dimension which we are currently dealing
//           with. For this dimension it is 1.
//
//           The starts arrays are 1 for all dimensions except
//           for the dimension (lets say k)  which we are dealing with.
//           There it is for sending:
//           - 1 for the left neighbor,
//           - ldims[k]-2*HWIDTH for the right neighbor
//           for receiving:
//           - 0 for the left neighbor
//           - ldims[k]-HWDITH for the right neighbor
//        */
//        if  ( nc > 0 ) {
//        for ( k=0; k < vecndims-1; k++ ) {
//            if ( k == i ) {
//            subdims[k] = hwidth;
//            sstarts[k] = (j == 2*i) ? hwidth : (vecdims[k]-2*hwidth);
//            rstarts[k] = (j == 2*i) ? 0 : (vecdims[k]-hwidth);
//            }
//            else {
//            subdims[k] = vecdims[k]- 2*hwidth;
//            sstarts[k] = hwidth;
//            rstarts[k] = hwidth;
//            }
//        }
//        subdims[vecndims-1] = nc;
//        sstarts[vecndims-1] = 0;
//        rstarts[vecndims-1] = 0;
//            }
//        else {
//        for ( k=0; k < vecndims; k++ ) {
//            if ( k == i ) {
//            subdims[k] = hwidth;
//            sstarts[k] = (j == 2*i) ? hwidth : (vecdims[k]-2*hwidth);
//            rstarts[k] = (j == 2*i) ? 0 : (vecdims[k]-hwidth);
//            }
//            else {
//            subdims[k] = vecdims[k]- 2*hwidth;
//            sstarts[k] = hwidth;
//            rstarts[k] = hwidth;
//            }
//        }
//        }
//        MPI_Type_create_subarray ( vecndims, vecdims, subdims, sstarts,
//                       order, btype, &(sdats[j]));
//        MPI_Type_create_subarray ( vecndims, vecdims, subdims, rstarts,
//                       order, btype, &(rdats[j]));
//        MPI_Type_commit ( &(sdats[j]));
//        MPI_Type_commit ( &(rdats[j]));
//    }
//    }
//
// exit:
//    if ( ret != ADCL_SUCCESS ) {
//    if ( NULL != subdims ) {
//        free ( subdims ) ;
//    }
//    if ( NULL != sstarts ) {
//        free ( sstarts );
//    }
//    if ( NULL != rstarts ) {
//        free ( rstarts );
//    }
//    if ( NULL != sdats ) {
//        free ( sdats );
//    }
//    if ( NULL != rdats ) {
//        free ( rdats );
//    }
//    }
//
//    *senddats = sdats;
//    *recvdats = rdats;
//    return ret;
//}

void ADCL_subarray_free ( int num, MPI_Datatype **senddats,
              MPI_Datatype **recvdats )
{
    int i;
    MPI_Datatype *sdats = *senddats;
    MPI_Datatype *rdats = *recvdats;

    if ( NULL != rdats ) {
    for ( i=0; i<num; i++ ){
        if ( MPI_DATATYPE_NULL != rdats[i]) {
        MPI_Type_free ( &(rdats[i]));
        }
    }
    free ( rdats );
    }

    if ( NULL != sdats ) {
    for ( i=0; i<num; i++ ){
        if ( MPI_DATATYPE_NULL != sdats[i]) {
        MPI_Type_free ( &(sdats[i]));
        }
    }
    free ( sdats );
    }

    *senddats = NULL;
    *recvdats = NULL;
    return;
}

int ADCL_subarray_init ( int ntopodim, int nvecdims, int *vecdims,
             int hwidth, int nc, int order, int nneigh, MPI_Datatype btype,
             int ndats, MPI_Datatype **senddats, MPI_Datatype **recvdats)
{
    int i, j, k;
    int ret = ADCL_SUCCESS;
    int *subdims=NULL, *sstarts=NULL, *rstarts=NULL;
    MPI_Datatype *sdats=NULL, *rdats=NULL;

    subdims  = ( int*) malloc ( nvecdims * sizeof(int) );
    if ( NULL == subdims ) {
        return ADCL_NO_MEMORY;
    }

    sstarts  = ( int*) malloc ( nvecdims * sizeof(int) );
    rstarts  = ( int*) malloc ( nvecdims * sizeof(int) );
    if ( NULL == sstarts || NULL == rstarts  ) {
        ret = ADCL_NO_MEMORY;
        goto exit;
    }

    sdats = ( MPI_Datatype *) malloc ( ndats * sizeof(MPI_Datatype));
    rdats = ( MPI_Datatype *) malloc ( ndats * sizeof(MPI_Datatype));
    if ( NULL == sdats || NULL == rdats  ) {
        ret = ADCL_NO_MEMORY;
        goto exit;
    }

    if  ( nc > 0 ) {
        subdims[nvecdims-1] = nc;
        sstarts[nvecdims-1] = 0;
        rstarts[nvecdims-1] = 0;
    }

    /* Loop over all topology dimensions */
    for ( i = 0; i < ntopodim; i++ ) {
        /* handle left and right neighbor separatly */
        for ( j=2*i; j<= 2*i+1; j++ ) {

            /* Set subdims and starts arrays. Basically, subdims is in each direction the total extent 
               of the according dimension of the data array without the halo-cells except for the 
               dimension which we are currently dealing with. For this dimension it is 1.

               The starts arrays are 1 for all dimensions except for the dimension (lets say k) 
               which we are dealing with. 
               There it is for sending:
               - 1 for the left neighbor,
               - ldims[k]-2*HWIDTH for the right neighbor
               for receiving:
               - 0 for the left neighbor
               - ldims[k]-HWDITH for the right neighbor
             */
            for ( k=0; k < ntopodim; k++ ) {
                if ( k == i ) {
                    subdims[k] = hwidth;
                    sstarts[k] = (j == 2*i) ? hwidth : (vecdims[k]-2*hwidth);
                    rstarts[k] = (j == 2*i) ? 0 : (vecdims[k]-hwidth);
                }
                else {
                    subdims[k] = vecdims[k]- 2*hwidth;
                    sstarts[k] = hwidth;
                    rstarts[k] = hwidth;
                }
            }
            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, sstarts,
                    order, btype, &(sdats[j]));
            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, rstarts,
                    order, btype, &(rdats[j]));
            MPI_Type_commit ( &(sdats[j]));
            MPI_Type_commit ( &(rdats[j]));
        }
    }

    if ( nneigh > ntopodim ) {
        if ( ntopodim == 2 && nneigh == 4 ) {
            subdims[0] = hwidth;   subdims[1] = hwidth;

            /* lower left and upper right corner */
            for ( j = 0; j<2; j++ ) {
                sstarts[0] = ( j == 0 ) ? hwidth : vecdims[0]-2*hwidth;   
                sstarts[1] = ( j == 0 ) ? hwidth : vecdims[1]-2*hwidth;
                rstarts[0] = ( j == 0 ) ? 0      : vecdims[0]-hwidth;        
                rstarts[1] = ( j == 0 ) ? 0      : vecdims[1]-hwidth;

                MPI_Type_create_subarray ( nvecdims, vecdims, subdims, sstarts, order, btype, &(sdats[4+j]));
                MPI_Type_create_subarray ( nvecdims, vecdims, subdims, rstarts, order, btype, &(rdats[4+j]));
                MPI_Type_commit ( &(sdats[4+j]));
                MPI_Type_commit ( &(rdats[4+j]));
            }

            /* lower right and upper left corner */
            for ( j = 0; j<2; j++ ) {
                sstarts[0] = ( j == 0 ) ? vecdims[0]-2*hwidth : hwidth;   
                sstarts[1] = ( j == 0 ) ? hwidth             : vecdims[1]-2*hwidth; 
                rstarts[0] = ( j == 0 ) ? vecdims[0]-hwidth   : 0;
                rstarts[1] = ( j == 0 ) ? 0                  : vecdims[1]-hwidth;

                MPI_Type_create_subarray ( nvecdims, vecdims, subdims, sstarts, order, btype, &(sdats[6+j]));
                MPI_Type_create_subarray ( nvecdims, vecdims, subdims, rstarts, order, btype, &(rdats[6+j]));
                MPI_Type_commit ( &(sdats[6+j]));
                MPI_Type_commit ( &(rdats[6+j]));
            }

            /* create additional datatypes for blocking send */
            for ( i = 0; i < ntopodim; i++ ) {
                /* handle left and right neighbor separatly */
                for ( j=2*i; j<= 2*i+1; j++ ) {

                    /* Set subdims and starts arrays. Basically, subdims is in each direction the total extent 
                       of the according dimension of the data array without the halo-cells except for the 
                       dimension which we are currently dealing with. For this dimension it is 1.

                       The starts arrays are 1 for all dimensions except for the dimension (lets say k) 
                       which we are dealing with. There it is for sending:
                       - 1 for the left neighbor,
                       - ldims[k]-2*HWIDTH for the right neighbor
                       for receiving:
                       - 0 for the left neighbor
                       - ldims[k]-HWDITH for the right neighbor
                     */
                    for ( k=0; k < ntopodim; k++ ) {
                        if ( k == i ) {
                            subdims[k] = hwidth;
                            sstarts[k] = (j == 2*i) ? hwidth : (vecdims[k]-2*hwidth);
                            rstarts[k] = (j == 2*i) ? 0 : (vecdims[k]-hwidth);
                        }
                        else {
                            subdims[k] = vecdims[k];
                            sstarts[k] = 0;
                            rstarts[k] = 0;
                        }
                    }
                    MPI_Type_create_subarray ( nvecdims, vecdims, subdims, sstarts,
                            order, btype, &(sdats[8+j]));
                    MPI_Type_create_subarray ( nvecdims, vecdims, subdims, rstarts,
                            order, btype, &(rdats[8+j]));
                    MPI_Type_commit ( &(sdats[8+j]));
                    MPI_Type_commit ( &(rdats[8+j]));
                }
            }
        }
        else if ( ntopodim == 3 &&  nneigh == 9 ) {
            /* *** VERTICAL EDGES *** */
            subdims[0] = hwidth;   subdims[1] = hwidth;   subdims[2] = vecdims[2]-2*hwidth;

            /* Set the send and recv derived datatype for the edge (0,0,z) */
            sstarts[0] = hwidth;   sstarts[1] = hwidth;   sstarts[2] = hwidth;
            rstarts[0] = 0;        rstarts[1] = 0;        rstarts[2] = hwidth;

            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, sstarts, order, btype, &(sdats[6]));
            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, rstarts, order, btype, &(rdats[6]));
            MPI_Type_commit ( &(sdats[6]));
            MPI_Type_commit ( &(rdats[6]));

            /* Set the send and recv derived datatype for the edge (1,1,z) */
            sstarts[0] = vecdims[0]-2*hwidth;   sstarts[1] = vecdims[1]-2*hwidth;   sstarts[2] = hwidth;
            rstarts[0] = vecdims[0]-hwidth;     rstarts[1] = vecdims[1]-hwidth;     rstarts[2] = hwidth;

            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, sstarts, order, btype, &(sdats[7]));
            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, rstarts, order, btype, &(rdats[7]));
            MPI_Type_commit ( &(sdats[7]));
            MPI_Type_commit ( &(rdats[7]));

            /* Set the send and recv derived datatypes for the edge (0,1,z) */
            sstarts[0] = hwidth;   sstarts[1] = vecdims[1]-2*hwidth;   sstarts[2] = hwidth;
            rstarts[0] = 0;        rstarts[1] = vecdims[1]-hwidth;     rstarts[2] = hwidth;
            
            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, sstarts, order, btype, &(sdats[8]));
            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, rstarts, order, btype, &(rdats[8]));
            MPI_Type_commit ( &(sdats[8]));
            MPI_Type_commit ( &(rdats[8]));

            /* Set the send and recv derived datatypes for the edge (1,0,z) */
            sstarts[0] = vecdims[0]-2*hwidth;   sstarts[1] = hwidth;   sstarts[2] = hwidth;
            rstarts[0] = vecdims[0]-hwidth;     rstarts[1] = 0;        rstarts[2] = hwidth;
            
            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, sstarts, order, btype, &(sdats[9]));
            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, rstarts, order, btype, &(rdats[9]));
            MPI_Type_commit ( &(sdats[9]));
            MPI_Type_commit ( &(rdats[9]));


             /* *** HORIZONTAL EDGES *** */
             /* (0,-1,-1) - (0,+1,+1) */
            subdims[0] = hwidth;   subdims[1] = vecdims[1]-2*hwidth;   subdims[2] = hwidth; 

             /* Set the send and recv derived datatypes for the edge (0,y,0) (3) */
            sstarts[0] = hwidth;   sstarts[1] = hwidth;               sstarts[2] = hwidth;
            rstarts[0] = 0;        rstarts[1] = hwidth;               rstarts[2] = 0; 

            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, sstarts, order, btype, &(sdats[10]));
            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, rstarts, order, btype, &(rdats[10]));
            MPI_Type_commit ( &(sdats[10]));
            MPI_Type_commit ( &(rdats[10]));

            /* Set the send and recv derived datatypes for the edge (1,y,1) (23) */
            sstarts[0] = vecdims[0]-2*hwidth;   sstarts[1] = hwidth;  sstarts[2] = vecdims[2]-2*hwidth;
            rstarts[0] = vecdims[0]-hwidth;     rstarts[1] = hwidth;  rstarts[2] = vecdims[2]-hwidth; 

            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, sstarts, order, btype, &(sdats[11]));
            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, rstarts, order, btype, &(rdats[11]));
            MPI_Type_commit ( &(sdats[11]));
            MPI_Type_commit ( &(rdats[11]));


            /* (1,0,-1) - (-1,0,+1) */
            subdims[0] = vecdims[0]-2*hwidth;   subdims[1] = hwidth;   subdims[2] = hwidth; 

            /* Set the send and recv derived datatypes for the edge (x,1,0) (15) */
            sstarts[0] = hwidth;   sstarts[1] = vecdims[1]-2*hwidth;   sstarts[2] = hwidth;
            rstarts[0] = hwidth;   rstarts[1] = vecdims[1]-hwidth;     rstarts[2] = 0; 

            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, sstarts, order, btype, &(sdats[12]));
            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, rstarts, order, btype, &(rdats[12]));
            MPI_Type_commit ( &(sdats[12]));
            MPI_Type_commit ( &(rdats[12]));


            /* Set the send and recv derived datatypes for the edge (x,0,1) (11) */
            sstarts[0] = hwidth;   sstarts[1] = hwidth;   sstarts[2] = vecdims[2]-2*hwidth;
            rstarts[0] = hwidth;   rstarts[1] = 0;        rstarts[2] = vecdims[2]-hwidth; 

            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, sstarts, order, btype, &(sdats[13]));
            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, rstarts, order, btype, &(rdats[13]));
            MPI_Type_commit ( &(sdats[13]));
            MPI_Type_commit ( &(rdats[13]));

            /* (0,-1,1) - (0,1,-1) */
            subdims[0] = hwidth;   subdims[1] = vecdims[1]-2*hwidth;   subdims[2] = hwidth; 

            /* Set the send and recv derived datatypes for the edge (1,y,0) (21) */
            sstarts[0] = vecdims[0]-2*hwidth;   sstarts[1] = hwidth;   sstarts[2] = hwidth;
            rstarts[0] = vecdims[0]-hwidth;     rstarts[1] = hwidth;   rstarts[2] = 0; 

            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, sstarts, order, btype, &(sdats[14]));
            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, rstarts, order, btype, &(rdats[14]));
            MPI_Type_commit ( &(sdats[14]));
            MPI_Type_commit ( &(rdats[14]));


            /* Set the send and recv derived datatypes for the edge (0,y,1) (5) */
            sstarts[0] = hwidth;   sstarts[1] = hwidth;   sstarts[2] = vecdims[2]-2*hwidth;
            rstarts[0] = 0;        rstarts[1] = hwidth;   rstarts[2] = vecdims[2]-hwidth; 

            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, sstarts, order, btype, &(sdats[15]));
            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, rstarts, order, btype, &(rdats[15]));
            MPI_Type_commit ( &(sdats[15]));
            MPI_Type_commit ( &(rdats[15]));


            /* (-1,0,-1) - (+1,0,+1) */
            subdims[0] = vecdims[0]-2*hwidth;   subdims[1] = hwidth;   subdims[2] = hwidth; 

            /* Set the send and recv derived datatypes for the edge (x,0,0) (9) */
            sstarts[0] = hwidth;   sstarts[1] = hwidth;   sstarts[2] = hwidth;
            rstarts[0] = hwidth;   rstarts[1] = 0;        rstarts[2] = 0; 

            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, sstarts, order, btype, &(sdats[16]));
            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, rstarts, order, btype, &(rdats[16]));
            MPI_Type_commit ( &(sdats[16]));
            MPI_Type_commit ( &(rdats[16]));

            /* Set the send and recv derived datatypes for the edge (x,1,1) (17) */
            sstarts[0] = hwidth;   sstarts[1] = vecdims[1]-2*hwidth;   sstarts[2] = vecdims[2]-2*hwidth;
            rstarts[0] = hwidth;   rstarts[1] = vecdims[1]-hwidth;     rstarts[2] = vecdims[2]-hwidth; 

            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, sstarts, order, btype, &(sdats[17]));
            MPI_Type_create_subarray ( nvecdims, vecdims, subdims, rstarts, order, btype, &(rdats[17]));
            MPI_Type_commit ( &(sdats[17]));
            MPI_Type_commit ( &(rdats[17]));

            /* create additional datatypes for blocking send */
            for ( i = 0; i < ntopodim; i++ ) {
                /* handle left and right neighbor separatly */
                for ( j=2*i; j<= 2*i+1; j++ ) {

                    /* Set subdims and starts arrays. Basically, subdims is in each direction the total extent 
                       of the according dimension of the data array without the halo-cells except for the 
                       dimension which we are currently dealing with. For this dimension it is 1.

                       The starts arrays are 1 for all dimensions except for the dimension (lets say k) 
                       which we are dealing with. There it is for sending:
                       - 1 for the left neighbor,
                       - ldims[k]-2*HWIDTH for the right neighbor
                       for receiving:
                       - 0 for the left neighbor
                       - ldims[k]-HWDITH for the right neighbor
                     */
                    for ( k=0; k < ntopodim; k++ ) {
                        if ( k == i ) {
                            subdims[k] = hwidth;
                            sstarts[k] = (j == 2*i) ? hwidth : (vecdims[k]-2*hwidth);
                            rstarts[k] = (j == 2*i) ? 0 : (vecdims[k]-hwidth);
                        }
                        else {
                            subdims[k] = vecdims[k];
                            sstarts[k] = 0;
                            rstarts[k] = 0;
                        }
                    }
                    MPI_Type_create_subarray ( nvecdims, vecdims, subdims, sstarts,
                            order, btype, &(sdats[18+j]));
                    MPI_Type_create_subarray ( nvecdims, vecdims, subdims, rstarts,
                            order, btype, &(rdats[18+j]));
                    MPI_Type_commit ( &(sdats[18+j]));
                    MPI_Type_commit ( &(rdats[18+j]));
                }
            }
        } 
        else {
            printf("not implemented\n"); exit(-1);
        }
    } 

exit:
    if ( ret != ADCL_SUCCESS ) {
        if ( NULL != subdims ) {
            free ( subdims ) ;
        }
        if ( NULL != sstarts ) {
            free ( sstarts );
        }
        if ( NULL != rstarts ) {
            free ( rstarts );
        }
        if ( NULL != sdats ) {
            free ( sdats );
        }
        if ( NULL != rdats ) {
            free ( rdats );
        }
    }

    *senddats = sdats;
    *recvdats = rdats;
    return ret;
}


