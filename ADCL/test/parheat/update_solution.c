/* update_solution: controls the time integration */
/*                  process.                      */
/* input:  double c_fact                          */
/*         double delta_x                         */
/*         double delta_t                         */
/*         int grid[3]                            */
/*         int start[3]                           */
/*         int end[3]                             */
/*         int neighbor[6]                        */
/*         double tstep_fac                       */
/*         double accuracy                        */
/*         int msg_fac                            */
/*         int cpt_fac                            */
/*         int *num_iter                          */
/*         struct point *set                      */
/*         struct timing *data                    */
/*         struct tstep *solution                 */
/*         MPI_Comm newcomm                       */
/* output: int update_solution                    */

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "parheat.h"

int update_solution( double c_fact, double delta_t, double delta_x,  
              int *grid, int *start, int *end, int *neighbor, 
              double tstep_fac, double accuracy, int msg_fac, 
              int cpt_fac, int *num_iter, struct point *set, struct timing *data, 
              struct tstep *solution, MPI_Comm newcomm )
{
    MPI_Datatype faces[3];
    int ierr, i, j, num_ifaces;
    int local_done, global_done;
    int s_off[6]={ 0, 0, 0, 0, 0, 0 };
    int r_off[6]={ 0, 0, 0, 0, 0, 0 };
    MPI_Request s_req[6], r_req[6];
    MPI_Status s_stat[6], r_stat[6];
    double lambda;
    
    /* variables for timing computation and */
    /* communication                        */
    double cpt_start, cpt_end;
    double msg_start, msg_end;
    double duration;
    
    /* initialize request arrays */
    for( i=0 ; i<6 ; i++ )
	s_req[i] = r_req[i] = MPI_REQUEST_NULL;
    
    *num_iter = 0;
    
    local_done = 0;
    global_done = 0;
    
    /* get the start time for the computation */
    cpt_start = MPI_Wtime();
    
    /* compute the offsets for the send and */
    /* receive operations.                  */
    if( (ierr=get_offset( grid, start, end, neighbor, s_off, r_off )) != 0 )
    {
	printf( "get_offset failed with code %d.\n", ierr );
	return ierr;
    } 
    
    /* compute the correct time step for the */
    /* grid considered.                      */
    /*  lambda = tstep_fac / 6.0; */ /*  dt<= h^2/2*n */
    /* MIR */
    lambda = delta_t / (delta_x * delta_x); 
    /* MIR */
    
    /* get datatypes for sendrecv */
    if( (ierr=get_datatypes( grid, start, end, faces, msg_fac )) != 0 )
    {
	printf( "get_datatypes failed with code %d.\n", ierr );
	return ierr;
    } 
        
    num_ifaces = 0;
    for( i=0 ; i<6 ; i++ )
    {
	if( neighbor[i] != MPI_PROC_NULL )
	    num_ifaces++;
    }
    
    /* finally: THE TIME LOOP!    */
    do
    {
	/* post a receive for all neighbors */
	/* Get time values for initiating messages */
	msg_start = MPI_Wtime();
	
	/* post a receive for all existing neighbors */
	for( i=0 ; i<6 ; i++ )	{
	    if( neighbor[i] != MPI_PROC_NULL )  {
		MPI_Irecv( (void *)&((*solution).old[r_off[i]]), 1, faces[i%3], 
			   neighbor[i], 1000, newcomm, &r_req[i] );
	    }
	}
	
	/* send to all neighbors            */
	
	for( i=0 ; i<6 ; i++ ) {
	    if( neighbor[i] != MPI_PROC_NULL )
		MPI_Isend( (void *)&((*solution).old[s_off[i]]), 1, faces[i%3], 
			   neighbor[i], 1000, newcomm, &s_req[i] );
	}
	
	msg_end = MPI_Wtime();
	duration = msg_end - msg_start;
	(*data).comm_start += duration;         /* add communication part */
	(*data).comp -= duration;         /* subtract non-computation part */
	
	/*
	 * from now on we either compute (local_done == 0)
	 * or provide neighbors with data (local_done == 1)
	 */
	
	if( local_done == 0) {
	    update_interior( c_fact, delta_t, delta_x, grid, start, 
			     end, lambda, solution );
		
	    msg_start = MPI_Wtime();
	    MPI_Waitall( 6, r_req, r_stat );
	    msg_end = MPI_Wtime();
	    duration = msg_end - msg_start;
	    (*data).recv_end += duration;         /* add communication part */
	    (*data).comp -= duration;         /* subtract non-computation part */	
		
	    update_faces( c_fact, delta_t, delta_x, grid, start,	
			  end, neighbor, lambda,			
			      solution );
	    
	    if( (ierr=check_done( accuracy, &local_done, grid, start, end, 
				  solution, set )) != 0 ) {
		printf( "check_done: check_done failed with code %d.\n", 
			ierr );
		return 1;
	    }
	} 
	else {
	    /* that means, local_done == 1*/  	    

	    msg_start = MPI_Wtime();
	    MPI_Waitall( 6, r_req, r_stat );
	    msg_end = MPI_Wtime();
	    duration = msg_end - msg_start;
	    (*data).recv_end += duration;         /* add communication part */
	    (*data).comp -= duration;         /* subtract non-computation part */
	    
	} /* end of providing information to neighbors */
		
	
	if( local_done == 0) {
	    if( (ierr=switch_steps( solution )) != 0 ) {
		printf( "update_solution: switch_steps failed with code %d.\n", 
			ierr );
		return 1;
	    }
	}
	
	MPI_Allreduce ( &local_done, &global_done, 1, MPI_INT, MPI_MIN, newcomm );

	(*num_iter) += 1; 
    } while (  global_done == 0);
    
    cpt_end = MPI_Wtime();
    duration = cpt_end - cpt_start;
    (*data).comp += duration;           /* add all time (communication was previously
                                           subtracted */
    
    return 0;
}
