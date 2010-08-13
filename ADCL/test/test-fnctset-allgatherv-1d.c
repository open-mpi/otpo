/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2008-2009      HLRS. All rights reserved.
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

static void allgatherv_test1(int cnt, int dims, int rank, int size, ADCL_Topology topo); 
static void allgatherv_test2(int cnt, int dims, int rank, int size, ADCL_Topology topo); 
static void allgatherv_test3(int cnt, int dims, int rank, int size, ADCL_Topology topo); 
static void allgatherv_test4(int cnt, int dims, int rank, int size, ADCL_Topology topo); 
static void allgatherv_test5(int cnt, int dims, int nc, int rank, int size, ADCL_Topology topo); 

void dump_vector_1D ( double *data, int rank, int dim);
void set_data_1D ( double *data, int rank, int dim); 
int check_data_1D ( double *data, int* rcounts, int *rdispl, int rank, int size);

int main ( int argc, char ** argv ) 
{
    int cnt, dims, err; 
    int rank, size;
    int cdims=0;
    int periods=0;
    int nc; 

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

    cnt = 11;
    dims = 13;

    /* AllGather with Vector_allocate */ 
    //allgatherv_test1(cnt, dims, rank, size, topo);

    /* true AllGatherV with Vector_allocate */ 
    //allgatherv_test2(cnt, dims, rank, size, topo);

    /* AllGather with Vector_register */ 
    //allgatherv_test3(cnt, dims, rank, size, topo);

    /* true AllGatherV with Vector_allocate and MPI_IN_PLACE */ 
    //allgatherv_test4(cnt, dims, rank, size, topo);

    dims=4; nc=2;
    /* AllGather with Vector_allocate */ 
    allgatherv_test5(cnt, dims, nc, rank, size, topo);

exit:
    if ( ADCL_TOPOLOGY_NULL != topo)   ADCL_Topology_free ( &topo );
    MPI_Comm_free ( &cart_comm );
    ADCL_Finalize ();
    MPI_Finalize ();
    return 0;
}

/**********************************************************************/
/**********************************************************************/
void allgatherv_test1(int cnt, int dims, int rank, int size, ADCL_Topology topo)
/**********************************************************************/
/**********************************************************************/
{
    double *sdata, *rdata;
    int sdim, rdim, i; 
    int* rcnts, *displ;
    int err, errc; 
    ADCL_Vector svec, rvec;
    ADCL_Vmap svmap, rvmap;
    ADCL_Request request;
 
    sdim = dims;
    rdim = dims*size;

    rcnts = (int*) calloc ( size, sizeof(int) ); 
    displ = (int*) calloc ( size, sizeof(int) );
    for ( i=0;i<size;i++){
       rcnts[i] = dims;
       displ[i] = dims * i; 
    }

    err = ADCL_Vmap_all_allocate( &svmap ); 
    if ( ADCL_SUCCESS != err) goto exit;   
    ADCL_Vmap_list_allocate( size, rcnts, displ, &rvmap ); 
    if ( ADCL_SUCCESS != err) goto exit;   

    err = ADCL_Vector_allocate_generic ( 1,  &sdim, 0, svmap, MPI_DOUBLE, &sdata, &svec );
    if ( ADCL_SUCCESS != err) goto exit;   
    err = ADCL_Vector_allocate_generic ( 1,  &rdim, 0, rvmap, MPI_DOUBLE, &rdata, &rvec );
    if ( ADCL_SUCCESS != err) goto exit;   

    err = ADCL_Request_create_generic ( svec, rvec, topo, ADCL_FNCTSET_ALLGATHERV, &request );
    if ( ADCL_SUCCESS != err) goto exit;   

    for (i=0; i<cnt; i++){
       set_data_1D ( sdata, rank, sdim);
       set_data_1D ( rdata, -1, rdim);

#ifdef VERBOSE
       dump_vector_1D ( sdata, rank, sdim);
       dump_vector_1D ( rdata, rank, rdim);
#endif

       err = ADCL_Request_start( request );
       if ( ADCL_SUCCESS != err) goto exit;   

       errc = check_data_1D ( rdata, rcnts, displ, rank, size);
       if (errc) goto exit;   
    }

    MPI_Barrier ( MPI_COMM_WORLD);

exit:
    if ( ADCL_SUCCESS != err) { printf("ADCL error nr. %d\n", err); } 

    if ( NULL != rcnts) free(rcnts);
    if ( NULL != displ) free(displ);
    if ( ADCL_REQUEST_NULL != request) ADCL_Request_free ( &request );
    if ( ADCL_VECTOR_NULL  != svec)    ADCL_Vector_free ( &svec );
    if ( ADCL_VECTOR_NULL  != rvec)    ADCL_Vector_free ( &rvec );
    if ( ADCL_VMAP_NULL    != svmap)   ADCL_Vmap_free (&svmap);
    if ( ADCL_VMAP_NULL    != rvmap)   ADCL_Vmap_free (&rvmap);

    return;
}

/**********************************************************************/
/**********************************************************************/
void allgatherv_test2(int cnt, int dims, int rank, int size, ADCL_Topology topo)
/**********************************************************************/
/**********************************************************************/
{
    double *sdata, *rdata;
    int sdim, rdim, i; 
    int* rcnts, *displ;
    int err, errc, offset; 
    ADCL_Vector svec, rvec;
    ADCL_Vmap svmap, rvmap;
    ADCL_Request request;
    
    sdim = dims+rank;

    rdim = dims*size + size*(size-1)/2;

    rcnts = (int*) calloc ( size, sizeof(int) ); 
    displ = (int*) calloc ( size, sizeof(int) );
    offset = 0; 
    for ( i=0;i<size;i++){
       rcnts[i] = dims+i;
       displ[i] = offset;
       offset += rcnts[i];
    }

    err = ADCL_Vmap_all_allocate( &svmap ); 
    if ( ADCL_SUCCESS != err) goto exit;   
    ADCL_Vmap_list_allocate( size, rcnts, displ, &rvmap ); 
    if ( ADCL_SUCCESS != err) goto exit;   

    err = ADCL_Vector_allocate_generic ( 1,  &sdim, 0, svmap, MPI_DOUBLE, &sdata, &svec );
    if ( ADCL_SUCCESS != err) goto exit;   
    err = ADCL_Vector_allocate_generic ( 1,  &rdim, 0, rvmap, MPI_DOUBLE, &rdata, &rvec );
    if ( ADCL_SUCCESS != err) goto exit;   

    err = ADCL_Request_create_generic ( svec, rvec, topo, ADCL_FNCTSET_ALLGATHERV, &request );
    if ( ADCL_SUCCESS != err) goto exit;   

    for (i=0; i<cnt; i++){
       set_data_1D ( sdata, rank, sdim);
       set_data_1D ( rdata, -1, rdim);

#ifdef VERBOSE
       dump_vector_1D ( sdata, rank, sdim);
       dump_vector_1D ( rdata, rank, rdim);
#endif

       err = ADCL_Request_start( request );
       if ( ADCL_SUCCESS != err) goto exit;   

       errc = check_data_1D ( rdata, rcnts, displ, rank, size);
       if (errc) goto exit;   
    }

    MPI_Barrier ( MPI_COMM_WORLD);

exit:
    if ( ADCL_SUCCESS != err) { printf("ADCL error nr. %d\n", err); } 

    if ( NULL != rcnts) free(rcnts);
    if ( NULL != displ) free(displ);
    if ( ADCL_REQUEST_NULL != request) ADCL_Request_free ( &request );
    if ( ADCL_VECTOR_NULL  != svec)    ADCL_Vector_free ( &svec );
    if ( ADCL_VECTOR_NULL  != rvec)    ADCL_Vector_free ( &rvec );
    if ( ADCL_VMAP_NULL    != svmap)   ADCL_Vmap_free (&svmap);
    if ( ADCL_VMAP_NULL    != rvmap)   ADCL_Vmap_free (&rvmap);
    return;
}

/**********************************************************************/
/**********************************************************************/
void allgatherv_test3(int cnt, int dims, int rank, int size, ADCL_Topology topo)
/**********************************************************************/
/**********************************************************************/
{
    double *sdata, *rdata;
    int sdim, rdim, i; 
    int* rcnts, *displ;
    int err, errc; 
    ADCL_Vector svec, rvec;
    ADCL_Vmap svmap, rvmap;
    ADCL_Request request;
    

 
    sdim = dims;
    rdim = dims*size;
    sdata = (double*) calloc(sdim, sizeof(double));
    rdata = (double*) calloc(rdim, sizeof(double));

    set_data_1D ( sdata, -5, sdim);
    set_data_1D ( rdata, -10, rdim);

    rcnts = (int*) calloc ( size, sizeof(int) ); 
    displ = (int*) calloc ( size, sizeof(int) );
    for ( i=0;i<size;i++){
       rcnts[i] = dims;
       displ[i] = dims * i; 
    }

    err = ADCL_Vmap_all_allocate( &svmap ); 
    if ( ADCL_SUCCESS != err) goto exit;   
    ADCL_Vmap_list_allocate( size, rcnts, displ, &rvmap ); 
    if ( ADCL_SUCCESS != err) goto exit;   

    err = ADCL_Vector_register_generic ( 1,  &sdim, 0, svmap, MPI_DOUBLE, sdata, &svec );
    if ( ADCL_SUCCESS != err) goto exit;   
    err = ADCL_Vector_register_generic ( 1,  &rdim, 0, rvmap, MPI_DOUBLE, rdata, &rvec );
    if ( ADCL_SUCCESS != err) goto exit;   

    err = ADCL_Request_create_generic ( svec, rvec, topo, ADCL_FNCTSET_ALLGATHERV, &request );
    if ( ADCL_SUCCESS != err) goto exit;   

    for (i=0; i<cnt; i++){
       set_data_1D ( sdata, rank, sdim);
       set_data_1D ( rdata, -1, rdim);

#ifdef VERBOSE
       dump_vector_1D ( sdata, rank, sdim);
       dump_vector_1D ( rdata, rank, rdim);
#endif

       err = ADCL_Request_start( request );
       if ( ADCL_SUCCESS != err) goto exit;   

       errc = check_data_1D ( rdata, rcnts, displ, rank, size);
       if (errc) goto exit;   
    }

    ADCL_Vector_deregister( &svec );
    ADCL_Vector_deregister( &rvec );

    MPI_Barrier ( MPI_COMM_WORLD);

exit:
    if ( ADCL_SUCCESS != err) { printf("ADCL error nr. %d\n", err); } 

    if ( NULL != sdata) free(sdata);
    if ( NULL != rdata) free(rdata);
    if ( NULL != rcnts) free(rcnts);
    if ( NULL != displ) free(displ);
    if ( ADCL_REQUEST_NULL != request) ADCL_Request_free ( &request );
    if ( ADCL_VECTOR_NULL  != svec)    ADCL_Vector_free ( &svec );
    if ( ADCL_VECTOR_NULL  != rvec)    ADCL_Vector_free ( &rvec );
    if ( ADCL_VMAP_NULL    != svmap)   ADCL_Vmap_free (&svmap);
    if ( ADCL_VMAP_NULL    != rvmap)   ADCL_Vmap_free (&rvmap);

    return;
}

/**********************************************************************/
/**********************************************************************/
void allgatherv_test4(int cnt, int dims, int rank, int size, ADCL_Topology topo)
/**********************************************************************/
/**********************************************************************/
{
    double *data;
    int sdim, rdim, i; 
    int* rcnts, *displ;
    int err, errc, offset; 
    ADCL_Vector svec, rvec;
    ADCL_Vmap svmap, rvmap;
    ADCL_Request request;
    
    sdim = dims+rank;

    rdim = dims*size + size*(size-1)/2;

    rcnts = (int*) calloc ( size, sizeof(int) ); 
    displ = (int*) calloc ( size, sizeof(int) );
    offset = 0; 
    for ( i=0;i<size;i++){
       rcnts[i] = dims+i;
       displ[i] = offset;
       offset += rcnts[i];
    }

    err = ADCL_Vmap_inplace_allocate( &svmap );
    if ( ADCL_SUCCESS != err) goto exit;
    ADCL_Vmap_list_allocate( size, rcnts, displ, &rvmap ); 
    if ( ADCL_SUCCESS != err) goto exit;   

    err = ADCL_Vector_allocate_generic ( 0, NULL, 0, svmap, MPI_DATATYPE_NULL, NULL, &svec );
    if ( ADCL_SUCCESS != err) goto exit;
    err = ADCL_Vector_allocate_generic ( 1,  &rdim, 0, rvmap, MPI_DOUBLE, &data, &rvec );
    if ( ADCL_SUCCESS != err) goto exit;   

    err = ADCL_Request_create_generic ( svec, rvec, topo, ADCL_FNCTSET_ALLGATHERV, &request );
    if ( ADCL_SUCCESS != err) goto exit;   

    for (i=0; i<cnt; i++){
       set_data_1D ( data, -1, rdim);
       set_data_1D ( &(data[displ[rank]]), rank, sdim);

#ifdef VERBOSE
       dump_vector_1D ( data, rank, sdim);
#endif

       err = ADCL_Request_start( request );
       if ( ADCL_SUCCESS != err) goto exit;   

       errc = check_data_1D ( data, rcnts, displ, rank, size);
       if (errc) goto exit;   
    }

    MPI_Barrier ( MPI_COMM_WORLD);

exit:
    if ( ADCL_SUCCESS != err) { printf("ADCL error nr. %d\n", err); } 

    if ( NULL != rcnts) free(rcnts);
    if ( NULL != displ) free(displ);
    if ( ADCL_REQUEST_NULL != request) ADCL_Request_free ( &request );
    if ( ADCL_VECTOR_NULL  != svec)    ADCL_Vector_free ( &svec );
    if ( ADCL_VECTOR_NULL  != rvec)    ADCL_Vector_free ( &rvec );
    if ( ADCL_VMAP_NULL    != svmap)   ADCL_Vmap_free (&svmap);
    if ( ADCL_VMAP_NULL    != rvmap)   ADCL_Vmap_free (&rvmap);
    return;
}

/**********************************************************************/
/**********************************************************************/
void allgatherv_test5(int cnt, int dims, int nc, int rank, int size, 
   ADCL_Topology topo)
/**********************************************************************/
/**********************************************************************/
{
    double *sdata, *rdata;
    int sdim, rdim, i, stdim, rtdim; 
    int* rcnts, *displ;
    int err, errc; 
    ADCL_Vector svec, rvec;
    ADCL_Vmap svmap, rvmap;
    ADCL_Request request;
 
    sdim = dims;
    rdim = dims*size;

    rcnts = (int*) calloc ( size, sizeof(int) ); 
    displ = (int*) calloc ( size, sizeof(int) );
    for ( i=0;i<size;i++){
       rcnts[i] = dims*nc;
       displ[i] = dims*nc * i; 
    }

    err = ADCL_Vmap_all_allocate( &svmap ); 
    if ( ADCL_SUCCESS != err) goto exit;   
    ADCL_Vmap_list_allocate( size, rcnts, displ, &rvmap ); 
    if ( ADCL_SUCCESS != err) goto exit;   

    stdim = sdim*nc;
    rtdim = rdim*nc;
    err = ADCL_Vector_allocate_generic ( 1,  &stdim, 0, svmap, MPI_DOUBLE, &sdata, &svec );
    if ( ADCL_SUCCESS != err) goto exit;   
    err = ADCL_Vector_allocate_generic ( 1,  &rtdim, 0, rvmap, MPI_DOUBLE, &rdata, &rvec );
    if ( ADCL_SUCCESS != err) goto exit;   

    err = ADCL_Request_create_generic ( svec, rvec, topo, ADCL_FNCTSET_ALLGATHERV, &request );
    if ( ADCL_SUCCESS != err) goto exit;   

    for (i=0; i<cnt; i++){
       set_data_1D ( sdata, rank, sdim*nc);
       set_data_1D ( rdata, -1,   rdim*nc);

#ifdef VERBOSE
       dump_vector_1D ( sdata, rank, sdim*nc);
       dump_vector_1D ( rdata, rank, rdim*nc);
#endif

       err = ADCL_Request_start( request );
       if ( ADCL_SUCCESS != err) goto exit;   

       errc = check_data_1D ( rdata, rcnts, displ, rank, size);
       if (errc) goto exit;   
    }

    MPI_Barrier ( MPI_COMM_WORLD);

exit:
    if ( ADCL_SUCCESS != err) { printf("ADCL error nr. %d\n", err); } 

    if ( NULL != rcnts) free(rcnts);
    if ( NULL != displ) free(displ);
    if ( ADCL_REQUEST_NULL != request) ADCL_Request_free ( &request );
    if ( ADCL_VECTOR_NULL  != svec)    ADCL_Vector_free ( &svec );
    if ( ADCL_VECTOR_NULL  != rvec)    ADCL_Vector_free ( &rvec );
    if ( ADCL_VMAP_NULL    != svmap)   ADCL_Vmap_free (&svmap);
    if ( ADCL_VMAP_NULL    != rvmap)   ADCL_Vmap_free (&rvmap);

    return;
}


