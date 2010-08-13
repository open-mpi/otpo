/*
 * Copyright (c) 2006-2007     University of Houston. All rights reserved.
 * Copyright (c) 2007          Cisco, Inc. All rights reserved.
 * $COPYRIGHTS$
 *
 * Additional copyrights may follow
 *
 * $HEADERS$
 */

#include <string.h>
#include "ADCL.h"

int MPI_Init      ( int *argc, char ***argv )
{
    return MPI_SUCCESS;
}
int MPI_Finalize  ( void )
{
    return MPI_SUCCESS;
}
int MPI_Comm_rank ( MPI_Comm comm, int *rank )
{
    return MPI_SUCCESS;
}
int MPI_Comm_size ( MPI_Comm comm, int *size )
{
    return MPI_SUCCESS;
}
int MPI_Comm_free ( MPI_Comm * comm )
{
    return MPI_SUCCESS;
}
MPI_Fint MPI_Comm_c2f (MPI_Comm comm )
{
    return MPI_SUCCESS;
}
int MPI_Comm_compare  ( MPI_Comm comm1, MPI_Comm comm2, int *result )
{
    return MPI_SUCCESS;
}
int MPI_Comm_group ( MPI_Comm comm, MPI_Group *group )
{
    return MPI_SUCCESS;
}
int MPI_Group_free ( MPI_Group *group)
{
    return MPI_SUCCESS;
}

int MPI_Topo_test   ( MPI_Comm comm, int *status )
{
    return MPI_SUCCESS;
}
int MPI_Cartdim_get ( MPI_Comm comm, int *ndims )
{
    return MPI_SUCCESS;
}
int MPI_Cart_shift  ( MPI_Comm comm, int direction, int disp,
		      int *rank_source, int *rank_dest )
{
    return MPI_SUCCESS;
}
int MPI_Cart_coords ( MPI_Comm comm, int rank, int maxdims, int *coords )
{
    return MPI_SUCCESS;
}
int MPI_Cart_create ( MPI_Comm old_comm, int ndims, int *dims,
		      int *periods, int reorder, MPI_Comm *comm_cart)
{
    return MPI_SUCCESS;
}
int MPI_Dims_create (  int nnodes, int ndims, int *dims )
{
    return MPI_SUCCESS;
}


MPI_Fint MPI_Type_f2c      ( MPI_Datatype datatype )
{
    return MPI_SUCCESS;
}
int MPI_Type_create_subarray( int ndims, int size_array[], int subsize_array[],
			     int start_array[], int order,
			     MPI_Datatype oldtype, MPI_Datatype *newtype)
{
    return MPI_SUCCESS;
}
int MPI_Type_get_envelope  ( MPI_Datatype type, int *num_integers,
			     int *num_addresses, int *num_datatypes,
			     int *combiner)
{
    return MPI_SUCCESS;
}
int MPI_Type_indexed       ( int count, int array_of_blocklengths[],
			     int array_of_displacements[],
			     MPI_Datatype oldtype, MPI_Datatype *newtype)
{
    return MPI_SUCCESS;
}
int MPI_Type_commit ( MPI_Datatype *dat )
{
    return MPI_SUCCESS;
}
int MPI_Type_free   ( MPI_Datatype *dat )
{
    return MPI_SUCCESS;
}
int MPI_Get_address ( void *location, MPI_Aint *address )
{
    return MPI_SUCCESS;
}

int MPI_Win_create ( void *base, MPI_Aint size, int disp_unit,
		     MPI_Info info, MPI_Comm comm, MPI_Win *win)
{
    return MPI_SUCCESS;
}
int MPI_Win_free   ( MPI_Win *win )
{
    return MPI_SUCCESS;
}
int MPI_Win_fence  ( int assert, MPI_Win win )
{
    return MPI_SUCCESS;
}
int MPI_Win_start  ( MPI_Group group, int assert, MPI_Win win )
{
    return MPI_SUCCESS;
}
int MPI_Win_post   ( MPI_Group group, int assert, MPI_Win win )
{
    return MPI_SUCCESS;
}
int MPI_Win_complete( MPI_Win win )
{
    return MPI_SUCCESS;
}
int MPI_Win_wait   ( MPI_Win win )
{
    return MPI_SUCCESS;
}
int MPI_Put ( void *origin_addr, int origin_count, MPI_Datatype origin_datatype,
	      int target_rank, MPI_Aint target_disp, int target_count,
	      MPI_Datatype target_datatype, MPI_Win win)
{
    return MPI_SUCCESS;
}
int MPI_Get ( void *origin_addr, int origin_count,
	      MPI_Datatype origin_datatype, int target_rank,
	      MPI_Aint target_disp, int target_count,
	      MPI_Datatype target_datatype, MPI_Win win )
{
    return MPI_SUCCESS;
}

int MPI_Abort     ( MPI_Comm comm, int errcode )
{
    return MPI_SUCCESS;
}
int MPI_Barrier   ( MPI_Comm comm )
{
    return MPI_SUCCESS;
}
int MPI_Allreduce ( void *inbuf, void* outbuf, int cnt, MPI_Datatype dat, 
		    MPI_Op op, MPI_Comm comm)
{
    memcpy( outbuf, inbuf, cnt*sizeof(double) );
    return MPI_SUCCESS;
}
int MPI_Alltoallv ( void *sendbuf, int *sendcounts, int *sdispl,  MPI_Datatype sendtype, 
                    void *recvbuf, int *recvcounts, int *rdispls, MPI_Datatype recvtype, MPI_Comm comm){
    return MPI_SUCCESS;
}
int MPI_Reduce ( void *inbuf, void* outbuf, int cnt, MPI_Datatype dat, 
		 MPI_Op op, int root, MPI_Comm comm)
{
    return MPI_SUCCESS;
}
int MPI_Bcast  ( void *buf, int cnt, MPI_Datatype dat, int root, 
		 MPI_Comm comm)
{
    return MPI_SUCCESS;
}

int MPI_Send  ( void *buf, int cnt, MPI_Datatype dat, int dest, int tag, MPI_Comm comm)
{
    return MPI_SUCCESS;
}
int MPI_Isend ( void *buf, int cnt, MPI_Datatype dat, int dest, int tag, MPI_Comm comm, 
		MPI_Request *req)
{
    return MPI_SUCCESS;
}
int MPI_Irecv ( void *buf, int cnt, MPI_Datatype dat, int src, int tag, MPI_Comm comm, 
		MPI_Request *req)
{
    return MPI_SUCCESS;
}
int MPI_Recv  ( void *buf, int cnt, MPI_Datatype dat, int src, int tag, MPI_Comm comm, 
		MPI_Status *stat)
{
    return MPI_SUCCESS;
}
int MPI_Sendrecv ( void *sendbuf, int sendcount, MPI_Datatype sendtype,
		   int dest, int sendtag, void *recvbuf, int recvcount,
		   MPI_Datatype recvtype, int source, int recvtag,
		   MPI_Comm comm,  MPI_Status *status)
{
    return MPI_SUCCESS;
}
int MPI_Waitall ( int count, MPI_Request *array_of_requests,
		  MPI_Status *array_of_statuses)
{
    return MPI_SUCCESS;
}
int MPI_Wait    ( MPI_Request *request, MPI_Status *status)
{
    return MPI_SUCCESS;
}

int MPI_Unpack  ( void *inbuf, int insize, int *position,
		  void *outbuf, int outcount, MPI_Datatype datatype,
                 MPI_Comm comm)
{
    return MPI_SUCCESS;
}
int MPI_Pack    ( void *inbuf, int incount, MPI_Datatype datatype,
		  void *outbuf, int outsize, int *position, MPI_Comm comm)
{
    return MPI_SUCCESS;
}
int MPI_Pack_size( int incount, MPI_Datatype datatype, MPI_Comm comm,
		   int *size)
{
    return MPI_SUCCESS;
}
double MPI_Wtime(void)
{
    return 0.0;
}
int MPI_Cart_get ( MPI_Comm comm, int maxdims, int *dims, int *periods, int *coords ){
    return MPI_SUCCESS;
}
int MPI_Type_contiguous( int count, MPI_Datatype old_type, MPI_Datatype *newtype){
    return MPI_SUCCESS;
}
int MPI_Allgatherv ( void *sendbuf, int sendcount, MPI_Datatype sendtype,
                     void *recvbuf, int *recvcounts, int *displs,
                    MPI_Datatype recvtype, MPI_Comm comm ){
    return MPI_SUCCESS;
}
int MPI_Type_get_extent( MPI_Datatype datatype, MPI_Aint *lb, MPI_Aint *extent ){
    return MPI_SUCCESS;
}
int MPI_Gatherv ( void *sendbuf, int sendcnt, MPI_Datatype sendtype,
                  void *recvbuf, int *recvcnts, int *displs,
                 MPI_Datatype recvtype, int root, MPI_Comm comm ){
    return MPI_SUCCESS;
}
int MPI_Type_get_true_extent(MPI_Datatype datatype, MPI_Aint *true_lb,
                           MPI_Aint *true_extent){
    return MPI_SUCCESS;
}
int MPI_Type_size ( MPI_Datatype datatype, int *size ){
    return MPI_SUCCESS;
}

#ifdef _SX
int mpi_win_null_delete_fn_(){
    return MPI_SUCCESS;
}
int mpi_nec_block_time_ (){
    return MPI_SUCCESS;
}
int mpi_dup_fn_ (){
    return MPI_SUCCESS;
}
int mpi_comm_dup_fn_ (){
    return MPI_SUCCESS;
}
int mpi_comm_null_copy_fn_ (){
    return MPI_SUCCESS;
}
int mpi_comm_null_delete_fn_ (){
    return MPI_SUCCESS;
}
int mpi_null_copy_fn_ (){
    return MPI_SUCCESS;
}
int mpi_null_delete_fn_ (){
    return MPI_SUCCESS;
}
int mpi_type_dup_fn_ (){
    return MPI_SUCCESS;
}
int mpi_type_null_copy_fn_ (){
    return MPI_SUCCESS;
}
int mpi_type_null_delete_fn_ (){
    return MPI_SUCCESS;
}
int mpi_win_dup_fn_ (){
    return MPI_SUCCESS;
}
int mpi_win_null_copy_fn_ (){
    return MPI_SUCCESS;
}
int mpi_wtick_ (){
    return MPI_SUCCESS;
}
double mpi_wtime_ (){
    return 0.0; 
}
#endif
