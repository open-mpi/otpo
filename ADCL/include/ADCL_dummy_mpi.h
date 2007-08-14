/*
 * Copyright (c) 2006-2007     University of Houston. All rights reserved.
 * Copyright (c) 2007          Cisco, Inc. All rights reserved.
 * $Copyrights$
 *
 * Additional Copyrights may follow
 *
 * $HEADERS$
 */
#ifndef __MPI_H__
#define __MPI_H__

/* define constants and dummy objects */

typedef  int * MPI_Comm;
typedef  int * MPI_Group;
typedef  int * MPI_Datatype;
typedef  int * MPI_Request;
typedef  int * MPI_Op;
typedef  int * MPI_Win;
typedef  int   MPI_Status;
typedef  int * MPI_Info;
typedef  long  MPI_Aint;
typedef  int   MPI_Fint;


#define MPI_COMM_WORLD    (MPI_Comm)0
#define MPI_COMM_NULL     (MPI_Comm)1
#define MPI_DATATYPE_NULL (MPI_Datatype)2
#define MPI_PROC_NULL     3
#define MPI_INFO_NULL     (MPI_Info)4
#define MPI_GROUP_NULL    (MPI_Group)5
#define MPI_WIN_NULL      (MPI_Win)6
#define MPI_REQUEST_NULL  (MPI_Request)7

#define MPI_ORDER_C       8
#define MPI_ORDER_FORTRAN 9

#define MPI_COMBINER_NAMED 10
#define MPI_UNDEFINED      11
#define MPI_GRAPH          12
#define MPI_IDENT          13
#define MPI_CONGRUENT      14
#define MPI_CART           15

#define MPI_MAX            (MPI_Op)16

#define MPI_STATUS_IGNORE   (MPI_Status*)17
#define MPI_STATUSES_IGNORE (MPI_Status*)18

#define MPI_SUCCESS        0

#define MPI_DOUBLE_PRECISION (MPI_Datatype)20
#define MPI_DOUBLE           (MPI_Datatype)21
#define MPI_REAL             (MPI_Datatype)22
#define MPI_FLOAT            (MPI_Datatype)23
#define MPI_INT              (MPI_Datatype)24
#define MPI_INTEGER          (MPI_Datatype)25
#define MPI_PACKED           (MPI_Datatype)26

/* Prototypes of the functions */
int MPI_Init      ( int *argc, char ***argv );
int MPI_Finalize  ( void );
int MPI_Comm_rank ( MPI_Comm comm, int *rank );
int MPI_Comm_size ( MPI_Comm comm, int *size );
int MPI_Comm_free ( MPI_Comm * comm );
MPI_Fint MPI_Comm_c2f (MPI_Comm comm );
int MPI_Comm_compare  ( MPI_Comm comm1, MPI_Comm comm2, int *result );
int MPI_Comm_group ( MPI_Comm comm, MPI_Group *group );
int MPI_Group_free ( MPI_Group *group);

int MPI_Topo_test   ( MPI_Comm comm, int *status );
int MPI_Cartdim_get ( MPI_Comm comm, int *ndims );
int MPI_Cart_shift  ( MPI_Comm comm, int direction, int disp,
		      int *rank_source, int *rank_dest );
int MPI_Cart_coords ( MPI_Comm comm, int rank, int maxdims, int *coords );
int MPI_Cart_create ( MPI_Comm old_comm, int ndims, int *dims,
		      int *periods, int reorder, MPI_Comm *comm_cart);
int MPI_Dims_create (  int nnodes, int ndims, int *dims );


MPI_Fint MPI_Type_f2c      ( MPI_Datatype datatype );
int MPI_Type_create_subarray( int ndims, int size_array[], int subsize_array[],
			     int start_array[], int order,
			     MPI_Datatype oldtype, MPI_Datatype *newtype);
int MPI_Type_get_envelope  ( MPI_Datatype type, int *num_integers,
			     int *num_addresses, int *num_datatypes,
			     int *combiner);
int MPI_Type_indexed       ( int count, int array_of_blocklengths[],
			     int array_of_displacements[],
			     MPI_Datatype oldtype, MPI_Datatype *newtype);
int MPI_Type_commit ( MPI_Datatype *dat );
int MPI_Type_free   ( MPI_Datatype *dat );
int MPI_Get_address ( void *location, MPI_Aint *address );

int MPI_Win_create ( void *base, MPI_Aint size, int disp_unit,
		     MPI_Info info, MPI_Comm comm, MPI_Win *win);
int MPI_Win_free   ( MPI_Win *win );
int MPI_Win_fence  ( int assert, MPI_Win win );
int MPI_Win_start  ( MPI_Group group, int assert, MPI_Win win );
int MPI_Win_post   ( MPI_Group group, int assert, MPI_Win win );
int MPI_Win_complete( MPI_Win win );
int MPI_Win_wait   ( MPI_Win win );
int MPI_Put ( void *origin_addr, int origin_count, MPI_Datatype origin_datatype,
	      int target_rank, MPI_Aint target_disp, int target_count,
	      MPI_Datatype target_datatype, MPI_Win win);
int MPI_Get ( void *origin_addr, int origin_count,
	      MPI_Datatype origin_datatype, int target_rank,
	      MPI_Aint target_disp, int target_count,
	      MPI_Datatype target_datatype, MPI_Win win );

int MPI_Abort     ( MPI_Comm comm, int errcode );
int MPI_Barrier   ( MPI_Comm comm );
int MPI_Allreduce ( void *inbuf, void *outbuf, int cnt, MPI_Datatype dat, 
		    MPI_Op op, MPI_Comm comm);
int MPI_Reduce ( void *inbuf, void *outbuf, int cnt, MPI_Datatype dat, 
		 MPI_Op op, int root, MPI_Comm comm);
int MPI_Bcast  ( void *buf, int cnt, MPI_Datatype dat, int root, 
		 MPI_Comm comm);

int MPI_Send  ( void *buf, int cnt, MPI_Datatype dat, int dest, int tag, MPI_Comm comm);
int MPI_Isend ( void *buf, int cnt, MPI_Datatype dat, int dest, int tag, MPI_Comm comm, 
		MPI_Request *req);
int MPI_Irecv ( void *buf, int cnt, MPI_Datatype dat, int src, int tag, MPI_Comm comm, 
		MPI_Request *req);
int MPI_Recv  ( void *buf, int cnt, MPI_Datatype dat, int src, int tag, MPI_Comm comm, 
		MPI_Status *stat);
int MPI_Sendrecv ( void *sendbuf, int sendcount, MPI_Datatype sendtype,
		   int dest, int sendtag, void *recvbuf, int recvcount,
		   MPI_Datatype recvtype, int source, int recvtag,
		   MPI_Comm comm,  MPI_Status *status);
int MPI_Waitall ( int count, MPI_Request *array_of_requests,
		  MPI_Status *array_of_statuses);
int MPI_Wait    ( MPI_Request *request, MPI_Status *status);

int MPI_Unpack  ( void *inbuf, int insize, int *position,
		  void *outbuf, int outcount, MPI_Datatype datatype,
                 MPI_Comm comm);
int MPI_Pack    ( void *inbuf, int incount, MPI_Datatype datatype,
		  void *outbuf, int outsize, int *position, MPI_Comm comm);
int MPI_Pack_size( int incount, MPI_Datatype datatype, MPI_Comm comm,
		   int *size);
double MPI_Wtime(void);

#endif /* __MPI_H__ */
