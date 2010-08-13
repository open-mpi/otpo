/*
 * file:  distmat.h
 *
 * desc:  Header file for distributed sparse matrix and vector types
 *
 */

#include <setjmp.h>
#include "mpi.h"

#ifndef INC_DISTMAT_H
#define INC_DISTMAT_H


#define D_ONE   ((double)1.)
#define D_ZERO  ((double)0.)


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



/* ----------
 * square non-symmetric distributed sparse matrix
 * (compressed row format)
 *
 * representation invariants:
 *
 *   dimensions of sparse matrix represented: N x N
 *   length(val) == length(col_ind) == nnz
 *   length(row_ptr) == (row_f - row_i) + 1
 *
 *   for all i in [0,nnz-1]
 *     column index of element val[i] is col_ind[i]
 *
 *   for all i in [row_i, row_f]
 *     the elements of the matrix in row i are
 *
 *       val[ start .. stop ]
 *
 *      where
 *
 *        start == row_ptr[i - row_i]
 *        stop  == row_ptr[i - row_i + 1]-1
 *
 *   after call to dspmat_vecmult_sync:
 *
 *     for all i in [0..g_numprocs-1]
 *
 *        xfer_len[i] == # of doubles to transfer from local processor
 *                       to processor i
 *        xfer_buf[i][0..xfer_len[i]-1] == increasing sequence of
 *                                         indices of values to send
 *                                         from local to processor i
 *     end_for
 */
typedef struct tag_dsp_matrix_t {

        int N;            /* matrix dimension (N-by-N) */

  int row_i, row_f; /* row range owned */

        int nnz;          /* number of locally stored non-zeros */
        double* val;      /* elements of the matrix */

        int* row_ptr;     /* beginning/end of rows */
        int* col_ind;     /* column indices */
  int* col;         /* ordered column indices of non zero elements */

  /* for sparse matrix-vector multiply */
  int*  xfer_len;   /* length of buffers to transfer to other procs */
  int** xfer_buf;   /* processor index data */
  int*  mask_send;  /* send mask */
  int*  mask_recv;  /* recv mask */

} dsp_matrix_t;


dsp_matrix_t* dspmat_newHB( const char* filename );
  /*
   *   pre :  `filename' is a sparse matrix stored in the
   *          Harwell-Boeing format.
   *
   *  post :  Returns a pointer to a new, distributed matrix
   *          containing the data stored in `filename'.
   *
   *          Automatically calls dspmat_vecmult_sync on the
   *          returned matrix.
   */


void dspmat_destroy( dsp_matrix_t* A );
  /*
   *  post :  Frees all memory associated with A.
   */


void dspmat_vecmult_sync( dsp_matrix_t* A );
  /*
   *  post :  Synchronize matrix data for A on all processors for
   *          matrix-vector multiply operations.
   */


/* ----------
 * distributed vector
 *
 * invariants:
 *
 *   elements [row_i .. row_f] of the logical vector
 *   are stored in
 *
 *     val[ 0 .. (row_f - row_i) ]
 */
typedef struct tag_d_vector_t {
  int N;              /* length of vector */
  int row_i, row_f;   /* rows owned by local process */

  double* val;        /* vector elements */
} d_vector_t;


d_vector_t* dvec_create( int N );
  /*
   *   pre :  N > 0
   *
   *  post :  Returns a pointer to a new distributed vector
   *          of length `N'
   */


d_vector_t* dvec_create_chkpt( int N );


void dvec_destroy( d_vector_t* v );
  /*
   *  post :  Frees all memory associated with `v'
   */



void dvec_fillzero( d_vector_t* v );
  /*
   *  post :  Fills all the elements of `v' with zero
   */


void dvec_copy_to( d_vector_t* dst, const d_vector_t* src );
  /*
   *   pre :  length(x) == length(y) AND same rows
   *
   *  post :  Performs the assignment  dst = src.
   */

int dvec_compare( d_vector_t* dst, const d_vector_t* src );


void dvec_axpy( d_vector_t* y, double a, const d_vector_t* x );
  /*
   *   pre :  length(x) == length(y) AND same rows
   *            if y != NULL
   *
   *  post :  Perform:  y = y + a*x
   */

void dvec_aypx( d_vector_t* y, double a, const d_vector_t* x );
  /*
   *   pre :  length(x) == length(y) AND same rows
   *            if y != NULL
   *
   *  post :  Perform:  y = x + a*y
   */


void dvec_scale( d_vector_t* y, double a );
  /*
   *  post :  Performs:  y *= a
   */


void dvec_accum( d_vector_t* y, const d_vector_t* x );
  /*
   *  post :  Performs y += x
   */


double dvec_all_dotprod( const d_vector_t* x, const d_vector_t* y );
  /*
   *   pre :  length(x) == length(y) AND same rows
   *
   *  post :  Performs the dot product x'*y.  Result is available
   *          on all processors.
   */

int dvec_mult_dspmat( d_vector_t* y,
                       const dsp_matrix_t* A, const d_vector_t* x );
  /*
   *   pre :  A, x, y have appropriate dimensions
   *          dspmat_vecmult_sync(A) has been called
   *
   *  post :  Perform sparse matrix-vector multiply:  y = A*x
   */


#endif

/* eof */
