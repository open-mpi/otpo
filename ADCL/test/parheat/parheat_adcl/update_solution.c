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
#include "ADCL.h"

int update_solution( double c_fact, double delta_t, double delta_x,  
                     int *grid, int *start, int *end, int *neighbor, 
                     double tstep_fac, double accuracy, int msg_fac, 
                     int cpt_fac, int max_iter, int *num_iter,
                     struct point *set, struct timing *data,
                     struct tstep *solution, MPI_Comm newcomm )
{
    MPI_Datatype faces[3];
    int ierr, i, j, num_ifaces;
    int local_done, global_done;
    double lambda;
   
    /* variables for timing computation and */
    /* communication                        */
    double cpt_start, cpt_end;
    double msg_start, msg_end;
    double duration;
   
    /*ADCL Declaration*/
    int hwidth = 1, dims[3];
    ADCL_Topology topo;
    ADCL_Vmap vmap; 
    ADCL_Vector ovec, nvec;
    ADCL_Request request;
    ADCL_Request nrequest;

    *num_iter = 0;
    
    local_done = 0;
    global_done = 0;
    
    /* get the start time for the computation */
    cpt_start = MPI_Wtime();
    
    lambda = delta_t / (delta_x * delta_x); 
 
    num_ifaces = 0;
    for( i=0 ; i<6 ; i++ ) {
	if( neighbor[i] != MPI_PROC_NULL ) {
	    num_ifaces++;
        }
    }
    ADCL_Topology_create(newcomm, &topo);
    dims[0] = grid[0];
    dims[1] = grid[1];
    dims[2] = grid[2];
    ADCL_Vmap_halo_allocate ( hwidth, &vmap );
    ADCL_Vector_register_generic (3, dims, 0, vmap, MPI_DOUBLE, 
			  (double***)(solution->old), &ovec);
    ADCL_Vector_register_generic (3, dims, 0, vmap, MPI_DOUBLE, 
			  (double***)(solution->neu), &nvec);
    ADCL_Request_create( ovec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &request);
    ADCL_Request_create( nvec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &nrequest);
    struct req requests = {&request, &nrequest};
    /* finally: THE TIME LOOP!    */
    do {
	/* post a receive for all neighbors */
	/* Get time values for initiating messages */
	msg_start = MPI_Wtime();
	ADCL_Request_start(*(requests.oreq));
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

	    if( (ierr=switch_steps( solution, &requests )) != 0 ) {
		printf( "update_solution: switch_steps failed with code %d.\n",
                        ierr );
		return 1;
	    }
	}
	
        MPI_Allreduce ( &local_done, &global_done, 1, MPI_INT, MPI_MIN, newcomm );

        (*num_iter) += 1;
        /* Check if max # of iterations (if set by user) is reached */
        if( (max_iter>0) && ((*num_iter)>= max_iter) ) {
            global_done = 1;
        }

    } while ( 0 == global_done );
    
    cpt_end = MPI_Wtime();
    duration = cpt_end - cpt_start;
    (*data).comp += duration;           /* add all time (communication was previously
                                           subtracted */

    ADCL_Request_free(&request);
    ADCL_Request_free(&nrequest);
    ADCL_Vector_free(&nvec);
    ADCL_Vector_free(&ovec);
    ADCL_Vmap_free(&vmap);
    ADCL_Topology_free(&topo);
    return 0;
}
