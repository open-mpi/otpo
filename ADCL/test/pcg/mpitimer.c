/*
 * $Id$
 *
 * file :  mpitimer.c
 * desc :  Implementation of an MPI timer based stopwatch
 *
 */

#include "mpitimer.h"


void
mpitim_reset( mpi_timer_t* T )
{
  T->t0 = 0.;
  T->t = 0.;
  T->running = 0;
}


void
mpitim_start( mpi_timer_t* T )
{
  T->running = 1;
  T->t0 = MPI_Wtime();
}


void
mpitim_stop( mpi_timer_t* T )
{
  double tf = MPI_Wtime();
  if( T->running ) {
    T->t += tf - T->t0;
    T->running = 0;
  }
}


double
mpitim_elapsed( mpi_timer_t* T )
{
  double tf = MPI_Wtime();
  if( !T->running ) {
    return T->t;
  }
  T->t += tf - T->t0;
  T->t0 = MPI_Wtime();
  return T->t;
}


double
mpitim_resolution( void )
{
  return MPI_Wtick();
}


double
mpitim_elapsed_units( mpi_timer_t* T )
{
  double tf = MPI_Wtime();
  double t_sofar;

  if( !T->running ) {
    return T->t / mpitim_resolution();
  }

  T->t += tf - T->t0;
  t_sofar = T->t / mpitim_resolution();
  T->t0 = MPI_Wtime();
  return t_sofar;
}


/*
 * $Log$
 */










































































