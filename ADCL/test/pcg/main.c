/*****************************************************************************
 
                         FTCG

                                                                         
  This is a fault tolerant parallel conjugate gradient solver based on FT_MPI.
  
  The solver read data ( matrix ) from a file with the Harwell-Boeing
  format(http://math.nist.gov/MatrixMarket/). The data is distributed  on
  the processors by row. Local data is organized as Compressed Row
  Storage format.
  
  The solver is fault tolerant in the following sense: Run the solver on N
  processors. You can kill any process, and the killed process
  will be restarted again.  After restarted, you can kill any process
  again. No mater how many times you kill, it will be restarted and
  continue to do computation untill it finishes the computation.
  
  The solver can tolerate only one fault at a time. The solver can also
  tolerate the fault happened during checkpoint. 
  
  Part of the code is borrowed from the CG code by Bob Fisher, Sebastien Toupet, 
  and Rich Vuduc at UCB. 

  
  Zizhong chen

  April 3, 2003


******************************************************************************/



#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi.h"
/* #include "ft-mpi-modes.h" */

#include "errhand.h"
#include "distmat.h"
#include "mpitimer.h"

#define STATUS_MSG(msg) { \
    if( myproc == 0 ) { \
      fprintf( stderr, "%s", msg ); \
    } \
  }





/* ------------- global variables ------------- */

MPI_Group  orig_group;
MPI_Group  group_work; 
MPI_Comm   comm_work; 
int        g_myproc;      /* local processor number in comm_work */
int        g_numprocs;    /* total number of processors in comm_work */
int        check_proc;
int        restart_proc;
int        myproc;        /* local processor number in MPI_COMM_WORLD */
int        numprocs;      /* total number of processors in MPI_COMM_WORLD */
int        state=0;       /* if state =1 error, need to recover. */
jmp_buf    env;
int        rs=0;            /* check error and  whether a process is restarted or not */
int        A_dim;

d_vector_t* r_chkpt;      
d_vector_t* p_chkpt;     
d_vector_t* x_chkpt;       
double      norm_r_2_chkpt=0.0; 
double      stop_chkpt=0.0; 
int         num_iters_chkpt=0;    




#ifdef DO_TIMING
mpi_timer_t g_time_comp;  /* computation time */
mpi_timer_t g_time_comm;  /* communication time */
mpi_timer_t g_time_mem;   /* memory alloc/dealloc time */
mpi_timer_t g_time_vec;   /* vector-vector operations */
#endif







/* --------------- prototypes -------------- */

int d_conjgrad( d_vector_t* xx, dsp_matrix_t* B, const d_vector_t* b,
                double errtol, char *filename );
/*
 *   pre :  x, A, b appropriately dimensioned.
 *          errtol > 0 is the absolute error tolerance.
 *
 *  post :  Performs x = inverse(A) * b
 *          using a parallel conjugate gradient solver.
 *          Returns the number of iterations that occurred
 *          before convergence.
 */


void close_MPI( void );
/*
 *  post :  close down the MPI stuff
 */

void usage( const char* progname );
/*
 *   pre :  progname is the name of this executable.
 *  post :  Prints program usage message to standard error.
 */

void d_init_rhs( d_vector_t* v , const dsp_matrix_t* A);
/*
 *   pre :  initialize the distributed right-hand side vector
 */

void dspmat_destroy( dsp_matrix_t* A );
  /*
   *  post :  Frees all memory associated with A.
   */

void d_dump_vector( const d_vector_t* b, FILE* fp );
/*
 *   pre :  dump the vector b to the file fp.  if fp == NULL,
 *          returns without error
 */

int dev_compare( d_vector_t* x, d_vector_t* y);
void checkpoint(d_vector_t* r, d_vector_t* p, d_vector_t* x, 
		double norm, double stop, int num_iters);
void recover(d_vector_t* r, d_vector_t* p, d_vector_t* x, 
	     double* p_norm, double* p_stop, int* p_num_iters);



/*This is for disk checkpointing */
void dvec_checkpoint(d_vector_t* p_dvec, char* filename);
void int_checkpoint( int* p_int, char* filename);
void double_checkpoint( double* p_double, char* filename);
void dvec_recover(d_vector_t* p_dvec, char* filename);
void int_recover( int* p_int, char* filename);
void double_recover( double* p_double, char* filename);
void checkpoint_disk(d_vector_t* p_dvec, d_vector_t* x_dvec, 
		     d_vector_t* r_dvec, double* p_norm, int* p_iter, int myrank);
void recover_disk(d_vector_t* p_dvec, d_vector_t* x_dvec, 
		  d_vector_t* r_dvec, double* p_norm, int* p_iter, int myrank);



/* ---------------------- main ------------------- */

int main( int argc, char* argv[] )
{
  dsp_matrix_t*   A;       /* the distributed sparse matrix */
  d_vector_t*     b;       /* right-hand side */
  d_vector_t*     x;       /* solution vector */
  
  char*  filename_HBmat;   /* sparse matrix input filename */
  
  const  char* filename_out;  /* solution vector output filename */
  FILE*  fp_out;
  double errtol;  /* error tolerance */
  int i;
  int num_iters;
  int rc;    /* check error and  whether a process is restarted or not */
  double       tstart, tend, mytime, time;

  /* default parameters */
  filename_HBmat = NULL;
  errtol = 1.e-50;
    
    
  /* Initilize the input and out put files by zizhong chen*/
  filename_HBmat = "../../examples/pcg/mfile/nos7.rsa";
  filename_out = NULL;
  fp_out = NULL;
 

  
  /* set up the MPI library */
  rc =  MPI_Init( &argc, &argv );
  rs = 0;
#ifdef FTMPI
  if (rc==MPI_INIT_RESTARTED_NODE) {
    printf("MPI_Init says I am a restart :)\n"); 
    fflush(stdout);
    rs = 1;
    state = 1; /* restarted procs need to attend recover */
  } else {
    printf("MPI_Init [%d]\n", rc); 
    fflush(stdout);
    rs = 0;
  }
#endif  
    
  /* Note that MPI_Comm_size() and MPI_Comm_rank() are collective. 
     So if this proc is restarted, we should have other procs which
     is not  restarted call it somewhere too!! */
  if ( rs == 0 ) {
    MPI_Comm_size( MPI_COMM_WORLD, &numprocs );
    MPI_Comm_rank( MPI_COMM_WORLD, &myproc );
    printf("rank %d size %d\n", myproc, numprocs);
    
    /*  Construct a communicator comm_work  */
    check_proc = numprocs-1;
    MPI_Comm_group(MPI_COMM_WORLD, &orig_group); 
    MPI_Group_excl(orig_group, 1, &check_proc, &group_work);
    MPI_Comm_create(MPI_COMM_WORLD, group_work, &comm_work);
    if ( myproc < numprocs-1) {
      MPI_Comm_size( comm_work, &g_numprocs );
      MPI_Comm_rank( comm_work, &g_myproc );  
      if( myproc < check_proc )
	printf("comm_work: rank %d size %d\n", g_myproc, g_numprocs);
    }
    
    
    if( myproc == 0 ) {
      fprintf( stderr, "\n--- Summary of parameters ---\n" );
      fprintf( stderr, "  Matrix input file: `%s'\n", filename_HBmat );
      fprintf( stderr, "  CG error parameter: %e\n", errtol );
      fprintf( stderr, "  Total number of processors: %d\n", numprocs );
      fprintf( stderr, "  Clock resolution : %f seconds\n",
	       mpitim_resolution() );
      fprintf( stderr, "  Output destination: %s\n",
	       filename_out == NULL
	       ? fp_out == NULL
	       ? "none"
	       : "standard output"
	       : filename_out );
      }    
    
    /* prepare output file: our output is standard output -- zizhong */
    if( filename_out != NULL ) {
      fp_out = fopen( filename_out, "wt" );
      if( fp_out == NULL ) {
	fprintf( stderr, "Error: can't open output file `%s'\n", filename_out );
	MPI_Abort ( MPI_COMM_WORLD, 1 );
	return -2;
      }
    }  
    
  }
  
  /* initialize timers */
#ifdef DO_TIMING
  mpitim_reset( &g_time_comp );
  mpitim_reset( &g_time_comm );
  mpitim_reset( &g_time_mem );
  mpitim_reset( &g_time_vec );
#endif
  
  /***************************************************************/
  /* initialize parameters for d_conjgrad() !!!!!!!!!!!!!!!!!!!!!*/  
  /***************************************************************/
  
  if ( rs ==0 ) {
    if ( myproc < check_proc ) {   
      /* This is for working process group */       
      /* set-up matrices and vectors */
      STATUS_MSG( "--- Initializing matrix ---\n" );
      /* look into dspmat_newHB() very carefully */   
      A = dspmat_newHB( filename_HBmat );      
      
      STATUS_MSG( "--- Creating a right-hand side ---\n" );
      b = dvec_create( A->N );  
      STATUS_MSG( "--- Initializing the right-hand side to some A*x0 ---\n" );
      /* Note that d_init_rhs() is collective. So if a restarted proc call it,
	 we should have other procs which is not restarted call it too!!
	 Here, we do not let restarted proc to call it, we let restarted proc
	 to call recover() to recover it*/  
      
      d_init_rhs( b , A );
      x = dvec_create( A->N ); 
    } else {
      printf("I am %d: I am dadicated to do checkpoint\n", check_proc);
    }
    
    
    /*******************************************************/
    /* Be careful to Bcast A->N to MPI_COMM_WORLD !!!!!!!!!*/
    /*******************************************************/
    if (myproc == 0 )
      A_dim = A->N;

    MPI_Bcast(&A_dim, 1, MPI_INT, 0, MPI_COMM_WORLD); /* initialize A_dim */    
    STATUS_MSG( "--- Solving Ax = b for x ---\n" );
  }
  
  
  /****************************************************/
  /*!!!!!!!!!!!  Solve the equation  !!!!!!!!!!!!!!!!!*/
  /******************************************************/
  
  MPI_START_TIMING( g_time_comp );
  tstart = MPI_Wtime();
  
  num_iters = d_conjgrad( x, A, b, errtol, filename_HBmat );
  
  tend = MPI_Wtime();
  mytime = tend - tstart; 
  MPI_Reduce (&mytime,&time,1,MPI_DOUBLE, MPI_MAX,0, MPI_COMM_WORLD);

  if ( myproc ==0 )
    printf("--------- Total time: %2.6f   ----------\n\n", time);   
  
  MPI_STOP_TIMING( g_time_comp );
  
  /* dump solution vector to stdout */
  /* if (myproc != check_proc ) {
    d_dump_vector( x, fp_out );
    fclose(fp_out);
  } */
  
  
  /* print timing statistics */
#ifdef DO_TIMING
  STATUS_MSG( "--- Timing results (seconds) ---\n" );
  if( myproc == 0 ) {
    fprintf( stdout, "Computation: %f\n",
	     mpitim_elapsed( &g_time_comp ) );
    fprintf( stdout, "Communication: %f\n",
	     mpitim_elapsed( &g_time_comm ) );
    fprintf( stdout, "Memory allocation: %f\n",
	     mpitim_elapsed( &g_time_mem ) );
    fprintf( stdout, "Vector-vector ops: %f\n",
	     mpitim_elapsed( &g_time_vec ) );
    
    fprintf( stdout, "---> Total: %f\n",
	     mpitim_elapsed( &g_time_comp ) +
	     mpitim_elapsed( &g_time_comm ) +
	     mpitim_elapsed( &g_time_mem ) +
	     mpitim_elapsed( &g_time_vec ) );
    
    fprintf( stdout, "\nNumber of iterations: %d\n", num_iters );
  }
#endif
  
  STATUS_MSG( "--- Finishing ---\n" );
  close_MPI();
  return 0;
}





/* ---------------------- conjugate gradient solver ------------------- */
int d_conjgrad( d_vector_t* xx, dsp_matrix_t* B, const d_vector_t* b,
		double errtol, char *filename )
{
  d_vector_t* r;        /* residual */
  d_vector_t* p;        /* search direction */
  d_vector_t* v;        /* next Krylov vector */
  
  d_vector_t*   x; 
  dsp_matrix_t* A=NULL;
  
  d_vector_t* x_disk;
  
  double alpha, gamma;  /* temporary scalars */
  double norm_r_2=0;      /* [norm(r)]^2 */
  double norm_newr_2;   /* [norm(new_r)]^2 */
  double pdotv;         /* p'*v */
  double stop=0;
  double tmp=1.0;
  
  double lprod; 
  int    i, j=0, Nloc;
  int    rc;
  MPI_Comm  comm, newcomm;
  
  int    num_iters=0;

  double       qqtstart, qqtend, qqmytime, qqtime;

  if ( myproc < check_proc ) {
    A = B;
    x = xx;
  }
  
  /* This is for original processes. */
  if (rs ==0 ) {    
    /*allocate memory for checkpoint variables */
    r_chkpt = dvec_create_chkpt( A_dim );
    p_chkpt = dvec_create_chkpt( A_dim );
    x_chkpt = dvec_create_chkpt( A_dim );
    
    if ( myproc < check_proc ) {     /* init working space */
      MPI_TIMER_SWITCH( g_time_comp, g_time_mem );
      r = dvec_create( A->N );
      p = dvec_create( A->N );
      v = dvec_create( A->N );
      x_disk = dvec_create( A->N );
      MPI_TIMER_SWITCH( g_time_mem, g_time_comp );
      
      assert( r != NULL );
      assert( p != NULL );
      assert( v != NULL );      
      
      MPI_TIMER_SWITCH( g_time_comp, g_time_vec );
      dvec_fillzero( x );
      dvec_copy_to( r, b );      /* r = b */
      dvec_copy_to( p, r );      /* p = r */
      MPI_TIMER_SWITCH( g_time_vec, g_time_comp );
      
      MPI_TIMER_SWITCH( g_time_comp, g_time_vec );
      stop = errtol * dvec_all_dotprod( b, b );
      printf("Before kill proc: stop=%3.10f\n\n",stop);
      norm_r_2 = dvec_all_dotprod( r, r );
      MPI_TIMER_SWITCH( g_time_vec, g_time_comp );
    }
    
    /* initialize stop in MPI_COMM_WORLD */
    rc = MPI_Bcast(&stop, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);   
    /* initialize norm_r_2 in MPI_COMM_WORLD */
    rc = MPI_Bcast(&norm_r_2, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  }


  /****************************************************/
  /*       This part is for recovered state           */
  /****************************************************/      

  j = setjmp (env);  
  printf("%d Test: state=%d\n", myproc, state);

  if ( rs ==1 )
    printf(" ******* I am reporting:  I was the restarted process**********\n");

  /* recover work space */
  if (state == 1)  {        
    
    if ( rs == 0 ) {      
      /*survival do comm dup */
      fprintf(stderr,"%d --- Communicator is duplicating... ---\n", myproc); 
#ifdef FTMPI
       comm    = MPI_COMM_WORLD;
       newcomm = FT_MPI_CHECK_RECOVER; 
#endif
      qqtstart = MPI_Wtime();
      MPI_Comm_dup(comm, &newcomm); 

      qqtend = MPI_Wtime();
      qqmytime = qqtend - qqtstart;
      fprintf(stderr,"@@@@@@@@@@ Time for recover MPI_COMM_WORLD: %2.6f   \n\n", qqmytime);
      /*
      MPI_Comm_free(&comm);
      */

      comm=newcomm;
    }

    rc = MPI_Comm_rank (MPI_COMM_WORLD, &myproc);
    rc = MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
    fprintf(stderr,"%d: --- NEW rank %d size %d ---\n", myproc, myproc, numprocs);

    qqtstart = MPI_Wtime();
    check_proc = numprocs-1;
    MPI_Comm_group(MPI_COMM_WORLD, &orig_group); 
    MPI_Group_excl(orig_group, 1, &check_proc, &group_work);
    MPI_Comm_create(MPI_COMM_WORLD, group_work, &comm_work);
    if ( myproc < numprocs-1) {
      MPI_Comm_size( comm_work, &g_numprocs );
      MPI_Comm_rank( comm_work, &g_myproc );
    }
    qqtend = MPI_Wtime();
    qqmytime = qqtend - qqtstart;
    fprintf(stderr,"%d: Time for recover work comm: %2.6f   \n\n", myproc, qqmytime); 
    
    /* The following code reinitilize A and A_dim */
    qqtstart = MPI_Wtime();
    
    if ( myproc < check_proc ) {         
      /********************************************************/
      /* restarted process will also call this routine!!!!!!!!*/
      /* Note that dspmat_newHB() is collective. So after     */
      /* MPI_Comm_dup(), survival procs also need to call it  */
      /********************************************************/
      
      printf( "%d: --- After MPI_Comm_dup: all processes read matrix A ---\n", myproc );
      
      // rc = MPI_Barrier(comm_work);
      if ( rs == 0 ) {
	dspmat_destroy( A );
	A = NULL;
      }

      //A = dspmat_newHB("/.autofs/home4/zchen/ftmpi/pcg/nos7.rsa");
      A = dspmat_newHB(filename);
      
      //rc = MPI_Barrier(comm_work);
      fprintf( stderr, "%d: --- Finished reading matrix A ---\n",myproc); 
    }
    
    /*******************************************************/
    /* Be careful to Bcast A->N to MPI_COMM_WORLD !!!!!!!!!*/
    /* restarted process will also call this routine!!!!!!!*/
    /* Note thatM PI_Bcast() is collective. So after       */
    /* MPI_Comm_dup(), survival procs also need to call it */
    /*******************************************************/
    
    // rc = MPI_Barrier(MPI_COMM_WORLD);
    if (myproc == 0 )
      A_dim = A->N;
    rc = MPI_Bcast(&A_dim, 1, MPI_INT, 0, MPI_COMM_WORLD); /* initialize A_dim */ 
    fprintf( stderr, "******rank %d: A_dim =%d \n", myproc, A_dim);
        
    if ( rs == 1 ) {  /* restarted process allocate memory */
      if ( myproc < check_proc ) {
	r = dvec_create( A_dim );
	p = dvec_create( A_dim );
	x = dvec_create( A_dim );  
	v = dvec_create( A_dim );
      }

      r_chkpt = dvec_create_chkpt( A_dim );
      p_chkpt = dvec_create_chkpt( A_dim );
      x_chkpt = dvec_create_chkpt( A_dim ); 
      fprintf( stderr, "---------- restarted process reporting before recover\n");   
    }
    
    rc = MPI_Barrier(MPI_COMM_WORLD);
    if ( rc != MPI_SUCCESS ) {
      state = 1;
      longjmp (env, state);
    }

    /* The following code recover r, p, x, norm_r_2, stop, num_iters */    
    recover(r, p, x, &norm_r_2, &stop, &num_iters); 
    
    qqtend = MPI_Wtime();
    qqmytime = qqtend - qqtstart;
    rc = MPI_Reduce (&qqmytime,&qqtime,1,MPI_DOUBLE, MPI_MAX,0, MPI_COMM_WORLD);
    if (myproc==0)
      fprintf(stderr,"Time for recover data: %f   \n\n", qqtime); 
    fprintf( stderr, "rank %d: The recovered norm_r_2=%f,  num_iters=%d\n", 
	     myproc, norm_r_2, num_iters);
    
    /* After restarted proc catch up, change rs to normal status */
    rs = 0;
    
    /* After recover, change state to work status and continue to work. */
    state = 0;
    
    fprintf( stderr, "%d: I finished recovery, continuing computation\n", myproc); 

  }

  /*******************************************************/
  /*   The following code do iterative solving of Ax=b   */  
  /*******************************************************/
  
  do {
    if ( (num_iters % 200) == 0 ) {
      checkpoint(r, p, x, norm_r_2, stop, num_iters);
        if (myproc == 0 )  
	printf("Number of iterations: %d norm_r_2: %lE\n", 
	       num_iters, norm_r_2);  
    }

    if ( myproc < check_proc ) {       
      /*  v = A * p  */
      
      dvec_mult_dspmat( v, A, p );
      
      MPI_TIMER_SWITCH( g_time_comp, g_time_vec );
      /* alpha = r'*r / (p'*v) */
      //pdotv = dvec_all_dotprod(p, v);
      lprod = 0.0;
      Nloc = x->row_f - x->row_i +1; 
      for (i=0;i<Nloc;i++) lprod += p->val[i] * v->val[i];
      MPI_TIMER_SWITCH( g_time_vec, g_time_comm );

      rc = MPI_Allreduce( &lprod, &pdotv, 1, MPI_DOUBLE, MPI_SUM, comm_work);   
      if ( rc != MPI_SUCCESS ) {
	state = 1; 
	longjmp (env, state);
      }
      MPI_TIMER_SWITCH( g_time_comm, g_time_vec );
      alpha = norm_r_2 / pdotv;
      
      /* x = x + alpha*p */
      dvec_axpy( x, alpha, p );
      
      /* r = r - alpha*v */
      dvec_axpy( r, -alpha, v );

      /* g = (r'*r) / (oldr'*oldr) */
      //norm_newr_2 = dvec_all_dotprod( r, r );
      lprod = 0.0;
      Nloc = x->row_f - x->row_i +1; 
      for (i=0;i<Nloc;i++) lprod += r->val[i] * r->val[i];
      MPI_TIMER_SWITCH( g_time_vec, g_time_comm );
      
      rc = MPI_Allreduce( &lprod, &norm_newr_2, 1, MPI_DOUBLE, 
			  MPI_SUM, comm_work);
      if ( rc != MPI_SUCCESS ) {
	state = 1; 
	longjmp (env, state);
      }
      MPI_TIMER_SWITCH( g_time_comm, g_time_vec );
      gamma = norm_newr_2 / norm_r_2;
      
      /* p = r + g*p */
      dvec_aypx( p, gamma, r );
      
      MPI_TIMER_SWITCH( g_time_vec, g_time_comp );
      
      norm_r_2 = norm_newr_2;
    }
    
    
    num_iters++;

    /* Broadcast norm_r_2 in MPI_COMM_WORLD */
    rc = MPI_Bcast(&norm_r_2, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);   
    if ( rc != MPI_SUCCESS ) {
      fprintf( stderr, " rank %d: Bcast problem in iteration \n",myproc);
      state = 1; 
      longjmp (env, state);
    }
    
  } while( norm_r_2 >= stop );  /* End of the iterative solving */

  fprintf( stderr, "Iteration %d: Finish solving\n",num_iters);
  printf("After iteration: norm_r_2 = %lE\n\n",norm_r_2);
  
  MPI_TIMER_SWITCH( g_time_comp, g_time_mem );
  if ( myproc < check_proc ) { 
    dvec_destroy( r );
    dvec_destroy( p );
    dvec_destroy( v );
  }
  MPI_TIMER_SWITCH( g_time_mem, g_time_comp );

  return num_iters;
}






void d_init_rhs( d_vector_t* b , const dsp_matrix_t* A)
{
  int i;
  int n, nloc;
  d_vector_t* p;
  int rc;

  assert( b != NULL );
  
  
  /* initialize all entries to 1. */
  
  n = b->N;
  p = dvec_create(n);
  
  assert(p->row_f==b->row_f);
  assert(p->row_i==b->row_i);
  
  nloc = p->row_f - p->row_i + 1; 
  for( i = 0; i < nloc; i++ ) {
    p->val[i] = D_ONE;
  }
  rc = MPI_Barrier(comm_work);
  if ( rc != MPI_SUCCESS ) {
    state = 1; 
    longjmp (env, state);
  } 
  dvec_mult_dspmat(b,A,p);
  
  //  MPI_Barrier(MPI_COMM_WORLD);
  //  d_dump_vector( b, stdout );
  
  dvec_destroy( p );
}


int dev_compare(d_vector_t* x, d_vector_t* y)
{
  int i,num=0;
  int nloc; 
  
  nloc = x->row_f - x->row_i + 1;
  for( i = 0; i < nloc; i++ )
    if ( x->val[i] != y->val[i] )
      num++;
  return num;
}



void
d_dump_vector( const d_vector_t* b, FILE* fp )
{
  int i;
  int nloc;
  
  assert( b != NULL );
  
  if( fp == NULL )
    return;
  
  nloc = b->row_f - b->row_i + 1;
  
  for( i = 0; i < nloc; i++ )
    fprintf( fp, "%d %f\n", i+b->row_i, b->val[i] );
}


/* ---------------------- supporting routines ------------------- */
void
setup_MPI( int* p_argc, char*** p_argv )
{
  MPI_Init( p_argc, p_argv );
  MPI_Comm_size( MPI_COMM_WORLD, &g_numprocs );
  MPI_Comm_rank( MPI_COMM_WORLD, &g_myproc );
}

void
close_MPI()
{
  MPI_Finalize();
}

void
usage( const char* progname )
{
  fprintf(stderr,
          "\nUsage: %s <matrix_filename> [-e <tol>] [-stdout | -o <outf>]\n",
          progname);
  fprintf(stderr, "where\n\n" );
  fprintf(stderr, "  <matrix_filename> : name of sparse input matrix file\n");
  fprintf(stderr, "                      (data in Harwell-Boeing format)\n");
  fprintf(stderr, "  <tol>             : conjugate gradient stop criterion\n");
  fprintf(stderr, "  <outf>            : output filename\n" );
  fprintf(stderr, "\n");
}

void
quit( int errcode, const char* msg )
{
  fprintf( stderr, "\n" );
  switch( errcode ) {
  case ERR_NONE:
    fprintf( stderr, "-- done! --\n" );
    break;
    
  case ERR_ARGS:
    if( msg != NULL ) {
      fprintf( stderr, "Unknown command-line argument: `%s'\n", msg );
    }
    break;
  }
  fprintf( stderr, "Program ending (%d).\n", errcode );
  exit( errcode );
}

void dspmat_destroy( dsp_matrix_t* A )
{

  if ( A!=NULL ) {
    if ( A->val)       free(A->val);
    if ( A->row_ptr)   free(A->row_ptr);
    if ( A->col_ind)   free(A->col_ind);
    if ( A->col)       free(A->col);
    if ( A->xfer_len)  free(A->xfer_len);
    if ( A->xfer_buf)  free(A->xfer_buf);
    if ( A->mask_send) free(A->mask_send);
    if ( A->mask_recv) free(A->mask_recv);  
    
    free(A);
  }
}


/* eof */











/*****************************************************
						      
This segment is for checkpoint and recover variables

******************************************************/


void checkpoint(d_vector_t* r, d_vector_t* p, d_vector_t* x, double norm, double stop, int num_iters)
{
  int          i, Nloc = 0;
  
  d_vector_t*  r_temp, *rt;
  d_vector_t*  p_temp, *pt;
  d_vector_t*  x_temp, *xt;
  double       norm_temp, stop_temp;
  int          num_iters_temp;
  int          rc;
  int          nit;
  double       nt, st;
  
  
  r_temp = dvec_create_chkpt( A_dim );
  p_temp = dvec_create_chkpt( A_dim );
  x_temp = dvec_create_chkpt( A_dim );

  rt = dvec_create_chkpt( A_dim );
  pt = dvec_create_chkpt( A_dim );
  xt = dvec_create_chkpt( A_dim );

  if( myproc != check_proc ) { /* checkpoint to local memory first */
    Nloc = r->row_f - r->row_i + 1;
    for (i=0;i<Nloc;i++) rt->val[i] = r->val[i];
    for (i=0;i<Nloc;i++) pt->val[i] = p->val[i];
    for (i=0;i<Nloc;i++) xt->val[i] = x->val[i];
    norm_temp = norm;
    stop_temp = stop;
    num_iters_temp = num_iters;
  } else {    /* initialize the check sum as 0 */
    Nloc = ( A_dim - 1)/(numprocs-1) + 1;
    for (i=0;i<Nloc;i++) r_temp->val[i] = 0;
    for (i=0;i<Nloc;i++) p_temp->val[i] = 0;
    for (i=0;i<Nloc;i++) x_temp->val[i] = 0;
    for (i=0;i<Nloc;i++) rt->val[i] = 0;
    for (i=0;i<Nloc;i++) pt->val[i] = 0;
    for (i=0;i<Nloc;i++) xt->val[i] = 0;
    norm_temp = 0;
    stop_temp = 0;
    num_iters_temp = 0;
  }
  
  /* encode local checkpoint to check_proc */
  rc = MPI_Reduce(rt->val, r_temp->val, (A_dim-1)/(numprocs-1)+1, MPI_DOUBLE, MPI_SUM, check_proc, MPI_COMM_WORLD);
  if ( rc != MPI_SUCCESS ) {
    state = 1; 
    longjmp (env, state);
  }
  rc = MPI_Reduce(pt->val, p_temp->val, (A_dim-1)/(numprocs-1)+1, MPI_DOUBLE, MPI_SUM, check_proc, MPI_COMM_WORLD);
  if ( rc != MPI_SUCCESS ) {
    state = 1; 
    longjmp (env, state);
  }
  rc = MPI_Reduce(xt->val, x_temp->val, (A_dim-1)/(numprocs-1)+1, MPI_DOUBLE, MPI_SUM, check_proc, MPI_COMM_WORLD);
  if ( rc != MPI_SUCCESS ) {
    state = 1; 
    longjmp (env, state);
  }
  rc = MPI_Reduce(&(norm_temp), &nt, 1, MPI_DOUBLE, MPI_SUM, check_proc, MPI_COMM_WORLD);
  if ( rc != MPI_SUCCESS ) {
    state = 1; 
    longjmp (env, state);
  }
  if ( myproc == check_proc )
    norm_temp = nt;

  rc = MPI_Reduce(&(stop_temp), &st, 1, MPI_DOUBLE, MPI_SUM, check_proc, MPI_COMM_WORLD);
  if ( rc != MPI_SUCCESS ) {
    state = 1; 
    longjmp (env, state);
  }
  if ( myproc == check_proc )
    stop_temp = st;

  rc = MPI_Reduce(&(num_iters_temp), &nit, 1, MPI_INT, MPI_SUM, check_proc, MPI_COMM_WORLD); 
  if ( rc != MPI_SUCCESS ) {
    state = 1; 
    longjmp (env, state);
  }
  if ( myproc == check_proc )
    num_iters_temp = nit;

  
  /********************************************************/
  /*  free(r_chkpt);
      free(p_chkpt);
      free(x_chkpt); 
  */
  dvec_destroy ( r_chkpt);
  dvec_destroy ( p_chkpt);
  dvec_destroy ( x_chkpt);
  if ( myproc == check_proc ) {
    r_chkpt = r_temp;
    p_chkpt = p_temp;  
    x_chkpt = x_temp; 

    dvec_destroy ( rt);
    dvec_destroy ( pt);
    dvec_destroy ( xt);
  }
  else {
    r_chkpt = rt;
    p_chkpt = pt;  
    x_chkpt = xt; 

    dvec_destroy ( r_temp);
    dvec_destroy ( p_temp);
    dvec_destroy ( x_temp);
  }

  norm_r_2_chkpt  = norm_temp; 
  stop_chkpt      = stop_temp;
  num_iters_chkpt = num_iters_temp;
}



void recover(d_vector_t* r, d_vector_t* p, d_vector_t* x, double* p_norm, double* p_stop, int* p_num_iters)
{
  int i, Nloc;
  MPI_Status status; 
  d_vector_t*  rt;
  d_vector_t*  pt;
  d_vector_t*  xt;
  int it=0;
  double st=0.0;
  double nrt=0.0;

  Nloc = ( A_dim - 1)/(numprocs-1) + 1;
  rt = dvec_create_chkpt( A_dim );
  pt = dvec_create_chkpt( A_dim );
  xt = dvec_create_chkpt( A_dim );
  for (i=0;i<Nloc;i++) rt->val[i] = 0;
  for (i=0;i<Nloc;i++) pt->val[i] = 0;
  for (i=0;i<Nloc;i++) xt->val[i] = 0;
  
  
  /********************************************************/
  /* Be careful to let every proc to know the rank of the */
  /* restarted proc so that they can collectively restore */
  /* the data on that rank.                               */                   
  /********************************************************/ 
  if ( ( rs == 1 ) && ( myproc == 0 ) ) { /* if rank 0 is restarted */ 
    restart_proc = 0;
  } else {
    if ( rs == 1 )
      MPI_Send( &myproc, 1, MPI_INT, 0, 9999, MPI_COMM_WORLD);/*restarted proc send his rank to rank 0*/
    if ( myproc == 0 ) 
      MPI_Recv( &restart_proc, 1, MPI_INT, MPI_ANY_SOURCE, 9999, MPI_COMM_WORLD, &status);
  }
  MPI_Bcast(&restart_proc, 1, MPI_INT, 0, MPI_COMM_WORLD); /*Bcast the rank of the restarted proc to MPI_COMM_WORLD*/  
  fprintf( stderr, "%d: restart_proc =%d\n", myproc, restart_proc); 
  
  
  /* do recover */
  /**********************************************************************/
  /**********************************************************************/
  /**********************************************************************/
  if ( restart_proc == check_proc ) {
    /* re-encode data to check_proc */
    if ( myproc == check_proc ) {
      MPI_Reduce(rt->val, r_chkpt->val, (A_dim-1)/(numprocs-1)+1, MPI_DOUBLE, MPI_SUM, check_proc, MPI_COMM_WORLD);
      MPI_Reduce(pt->val, p_chkpt->val, (A_dim-1)/(numprocs-1)+1, MPI_DOUBLE, MPI_SUM, check_proc, MPI_COMM_WORLD);
      MPI_Reduce(xt->val, x_chkpt->val, (A_dim-1)/(numprocs-1)+1, MPI_DOUBLE, MPI_SUM, check_proc, MPI_COMM_WORLD); 
      MPI_Reduce(&nrt, &(norm_r_2_chkpt), 1, MPI_DOUBLE, MPI_SUM, check_proc, MPI_COMM_WORLD);
      MPI_Reduce(&st, &(stop_chkpt), 1, MPI_DOUBLE, MPI_SUM, check_proc, MPI_COMM_WORLD);
      MPI_Reduce(&it, &(num_iters_chkpt), 1, MPI_INT, MPI_SUM, check_proc, MPI_COMM_WORLD);  
    }
    else {
      MPI_Reduce(r_chkpt->val, rt->val, (A_dim-1)/(numprocs-1)+1, MPI_DOUBLE, MPI_SUM, check_proc, MPI_COMM_WORLD);
      MPI_Reduce(p_chkpt->val, pt->val, (A_dim-1)/(numprocs-1)+1, MPI_DOUBLE, MPI_SUM, check_proc, MPI_COMM_WORLD);
      MPI_Reduce(x_chkpt->val, xt->val, (A_dim-1)/(numprocs-1)+1, MPI_DOUBLE, MPI_SUM, check_proc, MPI_COMM_WORLD); 
      MPI_Reduce(&(norm_r_2_chkpt), &nrt, 1, MPI_DOUBLE, MPI_SUM, check_proc, MPI_COMM_WORLD);
      MPI_Reduce(&(stop_chkpt), &st, 1, MPI_DOUBLE, MPI_SUM, check_proc, MPI_COMM_WORLD);
      MPI_Reduce(&(num_iters_chkpt), &it, 1, MPI_INT, MPI_SUM, check_proc, MPI_COMM_WORLD);  
    }
  } 
  /**********************************************************************/
  /**********************************************************************/
  /**********************************************************************/
  else {    
    /* recover data to restarted process */
    /* recover data to local checkpoint on restarted process */
    if( myproc == check_proc ) {  
      /* calculate -sum */
      for (i=0;i<Nloc;i++) r_chkpt->val[i] = - r_chkpt->val[i];
      for (i=0;i<Nloc;i++) p_chkpt->val[i] = - p_chkpt->val[i];
      for (i=0;i<Nloc;i++) x_chkpt->val[i] = - x_chkpt->val[i];
      norm_r_2_chkpt  = - norm_r_2_chkpt;
      stop_chkpt      = - stop_chkpt;
      num_iters_chkpt = - num_iters_chkpt;
    } 

    if( myproc == restart_proc ) {  
      for (i=0;i<Nloc;i++) rt->val[i] = 0;
      for (i=0;i<Nloc;i++) pt->val[i] = 0;
      for (i=0;i<Nloc;i++) xt->val[i] = 0;
      nrt = 0.0;
      st  = 0.0;
      it  = 0;
	
      MPI_Reduce(rt->val, r_chkpt->val, (A_dim-1)/(numprocs-1)+1, MPI_DOUBLE, MPI_SUM, restart_proc, MPI_COMM_WORLD);
      MPI_Reduce(pt->val, p_chkpt->val, (A_dim-1)/(numprocs-1)+1, MPI_DOUBLE, MPI_SUM, restart_proc, MPI_COMM_WORLD);
      MPI_Reduce(xt->val, x_chkpt->val, (A_dim-1)/(numprocs-1)+1, MPI_DOUBLE, MPI_SUM, restart_proc, MPI_COMM_WORLD);
      MPI_Reduce(&nrt, &(norm_r_2_chkpt), 1, MPI_DOUBLE, MPI_SUM, restart_proc, MPI_COMM_WORLD);
      MPI_Reduce(&st, &(stop_chkpt), 1, MPI_DOUBLE, MPI_SUM, restart_proc, MPI_COMM_WORLD);
      MPI_Reduce(&it, &(num_iters_chkpt), 1, MPI_INT, MPI_SUM, restart_proc, MPI_COMM_WORLD);  
      for (i=0;i<Nloc;i++) r_chkpt->val[i] = - r_chkpt->val[i];
      for (i=0;i<Nloc;i++) p_chkpt->val[i] = - p_chkpt->val[i];
      for (i=0;i<Nloc;i++) x_chkpt->val[i] = - x_chkpt->val[i];
      norm_r_2_chkpt  = - norm_r_2_chkpt;
      stop_chkpt      = - stop_chkpt;
      num_iters_chkpt = - num_iters_chkpt;
    }
    else {
      MPI_Reduce(r_chkpt->val, rt->val, (A_dim-1)/(numprocs-1)+1, MPI_DOUBLE, MPI_SUM, restart_proc, MPI_COMM_WORLD);
      MPI_Reduce(p_chkpt->val, pt->val, (A_dim-1)/(numprocs-1)+1, MPI_DOUBLE, MPI_SUM, restart_proc, MPI_COMM_WORLD);
      MPI_Reduce(x_chkpt->val, xt->val, (A_dim-1)/(numprocs-1)+1, MPI_DOUBLE, MPI_SUM, restart_proc, MPI_COMM_WORLD);
      MPI_Reduce(&(norm_r_2_chkpt), &nrt, 1, MPI_DOUBLE, MPI_SUM, restart_proc, MPI_COMM_WORLD);
      MPI_Reduce(&(stop_chkpt), &st, 1, MPI_DOUBLE, MPI_SUM, restart_proc, MPI_COMM_WORLD);
      MPI_Reduce(&(num_iters_chkpt), &it, 1, MPI_INT, MPI_SUM, restart_proc, MPI_COMM_WORLD);        
    } 

    /* recover from -sum to sum */
    if( myproc == check_proc ) {   
      for (i=0;i<Nloc;i++) r_chkpt->val[i] = - r_chkpt->val[i];
      for (i=0;i<Nloc;i++) p_chkpt->val[i] = - p_chkpt->val[i];
      for (i=0;i<Nloc;i++) x_chkpt->val[i] = - x_chkpt->val[i];
      norm_r_2_chkpt = - norm_r_2_chkpt;
      stop_chkpt = - stop_chkpt;
      num_iters_chkpt = - num_iters_chkpt;
    } 
 
    
    /* recover data to local working memory on work processes */   
    if( myproc != check_proc ) {
      Nloc = r->row_f - r->row_i + 1;
      for (i=0;i<Nloc;i++) r->val[i] = r_chkpt->val[i];
      for (i=0;i<Nloc;i++) p->val[i] = p_chkpt->val[i];
      for (i=0;i<Nloc;i++) x->val[i] = x_chkpt->val[i];
      *p_norm = norm_r_2_chkpt;
      *p_stop = stop_chkpt;
      *p_num_iters = num_iters_chkpt; 
    }
    MPI_Bcast(p_norm, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD); /*Bcast the rank of the restarted proc to MPI_COMM_WORLD*/ 
    MPI_Bcast(p_stop, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD); /*Bcast the rank of the restarted proc to MPI_COMM_WORLD*/ 
    MPI_Bcast(p_num_iters, 1, MPI_INT, 0, MPI_COMM_WORLD); /*Bcast the rank of the restarted proc to MPI_COMM_WORLD*/

  }  
  
  dvec_destroy ( rt);
  dvec_destroy ( pt);
  dvec_destroy ( xt);
}




//#if 0
/* the following code is for disk checkpointing */
void dvec_checkpoint(d_vector_t* p_dvec, char* filename)
{
  FILE*        f_dvec;
  int          num;
  
  f_dvec = fopen(filename, "w");
  if (f_dvec == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    exit(1);
  }
  num = fwrite( p_dvec, sizeof(d_vector_t), 1, f_dvec);
  if ( num != 1 ) {
    fprintf(stderr, "bad fwrite of size on %s\n", filename);
    exit(1);
  }  
  num = fwrite( p_dvec->val, sizeof(double), (p_dvec->row_f - p_dvec->row_i + 1), f_dvec);
  if ( num != (p_dvec->row_f - p_dvec->row_i + 1)) {
    fprintf(stderr, "bad fwrite of size on %s\n", filename);
    exit(1);
  }
  fclose(f_dvec);
}     


void int_checkpoint( int* p_int, char* filename)
{
  FILE*        f_int;
  int          num;
  
  f_int = fopen(filename, "w");
  if (f_int == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    exit(1);
  }
  num = fwrite( p_int, sizeof(int), 1, f_int);
  if ( num != 1 ) {
    fprintf(stderr, "bad fwrite of size on %s\n", filename);
    exit(1);
  }  
  fclose(f_int);
}     


void double_checkpoint( double* p_double, char* filename)
{
  FILE*        f_double;
  int          num;
  
  f_double = fopen(filename, "w");
  if (f_double == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    exit(1);
  }
  num = fwrite( p_double, sizeof(double), 1, f_double);
  if ( num != 1 ) {
    fprintf(stderr, "bad fwrite of size on %s\n", filename);
    exit(1);
  }  
  fclose(f_double);
}     


void dvec_recover(d_vector_t* p_dvec, char* filename)
{
  FILE*        f_dvec;
  //  int          num;
  int          count=0;        
  
  f_dvec = fopen(filename, "r");
  if (f_dvec == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    exit(1);
  }
  /*
    num = fread( p_dvec, sizeof(d_vector_t), 1, f_dvec);
    if ( num != 1 ) {
    fprintf(stderr, "num=%d , 11111bad fread of size on %s\n", num, filename);
    exit(1);
    }  
  */
  //sleep(5);
  count =0;
  while ( (fread( p_dvec, sizeof(d_vector_t), 1, f_dvec) != 1)&&(count<3) ) {
    count++;
    fprintf(stderr, "%d Try %d times, failed to read!!\n", g_myproc, count);
    //sleep(5);
  }
  if ( count == 3 ) {
    fprintf(stderr, "111 bad fread of size on %s: tried % times \n", filename, count);
    exit(1);    
  }
  /*
    num = fread( p_dvec->val, sizeof(double), (p_dvec->row_f - p_dvec->row_i + 1), f_dvec);
    if ( num != (p_dvec->row_f - p_dvec->row_i + 1)) {
    fprintf(stderr, "222222bad fread of size on %s\n", filename);
    exit(1);
    } 
  */
  count =0;
  //sleep(5);
  while ( (fread( p_dvec->val, sizeof(double), (p_dvec->row_f - p_dvec->row_i + 1), f_dvec) != (p_dvec->row_f - p_dvec->row_i + 1)) && (count<3) ) {
    count++;
    //sleep(5);
  }
  if ( count == 3 ) {   
    fprintf(stderr, "222 bad fread of size on %s\n", filename);
    exit(1);    
  }
  
  fclose(f_dvec);
}

void int_recover( int* p_int, char* filename)
{
  FILE*        f_int;
  //int          num;
  int          count;
  
  f_int = fopen(filename, "r");
  if (f_int == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    exit(1);
  }
  /*
    num = fread( p_int, sizeof(int), 1, f_int);
    if ( num != 1 ) {
    fprintf(stderr, "bad fread of size on %s\n", filename);
    exit(1);
    }  
  */
  count = 0;
  //sleep(5);
  while ( (fread( p_int, sizeof(int), 1, f_int) != 1)&& (count<3) ) {
    count++;
    //sleep(5);
  }
  if ( count == 3 ) {   
    fprintf(stderr, "333 bad fread of size on %s\n", filename);
    exit(1);    
  }
  fclose(f_int);
}     


void double_recover( double* p_double, char* filename)
{
  FILE*        f_double;
  //int          num;
  int          count;
  
  f_double = fopen(filename, "r");
  if (f_double == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    exit(1);
  }
  /*
    num = fread( p_double, sizeof(double), 1, f_double);
    if ( num != 1 ) {
    fprintf(stderr, "bad fread of size on %s\n", filename);
    exit(1);
    }
  */
  count = 0;
  //sleep(5);
  while ( (fread( p_double, sizeof(double), 1, f_double) != 1)&& (count<3) ) {
    count++;
    //sleep(5);
  }
  if ( count == 3 ) {   
    fprintf(stderr, "444 bad fread of size on %s\n", filename);
    exit(1);    
  }
  fclose(f_double);
}     


void checkpoint_disk(d_vector_t* p_dvec, d_vector_t* x_dvec, d_vector_t* r_dvec, double* p_norm, int* p_iter, int myrank)
{
  static char *p_file[]={"/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/p_proc_0",
			 "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/p_proc_1",
			 "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/p_proc_2", 
			 "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/p_proc_3"};
  static char *x_file[]={"/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/x_proc_0", 
			 "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/x_proc_1", 
			 "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/x_proc_2",  
			 "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/x_proc_3"};  
  static char *r_file[]={"/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/r_proc_0", 
			 "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/r_proc_1", 
			 "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/r_proc_2", 
			 "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/r_proc_3"};
  static char *norm_file[]={"/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/norm_proc_0",
			    "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/norm_proc_1", 
			    "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/norm_proc_2", 
			    "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/norm_proc_3"};
  static char *iter_file[]={"/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/iter_proc_0", 
			    "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/iter_proc_1",
			    "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/iter_proc_2",  
			    "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/iter_proc_3"};  
  
  
  dvec_checkpoint(p_dvec, p_file[myrank]);
  dvec_checkpoint(x_dvec, x_file[myrank]);
  dvec_checkpoint(r_dvec, r_file[myrank]);
  double_checkpoint(p_norm, norm_file[myrank]);
  int_checkpoint(p_iter, iter_file[myrank]);
  
}



void recover_disk(d_vector_t* p_dvec, d_vector_t* x_dvec, d_vector_t* r_dvec, double* p_norm, int* p_iter, int myrank)
{
  static char *p_file[]={"/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/p_proc_0",
			 "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/p_proc_1",
			 "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/p_proc_2", 
			 "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/p_proc_3"};
  static char *x_file[]={"/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/x_proc_0", 
			 "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/x_proc_1", 
			 "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/x_proc_2",  
			 "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/x_proc_3"};  
  static char *r_file[]={"/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/r_proc_0", 
			 "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/r_proc_1", 
			 "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/r_proc_2", 
			 "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/r_proc_3"};
  static char *norm_file[]={"/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/norm_proc_0",
			    "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/norm_proc_1", 
			    "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/norm_proc_2", 
			    "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/norm_proc_3"};
  static char *iter_file[]={"/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/iter_proc_0", 
			    "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/iter_proc_1",
			    "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/iter_proc_2",  
			    "/.autofs/home4/zchen/ftmpi/pcg/checkpoint_files/iter_proc_3"};   
  
  dvec_recover(p_dvec, p_file[myrank]);
  dvec_recover(x_dvec, x_file[myrank]);
  dvec_recover(r_dvec, r_file[myrank]);
  double_recover(p_norm, norm_file[myrank]);
  int_recover(p_iter, iter_file[myrank]);
  
}

//#endif










