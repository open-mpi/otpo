/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"


int ADCL_packunpack_init ( int num, int neighbors[], MPI_Comm comm,
               char ***sendbuf, MPI_Datatype *sdats, int **spsizes,
               char ***recvbuf, MPI_Datatype *rdats, int **rpsizes )
{
    int i;
    int *ssize, *rsize;
    char **sbuf=*sendbuf;
    char **rbuf=*recvbuf;

    rbuf  = (char **) calloc (1, num * sizeof(char*));
    sbuf  = (char **) calloc (1, num * sizeof(char*));
    ssize = (int *) calloc (1, num * sizeof(int));
    rsize = (int *) calloc (1, num * sizeof(int));
    if ( NULL == rbuf || NULL == sbuf ||
     NULL == ssize|| NULL == rsize ) {
    return ADCL_NO_MEMORY;
    }

    for ( i = 0; i < num; i++ ) {
    if ( MPI_PROC_NULL != neighbors[i] ) {
        MPI_Pack_size ( 1, rdats[i], comm, &rsize[i] );
        MPI_Pack_size ( 1, sdats[i], comm, &ssize[i] );
        rbuf[i] = (char *) malloc ( rsize[i] );
        sbuf[i] = (char *) malloc ( ssize[i] );
        if ( NULL == rbuf[i] || NULL == sbuf[i] ) {
        return ADCL_NO_MEMORY;
        }
    }
    }

    *sendbuf = sbuf;
    *recvbuf = rbuf;
    *spsizes = ssize;
    *rpsizes = rsize;
    return ADCL_SUCCESS;
}

void ADCL_packunpack_free ( int num, char ***sendbuf, char ***recvbuf,
                int **sp, int **rp )
{
    int i;
    char **sbuf = *sendbuf;
    char **rbuf = *recvbuf;
    int *ssize = *sp;
    int *rsize = *rp;

    if ( NULL != rbuf ) {
    for ( i = 0; i < num; i++ ) {
        if ( NULL != rbuf[i] ) {
        free ( rbuf[i] );
        }
    }
    free ( rbuf );
    }

    if ( NULL != sbuf ) {
    for ( i = 0; i < num; i++ ) {
        if ( NULL != sbuf[i] ) {
        free ( sbuf[i] );
        }
    }
    free ( sbuf );
    }

    if ( NULL != ssize ) {
    free ( ssize );
    }

    if ( NULL != rsize ) {
    free ( rsize );
    }



    *sp = NULL;
    *rp = NULL;
    *sendbuf=NULL;
    *recvbuf=NULL;
    return;
}
