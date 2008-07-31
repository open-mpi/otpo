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

void test_func_0_0_0 ( ADCL_Request req );
void test_func_0_0_1 ( ADCL_Request req );
void test_func_0_1_0 ( ADCL_Request req );
void test_func_0_1_1 ( ADCL_Request req );
void test_func_0_2_0 ( ADCL_Request req );
void test_func_0_2_1 ( ADCL_Request req );
void test_func_1_0_0 ( ADCL_Request req );
void test_func_1_0_1 ( ADCL_Request req );
void test_func_1_1_0 ( ADCL_Request req );
void test_func_1_1_1 ( ADCL_Request req );
void test_func_1_2_0 ( ADCL_Request req );
void test_func_1_2_1 ( ADCL_Request req );

int main ( int argc, char ** argv ) 
{
    /* General variables */
    int i, k, attrs_num, *attrs_values_num, rank, size;
    char *function_name;
    char **attrs_names;
    char **attrs_values_names;

    ADCL_Attribute attrs[3];
    ADCL_Attrset attrset;
    ADCL_Function funcs[12];
    ADCL_Fnctset fnctset;
    ADCL_Topology topo;
    ADCL_Request request;

    int attr1vals[2]={10,11};
    int attr2vals[3]={20,21,22};
    int attr3vals[2]={30,31};

    char* attr1vals_names[2]={"attr1_val0","attr1_val1"};
    char* attr2vals_names[3]={"attr2_val0","attr2_val1","attr2_val2"};
    char* attr3vals_names[2]={"attr3_val0","attr3_val1"};

    int funcvals0[3]={10,20,30};
    int funcvals1[3]={10,20,31};
    int funcvals2[3]={10,21,30};
    int funcvals3[3]={10,21,31};
    int funcvals4[3]={10,22,30};
    int funcvals5[3]={10,22,31};
    int funcvals6[3]={11,20,30};
    int funcvals7[3]={11,20,31};
    int funcvals8[3]={11,21,30};
    int funcvals9[3]={11,21,31};
    int funcvals10[3]={11,22,30};
    int funcvals11[3]={11,22,31};

    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    ADCL_Init ();

    /* Creation of the attribute set */
    ADCL_Attribute_create ( 2, attr1vals, attr1vals_names, "first_attr ", &(attrs[0]));
    ADCL_Attribute_create ( 3, attr2vals, attr2vals_names, "second_attr", &(attrs[1]));
    ADCL_Attribute_create ( 2, attr3vals, attr3vals_names, "third_attr ", &(attrs[2]));
    ADCL_Attrset_create ( 3, attrs, &attrset );
  
    /* Creation of the function set */
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_0_0_0, attrset, funcvals0,
                           "test_func_0_0_0", &(funcs[0]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_0_0_1, attrset, funcvals1,
                           "test_func_0_0_1", &(funcs[1]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_0_1_0, attrset, funcvals2, 
                           "test_func_0_1_0", &(funcs[2]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_0_1_1, attrset, funcvals3, 
                           "test_func_0_1_1", &(funcs[3]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_0_2_0, attrset, funcvals4, 
        	           "test_func_0_2_0", &(funcs[4]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_0_2_1, attrset, funcvals5, 
                           "test_func_0_2_1", &(funcs[5]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_1_0_0, attrset, funcvals6, 
                           "test_func_1_0_0", &(funcs[6]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_1_0_1, attrset, funcvals7, 
                           "test_func_1_0_1", &(funcs[7]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_1_1_0, attrset, funcvals8, 
                           "test_func_1_1_0", &(funcs[8]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_1_1_1, attrset, funcvals9, 
                           "test_func_1_1_1", &(funcs[9]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_1_2_0, attrset, funcvals10, 
                           "test_func_1_2_0", &(funcs[10]));
    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_1_2_1, attrset, funcvals11, 
                           "test_func_1_2_1", &(funcs[11]));

    ADCL_Fnctset_create ( 12, funcs, "test functions", &fnctset );
    /* Creation of the topology */
    ADCL_Topology_create_generic ( 0, NULL, NULL, NULL, ADCL_DIRECTION_BOTH, 
				   MPI_COMM_WORLD, &topo );
    /* Creation of the request */
    ADCL_Request_create ( ADCL_VECTOR_NULL, topo, fnctset, &request );

    for ( i=0; i<NIT; i++ ) {
        ADCL_Request_start( request );
        if ( rank == 0 ) {
            ADCL_Request_get_curr_function ( request, &function_name, &attrs_names, &attrs_num,
                                             &attrs_values_names, &attrs_values_num );
            printf("Running function is: %s \n", function_name);
            printf("Attr_name  |Val_name   |Val_num\n");
            printf("___________|___________|________\n");
            for (k=0; k<attrs_num; k++) {
                printf("%s|%s |  %d\n", attrs_names[k], attrs_values_names[k], attrs_values_num[k]);
            }
            printf("\n");
            /* Free the names variables */
            for ( k=0; k<attrs_num; k++ ) {
                free ( attrs_names[k] );
                free ( attrs_values_names[k] );
            }
            free ( function_name );
            free ( attrs_names );
            free ( attrs_values_names );
            free ( attrs_values_num );
        }
    }
    /* Free the request object */
    ADCL_Request_free ( &request );
    /* Free the function set object */
    ADCL_Fnctset_free ( &fnctset );
    /* Free the function objects */
    for ( i=0; i<12; i++ ) {
        ADCL_Function_free ( &funcs[i] );
    }
    /* Free the attribute set object */
    ADCL_Attrset_free ( &attrset);
    /* Free the attribute objects */
    for ( i=0; i<3; i++ ) {
        ADCL_Attribute_free ( &attrs[i]);
    }

    ADCL_Finalize ();
    MPI_Finalize ();
    return 0;
}
void test_func_0_0_0 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );

    return;
}
void test_func_0_0_1 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );

    return;
}
void test_func_0_1_0 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );

    return;
}
void test_func_0_1_1 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );

    return;
}
void test_func_0_2_0 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );

    return;
}
void test_func_0_2_1 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );

    return;
}
void test_func_1_0_0 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );

    return;
}
void test_func_1_0_1 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );

    return;
}
void test_func_1_1_0 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );

    return;
}
void test_func_1_1_1 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );

    return;
}
void test_func_1_2_0 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );

    return;
}
void test_func_1_2_1 ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );

    return;
}
