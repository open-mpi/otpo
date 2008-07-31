/* mpitest1: program executes on an arbitrary number of */
/*           nodes and creates an appropriate topology. */
/*           Then it prints the ranks and coordinates   */
/*           for each process.                          */

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "parheat.h"

int main( int argc, char *argv[] )
{
    /* loop counters                */
    int i, j;

    /* time variables               */
    double starttime, endtime;
    double delta_t, delta_x;
    struct timing data = { 0, 0, 0, 0, 0, 0 };
    
    /* error variable               */
    int ierr;
    
    /* iteration counter            */
    int num_iter;
    
    /* variables for the input file */
    int grid[3];
    int mem_fac, msg_fac, cpt_fac;
    double accuracy, tstep_fac, c_fact;
    double min[3], max[3];
    
    /* variables needed for setting */
    /* up the correct topology      */
    int numprocs, myid, oldmyid, ndim, dimlist[3];
    int reorder, periods[3], coords[3];
    MPI_Comm newcomm;
    int nodenum;
    
    /* variables for the get_domain  */
    /* routine                       */
    int ppnode[3];
    double lmin[3], lmax[3]; 
    
    /* variables for get_mesh_mem    */
    struct point **setlist;
    
    /* variable for get_time_mem     */
    struct tstep solution;
    
    /* array for the neighbor process ranks */
    int neighbor[6];
    
    /* variables to account for the modified */
    /* domain.                               */
    int start[3], end[3];
    int num_bfaces, num_ifaces;
    int num_bcnodes, max_bcnodes;
    int *bcnode;
    
    /* Filename for output file      */
    char filename[]="parheat.dat";
    
    /* this is necessary for getting the timing */
    /* information to the root process          */
    double tsend[6] = { 0, 0, 0, 0, 0, 0 };
    double *trecv;
    
    for( i=0 ; i<6 ; i++ )
	neighbor[i] = MPI_PROC_NULL;
    
    if( (ierr=MPI_Init( &argc, &argv )) != 0 )
    {
	printf( "MPI_Init failed with code %d.\n", ierr );
	exit( ierr );
    }
    
    MPI_Comm_size( MPI_COMM_WORLD, &numprocs );
    MPI_Comm_rank( MPI_COMM_WORLD, &oldmyid );
    
    get_dimlist( numprocs, &ndim, dimlist );
    
    periods[0] = 0;
    periods[1] = 0;
    periods[2] = 0;
    reorder = 1;
    
    /* take care of unused dimensions */
    for( i=0 ; i<3 ; i++ )
	coords[i] = 0;
    
    if( (ierr=MPI_Cart_create( MPI_COMM_WORLD, ndim, dimlist, periods, 
			       reorder, &newcomm )) != 0 )
    {
	printf( "MPI_Cart_create failed with code %d.\n", ierr );
	MPI_Abort ( MPI_COMM_WORLD, ierr );
    }
    
    MPI_Comm_rank( newcomm, &myid );
    MPI_Cart_coords( newcomm, myid, ndim, coords );
    
    starttime = MPI_Wtime();
    if( myid == 0  )
	printf( "There are %d processes.\n", numprocs );
    
    
    read_input( grid, &mem_fac, &msg_fac, &cpt_fac,
		&accuracy, &tstep_fac, &c_fact, min, max );
/* MIR */
    delta_x = 1.0/(grid[1]-1);
    delta_t = tstep_fac * delta_x *delta_x/6.0;
    if ( myid == 0 ) {
	printf (" delta_x = %f, delta_t = %f \n", delta_x, delta_t);
    }
/* MIR */
    
    
    
    for( i=0 ; i<ndim ; i++ ) {
	if( (ierr = MPI_Cart_shift( newcomm, i, 1, &neighbor[i], &neighbor[i+3])) != 0 ) {
	    printf( "MPI_Cart_shift failed with code %d.\n" , ierr );
	    MPI_Abort ( MPI_COMM_WORLD, ierr );
	} 
    }
    
#ifdef DEBUG
    printf("%d  coords %d %d %d  \n", myid ,coords[0],coords[1],coords[2]);
    printf("%d  neighbors %d %d %d  \n", myid ,neighbor[0],neighbor[1],neighbor[2]);
#endif 

    if( (ierr=get_domain( grid, coords, dimlist, ndim,		
			  min, max, ppnode, lmin, lmax )) != 0 )
    {
	printf( "Error in get_domain.\n" );
	MPI_Abort ( MPI_COMM_WORLD, 1 );
    }
    
    /* find out the right properties for */
    /* my particular domain.             */
    num_bfaces = 0;
    num_ifaces = 0;
    for( i=0 ; i<3 ; i++ )
    {
	if( neighbor[i] != MPI_PROC_NULL )
	{
	    ppnode[i]++;
	    start[i] = 1;
	    num_ifaces++;
	}
	else
	{
	    start[i] = 0;
	    num_bfaces++;
	}
	
	if( neighbor[i+3] != MPI_PROC_NULL )
	{
	    ppnode[i]++;
	    end[i] = ppnode[i] - 2;
	    num_ifaces++;
	}
	else
	{
	    end[i] = ppnode[i] - 1;
	    num_bfaces++;
	}
    }
    
    if( (ierr=get_mesh_mem( mem_fac, ppnode, &setlist )) != 0 )
	printf( "process %d: get_mesh_mem failed with code %d.\n", 
		myid, ierr );
    
    if( (ierr=get_coords( ppnode, start, end,
			  lmin, lmax, *setlist )) != 0 )
    {
	printf( "parheat: get_coords failed with code %d.\n", ierr );
	MPI_Abort ( MPI_COMM_WORLD, 1 );
    }
    
    if( (ierr=get_time_mem( ppnode, &solution )) != 0 )
    {
	printf( "parheat: get_time_mem failed with code %d.\n", ierr );
	MPI_Abort ( MPI_COMM_WORLD, 1 );
    }
    
    /* compute the maximal possible number   */
    /* of boundary nodes.                    */
    max_bcnodes = 0;
    for( i=0 ; i<6 ; i++ )
    {
	if( neighbor[i] == MPI_PROC_NULL )
	    max_bcnodes += ppnode[ (i+1)%3 ] * ppnode[ (i+2)%3 ];
    }
    
    if( (ierr=get_bcnode_mem( max_bcnodes, &bcnode )) != 0 )
    {
	printf( "parheat: get_bcnode_mem failed with code %d.\n", ierr );
	MPI_Abort ( MPI_COMM_WORLD, 1 );
    }
    
    if( (ierr=find_bcnodes( ppnode, start, end, neighbor,
			    bcnode, &num_bcnodes )) != 0 )
    {
	printf( "parheat: find_bcnodes failed with code %d.\n", ierr );
	MPI_Abort ( MPI_COMM_WORLD, 1 );
    }
    
    if( (ierr=set_initial( ppnode, solution.old )) != 0 )
    {
	printf( "parheat: set_initial failed with code %d.\n", ierr );
	MPI_Abort ( MPI_COMM_WORLD, 1 );
    }
    
    if( (ierr=apply_bc( num_bcnodes, *setlist, solution.old,
			bcnode )) != 0 )
    {
	printf( "parheat: apply_bc failed with code %d.\n", ierr );
	MPI_Abort ( MPI_COMM_WORLD, 1 );
    }
    
    if( (ierr=apply_bc( num_bcnodes, *setlist, solution.neu,
			bcnode )) != 0 )
    {
	printf( "parheat: apply_bc failed with code %d.\n", ierr );
	MPI_Abort ( MPI_COMM_WORLD, 1 );
    }
    
    /* Here comes the actual computation ! */
    if( (ierr=update_solution( c_fact, delta_t, delta_x, ppnode,   
			      start, end, neighbor,		
			      tstep_fac, accuracy, msg_fac, cpt_fac, &num_iter,
			       *setlist, &data, &solution, newcomm )) != 0 )
    {
	printf( "update_ solution failed with code %d.\n", ierr );
	MPI_Abort ( MPI_COMM_WORLD, ierr );
    }
    
    
    endtime = MPI_Wtime();
    data.total = endtime - starttime;
    tsend[0] = data.comp;
    tsend[1] = data.comm_start;
    tsend[2] = data.recv_end;
    tsend[3] = data.send_end;
    tsend[4] = data.sync;
    tsend[5] = data.total;
    
    
    if( myid == 0 )
	trecv = (double *)malloc( 6*numprocs*sizeof(double) );
    
    if( (ierr = MPI_Gather( (void *)&tsend, 6, MPI_DOUBLE, (void *)trecv, 6,
			    MPI_DOUBLE, 0, newcomm )) != 0 )
    {
	printf( "MPI_Gather failed with code %d.\n", ierr );
	MPI_Abort ( MPI_COMM_WORLD, ierr );
    }
    
    if( myid == 0 )
    {
	printf( "grid: %dx%dx%d\n", grid[0], grid[1], grid[2] );
	printf( "mem_fac=%d, msg_fac=%d, cpt_fac=%d\n", mem_fac, msg_fac, cpt_fac );
	printf( "accuracy=%le\n", accuracy );
	printf( "Number of iterations: %d.\n", num_iter );
	
	printf( "\ttime[sec]\t\tcomputation\tcomm_start  \trecv_end" );
	printf( "\tsend_end\tsync\ttotal\n" );
	for( i=0 ; i<numprocs*6 ; i+=6 )
	{
	    nodenum = i/6;
	    MPI_Cart_coords( newcomm, nodenum, ndim, coords );
	    
	    printf( "node %4d coords=", nodenum );
	    for( j=0 ; j<3 ; j++ )
		printf( "%3d ", coords[j] );
	    printf( ":" );
	    for( j=0 ; j<6 ; j++ )
		printf( "\t%lf ", trecv[i+j] );
	    printf( "\n" );
	}
	
	if( (ierr=write_step( ppnode, *setlist, solution.old, filename )) != 0 )
	{
	    printf( "process %d: write_step failed with code %d.\n", myid, ierr );
	    MPI_Abort ( MPI_COMM_WORLD, 1 );
	}
    }
    
    MPI_Finalize();
    return 0;
}
