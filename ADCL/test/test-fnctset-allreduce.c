/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include <stdio.h>
#include <string.h>

#include "ADCL.h"
#include "ADCL_internal.h"
#include "mpi.h"

static void allreduce_test1(int cnt, int dims, int rank, int size, ADCL_Topology topo); 
static void allreduce_test2(int cnt, int dims, int rank, int size, ADCL_Topology topo); 
static void allreduce_test3(int cnt, int dims, int rank, int size, ADCL_Topology topo); 
static void allreduce_test4(int cnt, int dims, int rank, int size, ADCL_Topology topo); 

static void dump_vector_double    ( double *data, int rank, int dim);
static void set_data_double       ( double *data, int rank, int dim); 
static int check_data_double_sum ( double *data, int rank, int dim, int size);
static void dump_vector_int       ( int *data, int rank, int dim);
static void set_data_int          ( int *data, int rank, int dim); 
static int check_data_int_min    ( int *data, int rank, int dim, int size);

int main ( int argc, char ** argv ) 
{
    int cnt, dims, err; 
    int rank, size;
    int cdims=0;
    int periods=0;

    ADCL_Topology topo;
    MPI_Comm cart_comm;

    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    ADCL_Init ();

    MPI_Dims_create ( size, 1, &cdims );
    MPI_Cart_create ( MPI_COMM_WORLD, 1, &cdims, &periods, 0, &cart_comm);

    err = ADCL_Topology_create ( cart_comm, &topo );
    if ( ADCL_SUCCESS != err) goto exit;   

    cnt = 20; //200;
    dims = 3;

    /* MPI_DOUBLE, MPI_SUM, Vector_allocate */
    //allreduce_test1(cnt, dims, rank, size, topo);

    /* MPI_INT, MPI_MIN, Vector_register */
    //allreduce_test2(cnt, dims, rank, size, topo);

    /* MPI_DOUBLE, MPI_SUM, Vector_allocate, MPI_INPLACE */
    //allreduce_test3(cnt, dims, rank, size, topo);

    /* MPI_INT, MPI_MIN, Vector_register, MPI_IN_PLACE */ 
    allreduce_test4(cnt, dims, rank, size, topo);

exit:
    if ( ADCL_TOPOLOGY_NULL != topo)   ADCL_Topology_free ( &topo );
    MPI_Comm_free ( &cart_comm );
    ADCL_Finalize ();
    MPI_Finalize ();
    return 0;
}

/**********************************************************************/
/**********************************************************************/
void allreduce_test1(int cnt, int dim, int rank, int size, ADCL_Topology topo)
/**********************************************************************/
/**********************************************************************/
{
    double *sdata, *rdata;
    int i, err, cerr; 
    ADCL_Vector svec, rvec;
    ADCL_Vmap svmap, rvmap;
    ADCL_Request request;
    
    err = ADCL_Vmap_allreduce_allocate( MPI_SUM, &svmap ); 
    if ( ADCL_SUCCESS != err) goto exit;   
    err = ADCL_Vmap_allreduce_allocate( MPI_SUM, &rvmap ); 
    if ( ADCL_SUCCESS != err) goto exit;   

    err = ADCL_Vector_allocate_generic ( 1,  &dim, 0, svmap, MPI_DOUBLE, &sdata, &svec );
    if ( ADCL_SUCCESS != err) goto exit;   
    err = ADCL_Vector_allocate_generic ( 1,  &dim, 0, rvmap, MPI_DOUBLE, &rdata, &rvec );
    if ( ADCL_SUCCESS != err) goto exit;   

    err = ADCL_Request_create_generic ( svec, rvec, topo, ADCL_FNCTSET_ALLREDUCE, &request );
    if ( ADCL_SUCCESS != err) goto exit;   

    for (i=0; i<cnt; i++){
       set_data_double ( sdata, rank, dim);
       set_data_double ( rdata, -1,   dim);

#ifdef VERBOSE
       dump_vector_double ( sdata, rank, dim);
       dump_vector_double ( rdata, rank, dim);
#endif

       err = ADCL_Request_start( request );
       if ( ADCL_SUCCESS != err) goto exit;   

       cerr = check_data_double_sum ( rdata, rank, dim, size);
       if (cerr) goto exit;   
    }

    MPI_Barrier ( MPI_COMM_WORLD);

exit:
    if ( ADCL_SUCCESS != err) { printf("ADCL error nr. %d\n", err); } 

    if ( ADCL_REQUEST_NULL != request) ADCL_Request_free ( &request );
    if ( ADCL_VECTOR_NULL  != svec)    ADCL_Vector_free ( &svec );
    if ( ADCL_VECTOR_NULL  != rvec)    ADCL_Vector_free ( &rvec );
    if ( ADCL_VMAP_NULL    != svmap)   ADCL_Vmap_free (&svmap);
    if ( ADCL_VMAP_NULL    != rvmap)   ADCL_Vmap_free (&rvmap);

   return;
}

/**********************************************************************/
/**********************************************************************/
void allreduce_test2(int cnt, int dim, int rank, int size, ADCL_Topology topo)
/**********************************************************************/
/**********************************************************************/
{
    int *sdata, *rdata;
    int i, err, cerr; 
    ADCL_Vector svec, rvec;
    ADCL_Vmap svmap, rvmap;
    ADCL_Request request;
    
    err = ADCL_Vmap_allreduce_allocate( MPI_MIN, &svmap ); 
    if ( ADCL_SUCCESS != err) goto exit;   
    err = ADCL_Vmap_allreduce_allocate( MPI_MIN, &rvmap ); 
    if ( ADCL_SUCCESS != err) goto exit;   

    sdata = (int *) calloc(dim, sizeof(int));
    rdata = (int *) calloc(dim, sizeof(int));

    err = ADCL_Vector_register_generic ( 1,  &dim, 0, svmap, MPI_INT, sdata, &svec );
    if ( ADCL_SUCCESS != err) goto exit;   
    err = ADCL_Vector_register_generic ( 1,  &dim, 0, rvmap, MPI_INT, rdata, &rvec );
    if ( ADCL_SUCCESS != err) goto exit;   

    err = ADCL_Request_create_generic ( svec, rvec, topo, ADCL_FNCTSET_ALLREDUCE, &request );
    if ( ADCL_SUCCESS != err) goto exit;   

    for (i=0; i<cnt; i++){
       set_data_int ( sdata, size-rank, dim);
       set_data_int ( rdata, -1,   dim);

#ifdef VERBOSE
       dump_vector_int ( sdata, rank, dim);
       dump_vector_int ( rdata, rank, dim);
#endif

       err = ADCL_Request_start( request );
       if ( ADCL_SUCCESS != err) goto exit;   

       cerr = check_data_int_min ( rdata, rank, dim, size);
       if (cerr) goto exit;
    }

    MPI_Barrier ( MPI_COMM_WORLD);

exit:
    if ( ADCL_SUCCESS != err) { printf("ADCL error nr. %d\n", err); } 

    if ( ADCL_REQUEST_NULL != request) ADCL_Request_free ( &request );
    if ( ADCL_VECTOR_NULL  != svec)    ADCL_Vector_deregister ( &svec );
    if ( ADCL_VECTOR_NULL  != rvec)    ADCL_Vector_deregister ( &rvec );
    if ( ADCL_VMAP_NULL    != svmap)   ADCL_Vmap_free (&svmap);
    if ( ADCL_VMAP_NULL    != rvmap)   ADCL_Vmap_free (&rvmap);
    free(sdata);
    free(rdata);
    return;
}

/**********************************************************************/
/**********************************************************************/
void allreduce_test3(int cnt, int dim, int rank, int size, ADCL_Topology topo)
/**********************************************************************/
/**********************************************************************/
{
    double *data;
    int i, err, cerr; 
    ADCL_Vector svec, rvec;
    ADCL_Vmap svmap, rvmap;
    ADCL_Request request;
    
    err = ADCL_Vmap_inplace_allocate( &svmap ); 
    if ( ADCL_SUCCESS != err) goto exit;
    err = ADCL_Vmap_allreduce_allocate( MPI_SUM, &rvmap ); 
    if ( ADCL_SUCCESS != err) goto exit;

    err = ADCL_Vector_allocate_generic ( 0, NULL, 0, svmap, MPI_DATATYPE_NULL, NULL, &svec );
    if ( ADCL_SUCCESS != err) goto exit;   
    err = ADCL_Vector_allocate_generic ( 1, &dim, 0, rvmap, MPI_DOUBLE, &data, &rvec );
    if ( ADCL_SUCCESS != err) goto exit;   

    err = ADCL_Request_create_generic ( svec, rvec, topo, ADCL_FNCTSET_ALLREDUCE, &request );
    if ( ADCL_SUCCESS != err) goto exit;   

    for (i=0; i<cnt; i++){
       set_data_double ( data, rank, dim);

#ifdef VERBOSE
       dump_vector_double ( data, rank, dim);
#endif

       err = ADCL_Request_start( request );
       if ( ADCL_SUCCESS != err) goto exit;   

       cerr = check_data_double_sum ( data, rank, dim, size);
       if (cerr) goto exit;   
    }

    MPI_Barrier ( MPI_COMM_WORLD);

exit:
    if ( ADCL_SUCCESS != err) { printf("ADCL error nr. %d\n", err); } 

    if ( ADCL_REQUEST_NULL != request) ADCL_Request_free ( &request );
    if ( ADCL_VECTOR_NULL  != svec)     ADCL_Vector_free ( &svec );
    if ( ADCL_VECTOR_NULL  != rvec)     ADCL_Vector_free ( &rvec );
    if ( ADCL_VMAP_NULL    != svmap)    ADCL_Vmap_free (&svmap);
    if ( ADCL_VMAP_NULL    != rvmap)    ADCL_Vmap_free (&rvmap);

   return;
}

/**********************************************************************/
/**********************************************************************/
void allreduce_test4(int cnt, int dim, int rank, int size, ADCL_Topology topo)
/**********************************************************************/
/**********************************************************************/
{
    int *data;
    int i, err, cerr; 
    ADCL_Vector svec, rvec;
    ADCL_Vmap svmap, rvmap;
    ADCL_Request request;

    data = (int*) calloc(dim, sizeof(int));

    err = ADCL_Vmap_inplace_allocate( &svmap ); 
    if ( ADCL_SUCCESS != err) goto exit;
    err = ADCL_Vmap_allreduce_allocate( MPI_MIN, &rvmap ); 
    if ( ADCL_SUCCESS != err) goto exit;

    err = ADCL_Vector_register_generic ( 0,  NULL, 0, svmap, MPI_DATATYPE_NULL, MPI_IN_PLACE, &svec );
    if ( ADCL_SUCCESS != err) goto exit;   
    err = ADCL_Vector_register_generic ( 1,  &dim, 0, rvmap, MPI_DOUBLE, data, &rvec );
    if ( ADCL_SUCCESS != err) goto exit;   

    err = ADCL_Request_create_generic ( svec, rvec, topo, ADCL_FNCTSET_ALLREDUCE, &request );
    if ( ADCL_SUCCESS != err) goto exit;   

    for (i=0; i<cnt; i++){
       set_data_int ( data, size-rank, dim);

#ifdef VERBOSE
       dump_vector_int ( data, rank, dim);
#endif

       err = ADCL_Request_start( request );
       if ( ADCL_SUCCESS != err) goto exit;   

       cerr = check_data_int_min ( data, rank, dim, size);
       if (cerr) goto exit;
    }

    MPI_Barrier ( MPI_COMM_WORLD);

exit:
    if ( ADCL_SUCCESS != err) { printf("ADCL error nr. %d\n", err); } 

    if ( ADCL_REQUEST_NULL != request) ADCL_Request_free ( &request );
    if ( ADCL_VECTOR_NULL  != svec)    ADCL_Vector_deregister ( &svec );
    if ( ADCL_VECTOR_NULL  != rvec)    ADCL_Vector_deregister ( &rvec );
    if ( ADCL_VMAP_NULL    != svmap)   ADCL_Vmap_free (&svmap);
    if ( ADCL_VMAP_NULL    != rvmap)   ADCL_Vmap_free (&rvmap);
    free(data);

    return;
}

/**********************************************************************/
/**********************************************************************/
int check_data_double_sum ( double *data, int rank, int dim, int size) 
/**********************************************************************/
/**********************************************************************/
{
    int i; 
    int err = 0, gerr = 0; 

    for ( i=0; i<dim; i++ ){ 
       if (data[i] != (size * (size-1))/2){
               printf("Wrong data: proc %d, pos %d, value %lf\n", 
	          rank, i, data[i]);
	       err++;
       }
    }

    MPI_Allreduce ( &err, &gerr, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
    if ( gerr == 0 ) {
	//if ( rank == 0 ) printf("1-D testsuite passed\n");
    }
    else {
	if ( rank == 0 ) printf("1-D testsuite failed\n");
        err = 1;
    }

    return err;
}

/**********************************************************************/
/**********************************************************************/
int check_data_int_min ( int *data, int rank, int dim, int size) 
/**********************************************************************/
/**********************************************************************/
{
    int i; 
    int err = 0, gerr = 0; 

    for ( i=0; i<dim; i++ ){ 
       if (data[i] != 1){
               printf("Wrong data: proc %d, pos %d, value %d\n", 
	          rank, i, data[i]);
	       err++;
       }
    }

    MPI_Allreduce ( &err, &gerr, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
    if ( gerr == 0 ) {
	//if ( rank == 0 ) printf("1-D testsuite passed\n");
    }
    else {
	if ( rank == 0 ) printf("1-D testsuite failed\n");
        err = 1;
    }

    return err;
}

/**********************************************************************/
/**********************************************************************/
static void set_data_double ( double *data, int rank, int dim) 
/**********************************************************************/
/**********************************************************************/
{
    int i;

    for ( i=0; i<dim; i++) {
	data[i] = rank;
    }

    return;
}

/**********************************************************************/
/**********************************************************************/
static void dump_vector_double ( double *data, int rank, int dim)
/**********************************************************************/
/**********************************************************************/
{
    int i;
    
    printf("%d : ", rank);
    for (i=0; i<dim; i++) {
	printf("%lf ", data[i]);
    }
    printf ("\n");

    return;
}

/**********************************************************************/
/**********************************************************************/
static void set_data_int ( int *data, int rank, int dim) 
/**********************************************************************/
/**********************************************************************/
{
    int i;

    for ( i=0; i<dim; i++) {
	data[i] = rank;
    }

    return;
}

/**********************************************************************/
/**********************************************************************/
static void dump_vector_int ( int *data, int rank, int dim)
/**********************************************************************/
/**********************************************************************/
{
    int i;
    
    printf("%d : ", rank);
    for (i=0; i<dim; i++) {
	printf("%d ", data[i]);
    }
    printf ("\n");

    return;
}


