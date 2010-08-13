/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2009           HLRS. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"


static int ADCL_max ( int cnt, int *vecdim, int nc, int hwidth );
static int compare ( const void *p, const void* q );

static int dist_4D_C ( int dim0, int dim1, int dim2, int dim3, int vecdim[3], int nc);
static int dist_4D_Fortran ( int dim0, int dim1, int dim2, int dim3, int vecdim[3], int nc);
static int dist_3D_Fortran ( int dim0, int dim1, int dim2, int vecdim[2], int nc);

int ADCL_indexed_1D_init ( int vecdim, int hwidth, int nc, int order, MPI_Datatype btype,
               MPI_Datatype **senddats, MPI_Datatype **recvdats)
{
    int j;
    int ret = ADCL_SUCCESS;
    int blength;
    int sdispl, rdispl;
    MPI_Datatype *sdats=NULL, *rdats=NULL;

    sdats = ( MPI_Datatype *) malloc ( 2 * sizeof(MPI_Datatype));
    rdats = ( MPI_Datatype *) malloc ( 2 * sizeof(MPI_Datatype));
    if ( NULL == sdats || NULL == rdats  ) {
    ret = ADCL_NO_MEMORY;
    return ret;
    }

    if ( MPI_ORDER_C == order ) {
        for ( j = 0; j<2; j++ ) {
            if ( nc > 1 ) {
                blength = hwidth * nc;
                sdispl  = ( j== 0 ) ? hwidth * nc : (vecdim - 2*hwidth)*nc;
                rdispl  = ( j== 0 ) ? 0 : (vecdim - hwidth)*nc;
            }
            else {
                blength = hwidth;
                sdispl  = ( j== 0 ) ? hwidth : (vecdim - 2*hwidth);
                rdispl  = ( j== 0 ) ? 0 : (vecdim - hwidth);
            }
            MPI_Type_indexed ( 1, &blength, &sdispl, btype, &(sdats[j]));
            MPI_Type_indexed ( 1, &blength, &rdispl, btype, &(rdats[j]));

            MPI_Type_commit ( &(sdats[j]));
            MPI_Type_commit ( &(rdats[j]));
        }
    }
    else {
        /* MPI_ORDER_FORTRAN */
        if ( nc <= 1 ) {
           /* sdat[0]/rdat[0]: sdispl=[hwidth],           rdispl=[0]
              sdat[1]/rdat[1]: sdispl=[vecdim-2*hwidth,], rdispl=[vecdim-hwidth] */
           for ( j = 0; j<2; j++ ) {
               blength = hwidth;
               sdispl  = ( j== 0 ) ? hwidth : (vecdim - 2*hwidth);
               rdispl  = ( j== 0 ) ? 0 : (vecdim - hwidth);

               MPI_Type_indexed ( 1, &blength, &sdispl, btype, &(sdats[j]));
               MPI_Type_indexed ( 1, &blength, &rdispl, btype, &(rdats[j]));
               
               MPI_Type_commit ( &(sdats[j]));
               MPI_Type_commit ( &(rdats[j]));
           }
        }
        else {
           /* n>1: in C, data is arranged kind of funny
                   sdat[0] with sdspl=[hwidth,           hwidth+vecdim  , ...], 
                                rdspl=[0,                vecdim         , ...],
                   sdat[1] with sdspl=[vecdim-2*hwidth,2*vecdim-2*hwidth, ...], 
                                rdspl=[vecdim-  hwidth,2*vecdim-  hwidth, ...] */
           int *sdspl=NULL, *rdspl=NULL;
           int i, *bl=NULL;

           bl    = (int * ) malloc ( nc * sizeof(int));
           sdspl = (int * ) malloc ( 2 * nc * sizeof(int));
           if (NULL == bl || NULL == sdspl ){
               ret = ADCL_NO_MEMORY;
           return ret;
           }
           rdspl = &(sdspl[nc]);

           for ( j = 0; j<2; j++ ) {
               for ( i = 0; i< nc; i++ ) {
                   bl[i] = hwidth;

                   sdspl[i]=(j==0) ? (hwidth + i * vecdim) : (((i+1)*vecdim) - 2*hwidth);
                   rdspl[i]=(j==0) ? (i*vecdim) : (((i+1)*vecdim) - hwidth);

               }
               MPI_Type_indexed ( nc, bl, sdspl, btype, &(sdats[j]));
               MPI_Type_indexed ( nc, bl, rdspl, btype, &(rdats[j]));
               MPI_Type_commit ( &(sdats[j]));
               MPI_Type_commit ( &(rdats[j]));
           }

           free ( bl );
           free ( sdspl );
        }

    }


    *senddats = sdats;
    *recvdats = rdats;
    return ret;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_indexed_2D_init ( int *vecdim, int hwidth, int nc, int order, int nneigh, MPI_Datatype btype,
               MPI_Datatype **senddats, MPI_Datatype **recvdats)
{
    int i, j, k, maxdim, count;
    int ret = ADCL_SUCCESS;
    int *blength=NULL, baselen;
    int *sdispls=NULL, *rdispls=NULL, basedisp, basewidth;
    MPI_Datatype *sdats=NULL, *rdats=NULL;

    maxdim = ADCL_max ( 2, vecdim, nc, hwidth );

    sdats   = ( MPI_Datatype *) malloc ( 2 * nneigh * sizeof(MPI_Datatype));
    rdats   = ( MPI_Datatype *) malloc ( 2 * nneigh * sizeof(MPI_Datatype));
    blength = ( int *)          malloc ( maxdim * sizeof (int));
    sdispls = ( int *)          malloc ( 2 * maxdim * sizeof(int));
    if ( NULL == sdats || NULL == rdats || NULL == blength || NULL == sdispls ) {
        ret = ADCL_NO_MEMORY;
        goto exit;
    }
    rdispls = &(sdispls[maxdim]);

    if ( MPI_ORDER_C == order ) {
        /* Dimension 0 */
        baselen   = (nc < 1) ? vecdim[1]-2*hwidth : (vecdim[1]-2*hwidth) * nc;
        basedisp  = (nc < 1) ? vecdim[1] : vecdim[1] * nc;
        basewidth = (nc < 1) ? hwidth : hwidth * nc;

        for ( j = 0; j<2; j++ ) {
            for ( k=0; k<hwidth; k++ ) {
                 blength[k] = baselen;
                 sdispls[k] = (j == 0 ) ? (k+hwidth)*basedisp + basewidth :
                     (vecdim[0]-2*hwidth+k)*basedisp + basewidth;
                 rdispls[k] = (j == 0 ) ? k*basedisp+basewidth :
                     (vecdim[0]-hwidth+k)*basedisp  + basewidth;
            }
            MPI_Type_indexed ( hwidth, blength, sdispls, btype, &(sdats[j]));
            MPI_Type_indexed ( hwidth, blength, rdispls, btype, &(rdats[j]));
            MPI_Type_commit ( &(sdats[j]) );
            MPI_Type_commit ( &(rdats[j]) );
        }

        /* Dimension 1 */
        baselen  = (nc < 1) ? hwidth : hwidth * nc;
        basedisp = (nc < 1) ? vecdim[1] : vecdim[1] * nc;

        for ( j = 0; j<2; j++ ) {
            for ( k=0; k< vecdim[0]-2*hwidth; k++ ) {
                blength[k] = baselen;
                sdispls[k] = (j == 0 ) ? (hwidth+k)*basedisp+basewidth:
                    (hwidth+k+1)*basedisp - 2*basewidth;
                rdispls[k] = (j == 0 ) ? (hwidth+k)*basedisp :
                    (hwidth+k+1)*basedisp - basewidth;
            }
            MPI_Type_indexed ( k, blength, sdispls, btype, &(sdats[2+j]));
            MPI_Type_indexed ( k, blength, rdispls, btype, &(rdats[2+j]));
            MPI_Type_commit ( &(sdats[2+j]) );
            MPI_Type_commit ( &(rdats[2+j]) );
        }

        if ( nneigh > 2 ) {
           /* upper right and lower left corner */
           for ( j = 0; j<2; j++ ) {
               for ( k=0; k<hwidth; k++ ) {
                   blength[k] = baselen;
                   sdispls[k] = (j == 0 ) ? (hwidth+k)*basedisp+basewidth:
                       (vecdim[0]-2*hwidth+1+k)*basedisp - 2*basewidth;
                   rdispls[k] = (j == 0 ) ? k*basedisp : 
                      (vecdim[0]-hwidth+1+k)*basedisp - basewidth;
               }
               MPI_Type_indexed ( k, blength, sdispls, btype, &(sdats[4+j]));
               MPI_Type_indexed ( k, blength, rdispls, btype, &(rdats[4+j]));
               MPI_Type_commit ( &(sdats[4+j]) );
               MPI_Type_commit ( &(rdats[4+j]) );
           }

           /* lower right and upper left corner */
           for ( j = 0; j<2; j++ ) {
               for ( k=0; k<hwidth; k++ ) {
                   blength[k] = baselen;
                   sdispls[k] = (j == 0 ) ? (vecdim[0]-2*hwidth+k)*basedisp + basewidth :
                       (hwidth+1+k)*basedisp - 2*basewidth;
                   rdispls[k] = (j == 0 ) ? (vecdim[0]-hwidth+k)*basedisp :
                       (k+1)*basedisp - basewidth;
               }
               MPI_Type_indexed ( k, blength, sdispls, btype, &(sdats[6+j]));
               MPI_Type_indexed ( k, blength, rdispls, btype, &(rdats[6+j]));
               MPI_Type_commit ( &(sdats[6+j]) );
               MPI_Type_commit ( &(rdats[6+j]) );
           }
        }
    }
    else {
        /* MPI_ORDER_FORTRAN */
        if ( nc < 2 ) nc = 1;

        /* Set the send and recv derived datatype for the lower end of x-direction */
        for ( count=0, i=0; i<nc; i++ ) {
            for ( k=hwidth; k<(vecdim[1]-hwidth); k++, count++ ) {
                blength[count] = hwidth;
                sdispls[count] = dist_3D_Fortran( hwidth, k, i, vecdim, nc);
                rdispls[count] = dist_3D_Fortran( 0, k, i, vecdim, nc);
            }
        }
        MPI_Type_indexed ( count, blength, sdispls, btype, &sdats[0]);
        MPI_Type_indexed ( count, blength, rdispls, btype, &rdats[0]);
        MPI_Type_commit ( &sdats[0] );
        MPI_Type_commit ( &rdats[0] );

        /* Set the send and recv derived datatype for the upper end of x-direction */
        for ( count=0, i=0; i<nc; i++ ) {
            for ( k=hwidth; k<(vecdim[1]-hwidth); k++, count++ ) {
                blength[count] = hwidth;
                sdispls[count] = dist_3D_Fortran( vecdim[0]-2*hwidth, k, i, vecdim, nc);
                rdispls[count] = dist_3D_Fortran( vecdim[0]-hwidth, k, i, vecdim, nc);
            }
        }
        MPI_Type_indexed ( count, blength, sdispls, btype, &sdats[1]);
        MPI_Type_indexed ( count, blength, rdispls, btype, &rdats[1]);
        MPI_Type_commit ( &sdats[1] );
        MPI_Type_commit ( &rdats[1] );

        /* Set the send and recv derived datatype for the lower end of y-direction */
        for ( count=0, i=0; i<nc; i++ ) {
            for ( k=0; k<hwidth; k++, count++ ) {
            blength[count]  = vecdim[0]-2*hwidth;
            sdispls[count]   = dist_3D_Fortran( hwidth, hwidth+k, i, vecdim, nc);
            rdispls[count]   = dist_3D_Fortran( hwidth, k, i, vecdim, nc);
            }
        }
        MPI_Type_indexed ( count, blength, sdispls, btype, &sdats[2]);
        MPI_Type_indexed ( count, blength, rdispls, btype, &rdats[2]);
        MPI_Type_commit ( &sdats[2] );
        MPI_Type_commit ( &rdats[2] );

        /* Set the send and recv derived datatype for the upper end of y-direction */
        for ( count=0, i=0; i<nc; i++ ) {
            for ( k=0; k<hwidth; k++, count++ ) {
            blength[count]  = vecdim[0]-2*hwidth;
            sdispls[count]   = dist_3D_Fortran( hwidth, vecdim[1]-2*hwidth+k, i, vecdim, nc);
            rdispls[count]   = dist_3D_Fortran( hwidth, vecdim[1]-hwidth+k, i, vecdim, nc);
            }
        }
        MPI_Type_indexed ( count, blength, sdispls, btype, &sdats[3]);
        MPI_Type_indexed ( count, blength, rdispls, btype, &rdats[3]);
        MPI_Type_commit ( &sdats[3] );
        MPI_Type_commit ( &rdats[3] );

        if ( nneigh > 2 ) {
            /* Set the send and recv derived datatype for the lower left corner */
            for ( count=0, i=0; i<nc; i++ ) {
                for ( k=0; k<hwidth; k++, count++ ) {
                blength[count] = hwidth;
                sdispls[count] = dist_3D_Fortran( hwidth, hwidth+k, i, vecdim, nc);
                rdispls[count] = dist_3D_Fortran( 0, k, i, vecdim, nc);
                }
            }
            MPI_Type_indexed ( count, blength, sdispls, btype, &sdats[4]);
            MPI_Type_indexed ( count, blength, rdispls, btype, &rdats[4]);
            MPI_Type_commit ( &sdats[4] );
            MPI_Type_commit ( &rdats[4] );

            /* Set the send and recv derived datatype for the upper right corner */
            for ( count=0, i=0; i<nc; i++ ) {
                for ( k=0; k<hwidth; k++, count++ ) {
                blength[count] = hwidth;
                sdispls[count] = dist_3D_Fortran( vecdim[0]-2*hwidth, vecdim[1]-2*hwidth+k, i, vecdim, nc);
                rdispls[count] = dist_3D_Fortran( vecdim[0]-hwidth, vecdim[1]-hwidth+k, i, vecdim, nc);
                }
            }
            MPI_Type_indexed ( count, blength, sdispls, btype, &sdats[5]);
            MPI_Type_indexed ( count, blength, rdispls, btype, &rdats[5]);
            MPI_Type_commit ( &sdats[5] );
            MPI_Type_commit ( &rdats[5] );

            /* Set the send and recv derived datatype for the lower right corner */
            for ( count=0, i=0; i<nc; i++ ) {
                for ( k=0; k<hwidth; k++, count++ ) {
                blength[count] = hwidth;
                sdispls[count] = dist_3D_Fortran( vecdim[0]-2*hwidth, hwidth+k, i, vecdim, nc);
                rdispls[count] = dist_3D_Fortran( vecdim[0]-hwidth, k, i, vecdim, nc);
                }
            }
            MPI_Type_indexed ( count, blength, sdispls, btype, &sdats[6]);
            MPI_Type_indexed ( count, blength, rdispls, btype, &rdats[6]);
            MPI_Type_commit ( &sdats[6] );
            MPI_Type_commit ( &rdats[6] );

            /* Set the send and recv derived datatype for the upper left corner */
            for ( count=0, i=0; i<nc; i++ ) {
                for ( k=0; k<hwidth; k++, count++ ) {
                blength[count] = hwidth;
                sdispls[count] = dist_3D_Fortran( hwidth, vecdim[1]-2*hwidth+k, i, vecdim, nc);
                rdispls[count] = dist_3D_Fortran( 0, vecdim[1]-hwidth+k, i, vecdim, nc);
                }
            }
            MPI_Type_indexed ( count, blength, sdispls, btype, &sdats[7]);
            MPI_Type_indexed ( count, blength, rdispls, btype, &rdats[7]);
            MPI_Type_commit ( &sdats[7] );
            MPI_Type_commit ( &rdats[7] );
        }
    }



 exit:
    if ( ADCL_SUCCESS != ret ) {
      if ( NULL != sdats ) {
    free ( sdats );
      }
      if ( NULL != rdats ) {
    free ( rdats );
      }

    }

    if ( NULL != blength ) {
      free ( blength );
    }
    if ( NULL != sdispls ) {
      free ( sdispls );
    }

    *senddats = sdats;
    *recvdats = rdats;
    return ret;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_indexed_3D_init ( int *vecdim, int hwidth, int nc, int order, int nneigh, MPI_Datatype btype,
               MPI_Datatype **senddats, MPI_Datatype **recvdats)
{
    int i, j, k, l, maxdim;
    int ret = ADCL_SUCCESS;
    int *countarr, count;
    int *sdisps=NULL, *rdisps=NULL;
    MPI_Datatype *sdats=NULL, *rdats=NULL;

    maxdim = ADCL_max ( 3, vecdim,  nc,  hwidth);

    sdats = ( MPI_Datatype *) malloc ( 2 * nneigh * sizeof(MPI_Datatype));
    rdats = ( MPI_Datatype *) malloc ( 2 * nneigh * sizeof(MPI_Datatype));
    countarr = ( int *)  malloc ( maxdim * sizeof (int));
    sdisps  = ( int *)  malloc ( 2 * maxdim * sizeof(int));
    if ( NULL == sdats || NULL == rdats  || NULL == countarr || NULL == sdisps ) {
    ret = ADCL_NO_MEMORY;
    return ret;
    }
    rdisps = &(sdisps[maxdim]);


    if ( MPI_ORDER_C == order ) {
       if ( nc < 1 ) nc = 1;

       /* Set the send and recv derived datatype for the lower end of x-direction (4) */
       for ( count=0, i=0; i<hwidth; i++ ) {
           for ( j=hwidth; j<(vecdim[1]-hwidth); j++, count++ ) {
           countarr[count]  = (vecdim[2]-2*hwidth) * nc;
           sdisps[count]   = dist_4D_C( hwidth+i, j, hwidth, 0, vecdim, nc);
           rdisps[count]   = dist_4D_C( i, j, hwidth, 0, vecdim, nc);
           }
       }
       MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[0]);
           MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[0]);
       MPI_Type_commit ( &sdats[0] );
       MPI_Type_commit ( &rdats[0] );

       /* Set the send and recv derived datatype for the upper end of x-direction (22) */
       for ( count=0, i=0; i<hwidth; i++ ) {
           for ( j=hwidth; j<(vecdim[1]-hwidth); j++, count++ ) {
           countarr[count]  = (vecdim[2]-2*hwidth) * nc;
           sdisps[count]   = dist_4D_C( vecdim[0]-2*hwidth+i, j, hwidth, 0, vecdim, nc);
           rdisps[count]   = dist_4D_C( vecdim[0]-hwidth+i, j, hwidth, 0, vecdim, nc);
           }
       }
       MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[1]);
           MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[1]);
       MPI_Type_commit ( &sdats[1] );
       MPI_Type_commit ( &rdats[1] );

       /* Set the send and recv derived datatypes for the lower end of the y-direction (10) */
       for ( count=0, i=hwidth; i < (vecdim[0]-hwidth); i++) {
           for ( j=0; j<hwidth; j++, count++ ) {
           countarr[count] = (vecdim[2]-2*hwidth) * nc;
           sdisps[count] = dist_4D_C( i, hwidth+j, hwidth, 0, vecdim, nc );
           rdisps[count] = dist_4D_C( i, j, hwidth, 0, vecdim, nc );
           }
       }
       MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[2]);
       MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[2]);
       MPI_Type_commit ( &rdats[2]);
       MPI_Type_commit ( &sdats[2]);

       /* Set the send and recv derived datatypes for the upper end of the y-direction (16) */
       for ( count=0, i=hwidth; i < (vecdim[0]-hwidth); i++ ) {
           for ( j=0; j<hwidth; j++, count++ ) {
           countarr[count] = (vecdim[2]-2*hwidth) * nc;
           sdisps[count] = dist_4D_C( i, vecdim[1]-2*hwidth+j, hwidth, 0, vecdim, nc );
           rdisps[count] = dist_4D_C( i, vecdim[1]-hwidth+j, hwidth, 0, vecdim, nc );
           }
       }
       MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[3]);
       MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[3]);
       MPI_Type_commit ( &rdats[3]);
       MPI_Type_commit ( &sdats[3]);

       /* Set the send and recv derived datatypes for the lower end of the z-direction */
       for ( count=0, i=hwidth; i< (vecdim[0]-hwidth); i++ ) {
           for ( j=hwidth; j < (vecdim[1]-hwidth); j++, count++ ) {
           countarr[count] = hwidth*nc;
           sdisps[count]  = dist_4D_C( i, j, hwidth, 0, vecdim, nc );
           rdisps[count]  = dist_4D_C( i, j, 0, 0, vecdim, nc );
           }
       }
       MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[4]);
       MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[4]);
       MPI_Type_commit ( &sdats[4]);
       MPI_Type_commit ( &rdats[4]);

       /* Set the send and recv derived datatypes for the upper end of the z-direction */
       for ( count=0, i=hwidth; i< (vecdim[0]-hwidth); i++ ) {
           for ( j=hwidth; j < (vecdim[1]-hwidth); j++, count++ ) {
           countarr[count] = hwidth*nc;
           sdisps[count]  = dist_4D_C( i, j, vecdim[2]-2*hwidth, 0, vecdim, nc );
           rdisps[count]  = dist_4D_C( i, j, vecdim[2]-hwidth, 0, vecdim, nc );
           }
       }
       MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[5]);
       MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[5]);
       MPI_Type_commit ( &sdats[5]);
       MPI_Type_commit ( &rdats[5]);

       if ( nneigh > 3 ) {

          /* *** VERTICAL EDGES *** */

          for ( count=0, i=0; i<hwidth; i++ ) {
              for ( j=0; j<hwidth; j++, count++ ) {
                  countarr[count]  = (vecdim[2]-2*hwidth) * nc;
              }
          }

          /* Set the send and recv derived datatype for the edge (0,0,z) */
          for ( count=0, i=0; i<hwidth; i++ ) {
              for ( j=0; j<hwidth; j++, count++ ) {
                  sdisps[count]   = dist_4D_C( hwidth+i, hwidth+j, hwidth, 0, vecdim, nc);
                  rdisps[count]   = dist_4D_C( i, j, hwidth, 0, vecdim, nc);
              }
          }
          MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[6]);
          MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[6]);
          MPI_Type_commit ( &sdats[6] );
          MPI_Type_commit ( &rdats[6] );

          /* Set the send and recv derived datatype for the edge (1,1,z) */
          for ( count=0, i=0; i<hwidth; i++ ) {
              for ( j=0; j<hwidth; j++, count++ ) {
                  sdisps[count]   = dist_4D_C( vecdim[0]-2*hwidth+i, vecdim[1]-2*hwidth+j, hwidth, 0, vecdim, nc);
                  rdisps[count]   = dist_4D_C( vecdim[0]-hwidth+i, vecdim[1]-hwidth+j, hwidth, 0, vecdim, nc);
              }
          }
          MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[7]);
          MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[7]);
          MPI_Type_commit ( &sdats[7] );
          MPI_Type_commit ( &rdats[7] );

          /* Set the send and recv derived datatypes for the edge (0,1,z) */
          for ( count=0, i=0; i<hwidth; i++ ) {
              for ( j=0; j<hwidth; j++, count++ ) {
                  sdisps[count] = dist_4D_C( hwidth+i, vecdim[1]-2*hwidth+j, hwidth, 0, vecdim, nc );
                  rdisps[count] = dist_4D_C( i, vecdim[1]-hwidth+j, hwidth, 0, vecdim, nc );
              }
          }
          MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[8]);
          MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[8]);
          MPI_Type_commit ( &rdats[8]);
          MPI_Type_commit ( &sdats[8]);

          /* Set the send and recv derived datatypes for the edge (1,0,z) */
          for ( count=0, i=0; i<hwidth; i++ ) {
              for ( j=0; j<hwidth; j++, count++ ) {
                  sdisps[count] = dist_4D_C( vecdim[0]-2*hwidth+i, hwidth+j, hwidth, 0, vecdim, nc );
                  rdisps[count] = dist_4D_C( vecdim[0]-hwidth+i, j, hwidth, 0, vecdim, nc );
              }
          }
          MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[9]);
          MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[9]);
          MPI_Type_commit ( &rdats[9]);
          MPI_Type_commit ( &sdats[9]);

          /* *** HORIZONTAL EDGES *** */

          /* (0,-1,-1) - (0,+1,+1) */
          /* Set the send and recv derived datatypes for the edge (0,y,0) (3) */
          for ( count=0, i=0; i<hwidth; i++ ) {
              for ( j=hwidth; j<(vecdim[1]-hwidth); j++, count++ ) {
                  countarr[count] = hwidth * nc;
                  sdisps[count]   = dist_4D_C( hwidth+i, j, hwidth, 0, vecdim, nc);
                  rdisps[count]   = dist_4D_C( i, j, 0, 0, vecdim, nc);
              }
          }
          MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[10]);
          MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[10]);
          MPI_Type_commit ( &sdats[10] );
          MPI_Type_commit ( &rdats[10] );

          /* Set the send and recv derived datatypes for the edge (1,y,1) (23) */
          for ( count=0, i=0; i<hwidth; i++ ) {
              for ( j=hwidth; j<(vecdim[1]-hwidth); j++, count++ ) {
                  countarr[count]  = hwidth * nc;
                  sdisps[count]   = dist_4D_C( vecdim[0]-2*hwidth+i, j, vecdim[2]-2*hwidth, 0, vecdim, nc);
                  rdisps[count]   = dist_4D_C( vecdim[0]-hwidth+i, j, vecdim[2]-hwidth, 0, vecdim, nc);
              }
          }
          MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[11]);
          MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[11]);
          MPI_Type_commit ( &sdats[11] );
          MPI_Type_commit ( &rdats[11] );

          /* (1,0,-1) - (-1,0,+1) */
          /* Set the send and recv derived datatypes for the edge (x,1,0) (15) */
          for ( count=0, i=hwidth; i < (vecdim[0]-hwidth); i++) {
              for ( j=0; j<hwidth; j++, count++ ) {
                  countarr[count] = hwidth * nc;
                  sdisps[count] = dist_4D_C( i, vecdim[1]-2*hwidth+j, hwidth, 0, vecdim, nc );
                  rdisps[count] = dist_4D_C( i, vecdim[1]-hwidth+j, 0, 0, vecdim, nc );
              }
          }
          MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[12]);
          MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[12]);
          MPI_Type_commit ( &sdats[12]);
          MPI_Type_commit ( &rdats[12]);

          /* Set the send and recv derived datatypes for the edge (x,0,1) (11) */
          for ( count=0, i=hwidth; i< (vecdim[0]-hwidth); i++ ) {
              for ( j=0; j < hwidth; j++, count++ ) {
                  countarr[count] = hwidth*nc;
                  sdisps[count]   = dist_4D_C( i, hwidth+j, vecdim[2]-2*hwidth, 0, vecdim, nc );
                  rdisps[count]   = dist_4D_C( i, j, vecdim[2]-hwidth, 0, vecdim, nc );
              }
          }
          MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[13]);
          MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[13]);
          MPI_Type_commit ( &sdats[13]);
          MPI_Type_commit ( &rdats[13]);


          /* (0,-1,1) - (0,1,-1) */
          /* Set the send and recv derived datatypes for the edge (1,y,0) (21) */
          for ( count=0, i=0; i<hwidth; i++ ) {
              for ( j=hwidth; j<(vecdim[1]-hwidth); j++, count++ ) {
                  countarr[count] = hwidth * nc;
                  sdisps[count]   = dist_4D_C( vecdim[0]-2*hwidth+i, j, hwidth, 0, vecdim, nc);
                  rdisps[count]   = dist_4D_C( vecdim[0]-hwidth+i, j, 0, 0, vecdim, nc);
              }
          }
          MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[14]);
          MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[14]);
          MPI_Type_commit ( &sdats[14] );
          MPI_Type_commit ( &rdats[14] );

          /* Set the send and recv derived datatypes for the edge (0,y,1) (5) */
          for ( count=0, i=0; i<hwidth; i++ ) {
              for ( j=hwidth; j<(vecdim[1]-hwidth); j++, count++ ) {
                  countarr[count]  = hwidth * nc;
                  sdisps[count]   = dist_4D_C( hwidth+i, j, vecdim[2]-2*hwidth, 0, vecdim, nc);
                  rdisps[count]   = dist_4D_C( i, j, vecdim[2]-hwidth, 0, vecdim, nc);
              }
          }
          MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[15]);
          MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[15]);
          MPI_Type_commit ( &sdats[15] );
          MPI_Type_commit ( &rdats[15] );

          /* (-1,0,-1) - (+1,0,+1) */
          /* Set the send and recv derived datatypes for the edge (x,0,0) (9) */
          for ( count=0, i=hwidth; i < (vecdim[0]-hwidth); i++) {
              for ( j=0; j<hwidth; j++, count++ ) {
                  countarr[count] = hwidth * nc;
                  sdisps[count] = dist_4D_C( i, hwidth+j, hwidth, 0, vecdim, nc );
                  rdisps[count] = dist_4D_C( i, j, 0, 0, vecdim, nc );
              }
          }
          MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[16]);
          MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[16]);
          MPI_Type_commit ( &sdats[16]);
          MPI_Type_commit ( &rdats[16]);

          /* Set the send and recv derived datatypes for the edge (x,1,1) (17) */ 
          for ( count=0, i=hwidth; i< (vecdim[0]-hwidth); i++ ) {
              for ( j=0; j <hwidth; j++, count++ ) {
                  countarr[count] = hwidth*nc;
                  sdisps[count]   = dist_4D_C( i, vecdim[1]-2*hwidth+j, vecdim[2]-2*hwidth, 0, vecdim, nc );
                  rdisps[count]   = dist_4D_C( i, vecdim[1]-hwidth+j, vecdim[2]-hwidth, 0, vecdim, nc );
              }
          }
          MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[17]);
          MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[17]);
          MPI_Type_commit ( &sdats[17]);
          MPI_Type_commit ( &rdats[17]);


          ///* *** CORNERS *** */

          ///* Set the send and recv derived datatype for (0,0,0) to (1,1,1) */
          //for ( count=0, i=0; i<hwidth; i++ ) {
          //    for ( j=0; j<hwidth; j++, count++ ) {
          //       sdisps[count]   = dist_4D_C( hwidth+i, hwidth+j, hwidth, 0, vecdim, nc);
          //       rdisps[count]   = dist_4D_C( i, j, 0, 0, vecdim, nc);
          //    }
          //}
          //MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[10]);
          //MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[10]);
          //MPI_Type_commit ( &sdats[10] );
          //MPI_Type_commit ( &rdats[10] );

          ///* Set the send and recv derived datatype for (1,1,1) to (0,0,0)*/
          //for ( count=0, i=0; i<hwidth; i++ ) {
          //    for ( j=0; j<hwidth; j++, count++ ) {
          //        sdisps[count]   = dist_4D_C( vecdim[0]-2*hwidth+i, vecdim[1]-2*hwidth+j, vecdim[2]-2*hwidth, 0, vecdim, nc);
          //        rdisps[count]   = dist_4D_C( vecdim[0]-hwidth+i, vecdim[1]-hwidth+j, vecdim[2]-hwidth, 0, vecdim, nc);
          //    }
          //}
          //MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[11]);
          //MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[11]);
          //MPI_Type_commit ( &sdats[11] );
          //MPI_Type_commit ( &rdats[11] );

          ///* Set the send and recv derived datatype for (1,0,0) to (0,1,1) */
          //for ( count=0, i=0; i<hwidth; i++ ) {
          //    for ( j=0; j<hwidth; j++, count++ ) {
          //       sdisps[count]   = dist_4D_C( vecdim[0]-2*hwidth+i, hwidth+j, hwidth, 0, vecdim, nc);
          //       rdisps[count]   = dist_4D_C( vecdim[0]-hwidth+i, j, 0, 0, vecdim, nc);
          //    }
          //}
          //MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[12]);
          //MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[12]);
          //MPI_Type_commit ( &sdats[12] );
          //MPI_Type_commit ( &rdats[12] );

          ///* Set the send and recv derived datatype for (0,1,1) to (1,0,0)*/
          //for ( count=0, i=0; i<hwidth; i++ ) {
          //    for ( j=0; j<hwidth; j++, count++ ) {
          //        sdisps[count]   = dist_4D_C( hwidth+i, vecdim[1]-2*hwidth+j, vecdim[2]-2*hwidth, 0, vecdim, nc);
          //        rdisps[count]   = dist_4D_C( i, vecdim[1]-hwidth+j, vecdim[2]-hwidth, 0, vecdim, nc);
          //    }
          //}
          //MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[13]);
          //MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[13]);
          //MPI_Type_commit ( &sdats[13] );
          //MPI_Type_commit ( &rdats[13] );

          ///* Set the send and recv derived datatype for (1,1,0) to (0,0,1) */
          //for ( count=0, i=0; i<hwidth; i++ ) {
          //    for ( j=0; j<hwidth; j++, count++ ) {
          //       sdisps[count]   = dist_4D_C( vecdim[0]-2*hwidth+i, vecdim[1]-2*hwidth+j, hwidth, 0, vecdim, nc);
          //       rdisps[count]   = dist_4D_C( vecdim[0]-hwidth+i, vecdim[1]-hwidth+j, 0, 0, vecdim, nc);
          //    }
          //}
          //MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[14]);
          //MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[14]);
          //MPI_Type_commit ( &sdats[14] );
          //MPI_Type_commit ( &rdats[14] );

          ///* Set the send and recv derived datatype for (0,0,1) to (1,1,0)*/
          //for ( count=0, i=0; i<hwidth; i++ ) {
          //    for ( j=0; j<hwidth; j++, count++ ) {
          //        sdisps[count]   = dist_4D_C( hwidth+i, hwidth+j, vecdim[2]-2*hwidth, 0, vecdim, nc);
          //        rdisps[count]   = dist_4D_C( i, j, vecdim[2]-hwidth, 0, vecdim, nc);
          //    }
          //}
          //MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[15]);
          //MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[15]);
          //MPI_Type_commit ( &sdats[15] );
          //MPI_Type_commit ( &rdats[15] );

          ///* Set the send and recv derived datatype for (0,1,0) to (1,0,1) */
          //for ( count=0, i=0; i<hwidth; i++ ) {
          //    for ( j=0; j<hwidth; j++, count++ ) {
          //        sdisps[count]   = dist_4D_C( hwidth+i, vecdim[1]-2*hwidth+j, hwidth, 0, vecdim, nc);
          //        rdisps[count]   = dist_4D_C( i, vecdim[1]-hwidth+j, 0, 0, vecdim, nc);
          //    }
          //}
          //MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[16]);
          //MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[16]);
          //MPI_Type_commit ( &sdats[16] );
          //MPI_Type_commit ( &rdats[16] );

          ///* Set the send and recv derived datatype for (1,0,1) to (0,1,0)*/
          //for ( count=0, i=0; i<hwidth; i++ ) {
          //    for ( j=0; j<hwidth; j++, count++ ) {
          //        sdisps[count]   = dist_4D_C( vecdim[0]-2*hwidth+i, hwidth+j, vecdim[2]-2*hwidth, 0, vecdim, nc);
          //        rdisps[count]   = dist_4D_C( vecdim[0]-hwidth+i, j, vecdim[2]-hwidth, 0, vecdim, nc);
          //    }
          //}
          //MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[17]);
          //MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[17]);
          //MPI_Type_commit ( &sdats[17] );
          //MPI_Type_commit ( &rdats[17] );
       }
    }
    else {
        /* MPI_ORDER_FORTRAN */
        if ( nc < 1 ) nc = 1;

        /* Set the send and recv derived datatype for the lower end of x-direction */
        for ( count=0, i=0; i<nc; i++ ) {
            for ( j=hwidth; j<vecdim[2]-hwidth; j++ ) {
                for ( k=hwidth; k<(vecdim[1]-hwidth); k++, count++ ) {
                    countarr[count]  = hwidth;
                    sdisps[count]   = dist_4D_Fortran( hwidth, k, j, i, vecdim, nc);
                    rdisps[count]   = dist_4D_Fortran( 0, k, j, i, vecdim, nc);
                }
            }
        }
        MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[0]);
        MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[0]);
        MPI_Type_commit ( &sdats[0] );
        MPI_Type_commit ( &rdats[0] );

        /* Set the send and recv derived datatype for the upper end of x-direction */
        for ( count=0, i=0; i<nc; i++ ) {
            for ( j=hwidth; j<vecdim[2]-hwidth; j++ ) {
                for ( k=hwidth; k<(vecdim[1]-hwidth); k++, count++ ) {
                    countarr[count]  = hwidth;
                    sdisps[count]   = dist_4D_Fortran( vecdim[0]-2*hwidth, k, j, i, vecdim, nc);
                    rdisps[count]   = dist_4D_Fortran( vecdim[0]-hwidth, k, j, i, vecdim, nc);
                }
            }
        }
        MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[1]);
        MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[1]);
        MPI_Type_commit ( &sdats[1] );
        MPI_Type_commit ( &rdats[1] );

        /* Set the send and recv derived datatype for the lower end of y-direction */
        for ( count=0, i=0; i<nc; i++ ) {
            for ( j=hwidth; j<(vecdim[2]-hwidth); j++ ) {
                for ( k=0; k<hwidth; k++, count++ ) {
                    countarr[count]  = vecdim[0]-2*hwidth;
                    sdisps[count]   = dist_4D_Fortran( hwidth, hwidth+k, j, i, vecdim, nc);
                    rdisps[count]   = dist_4D_Fortran( hwidth, k, j, i, vecdim, nc);
                }
            }
        }
        MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[2]);
        MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[2]);
        MPI_Type_commit ( &sdats[2] );
        MPI_Type_commit ( &rdats[2] );

        /* Set the send and recv derived datatype for the upper end of y-direction */
        for ( count=0, i=0; i<nc; i++ ) {
            for ( j=hwidth; j<(vecdim[2]-hwidth); j++ ) {
                for ( k=0; k<hwidth; k++, count++ ) {
                    countarr[count]  = vecdim[0]-2*hwidth;
                    sdisps[count]   = dist_4D_Fortran( hwidth, vecdim[1]-2*hwidth+k, j, i, vecdim, nc);
                    rdisps[count]   = dist_4D_Fortran( hwidth, vecdim[1]-hwidth+k, j, i, vecdim, nc);
                }
            }
        }
        MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[3]);
        MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[3]);
        MPI_Type_commit ( &sdats[3] );
        MPI_Type_commit ( &rdats[3] );


        /* Set the send and recv derived datatype for the lower end of z-direction */
        for ( count=0, i=0; i<nc; i++ ) {
            for ( k=0; k<hwidth; k++ ) {
                for ( j=hwidth; j<(vecdim[1]-hwidth); j++, count++ ) {
                    countarr[count] = vecdim[0]-2*hwidth;
                    sdisps[count]   = dist_4D_Fortran( hwidth, j, hwidth+k, i, vecdim, nc);
                    rdisps[count]   = dist_4D_Fortran( hwidth, j, k, i, vecdim, nc);
                }
            }
        }
        MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[4]);
        MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[4]);
        MPI_Type_commit ( &sdats[4] );
        MPI_Type_commit ( &rdats[4] );

       /* Set the send and recv derived datatype for the upper end of z-direction */
       for ( count=0, i=0; i<nc; i++ ) {
           for ( k=0; k<hwidth; k++ ) {
               for ( j=hwidth; j<(vecdim[1]-hwidth); j++, count++ ) {
                   countarr[count]  = vecdim[0]-2*hwidth;
                   sdisps[count]   = dist_4D_Fortran( hwidth, j, vecdim[2]-2*hwidth+k, i, vecdim, nc);
                   rdisps[count]   = dist_4D_Fortran( hwidth, j, vecdim[2]-hwidth+k, i, vecdim, nc);
               }
           }
       }
       MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[5]);
       MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[5]);
       MPI_Type_commit ( &sdats[5] );
       MPI_Type_commit ( &rdats[5] );


        if ( nneigh > 3 ) {

           /* *** VERTICAL EDGES *** */

           /* Set the send and recv derived datatype for the edge (0,0,z) */
           for ( count=0, l=0; l<nc; l++ ) {
               for ( k=hwidth; k<vecdim[2]-hwidth; k++ ) {
                   for ( j=0; j<hwidth; j++, count++ ) {
                       countarr[count] = hwidth;
                       sdisps[count]   = dist_4D_Fortran( hwidth, hwidth+j, k, l, vecdim, nc);
                       rdisps[count]   = dist_4D_Fortran( 0, j, k, l, vecdim, nc);
                   }
               }
           }
           MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[6]);
           MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[6]);
           MPI_Type_commit ( &sdats[6] );
           MPI_Type_commit ( &rdats[6] );

           /* Set the send and recv derived datatype for the edge (1,1,z) */
           for ( count=0, l=0; l<nc; l++ ) {
               for ( k=hwidth; k<vecdim[2]-hwidth; k++ ) {
                   for ( j=0; j<hwidth; j++, count++ ) {
                       countarr[count] = hwidth;
                       sdisps[count]   = dist_4D_Fortran( vecdim[0]-2*hwidth, vecdim[1]-2*hwidth+j, k, l, vecdim, nc);
                       rdisps[count]   = dist_4D_Fortran( vecdim[0]-hwidth, vecdim[1]-hwidth+j, k, l, vecdim, nc);
                   }
               }
           }
           MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[7]);
           MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[7]);
           MPI_Type_commit ( &sdats[7] );
           MPI_Type_commit ( &rdats[7] );

           /* Set the send and recv derived datatypes for the edge (0,1,z) */
           for ( count=0, l=0; l<nc; l++ ) {
               for ( k=hwidth; k<vecdim[2]-hwidth; k++ ) {
                   for ( j=0; j<hwidth; j++, count++ ) {
                       countarr[count]  = hwidth;
                       sdisps[count] = dist_4D_Fortran( hwidth, vecdim[1]-2*hwidth+j, k, l, vecdim, nc );
                       rdisps[count] = dist_4D_Fortran( 0, vecdim[1]-hwidth+j, k, l, vecdim, nc );
                   }
               }
           }
           MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[8]);
           MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[8]);
           MPI_Type_commit ( &rdats[8]);
           MPI_Type_commit ( &sdats[8]);

           /* Set the send and recv derived datatypes for the edge (1,0,z) */
           for ( count=0, l=0; l<nc; l++ ) {
               for ( k=hwidth; k<vecdim[2]-hwidth; k++ ) {
                   for ( j=0; j<hwidth; j++, count++ ) {
                       sdisps[count] = dist_4D_Fortran( vecdim[0]-2*hwidth, hwidth+j, k, l, vecdim, nc );
                       rdisps[count] = dist_4D_Fortran( vecdim[0]-hwidth, j, k, l, vecdim, nc );
                   }
               }
           }
           MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[9]);
           MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[9]);
           MPI_Type_commit ( &rdats[9]);
           MPI_Type_commit ( &sdats[9]);

           /* *** HORIZONTAL EDGES *** */

          /* (0,-1,-1) - (0,+1,+1) */
          /* Set the send and recv derived datatypes for the edge (0,y,0) (3) */
          for ( count=0, l=0; l<nc; l++ ) {
              for ( k=0; k<hwidth; k++ ) {
                  for ( j=hwidth; j<(vecdim[1]-hwidth); j++, count++ ) {
                      countarr[count] = hwidth;
                      sdisps[count]   = dist_4D_Fortran( hwidth, j, hwidth+k, l, vecdim, nc);
                      rdisps[count]   = dist_4D_Fortran( 0, j, k, l, vecdim, nc);
                  }
              }
          }
          MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[10]);
          MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[10]);
          MPI_Type_commit ( &sdats[10] );
          MPI_Type_commit ( &rdats[10] );

          /* Set the send and recv derived datatypes for the edge (1,y,1) (23) */
          for ( count=0, l=0; l<nc; l++ ) {
              for ( k=0; k<hwidth; k++ ) {
                  for ( j=hwidth; j<(vecdim[1]-hwidth); j++, count++ ) {
                      countarr[count]  = hwidth;
                      sdisps[count]   = dist_4D_Fortran( vecdim[0]-2*hwidth, j, vecdim[2]-2*hwidth+k, l, vecdim, nc);
                      rdisps[count]   = dist_4D_Fortran( vecdim[0]-hwidth, j, vecdim[2]-hwidth+k, l, vecdim, nc);
                  }
              }
          }
          MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[11]);
          MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[11]);
          MPI_Type_commit ( &sdats[11] );
          MPI_Type_commit ( &rdats[11] );

           /* (1,0,-1) - (-1,0,+1) */
           /* Set the send and recv derived datatypes for the edge (x,1,0) (15) */
           for ( count=0, l=0; l<nc; l++ ) {
               for ( k=0; k<hwidth; k++ ) {
                   for ( j=0; j<hwidth; j++, count++ ) {
                       countarr[count] = vecdim[0]-2*hwidth;
                       sdisps[count] = dist_4D_Fortran( hwidth, vecdim[1]-2*hwidth+j, hwidth+k, l, vecdim, nc );
                       rdisps[count] = dist_4D_Fortran( hwidth, vecdim[1]-hwidth+j, k, l, vecdim, nc );
                   }
               }
           }
           MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[12]);
           MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[12]);
           MPI_Type_commit ( &sdats[12]);
           MPI_Type_commit ( &rdats[12]);

           /* Set the send and recv derived datatypes for the edge (x,0,1) (11) */
           for ( count=0, l=0; l<nc; l++ ) {
               for ( k=0; k<hwidth; k++ ) {
                   for ( j=0; j<hwidth; j++, count++ ) {
                       countarr[count] = vecdim[0]-2*hwidth;
                       sdisps[count]   = dist_4D_Fortran( hwidth, hwidth+j, vecdim[2]-2*hwidth+k, l, vecdim, nc );
                       rdisps[count]   = dist_4D_Fortran( hwidth, j, vecdim[2]-hwidth+k, l, vecdim, nc );
                   }
               }
           }
           MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[13]);
           MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[13]);
           MPI_Type_commit ( &sdats[13]);
           MPI_Type_commit ( &rdats[13]);

           /* (0,-1,1) - (0,1,-1) */
           /* Set the send and recv derived datatypes for the edge (1,y,0) (21) */
           for ( count=0, l=0; l<nc; l++ ) {
               for ( k=0; k<hwidth; k++ ) {
                   for ( j=hwidth; j<(vecdim[1]-hwidth); j++, count++ ) {
                       countarr[count] = hwidth;
                       sdisps[count]   = dist_4D_Fortran( vecdim[0]-2*hwidth, j, hwidth+k, l, vecdim, nc);
                       rdisps[count]   = dist_4D_Fortran( vecdim[0]-hwidth, j, k, l, vecdim, nc);
                   }
               }
           }
           MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[14]);
           MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[14]);
           MPI_Type_commit ( &sdats[14] );
           MPI_Type_commit ( &rdats[14] );

           /* Set the send and recv derived datatypes for the edge (0,y,1) (5) */
           for ( count=0, l=0; l<nc; l++ ) {
               for ( k=0; k<hwidth; k++ ) {
                   for ( j=hwidth; j<(vecdim[1]-hwidth); j++, count++ ) {
                       countarr[count]  = hwidth;
                       sdisps[count]   = dist_4D_Fortran( hwidth, j, vecdim[2]-2*hwidth+k, l, vecdim, nc);
                       rdisps[count]   = dist_4D_Fortran( 0, j, vecdim[2]-hwidth+k, l, vecdim, nc);
                   }
               }
           }
           MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[15]);
           MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[15]);
           MPI_Type_commit ( &sdats[15] );
           MPI_Type_commit ( &rdats[15] );

           /* (-1,0,-1) - (+1,0,+1) */
           /* Set the send and recv derived datatypes for the edge (x,0,0) (9) */
           for ( count=0, l=0; l<nc; l++ ) {
               for ( k=0; k<hwidth; k++ ) {
                   for ( j=0; j<hwidth; j++, count++ ) {
                       countarr[count] = vecdim[0]-2*hwidth;
                       sdisps[count] = dist_4D_Fortran( hwidth, hwidth+j, hwidth+k, l, vecdim, nc );
                       rdisps[count] = dist_4D_Fortran( hwidth, j, k, l, vecdim, nc );
                   }
               }
           }
           MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[16]);
           MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[16]);
           MPI_Type_commit ( &sdats[16]);
           MPI_Type_commit ( &rdats[16]);

           /* Set the send and recv derived datatypes for the edge (x,1,1) (17) */ 
           for ( count=0, l=0; l<nc; l++ ) {
               for ( k=0; k<hwidth; k++ ) {
                   for ( j=0; j<hwidth; j++, count++ ) {
                       countarr[count] = vecdim[0]-2*hwidth;
                       sdisps[count]   = dist_4D_Fortran( hwidth, vecdim[1]-2*hwidth+j, vecdim[2]-2*hwidth+k, l, vecdim, nc );
                       rdisps[count]   = dist_4D_Fortran( hwidth, vecdim[1]-hwidth+j, vecdim[2]-hwidth+k, l, vecdim, nc );
                   }
               }
           }
           MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[17]);
           MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[17]);
           MPI_Type_commit ( &sdats[17]);
           MPI_Type_commit ( &rdats[17]);



            //            /* *** CORNERS *** */
            //
            //            /* Set the send and recv derived datatype for (0,0,0) to (1,1,1) */
            //            for ( count=0, l=0; l<nc; l++ ) {
            //                for ( i=0; i<hwidth; i++ ) {
            //                    for ( j=0; j<hwidth; j++, count++ ) {
            //                        sdisps[count]   = dist_4D_Fortran( hwidth, hwidth+j, hwidth+i, l, vecdim, nc);
            //                        rdisps[count]   = dist_4D_Fortran( 0, j, i, l, vecdim, nc);
            //                    }
            //                }
            //            }
            //            MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[6]);
            //            MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[6]);
            //            MPI_Type_commit ( &sdats[6] );
            //            MPI_Type_commit ( &rdats[6] );
            //
            //            /* Set the send and recv derived datatype for (1,1,1) to (0,0,0)*/
            //            for ( count=0, l=0; l<nc; l++ ) {
            //                for ( i=0; i<hwidth; i++ ) {
            //                    for ( j=0; j<hwidth; j++, count++ ) {
            //                        sdisps[count]   = dist_4D_Fortran( vecdim[0]-2*hwidth, vecdim[1]-2*hwidth+j, vecdim[2]-2*hwidth+i, l, vecdim, nc);
            //                        rdisps[count]   = dist_4D_Fortran( vecdim[0]-hwidth, vecdim[1]-hwidth+j, vecdim[2]-hwidth+i, l, vecdim, nc);
            //                    }
            //                }
            //            }
            //            MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[7]);
            //            MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[7]);
            //            MPI_Type_commit ( &sdats[7] );
            //            MPI_Type_commit ( &rdats[7] );
            //
            //            /* Set the send and recv derived datatype for (1,0,0) to (0,1,1) */
            //            for ( count=0, l=0; l<nc; l++ ) {
            //                for ( i=0; i<hwidth; i++ ) {
//                    for ( j=0; j<hwidth; j++, count++ ) {
//                        sdisps[count]   = dist_4D_Fortran( vecdim[0]-2*hwidth, hwidth+j, hwidth+i, l, vecdim, nc);
//                        rdisps[count]   = dist_4D_Fortran( vecdim[0]-hwidth, j, i, l, vecdim, nc);
//                    }
//                }
//            }
//            MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[8]);
//            MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[8]);
//            MPI_Type_commit ( &sdats[8] );
//            MPI_Type_commit ( &rdats[8] );
//
//            /* Set the send and recv derived datatype for (0,1,1) to (1,0,0)*/
//            for ( count=0, l=0; l<nc; l++ ) {
//                for ( i=0; i<hwidth; i++ ) {
//                    for ( j=0; j<hwidth; j++, count++ ) {
//                        sdisps[count]   = dist_4D_Fortran( hwidth, vecdim[1]-2*hwidth+j, vecdim[2]-2*hwidth+i, l, vecdim, nc);
//                        rdisps[count]   = dist_4D_Fortran( 0, vecdim[1]-hwidth+j, vecdim[2]-hwidth+i, l, vecdim, nc);
//                    }
//                }
//            }
//            MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[9]);
//            MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[9]);
//            MPI_Type_commit ( &sdats[9] );
//            MPI_Type_commit ( &rdats[9] );
//
//            /* Set the send and recv derived datatype for (1,1,0) to (0,0,1) */
//            for ( count=0, l=0; l<nc; l++ ) {
//                for ( i=0; i<hwidth; i++ ) {
//                    for ( j=0; j<hwidth; j++, count++ ) {
//                        sdisps[count]   = dist_4D_Fortran( vecdim[0]-2*hwidth, vecdim[1]-2*hwidth+j, hwidth+i, l, vecdim, nc);
//                        rdisps[count]   = dist_4D_Fortran( vecdim[0]-hwidth, vecdim[1]-hwidth+j, i, l, vecdim, nc);
//                    }
//                }
//            }
//            MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[10]);
//            MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[10]);
//            MPI_Type_commit ( &sdats[10] );
//            MPI_Type_commit ( &rdats[10] );
//
//            /* Set the send and recv derived datatype for (0,0,1) to (1,1,0)*/
//            for ( count=0, l=0; l<nc; l++ ) {
//                for ( i=0; i<hwidth; i++ ) {
//                    for ( j=0; j<hwidth; j++, count++ ) {
//                        sdisps[count]   = dist_4D_Fortran( hwidth, hwidth+j, vecdim[2]-2*hwidth+i, l, vecdim, nc);
//                        rdisps[count]   = dist_4D_Fortran( 0, j, vecdim[2]-hwidth+i, l, vecdim, nc);
//                    }
//                }
//            }
//            MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[11]);
//            MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[11]);
//            MPI_Type_commit ( &sdats[11] );
//            MPI_Type_commit ( &rdats[11] );
//
//            /* Set the send and recv derived datatype for (0,1,0) to (1,0,1) */
//            for ( count=0, l=0; l<nc; l++ ) {
//                for ( i=0; i<hwidth; i++ ) {
//                    for ( j=0; j<hwidth; j++, count++ ) {
//                        sdisps[count]   = dist_4D_Fortran( hwidth, vecdim[1]-2*hwidth+j, hwidth+i, l, vecdim, nc);
//                        rdisps[count]   = dist_4D_Fortran( 0, vecdim[1]-hwidth+j, i, l, vecdim, nc);
//                    }
//                }
//            }
//            MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[12]);
//            MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[12]);
//            MPI_Type_commit ( &sdats[12] );
//            MPI_Type_commit ( &rdats[12] );
//
//            /* Set the send and recv derived datatype for (1,0,1) to (0,1,0)*/
//            for ( count=0, l=0; l<nc; l++ ) {
//                for ( i=0; i<hwidth; i++ ) {
//                    for ( j=0; j<hwidth; j++, count++ ) {
//                        sdisps[count]   = dist_4D_Fortran( vecdim[0]-2*hwidth, hwidth+j, vecdim[2]-2*hwidth+i, l, vecdim, nc);
//                        rdisps[count]   = dist_4D_Fortran( vecdim[0]-hwidth, j, vecdim[2]-hwidth+i, l, vecdim, nc);
//                    }
//                }
//            }
//            MPI_Type_indexed ( count, countarr, sdisps, btype, &sdats[13]);
//            MPI_Type_indexed ( count, countarr, rdisps, btype, &rdats[13]);
//            MPI_Type_commit ( &sdats[13] );
//            MPI_Type_commit ( &rdats[13] );
        }
    }


    if ( ADCL_SUCCESS != ret ) {
    if ( NULL != sdats ) {
        free ( sdats );
    }
    if ( NULL != rdats ) {
        free ( rdats );
    }

    }

    if ( NULL != countarr ) {
    free ( countarr );
    }
    if ( NULL != sdisps ) {
    free ( sdisps );
    }


    *senddats = sdats;
    *recvdats = rdats;

    return ret;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int dist_4D_C ( int dim0, int dim1, int dim2, int dim3,
        int vecdim[2], int nc)
{
    int distance=0;

    distance = dim3+dim2*nc+dim1*vecdim[2]*nc;
    distance += dim0*nc*vecdim[2]*vecdim[1];

    return distance;
}

int dist_4D_Fortran ( int dim0, int dim1, int dim2, int dim3,
              int vecdim[3], int nc)
{
    int distance=0;

    distance = dim0+ dim1*vecdim[0]+ dim2*vecdim[0]*vecdim[1];
    distance += dim3*vecdim[0]*vecdim[1]*vecdim[2];

    return distance;
}

int dist_3D_Fortran ( int dim0, int dim1, int dim2,
                 int vecdim[2], int nc)
{
    int distance=0;

    distance = dim0+ dim1*vecdim[0]+ dim2*vecdim[0]*vecdim[1];

    return distance;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_max ( int cnt, int *vecdim, int nc, int hwidth )
{
    int i;
    int maxdim;
    int *sorted=NULL;

    sorted = (int *) malloc ( cnt * sizeof (int));
    if ( NULL == sorted ) {
    return ADCL_NO_MEMORY;
    }

    memcpy ( sorted, vecdim, cnt*sizeof(int));
    qsort (sorted, cnt, sizeof(int), compare );

    maxdim = sorted[cnt-1];
    for ( i=cnt-2; i > 0; i-- ) {
    maxdim *= sorted[i];
    }

    if ( nc > 1 ) {
        maxdim *= nc * hwidth;
    }
    else {
        maxdim *= hwidth;
    }

    return (maxdim);
}


int compare ( const void *p, const void* q )
{
    int *a, *b;

    a = (int*) p;
    b = (int*) q;

    if ( *a < *b ) {
    return -1;
    }
    if ( *a > *b ) {
    return 1;
    }

    return 0;
}
