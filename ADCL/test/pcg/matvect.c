/*
 * file :  dummmatvect.c
 * desc :  Dumb sparse matrix vector multiply
 *
 * by Bob Fisher, Sebastien Toupet, and Rich Vuduc
 * for CS 267 (Applications of Parallel Computers), Spring 1998
 *
 * Notes:
 * -----
 * (0)  See note (1)  
 *
 * (1)  Program uses MPI for communication
 *
 * (2)  Number of processors and local processor ID stored in
 *      the global variables g_myproc and g_numprocs, respectively.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

#include "errhand.h"
#include "distmat.h"
#include "mpitimer.h"

#define MAX_PROC   100
#define max(a,b)   (((a)>(b)) ? (a) : (b)) 

#ifdef DO_TIMING
extern mpi_timer_t g_time_comp;
extern mpi_timer_t g_time_comm;
extern mpi_timer_t g_time_mem;
#endif


extern MPI_Group  orig_group;
extern MPI_Group  group_work; 
extern MPI_Comm   comm_work; 
extern int        g_myproc;      /* local processor number in comm_work */
extern int        g_numprocs;    /* total number of processors in comm_work */
extern int        check_proc;
extern int        myproc;        /* local processor number in MPI_COMM_WORLD */
extern int        numprocs;      /* total number of processors in MPI_COMM_WORLD */
extern int        state;         /* if state =1 error, need to recover. */
extern jmp_buf    env;
extern int        rs;            /* check error and  whether a process is restarted or not */




int dvec_mult_dspmat( d_vector_t* y,
		      const dsp_matrix_t* A, const d_vector_t* x ) {
  
  MPI_Status status;
  MPI_Request request;
  
  int i,j,l,lengthmyx[MAX_PROC],N;
  int row_i[MAX_PROC], row_f[MAX_PROC];
  double* myx;
  
  /* message passing local vars */
  int p_send;  /* id of next process to send our local vector data */
  int p_recv;  /* id of next process from whom to receive vector data */
  
  int max_buflen;
  double* xfer_buf;    /* transfer buffer */
  
  int remainder;
  
#ifndef UNOPTIMIZED
  register int* p_Ac;
  register double* p_Av;
  register double* p_yv;
  register int* p_Arp;
  register int Arp0;
  register int Ari, Arf_minus_Ari;
#endif
  
  int rc;
  
  
  
  assert(y->N==x->N);
  assert(y->row_f==x->row_f);
  assert(y->row_i==x->row_i);
  assert( A->row_i == x->row_i );
  assert( A->row_f == x->row_f );
  
  
#ifndef UNOPTIMIZED
  p_Ac = A->col_ind;
  p_Av = A->val;
  p_yv = y->val;
  p_Arp = A->row_ptr;
  Arp0 = p_Arp[0];
  Ari = A->row_i;
  Arf_minus_Ari = A->row_f - Ari;
#endif
  
  
  /* send buffer */
  p_send = g_myproc;
  row_i [g_myproc] = x->row_i;
  row_f [g_myproc] = x->row_f;
  lengthmyx [g_myproc] = row_f [g_myproc] - row_i [g_myproc] + 1;
  
  myx = x->val;
  
  /* stuff */
  N = x->N;
  remainder = N % g_numprocs;
  
  /* receive buffer */
  max_buflen = N / g_numprocs + 1;
  
  MPI_TIMER_SWITCH( g_time_comp, g_time_mem );
  xfer_buf = (double *)malloc( sizeof(double)*max_buflen );
  assert( xfer_buf != NULL );
  MPI_TIMER_SWITCH( g_time_mem, g_time_comp );
  
  memcpy( xfer_buf, x->val, (x->row_f - x->row_i + 1)*sizeof(double) );
  p_recv = g_myproc;
  
  /* zero out y */
  dvec_fillzero( y );
  
  for (i=0;i<g_numprocs;i++) {
    
#define MSG_TAGBASE 0x0100
    /* Send the elements of x needed by processor myproc+1 that i own */
    p_send = (p_send + 1) % g_numprocs;
    
    if( A->mask_send[p_send] ) {
      
      MPI_TIMER_SWITCH( g_time_comp, g_time_comm );
      
      /* original code */
      /*
	MPI_Isend( myx, lengthmyx[g_myproc], MPI_DOUBLE,
	p_send, MSG_TAGBASE + g_myproc,
	MPI_COMM_WORLD, &request );
      */
      
      
      /* code changed by zizhong */
      rc = MPI_Send( myx, lengthmyx[g_myproc], MPI_DOUBLE,
		     p_send, MSG_TAGBASE + g_myproc,
		     comm_work);
      if ( rc != MPI_SUCCESS ) {
	fprintf( stderr, "11111111************************11111111111\n");
	state = 1; 
	longjmp (env, state);
      }
      
      
      MPI_TIMER_SWITCH( g_time_comm, g_time_comp );
    }
    
    
    /* Chunk of local computation */
    
    if( A->mask_recv[p_recv] ) {
      /*
	#ifdef UNOPTIMIZED
	for( l = row_i[g_myproc] - A->row_i;
	l <= row_f[g_myproc] - A->row_i;
	l++) {
	int j_min = A->row_ptr[l] - A->row_ptr[0];
	int j_max = A->row_ptr[l+1] - 1 - A->row_ptr[0];
	
	for( j = j_min; j <= j_max; j++ ) {
	#else
      */
      for( l = 0; l <= Arf_minus_Ari; l++) {
	int j_min = p_Arp[l] - Arp0;
	int j_max = p_Arp[l+1] - 1 - Arp0;
	
	for( j = j_min; j <= j_max; j++ ) {
	  //#endif
	  /*
	    #ifdef UNOPTIMIZED
	    if( (A->col_ind[j] >= row_i[p_recv])
	    && (A->col_ind[j] <= row_f[p_recv]) ) {
	    
	    int ix = A->col_ind[j] - row_i[ p_recv ];
	    
	    y->val[l] += A->val[j] * xfer_buf[ ix ];
	    
	    } 
	    #else
	  */
	  register int Acj = p_Ac[j];
	  register int ri = row_i[p_recv];
	  register int rf = row_f[p_recv];
	  if( (Acj >= ri) && (Acj <= rf) ) {
	    
	    register int ix = Acj - ri;
	    
	    p_yv[l] += p_Av[j] * xfer_buf[ ix ];
	    
	  } /* if */
	  
	  //#endif
	} /* j */
      } /* l */
    } /* if */
    
    /* Receive the elements of x owned by processor myproc+1 that i need */
    if( p_recv == 0 )
      p_recv = g_numprocs - 1;
    else p_recv--;
    
    row_i[ p_recv ] = (p_recv < remainder) ? \
      (N / g_numprocs +1) * p_recv :
      N / g_numprocs * p_recv + remainder ;
    row_f[ p_recv ] = (p_recv < remainder) ? \
      (N / g_numprocs * (p_recv+1) + p_recv) : \
      (N / g_numprocs * (p_recv+1) + remainder -1);
    lengthmyx[ p_recv ] = row_f[ p_recv ] - row_i[ p_recv ] + 1;
    
    
    if( A->mask_recv[p_recv] ) {
      
      MPI_TIMER_SWITCH( g_time_comp, g_time_comm );
      rc = MPI_Recv( xfer_buf, lengthmyx[ p_recv ], MPI_DOUBLE,
		     p_recv, MSG_TAGBASE + p_recv,
		     comm_work, &status );
      if ( rc != MPI_SUCCESS ) {
	fprintf( stderr, "22222222222************************\n");
	state = 1; 
	longjmp (env, state);
      } 
      
      MPI_TIMER_SWITCH( g_time_comm, g_time_comp );
    }
    
  }
  
  MPI_TIMER_SWITCH( g_time_comp, g_time_mem );
  free( xfer_buf );
  MPI_TIMER_SWITCH( g_time_mem, g_time_comp );
  
  return 0;
  
}


/* ------ dspmat_vecmult_sync ------- */

static int
int_compare( const void* p_a, const void* p_b )
  /*
   *   pre :  (*p_a) and (*p_b) are integers
   *  post :  returns -1  <==>  *p_a < *p_b
   *                   0  <==>  *p_a == *p_b
   *                   1  <==>  *p_a > *p_b
   */
{
  int a = *((int *)p_a);
  int b = *((int *)p_b);

  return (a < b) ? (-1) : ((a == b) ? 0 : 1);
}

static void
array_to_unique( const int* A, int n, int** p_B, int* p_m )
  /*
   *   pre :  A is an array of n elements
   *
   *  post :  Returns an array B (== *p_B) of length m (== *p_m) such
   *          that B is an ordered sequence of unique elements of A.
   */
{
  int* sorted_A;
  int* B;
  int  m;
  int  i;

  if( n == 0 ) {
    *p_B = NULL;
    *p_m = 0;
    return;
  }

  /* n >= 1 */
  sorted_A = (int *)malloc( sizeof(int)*n );
  B = (int *)malloc( sizeof(int)*n );
  assert( sorted_A != NULL );
  memcpy( sorted_A, A, sizeof(int)*n );
  qsort( (void *)sorted_A, n, sizeof(int), int_compare );

  B[0] = sorted_A[0];
  m = 1;
  for( i = 1; i < n; i++ ) {
    /* loop invariant:
     *   B[0..m-1] is array of the unique elements of A[0..i-1]
     */
    if( sorted_A[i] == B[m-1] )
      continue;
    B[m] = sorted_A[i];
    m++;
  }

  *p_B = (int *)malloc( sizeof(int)*m );
  assert( *p_B != NULL );
  memcpy( *p_B, B, sizeof(int)*m );
  *p_m = m;
}

void
dspmat_vecmult_sync( dsp_matrix_t* A )
{
  int* disp;
  int* rlen;
  int  i;
  
  int  rem_rows;
  int  min_rows_per_proc;
  int rc;
  
  assert( A != NULL );

  A->xfer_len = (int *)malloc( sizeof(int)*g_numprocs );
  A->xfer_buf = (int **)malloc( sizeof(int *)*g_numprocs );
  
  assert( A->xfer_len != NULL );
  assert( A->xfer_buf != NULL );
  
  for( i = 0; i < g_numprocs; i++ ) {
    A->xfer_len[i] = 0;
    A->xfer_buf[i] = NULL;
  }
  
  /* build list of elements g_myproc needs */
  array_to_unique( A->col_ind, A->nnz,
                   &(A->xfer_buf[g_myproc]), &(A->xfer_len[g_myproc]) );
  
  disp = (int *)malloc( sizeof(int)*g_numprocs );
  rlen = (int *)malloc( sizeof(int)*g_numprocs );
  assert( disp != NULL && rlen != NULL );
  
  /* send/recieve transfer lengths */
  for( i = 0; i < g_numprocs; i++ ) {
    disp[i] = i;
    rlen[i] = 1;
  }
  rc = MPI_Allgatherv( &(A->xfer_len[g_myproc]), 1, MPI_INT,
                  A->xfer_len, rlen, disp, MPI_INT,
                  comm_work );
  if ( rc != MPI_SUCCESS ) {
    state = 1; 
    longjmp (env, state);
  } 
  


  /* send/recieve transfer indices */
  for( i = 0; i < g_numprocs; i++ ) {
    if( i != g_myproc )
      A->xfer_buf[i] = (int *)malloc( sizeof(int)*A->xfer_len[i] );
    
    rc = MPI_Bcast( A->xfer_buf[i], A->xfer_len[i], MPI_INT, i, comm_work );
    if ( rc != MPI_SUCCESS ) {
      state = 1; 
      longjmp (env, state);
    } 
  }

  free( disp );
  free( rlen );
  
  
  /* initialize send/receive masks */
  A->mask_send = (int *)malloc( sizeof(int)*g_numprocs );
  A->mask_recv = (int *)malloc( sizeof(int)*g_numprocs );
  assert( A->mask_send != NULL && A->mask_recv != NULL );
  
#ifdef NO_MASK
  
  for( i = 0; i < g_numprocs; i++ ) {
    A->mask_send[i] = 1;
    A->mask_recv[i] = 1;
  }
  
#else
  
  min_rows_per_proc = A->N / g_numprocs;
  rem_rows = A->N % g_numprocs;
  for( i = 0; i < g_numprocs; i++ ) {
    int r_i, r_f;
    int ind;
    int k;

    /* send mask */
    A->mask_send[i] = 0;
    for( k = 0; k < A->xfer_len[i]; k++ ) {
      ind = A->xfer_buf[i][k];
      if( ind > A->row_f )
        break;
      if( ind >= A->row_i ) {
        A->mask_send[i] = 1;
        break;
      }
    }

    /* receive mask */
    if( i < rem_rows ) {
      r_i = i * (min_rows_per_proc + 1);
      r_f = r_i + min_rows_per_proc;
    } else {
      r_i = i * min_rows_per_proc + rem_rows;
      r_f = r_i + min_rows_per_proc - 1;
    }

    A->mask_recv[i] = 0;
    for( k = 0; k < A->xfer_len[g_myproc]; k++ ) {
      int ind = A->xfer_buf[g_myproc][k];
      if( ind > r_f )
        break;
      if( ind >= r_i ) {
        A->mask_recv[i] = 1;
        break;
      }
    }

  } /* i */

#endif

#ifdef DUMP_MASK
printf( "send mask: " );
for( i = 0; i < g_numprocs; i++ ) {
  printf( " %d", A->mask_send[i] );
}
printf( "\n" );
printf( "recv mask: " );
for( i = 0; i < g_numprocs; i++ ) {
  printf( " %d", A->mask_recv[i] );
}
printf( "\n" );
#endif
}

/* eof */
