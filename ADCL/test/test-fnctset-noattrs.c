/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include <stdio.h>

#include "ADCL.h"
#include "mpi.h"

#define NIT 500

void test_func_1 ( ADCL_Request req, int *a, int* b, int *c );
void test_func_2 ( ADCL_Request req, int *a, int* b, int *c );
void test_func_3 ( ADCL_Request req, int *a, int* b, int *c );



int main ( int argc, char ** argv ) 
{
    int i, rank, size;

    ADCL_Function funcs[3];
    ADCL_Fnctset fnctset;
    ADCL_Request request;
    ADCL_Topology topo;
    
    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    ADCL_Init ();

    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_1, ADCL_ATTRSET_NULL, NULL, 
			                "test_func_1", &(funcs[0]));

    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_2, ADCL_ATTRSET_NULL, NULL, 
			                "test_func_2", &(funcs[1]));

    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_3, ADCL_ATTRSET_NULL, NULL, 
			                "test_func_3", &(funcs[2]));

    ADCL_Fnctset_create ( 3, funcs, "trivial functions", &fnctset );

    ADCL_Topology_create_generic ( 0, NULL, NULL, NULL, ADCL_DIRECTION_BOTH, 
				                   MPI_COMM_WORLD, &topo );
    ADCL_Request_create ( ADCL_VECTOR_NULL, topo, fnctset, &request );
    

    for ( i=0; i<NIT; i++ ) {
	    ADCL_Request_start( request );
    }

    ADCL_Request_free ( &request );
    ADCL_Topology_free ( &topo );
    ADCL_Fnctset_free ( &fnctset );
    
    for ( i=0; i<3; i++ ) {
	    ADCL_Function_free ( &funcs[i] );
    }

    ADCL_Finalize ();
    MPI_Finalize ();
    return 0;
}

void test_func_1 ( ADCL_Request req, int *a, int* b, int *c )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
/*     printf("%d: In test_func_1, a=%d, b=%d, c=%d \n", rank, *a, *b, *c ); */
    printf("%d: In test_func_1 , size=%d\n", rank, size);
    return;
}

void test_func_2 ( ADCL_Request req, int *a, int* b, int *c )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
/*     printf("%d: In test_func_2, a=%d, b=%d, c=%d \n", rank, *a, *b, *c ); */
    printf("%d: In test_func_2 , size=%d\n", rank, size);
    return;
}

void test_func_3 ( ADCL_Request req, int *a, int* b, int *c )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
/*     printf("%d: In test_func_3, a=%d, b=%d, c=%d \n", rank, *a, *b, *c ); */
    printf("%d: In test_func_3 , size=%d\n", rank, size);
    return;
}
