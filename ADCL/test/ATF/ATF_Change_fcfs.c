/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ATF.h"


MPI_Request ATF_recvhandle[6];
MPI_Request ATF_sendhandle[6];

int ATF_Change_fcfs(double ****vec)
{
    MPI_Status statusarr[6];
   
    /* Asynchrones receiving of the vector from the neigbor processes  */
    if (! ATF_rand_ab){
	MPI_Irecv( &(vec[0][0][0][0]), ATF_recvcount[ATF_tid_io], 
		   ATF_recvtype[ATF_tid_io], ATF_tid_io, 10001, 
		   MPI_COMM_WORLD, &ATF_recvhandle[0]);
    }
    else
	ATF_recvhandle[0] = MPI_REQUEST_NULL;
    
    if (! ATF_rand_sing)
	MPI_Irecv(&vec[0][0][0][0], ATF_recvcount[ATF_tid_iu], 
		  ATF_recvtype[ATF_tid_iu], ATF_tid_iu, 10002, 
		  MPI_COMM_WORLD, &ATF_recvhandle[1]);
    else
	ATF_recvhandle[1] = MPI_REQUEST_NULL;

    if (! ATF_rand_zu)
	MPI_Irecv(&(vec[0][0][0][0]), ATF_recvcount[ATF_tid_jo], 
		  ATF_recvtype[ATF_tid_jo], ATF_tid_iu, 10003, 
		  MPI_COMM_WORLD, &ATF_recvhandle[2]);
    else
	ATF_recvhandle[2] = MPI_REQUEST_NULL;

    if (! ATF_rand_festk)
	MPI_Irecv(&(vec[0][0][0][0]), ATF_recvcount[ATF_tid_ju], 
		  ATF_recvtype[ATF_tid_ju], ATF_tid_iu, 10004, 
		  MPI_COMM_WORLD, &ATF_recvhandle[3]);
    else
	ATF_recvhandle[3] = MPI_REQUEST_NULL;


    if (! ATF_rand_symo)
	MPI_Irecv(&(vec[0][0][0][0]), ATF_recvcount[ATF_tid_ko], 
		  ATF_recvtype[ATF_tid_ko], ATF_tid_ko, 10005, 
		  MPI_COMM_WORLD, &ATF_recvhandle[4]);
    else
	ATF_recvhandle[4] = MPI_REQUEST_NULL;

    if (! ATF_rand_symu)
	MPI_Irecv(&(vec[0][0][0][0]), ATF_recvcount[ATF_tid_ku], 
		  ATF_recvtype[ATF_tid_ku], ATF_tid_ku, 10006, 
		  MPI_COMM_WORLD, &ATF_recvhandle[5]);
    else
	ATF_recvhandle[5] = MPI_REQUEST_NULL;

    /* Asynchrones Sending of the vector to the neigbor processes  */
	
    if (! ATF_rand_sing)
	MPI_Isend(&(vec[0][0][0][0]), ATF_sendcount[ATF_tid_iu], 
		  ATF_sendtype[ATF_tid_iu], ATF_tid_iu, 10001, 
		  MPI_COMM_WORLD, &ATF_sendhandle[0]);
    else
	ATF_sendhandle[0] = MPI_REQUEST_NULL;

    if (! ATF_rand_ab)
	MPI_Isend(&(vec[0][0][0][0]), ATF_sendcount[ATF_tid_io], 
		  ATF_sendtype[ATF_tid_io], ATF_tid_io, 10002, 
		  MPI_COMM_WORLD, &ATF_sendhandle[1]);
    else
	ATF_sendhandle[1] = MPI_REQUEST_NULL;


    if (! ATF_rand_festk)
	MPI_Isend(&(vec[0][0][0][0]), ATF_sendcount[ATF_tid_ju], 
		  ATF_sendtype[ATF_tid_ju], ATF_tid_ju, 10003, 
		  MPI_COMM_WORLD, &ATF_sendhandle[2]);
    else
	ATF_sendhandle[2] = MPI_REQUEST_NULL;
    

    if (! ATF_rand_zu)
	MPI_Isend(&(vec[0][0][0][0]), ATF_sendcount[ATF_tid_jo], 
		  ATF_sendtype[ATF_tid_jo], ATF_tid_jo, 10004, 
		  MPI_COMM_WORLD, &ATF_sendhandle[3]);
    else
	ATF_sendhandle[3] = MPI_REQUEST_NULL;

    if (! ATF_rand_symu)
	MPI_Isend(&(vec[0][0][0][0]), ATF_sendcount[ATF_tid_ku], 
		  ATF_sendtype[ATF_tid_ku], ATF_tid_ku, 10005, 
		  MPI_COMM_WORLD, &ATF_sendhandle[4]);
    else
	ATF_sendhandle[4] = MPI_REQUEST_NULL;
    
    if (! ATF_rand_symo)
	MPI_Isend(&(vec[0][0][0][0]), ATF_sendcount[ATF_tid_ko], 
		  ATF_sendtype[ATF_tid_ko], ATF_tid_ko, 10006, 
		  MPI_COMM_WORLD, &ATF_sendhandle[5]);
    else
	ATF_sendhandle[5] = MPI_REQUEST_NULL;
    
    
    MPI_Waitall (6, ATF_sendhandle, statusarr);
    MPI_Waitall (6, ATF_recvhandle, statusarr);

    return ATF_SUCCESS;
}
