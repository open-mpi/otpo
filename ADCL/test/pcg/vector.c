/*
 * file :  vector.c
 * desc :  Implements routines needed by the parallel conjugate gradient
 *         solver, except the sparse matrix vector multiply
 *
 * by Bob Fisher, Sebastien Toupet, and Rich Vuduc
 * for CS 267 (Applications of Parallel Computers), Spring 1998
 *
 * Notes:
 * -----
 * (0)  implements the following routines:
 *      - dvec_create (n) creates a distributed vector
 *      - dvec_copy_to (x,y) performs y=x for dist vectors
 *      - dvec_all_dotprod(x,y) returns x.y to each processor
 *      - dvec_axpy (x,alpha,y) returns y=alpha*x+y  
 *      - dvec_aypx (x,alpha,y) returns y=x+alpha*y
 *      - dvec_destroy (x)
 *
 * (1)  Program uses MPI for communication
 *
 * (2)  Number of processors and local processor ID stored in
 *      the global variables g_myproc and g_numprocs, respectively.
 */

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

#include "errhand.h"
#include "distmat.h"
#include "mpitimer.h"

#define STATUS_MSG(msg) { \
    if( myproc == 0 ) { \
      fprintf( stderr, "%s", msg ); \
    } \
  }

#ifdef DO_TIMING
extern mpi_timer_t g_time_comm;
extern mpi_timer_t g_time_vec;
#endif


d_vector_t* dvec_create( int N ) {
  /*
   *   pre :  N > 0
   *
   *  post :  Returns a pointer to a new distributed vector
   *          of length `N'
   */
  
  d_vector_t *d;
  int remainder = N % g_numprocs;
  int Nloc;
  
  d = (d_vector_t *)malloc( sizeof(d_vector_t) );
  d->N = N;
  d->row_i = (g_myproc < remainder) ? \
    (N / g_numprocs +1) * g_myproc :
    N / g_numprocs * g_myproc + remainder ;
  d->row_f = (g_myproc < remainder) ? \
    (N / g_numprocs * (g_myproc+1) + g_myproc) : \
    (N / g_numprocs * (g_myproc+1) + remainder -1);
  Nloc = d->row_f - d->row_i + 1;
  d->val = (double*) malloc (Nloc*sizeof(*d->val)); 
  return(d);
}


d_vector_t* dvec_create_chkpt( int N ) {
  /*
   *   pre :  N > 0
   *
   *  post :  Returns a pointer to a new distributed vector
   *          of length `N'
   */
  
  d_vector_t *d;
  int remainder = N % (numprocs-1);
  int Nloc,i;
  
  d = (d_vector_t *)malloc( sizeof(d_vector_t) );
  
  if ( myproc != check_proc ) {
    d->N = N;
    d->row_i = (g_myproc < remainder) ? \
      (N / g_numprocs +1) * g_myproc :
      N / g_numprocs * g_myproc + remainder ;
    d->row_f = (g_myproc < remainder) ? \
      (N / g_numprocs * (g_myproc+1) + g_myproc) : \
      (N / g_numprocs * (g_myproc+1) + remainder -1);
  } else {
    d->N = 0;
    d->row_i = 0;
    d->row_f = 0;
  }   
  Nloc = (N - 1)/(numprocs-1) + 1;
  d->val = (double*) malloc (Nloc*sizeof(*d->val));
  for (i=0;i<Nloc;i++) d->val[i] = 0.0;    /* initialize as 0 for all procs */
  return(d);
}



void dvec_destroy( d_vector_t* v ) {
  /*
   *  post :  Frees all memory associated with `v'
   */
  free(v->val);
/* rich fix: */
free( v );
}

void dvec_fillzero( d_vector_t* v ) {
  /*
   *  post :  Fills all the elements of `v' with zero
   */
int i;
int Nloc = v->row_f - v->row_i +1;

for (i=0;i<Nloc;i++) v->val[i] = 0. ;
}

void dvec_copy_to( d_vector_t* dst, const d_vector_t* src ) {
  /*
   *  pre :  length(x) == length(y) AND same rows
   *
   *  post :  Performs the assignment  dst = src.
   */
int i;
int Nloc = src->row_f - src->row_i +1;

assert( src->N == dst->N);
assert( src->row_i == dst->row_i);
assert( src->row_f == dst->row_f);

for (i=0;i<Nloc;i++) dst->val[i] = src->val[i] ;
}

void dvec_axpy( d_vector_t* y, double a, const d_vector_t* x ) {
  /*
   *   pre :  length(x) == length(y) AND same rows
   *            if y != NULL
   *
   *  post :  Perform:  y = y + a*x
   */
int i;
int Nloc = x->row_f - x->row_i +1;

assert( y!= NULL);
assert( x->N == y->N);
assert( x->row_i == y->row_i);
assert( x->row_f == y->row_f);

for (i=0;i<Nloc;i++) y->val[i] = a * x->val[i] + y->val[i] ;
}

void dvec_aypx( d_vector_t* y, double a, const d_vector_t* x ) {
  /*
   *   pre :  length(x) == length(y) AND same rows
   *            if x != NULL
   *
   *  post :  Perform:  y = x + a*y
   */
int i;
int Nloc = x->row_f - x->row_i +1;

assert( y!= NULL);
assert( x->N == y->N);
assert( x->row_i == y->row_i);
assert( x->row_f == y->row_f);

//for (i=0;i<Nloc-1;i++) y->val[i] = x->val[i] + a * y->val[i] ;
/* rich fix: */
for (i=0;i<Nloc;i++) y->val[i] = x->val[i] + a * y->val[i] ;
}

double dvec_all_dotprod( const d_vector_t* x, const d_vector_t* y ) {
  /*
   *   pre :  length(x) == length(y) AND same rows
   *
   *  post :  Performs the dot product x'*y.  Result is available
   *          on all processors.
   */
int i, Nloc = x->row_f - x->row_i +1;
double lprod = 0.0; 
double gprod = 0.0;
 int rc; 

assert( x->N == y->N);
assert( x->row_i == y->row_i);
assert( x->row_f == y->row_f);

/*local dot product*/
for (i=0;i<Nloc;i++) lprod += x->val[i] * y->val[i];

/*sum of local products*/
MPI_TIMER_SWITCH( g_time_vec, g_time_comm );
rc = MPI_Allreduce( &lprod, &gprod, 1, MPI_DOUBLE, MPI_SUM, comm_work);
 if ( rc != MPI_SUCCESS ) {
   state = 1;
   longjmp (env, state);
 }
   
MPI_TIMER_SWITCH( g_time_comm, g_time_vec );
return (gprod);
}


int dvec_compare( d_vector_t* dst, const d_vector_t* src ) 
{
  /*
   *  pre :  length(x) == length(y) AND same rows
   *
   *  post :  Performs the comparasion  dst == src.
   */
  int i, num=0, mynum=0;
  int Nloc = src->row_f - src->row_i +1;
  int rc;
  
  assert( src->N == dst->N);
  assert( src->row_i == dst->row_i);
  assert( src->row_f == dst->row_f);
  
  for (i=0;i<Nloc;i++) 
    if (dst->val[i] != src->val[i])
      mynum++;
  rc = MPI_Allreduce( &mynum, &num, 1, MPI_INT, MPI_SUM, comm_work);
  if ( rc != MPI_SUCCESS ) {
    state = 1;
    longjmp (env, state);
  }

  return num;
}
