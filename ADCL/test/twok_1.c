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
#include <unistd.h>

#define NIT 500

static void test_func_0 ( ADCL_Request req );
static void test_func_1 ( ADCL_Request req );
static void test_func_2 ( ADCL_Request req );
static void test_func_3 ( ADCL_Request req );
static void test_func_4 ( ADCL_Request req );
static void test_func_5 ( ADCL_Request req );
static void test_func_6 ( ADCL_Request req );
static void test_func_7 ( ADCL_Request req );
static void test_func_8 ( ADCL_Request req );
static void test_func_9 ( ADCL_Request req );
static void test_func_10 ( ADCL_Request req );
static void test_func_11 ( ADCL_Request req );
static void test_func_12 ( ADCL_Request req );
static void test_func_13 ( ADCL_Request req );
static void test_func_14 ( ADCL_Request req );
static void test_func_15 ( ADCL_Request req );
static void test_func_16 ( ADCL_Request req );
static void test_func_17 ( ADCL_Request req );

/* Optimal values shall be {0,4,6}  = Func9*/

#define TIME0 14000
#define TIME1 40000
#define TIME2 22000

#define TIME3 10100
#define TIME4 30000
#define TIME5 34000

#define TIME6 11000
#define TIME7 20000
#define TIME8 30000

#define TIME9 10000
#define TIME10 20000
#define TIME11 40000

#define TIME12 46000
#define TIME13 60000
#define TIME14 58000

#define TIME15 50000
#define TIME16 50000
#define TIME17 86000



int main ( int argc, char ** argv ) 
{
    int i, rank, size, ret;
    char *function_name;
    double filtered_avg, unfiltered_avg, outliers_num;

    ADCL_Function funcs[18];
    ADCL_Fnctset fnctset;
    ADCL_Request request;
    ADCL_Topology topo;
    ADCL_Attribute attrs[3];
    ADCL_Attrset attrset;
    int attr1vals[3]={0,1,2};
    int attr2vals[2]={3,4};
    int attr3vals[3]={5,6,7};

    int funcvals0[3]={0,3,5};
    int funcvals1[3]={1,3,5};
    int funcvals2[3]={2,3,5};
    int funcvals3[3]={0,4,5};
    int funcvals4[3]={1,4,5};
    int funcvals5[3]={2,4,5};

    int funcvals6[3]={0,3,6};
    int funcvals7[3]={1,3,6};
    int funcvals8[3]={2,3,6};
    int funcvals9[3]={0,4,6};
    int funcvals10[3]={1,4,6};
    int funcvals11[3]={2,4,6};

    int funcvals12[3]={0,3,7};
    int funcvals13[3]={1,3,7};
    int funcvals14[3]={2,3,7};
    int funcvals15[3]={0,4,7};
    int funcvals16[3]={1,4,7};
    int funcvals17[3]={2,4,7};


    
    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    ADCL_Init ();

    ADCL_Attribute_create ( 3, attr1vals, NULL, NULL, &(attrs[0]));
    ADCL_Attribute_create ( 2, attr2vals, NULL, NULL, &(attrs[1]));
    ADCL_Attribute_create ( 3, attr3vals, NULL, NULL, &(attrs[2]));
    ADCL_Attrset_create ( 3, attrs, &attrset );


    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_0, attrset, funcvals0,
			   "test_func_0", &(funcs[0]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_1, attrset, funcvals1,
			   "test_func_1", &(funcs[1]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_2, attrset, funcvals2, 
			   "test_func_2", &(funcs[2]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_3, attrset, funcvals3, 
			   "test_func_3", &(funcs[3]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_4, attrset, funcvals4, 
			   "test_func_4", &(funcs[4]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_5, attrset, funcvals5, 
			   "test_func_5", &(funcs[5]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_6, attrset, funcvals6, 
			   "test_func_6", &(funcs[6]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_7, attrset, funcvals7, 
			   "test_func_7", &(funcs[7]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_8, attrset, funcvals8, 
			   "test_func_8", &(funcs[8]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_9, attrset, funcvals9, 
			   "test_func_9", &(funcs[9]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_10, attrset, funcvals10, 
			   "test_func_10", &(funcs[10]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_11, attrset, funcvals11, 
			   "test_func_11", &(funcs[11]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_12, attrset, funcvals12, 
			   "test_func_12", &(funcs[12]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_13, attrset, funcvals13, 
			   "test_func_13", &(funcs[13]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_14, attrset, funcvals14, 
			   "test_func_14", &(funcs[14]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_15, attrset, funcvals15, 
			   "test_func_15", &(funcs[15]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_16, attrset, funcvals16, 
			   "test_func_16", &(funcs[16]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_17, attrset, funcvals17, 
			   "test_func_17", &(funcs[17]));

    ADCL_Fnctset_create ( 18, funcs, "trivial functions", &fnctset );

    ADCL_Topology_create_generic ( 0, 0, NULL, NULL, NULL, NULL, ADCL_DIRECTION_BOTH, 
				   MPI_COMM_WORLD, &topo );
    ADCL_Request_create ( ADCL_VECTOR_NULL, topo, fnctset, &request );
    

    for ( i=0; i<NIT; i++ ) {
	ADCL_Request_start( request );
    }

    ADCL_Request_free ( &request );
    ADCL_Fnctset_free ( &fnctset );
    
    for ( i=0; i<18; i++ ) {
	ADCL_Function_free ( &funcs[i] );
    }

    ADCL_Attrset_free ( &attrset);
    for ( i=0; i<3; i++ ) {
	ADCL_Attribute_free ( &attrs[i]);
    }
    
    ADCL_Finalize ();
    MPI_Finalize ();
    return 0;
}

void test_func_0 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
#ifdef VERBOSE
    printf("%d: In test_func_0 , size=%d\n", rank, size);
#endif
    usleep ( TIME0 );
    return;
}

void test_func_1 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
#ifdef VERBOSE
    printf("%d: In test_func_1 , size=%d\n", rank, size);
#endif
    usleep ( TIME1 );
    return;
}

void test_func_2 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
#ifdef VERBOSE
    printf("%d: In test_func_2 , size=%d\n", rank, size);
#endif
    usleep ( TIME2 );
    return;
}

void test_func_3 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
#ifdef VERBOSE
    printf("%d: In test_func_3 , size=%d\n", rank, size);
#endif
    usleep ( TIME3 );
    return;
}

void test_func_4 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
#ifdef VERBOSE
    printf("%d: In test_func_4 , size=%d\n", rank, size);
#endif
    usleep ( TIME4 );
    return;
}

void test_func_5 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
#ifdef VERBOSE
    printf("%d: In test_func_5 , size=%d\n", rank, size);
#endif
    usleep ( TIME5 );
    return;
}

void test_func_6 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
#ifdef VERBOSE
    printf("%d: In test_func_6 , size=%d\n", rank, size);
#endif
    usleep ( TIME6 );
    return;
}
void test_func_7 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
#ifdef VERBOSE
    printf("%d: In test_func_7 , size=%d\n", rank, size);
#endif
    usleep ( TIME7 );
    return;
}
void test_func_8 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
#ifdef VERBOSE
    printf("%d: In test_func_8 , size=%d\n", rank, size);
#endif
    usleep ( TIME8 );
    return;
}
void test_func_9 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
#ifdef VERBOSE
    printf("%d: In test_func_9 , size=%d\n", rank, size);
#endif
    usleep ( TIME9 );
    return;
}
void test_func_10 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
#ifdef VERBOSE
    printf("%d: In test_func_10 , size=%d\n", rank, size);
#endif
    usleep ( TIME10 );
    return;
}
void test_func_11 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
#ifdef VERBOSE
    printf("%d: In test_func_1 , size=%d\n", rank, size);
#endif
    usleep ( TIME11 );
    return;
}
void test_func_12 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
#ifdef VERBOSE
    printf("%d: In test_func_12 , size=%d\n", rank, size);
#endif
    usleep ( TIME12 );
    return;
}
void test_func_13 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
#ifdef VERBOSE
    printf("%d: In test_func_13 , size=%d\n", rank, size);
#endif
    usleep ( TIME13 );
    return;
}
void test_func_14 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
#ifdef VERBOSE
    printf("%d: In test_func_14 , size=%d\n", rank, size);
#endif
    usleep ( TIME14 );
    return;
}
void test_func_15 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
#ifdef VERBOSE
    printf("%d: In test_func_15 , size=%d\n", rank, size);
#endif
    usleep ( TIME15 );
    return;
}
void test_func_16 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
#ifdef VERBOSE
    printf("%d: In test_func_16 , size=%d\n", rank, size);
#endif
    usleep ( TIME16 );
    return;
}
void test_func_17 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );
#ifdef VERBOSE
    printf("%d: In test_func_17 , size=%d\n", rank, size);
#endif
    usleep ( TIME17 );
    return;
}
