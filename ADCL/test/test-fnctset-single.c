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

static void init_test_func ( ADCL_Request req );
static void wait_test_func ( ADCL_Request req );

int main ( int argc, char ** argv ) 
{
    /* General variables */
    int i, k, attrs_num, *attrs_values_num, rank, size;
    char *function_name;
    char **attrs_names;
    char **attrs_values_names;

    ADCL_Attribute attrs[3];
    ADCL_Attrset attrset;
    ADCL_Fnctset fnctset;
    ADCL_Topology topo;
    ADCL_Request request;

    int attr1vals[2]={10,11};
    int attr2vals[3]={20,21,22};
    int attr3vals[2]={30,31};

    char* attr1vals_names[2]={"attr1_val0","attr1_val1"};
    char* attr2vals_names[3]={"attr2_val0","attr2_val1","attr2_val2"};
    char* attr3vals_names[2]={"attr3_val0","attr3_val1"};

    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    ADCL_Init ();

    /* Creation of the attribute set */
    ADCL_Attribute_create ( 2, attr1vals, attr1vals_names, "first_attr ", &(attrs[0]));
    ADCL_Attribute_create ( 3, attr2vals, attr2vals_names, "second_attr", &(attrs[1]));
    ADCL_Attribute_create ( 2, attr3vals, attr3vals_names, "third_attr ", &(attrs[2]));
    ADCL_Attrset_create ( 3, attrs, &attrset );

    /* Creation of the function set from a single function */
    ADCL_Fnctset_create_single_fnct_async ( (ADCL_work_fnct_ptr *)init_test_func,
                                            (ADCL_work_fnct_ptr *)wait_test_func,
                                             attrset, "test function", NULL, 0,
                                             &fnctset );
    /* Creation of the topology */
    ADCL_Topology_create_generic ( 0, NULL, NULL, NULL, ADCL_DIRECTION_BOTH, 
				   MPI_COMM_WORLD, &topo );
    /* Creation of the request */
    ADCL_Request_create ( ADCL_VECTOR_NULL, topo, fnctset, &request );

    for ( i=0; i<NIT; i++ ) {
        ADCL_Request_start( request );

        ADCL_Request_get_curr_function ( request, &function_name, &attrs_names, &attrs_num,
                                         &attrs_values_names, &attrs_values_num );
        if ( rank == 0 ) {
            printf("Running function is: %s \n", function_name);
            printf("Attr_name  |Val_name   |Val_num\n");
            printf("___________|___________|________\n");
            for (k=0; k<attrs_num; k++) {
                printf("%s|%s |  %d\n", attrs_names[k], attrs_values_names[k], attrs_values_num[k]);
            }
            printf("\n");
        }
            /* don't forget to free the names variables */
            for ( k=0; k<attrs_num; k++ ) {
                free ( attrs_names[k] );
                free ( attrs_values_names[k] );
            }
	
            free ( function_name );
            free ( attrs_names );
            free ( attrs_values_names );
            free ( attrs_values_num );

    }


    /* Free the request object */
    ADCL_Request_free ( &request );
    /* Free the topology object */
    ADCL_Topology_free ( &topo );
    /* Free the function set object */
    ADCL_Fnctset_free ( &fnctset );
    /* Free the attribute set object */
    ADCL_Attrset_free ( &attrset );
    /* Free the attribute objects */
    for ( i=0; i<3; i++ ) {
        ADCL_Attribute_free ( &attrs[i]);
    }

    ADCL_Finalize ();
    MPI_Finalize ();
    return 0;
}
void init_test_func ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );

    return;
}
void wait_test_func ( ADCL_Request req )
{
    MPI_Comm comm;
    int rank, size;

    ADCL_Request_get_comm ( req, &comm, &rank, &size );

    return;
}
