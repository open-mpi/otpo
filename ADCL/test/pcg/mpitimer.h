/*
 * $Id$
 *
 * file :  mpitimer.h
 * desc :  Implementation of an MPI timer based stopwatch
 *
 */

#ifndef INC_MPITIMER_H
#define INC_MPITIMER_H


#include <mpi.h>

typedef struct tag_mpi_timer_t {
  double t0;      /* last start time */
  double t;       /* elapsed time */
  int running;    /* true <==> timer running */
} mpi_timer_t;


void mpitim_reset( mpi_timer_t* T );
  /*
   *  post :  sets T's elapsed time to zero; sets T to ``not running'' state
   */


void mpitim_start( mpi_timer_t* T );
  /*
   *  post :  start timer.  time is accumulated (i.e., not reset
   *          to zero).  ignored if timer already running
   */


void mpitim_stop( mpi_timer_t* T );
  /*
   *  post :  stop timer and record elapsed time.  if the timer is
   *          is not running, this has no effect.
   */


double mpitim_elapsed( mpi_timer_t* T );
  /*
   *  post :  returns the elapsed time in seconds.  if the timer
   *          is running, it continues to run after the call.
   */


double mpitim_resolution( void );
  /*
   *  post :  returns the timing resolution
   */


double mpitim_elapsed_units( mpi_timer_t* T );
  /*
   *  post :  returns the elapsed time in units of the timer resolution.
   *          behavior is otherwise identical to mpitim_elapsed
   */



#define DO_TIMING

#ifdef DO_TIMING
  #define MPI_START_TIMING(T) { \
      mpitim_start( &T ); \
    }
  #define MPI_STOP_TIMING(T) { \
      mpitim_stop( &T ); \
    }
  #define MPI_TIMER_SWITCH(Ta,Tb) { \
      mpitim_stop( &Ta ); \
      mpitim_start( &Tb ); \
    }
#else
  #define MPI_START_TIMING(T)
  #define MPI_STOP_TIMING(T)
  #define MPI_TIMER_SWITCH(Ta,Tb)
#endif


#endif

/*
 * $Log$
 */











