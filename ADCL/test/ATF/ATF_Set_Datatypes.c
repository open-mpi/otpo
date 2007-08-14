/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ATF.h"

int ATF_Set_Datatypes ( double ****vec )
{
    int size, totalcount;
    int *countarr, count;
    int i, j, tsize;
    int *disparr;
    MPI_Aint base, address;
    
    int nc=1; /* for now! */

    totalcount = (ATF_dim[1]+2) * (ATF_dim[2]+2);

    /* Allocate the arrays for setting derived datatypes */
    countarr = (int *) malloc ( totalcount * sizeof(int));
    disparr  = (int *) malloc ( totalcount * sizeof(int));
    
    if ( NULL == countarr || NULL == disparr ) {
	printf("%d @ %s, Could not allocate memory\n",  __LINE__,__FILE__);
	return ATF_ERROR;
    }
    
    MPI_Comm_size ( MPI_COMM_WORLD, &size );
    MPI_Type_size ( MPI_DOUBLE, &tsize );

    /* Initialize the counters and datatypes */
    for ( i=0; i<size; i++ ) {
	ATF_sendcount[i] = 0;
	ATF_recvcount[i] = 0;
	ATF_senddisps[i] = 0;
	ATF_recvdisps[i] = 0;

        ATF_sendtype[i] = MPI_DATATYPE_NULL;
	ATF_recvtype[i] = MPI_DATATYPE_NULL;
    }

    MPI_Get_address ( &(vec[0][0][0][0]), &base );

    /* Set the send and recv derived datatype for the lower end of x-direction */
    if ( !ATF_rand_sing ) {
	for ( count=0, j=1; j<= ATF_dim[1]; j++ ) {
	    countarr[count]  = ATF_dim[2] * nc;
	    MPI_Get_address ( &(vec[1][j][1][0]), &address);
	    disparr[count]  = (address - base) / tsize;
	    count++;
	}
	MPI_Type_indexed ( count-1, countarr, disparr, MPI_DOUBLE, &ATF_sendtype[ATF_tid_iu]);
	MPI_Type_commit ( &ATF_sendtype[ATF_tid_iu]);

	for ( count=0, j=1; j<= ATF_dim[1]; j++ ) {
	    countarr[count]  = ATF_dim[2] * nc;
	    MPI_Get_address ( &(vec[0][j][1][0]), &address);
	    disparr[count]  = (address - base) / tsize;
	    count++;
	}
    	MPI_Type_indexed ( count-1, countarr, disparr, MPI_DOUBLE, &ATF_recvtype [ATF_tid_iu]);
	MPI_Type_commit ( &ATF_recvtype [ATF_tid_iu]);

	ATF_sendcount[ATF_tid_iu] = 1;
	ATF_recvcount[ATF_tid_iu] = 1;
    }

    /* Set the send and recv derived datatype for the upper end of x-direction */
    if ( !ATF_rand_ab ) {
	for ( count=0, j=1; j<= ATF_dim[1]; j++ ) {
	    countarr[count]  = ATF_dim[2] * nc;
	    MPI_Get_address ( &(vec[ATF_dim[0]][j][1][0]), &address);
	    disparr[count]  = (address - base) / tsize;
	    count++;
	}
	MPI_Type_indexed ( count-1, countarr, disparr, MPI_DOUBLE, &ATF_sendtype[ATF_tid_io]);
	MPI_Type_commit ( &ATF_sendtype [ATF_tid_io]);
	
	for ( count=0, j=1; j<= ATF_dim[1]; j++ ) {
	    countarr[count]  = ATF_dim[2] * nc;
	    MPI_Get_address ( &(vec[ATF_dim[0]+1][j][1][0]), &address);
	    disparr[count]  = (address - base) / tsize;
	    count++;
	}
	MPI_Type_indexed ( count-1, countarr, disparr, MPI_DOUBLE, &ATF_recvtype [ATF_tid_io]);
	MPI_Type_commit ( &ATF_recvtype [ATF_tid_io] );
	
	ATF_sendcount[ATF_tid_io] = 1;
	ATF_recvcount[ATF_tid_io] = 1;
    }
    
    
    /* Set the send and recv derived datatypes for the lower end of the y-direction */
    if ( !ATF_rand_festk ) {
	for ( count=0, i=1; i <= ATF_dim[0]; i++ ) {
	    countarr[count] = ATF_dim[2] * nc;
	    MPI_Get_address ( &(vec[i][1][1][0]), &address );
	    disparr[count] = (address - base ) /tsize;
	    count++;
	}
	MPI_Type_indexed ( count-1, countarr, disparr, MPI_DOUBLE, &ATF_sendtype[ATF_tid_ju]);
	MPI_Type_commit ( &ATF_sendtype[ATF_tid_ju]);
	
	for ( count=0, i=1; i <= ATF_dim[0]; i++ ) {
	    countarr[count] = ATF_dim[2] * nc;
	    MPI_Get_address ( &(vec[i][0][1][0]), &address );
	    disparr[count] = (address - base ) /tsize;
	    count++;
	}
	MPI_Type_indexed ( count-1, countarr, disparr, MPI_DOUBLE, &ATF_recvtype[ATF_tid_ju]);
	MPI_Type_commit ( &ATF_recvtype[ATF_tid_ju]);
	
	ATF_sendcount[ATF_tid_ju] = 1;
	ATF_recvcount[ATF_tid_ju] = 1;
    }
    
    /* Set the send and recv derived datatypes for the upper end of the y-direction */
    if ( !ATF_rand_zu ) {
	for ( count=0, i=1; i <= ATF_dim[0]; i++ ) {
	    countarr[count] = ATF_dim[2] * nc;
	    MPI_Get_address ( &(vec[i][ATF_dim[1]][1][0]), &address );
	    disparr[count] = (address - base ) /tsize;	count++;
	}
        MPI_Type_indexed ( count-1, countarr, disparr, MPI_DOUBLE, &ATF_sendtype[ATF_tid_jo]);
	MPI_Type_commit ( &ATF_sendtype[ATF_tid_jo]);

	for ( count=0, i=1; i <= ATF_dim[0]; i++ ) {
	    countarr[count] = ATF_dim[2] * nc;
	    MPI_Get_address ( &(vec[i][ATF_dim[1]+1][1][0]), &address );
	    disparr[count] = (address - base ) /tsize;
	    count++;
	}
	MPI_Type_indexed ( count-1, countarr, disparr, MPI_DOUBLE, &ATF_recvtype[ATF_tid_jo]);
	MPI_Type_commit ( &ATF_recvtype[ATF_tid_jo] );

	ATF_sendcount[ATF_tid_jo] = 1;
	ATF_recvcount[ATF_tid_jo] = 1;
    }

    /* Set the send and recv derived datatypes for the lower end of the z-direction */
    if ( !ATF_rand_symu ) {
	for ( count=0, i=1; i<= ATF_dim[0]; i++ ) {
	    for ( j=1; j <= ATF_dim[1]; j++ ) {
		countarr[count] = nc;
		MPI_Get_address ( &(vec[i][j][1][0]), &address );
		disparr[count] = ( address - base ) / tsize;
		count ++;
	    }
	}
	MPI_Type_indexed ( count-1, countarr, disparr, MPI_DOUBLE, &ATF_sendtype[ATF_tid_ku]);
	MPI_Type_commit ( &ATF_sendtype[ATF_tid_ku]);

	for ( count=0, i=1; i<= ATF_dim[0]; i++ ) {
	    for ( j=1; j <= ATF_dim[1]; j++ ) {
		countarr[count] = nc;
		MPI_Get_address ( &(vec[i][j][0][0]), &address );
		disparr[count] = ( address - base ) / tsize;
		count ++;
	    }
	}
	MPI_Type_indexed ( count-1, countarr, disparr, MPI_DOUBLE, &ATF_recvtype[ATF_tid_ku]);
	MPI_Type_commit ( &ATF_recvtype[ATF_tid_ku]);
	
	ATF_sendcount[ATF_tid_ku] = 1;
	ATF_recvcount[ATF_tid_ku] = 1;
    }

    /* Set the send and recv derived datatypes for the upper end of the z-direction */
    if ( !ATF_rand_symo ) {
	for ( count=0, i=1; i<= ATF_dim[0]; i++ ) {
	    for ( j=1; j <= ATF_dim[1]; j++ ) {
		countarr[count] = nc;
		MPI_Get_address ( &(vec[i][j][ATF_dim[2]][0]), &address );
		disparr[count] = ( address - base ) / tsize;
		count ++;
	    }
	}
	MPI_Type_indexed ( count-1, countarr, disparr, MPI_DOUBLE, &ATF_sendtype[ATF_tid_ko]);
	MPI_Type_commit ( &ATF_sendtype[ATF_tid_ko]);
	
	for ( count=0, i=1; i<= ATF_dim[0]; i++ ) {
	    for ( j=1; j <= ATF_dim[1]; j++ ) {
		countarr[count] = nc;
		MPI_Get_address ( &(vec[i][j][ATF_dim[2]+1][0]), &address );
		disparr[count] = ( address - base ) / tsize;
		count ++;
	    }
	}
	MPI_Type_indexed ( count-1, countarr, disparr, MPI_DOUBLE, &ATF_recvtype[ATF_tid_ko]);
	MPI_Type_commit ( &ATF_recvtype[ATF_tid_ko]);
	
	ATF_sendcount[ATF_tid_ko] = 1;
	ATF_recvcount[ATF_tid_ko] = 1;
    }

    free ( countarr );
    free ( disparr );

    return ATF_SUCCESS;
}

