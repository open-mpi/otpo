/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2007           Cisco, Inc. All rights reserved.
 * Copyright (c) 2009           HLRS. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef __ADCL_H__
#define __ADCL_H__
#include <stdio.h>
#include "ADCL_config.h"

#ifdef ADCL_DUMMY_MPI
#include "ADCL_dummy_mpi.h"
#else
#include "mpi.h"
#endif

/* define constants and error codes */
#define ADCL_SUCCESS        0
#define ADCL_NO_MEMORY      -1
#define ADCL_ERROR_INTERNAL -2
#define ADCL_USER_ERROR     -3
#define ADCL_UNDEFINED      -4
#define ADCL_NOT_FOUND      -5
#define ADCL_CHANGE_OCCURED -6

#define ADCL_INVALID_ARG       10
#define ADCL_INVALID_NDIMS     11
#define ADCL_INVALID_DIMS      12
#define ADCL_INVALID_HWIDTH    13
#define ADCL_INVALID_DAT       14
#define ADCL_INVALID_DATA      15
#define ADCL_INVALID_VECTYPE   16
#define ADCL_INVALID_COMM      17
#define ADCL_INVALID_REQUEST   18
#define ADCL_INVALID_NC        19
#define ADCL_INVALID_TYPE      20
#define ADCL_INVALID_TOPOLOGY  21
#define ADCL_INVALID_ATTRIBUTE 22
#define ADCL_INVALID_ATTRSET   23
#define ADCL_INVALID_FUNCTION  24
#define ADCL_INVALID_WORK_FUNCTION_PTR  25
#define ADCL_INVALID_FNCTSET   26
#define ADCL_INVALID_VECTOR    27
#define ADCL_INVALID_VECTSET   28
#define ADCL_INVALID_DIRECTION 29
#define ADCL_INVALID_VMAP      30
#define ADCL_INVALID_OP        31

#ifdef ADCL_PAPI
#define ADCL_INVALID_PAPI      32
#endif

#define ADCL_VECTOR_NULL    (void*) -1
#define ADCL_VECTSET_NULL   (void*) -2
#define ADCL_REQUEST_NULL   (void*) -3
#define ADCL_TOPOLOGY_NULL  (void*) -4
#define ADCL_ATTRIBUTE_NULL (void*) -5
#define ADCL_ATTRSET_NULL   (void*) -6
#define ADCL_FUNCTION_NULL  (void*) -7
#define ADCL_FNCTSET_NULL   (void*) -8
#define ADCL_NULL_FNCT_PTR  (void*) -9
#define ADCL_VMAP_NULL      (void*) -10
#define ADCL_VECTYPE_NULL           -11

#ifdef ADCL_PAPI
#define ADCL_PAPI_NULL      (void*) -15
#endif

#define ADCL_MAX_ATTRLEN 32
#define ADCL_MAX_NAMELEN 32

#define ADCL_DIRECTION_BOTH          1
#define ADCL_DIRECTION_LEFT_TO_RIGHT 2
#define ADCL_DIRECTION_RIGHT_TO_LEFT 3

/* ADCL vector types */
#define ADCL_VECTOR_HALO       1
#define ADCL_VECTOR_ALL        2
#define ADCL_VECTOR_UP_TRIANG  3
#define ADCL_VECTOR_LO_TRIANG  4
#define ADCL_VECTOR_LIST       5 
#define ADCL_VECTOR_ALLREDUCE  6 
#define ADCL_VECTOR_INPLACE    7 
#define ADCL_VECTOR_ALLTOALL   8
#define ADCL_VECTOR_REDUCE     9 

/* ADCL timer options */
#define TIMER_GETTIMEOFDAY    0
#define TIMER_MPI_WTIME       1
#define TIMER_PAPI_REAL_USEC  2

/* Selection algorithms */
#define ADCL_BRUTE_FORCE      0
#define ADCL_PERF_HYPO        1
#define ADCL_TWOK_FACTORIAL   2

/* ADCL prediction algorithm options */
#define ADCL_CLOSEST  0
#define ADCL_WMV      1
#define ADCL_SVM      2

/* Define ADCL public structures */
struct ADCL_hist_s{
    int                     h_id; /* id of the object */
    int                 h_findex; /* index of this object in the fortran array */
    int                 h_refcnt; /* reference counter of this object */
    /* Network Topology information */
    int                     h_np; /* Number of processors */
    /* Logical Topology information */
    int                 h_tndims; /* Topology number of dimensions */
    int              *h_tperiods; /* periodicity for each cartesian dimension */
    /* Vector information */
    int                 h_vndims; /* Vector number of dimensions */
    int                 *h_vdims; /* Vector extent of each the dimensions */
    int                     h_nc; /* Extent of each data point  */
   /* Vector map information */
    int                h_vectype; /* Vector type */
    int                 h_hwidth; /* Halo cells width */
    int                 *h_rcnts; /* receive counts for AllGatherV */ 
    int                 *h_displ; /* displacements for AllGatherV */
    MPI_Op                  h_op; /* MPI operator for AllReduce */
    int                h_inplace; /* MPI_IN_PLACE */
    /* Attribute information */
    int               h_asmaxnum; /* Number of attributes in the attribute set */
    int              *h_attrvals; /* Values of the winning attributes */
    /* Function set and winner function */
    char               *h_fsname; /* Function set name */
    char               *h_wfname; /* Winner function name */
    int                  h_wfnum; /* Winner function number */
    int                  h_fsnum; /* Number of available functions in the function set */
    /* Performance Data */
    double               *h_perf; /* Array of performance data of size = number of functions */
    int                 *h_class; /* Class of the function/implementation */
    int               h_perf_win; /* The acceptable perofrmance window used for the clasification
                                     and to compute dmax */
    double            h_distance; /* Distance to the target problem (To be predicted) */
    int               h_relation; /* Relation between the history entry and the target problem */
    double                h_dmax; /* Maximum distance for a given problem size */
};
typedef struct ADCL_hist_s ADCL_hist_t;

struct ADCL_hist_list_s{
    ADCL_hist_t             *hl_curr;
    struct ADCL_hist_list_s *hl_next;
};
typedef struct ADCL_hist_list_s ADCL_hist_list_t;

/* define the object types visible to the user */
typedef struct ADCL_vmap_s*      ADCL_Vmap;
typedef struct ADCL_vector_s*    ADCL_Vector;
typedef struct ADCL_vectset_s*   ADCL_Vectset;
typedef struct ADCL_request_s*   ADCL_Request;
typedef struct ADCL_topology_s*  ADCL_Topology;
typedef struct ADCL_attribute_s* ADCL_Attribute;
typedef struct ADCL_attrset_s*   ADCL_Attrset;
typedef struct ADCL_function_s*  ADCL_Function;
typedef struct ADCL_fnctset_s*   ADCL_Fnctset;
typedef struct ADCL_hist_s*      ADCL_Hist;
#ifdef ADCL_PAPI
typedef struct ADCL_papi_s*      ADCL_Papi;
#endif

/* define predefined functionsets */
extern struct ADCL_fnctset_s *ADCL_neighborhood_fnctset;
extern struct ADCL_fnctset_s *ADCL_allgatherv_fnctset;
extern struct ADCL_fnctset_s *ADCL_allreduce_fnctset;
extern struct ADCL_fnctset_s *ADCL_reduce_fnctset;
extern struct ADCL_fnctset_s *ADCL_alltoallv_fnctset;
extern struct ADCL_fnctset_s *ADCL_alltoall_fnctset;
extern struct ADCL_fnctset_s *ADCL_fnctset_rtol;
extern struct ADCL_fnctset_s *ADCL_fnctset_ltor;

#define ADCL_FNCTSET_NEIGHBORHOOD ADCL_neighborhood_fnctset
#define ADCL_FNCTSET_ALLGATHERV   ADCL_allgatherv_fnctset
#define ADCL_FNCTSET_ALLREDUCE    ADCL_allreduce_fnctset
#define ADCL_FNCTSET_REDUCE       ADCL_reduce_fnctset
#define ADCL_FNCTSET_ALLTOALL     ADCL_alltoall_fnctset
#define ADCL_FNCTSET_ALLTOALLV    ADCL_alltoallv_fnctset
#define ADCL_FNCTSET_SHIFT_LTOR   ADCL_fnctset_shift_ltor
#define ADCL_FNCTSET_SHIFT_RTOL   ADCL_fnctset_shift_rtol

struct ADCL_neighborhood_criteria_s {
    char    *c_fsname; /* Function set name */
    int      c_tndims; /* Dimensions */
};
typedef struct ADCL_neighborhood_criteria_s ADCL_neighborhood_criteria_t;

void ADCL_neighborhood_set_criteria( ADCL_Request req, void *filter_criteria );

#define TIME_TYPE double

/* Prototypes of the User level interface functions */

/* ADCL environment functions */
int ADCL_Init (void );
int ADCL_Finalize (void );

/* ADCL Vmap functions */
int ADCL_Vmap_halo_allocate ( int hwidth, ADCL_Vmap *vec );
int ADCL_Vmap_list_allocate ( int size, int* rcnts, int* displ, ADCL_Vmap *vec );
int ADCL_Vmap_allreduce_allocate ( MPI_Op op, ADCL_Vmap *vec );
int ADCL_Vmap_reduce_allocate ( MPI_Op op, ADCL_Vmap *vec );
int ADCL_Vmap_alltoall_allocate ( int scnt, int rcnt, ADCL_Vmap *vec );
int ADCL_Vmap_all_allocate ( ADCL_Vmap *vec );
int ADCL_Vmap_inplace_allocate ( ADCL_Vmap *vec );
int ADCL_Vmap_free  ( ADCL_Vmap *vec );

/* ADCL Vector functions and ADCL Vectorset functions */
int ADCL_Vector_allocate   ( int ndims, int *dims, int nc, int vectype, int hwidth,
                             MPI_Datatype dat, void *data, ADCL_Vector *vec );
int ADCL_Vector_allocate_generic ( int ndims, int *dims, int nc, ADCL_Vmap vmap,
                           MPI_Datatype dat, void *data, ADCL_Vector *vec ); 
int ADCL_Vector_free       ( ADCL_Vector *vec );
int ADCL_Vector_register   ( int ndims, int *dims, int nc, int vectype, int hwidth,
                             MPI_Datatype dat, void *data, ADCL_Vector *vec );
int ADCL_Vector_register_generic ( int ndims, int *dims, int nc, ADCL_Vmap vmap,
                           MPI_Datatype dat, void *data, ADCL_Vector *vec );
int ADCL_Vector_deregister ( ADCL_Vector *vec );

int ADCL_Vectset_create ( int maxnum,
                          ADCL_Vector  *svecs,
                          ADCL_Vector  *rvecs,
                          ADCL_Vectset *vectset );

int ADCL_Vectset_free   ( ADCL_Vectset *vectset );

/* ADCL Topology functions */
int ADCL_Topology_create_generic ( int ndims, int nneigh, int *lneighbors, int *flip, 
                                   int *rneighbors, int *coords, int direction,
                                   MPI_Comm comm, ADCL_Topology *topo);
int ADCL_Topology_create  ( MPI_Comm cart_comm, ADCL_Topology *topo);
int ADCL_Topology_create_extended ( MPI_Comm cart_comm, ADCL_Topology *topo);
int ADCL_Topology_free    ( ADCL_Topology *topo );
int ADCL_Topology_dump  ( ADCL_Topology topo );

int ADCL_Topology_get_cart_number_neighbors ( int ndims, int extended, int* nneigh );
int ADCL_Topology_get_cart_neighbors ( int nneigh, int* lneighbors, int* rneighbors, 
        int *flip, MPI_Comm cart_comm );

#ifdef ADCL_PAPI
/* ADCL PAPI functions */
int ADCL_Papi_create (ADCL_Papi *papi);
int ADCL_Papi_free   (ADCL_Papi *papi);
int ADCL_Papi_enter  (ADCL_Papi papi);
int ADCL_Papi_leave  (ADCL_Papi papi);
int ADCL_Papi_print  (ADCL_Papi papi);
#endif

/* ADCL Attributes and Attributeset fucntions */
int ADCL_Attribute_create ( int maxnvalues, int *array_of_values, char **values_names,
                            char *attr_name, ADCL_Attribute *attr );
int ADCL_Attribute_free   ( ADCL_Attribute *attr );

int ADCL_Attrset_create   ( int maxnum, ADCL_Attribute *array_of_attributes,
                            ADCL_Attrset *attrset );
int ADCL_Attrset_free     ( ADCL_Attrset *attrset );

/* ADCL History functions types definition */
typedef void ADCL_hist_reader ( FILE *fp, ADCL_Hist hist);
typedef void ADCL_hist_writer ( FILE *fp, ADCL_Hist hist);
typedef int ADCL_hist_filter ( ADCL_Hist hist, void *filter_criteria );
typedef double ADCL_hist_distance ( ADCL_Hist hist1 , ADCL_Hist hist2 );
typedef void ADCL_hist_set_criteria ( ADCL_Request request, void *filter_criteria );

/* Structure holding the hstory functions and related information */
struct ADCL_hist_functions_s{
    ADCL_hist_reader             *hf_reader; /* Hist reading function */
    ADCL_hist_writer             *hf_writer; /* Hist writing function */
    ADCL_hist_filter             *hf_filter; /* Filter function according to the given criteria */
    ADCL_hist_distance         *hf_distance; /* Distance function between two Hist */
};
typedef struct ADCL_hist_functions_s ADCL_hist_functions_t;
typedef struct ADCL_hist_functions_s  *ADCL_Hist_functions;

/* Structure holding the criteria and function to set them */
struct ADCL_hist_criteria_s{

    void                *hc_filter_criteria; /* Filter criteria structure */
    ADCL_hist_set_criteria *hc_set_criteria; /* Function setting the filter criteria structure */
    int                     hc_criteria_set; /* Flag whether the filtering criteria are set */
};
typedef struct ADCL_hist_criteria_s ADCL_hist_criteria_t;
typedef struct ADCL_hist_criteria_s *ADCL_Hist_criteria;

/* ADCL Function and ADCL Functionset functions */
typedef void ADCL_work_fnct_ptr ( ADCL_Request req );

int ADCL_Function_create       ( ADCL_work_fnct_ptr *fnctp,
                                 ADCL_Attrset attrset,
                                 int *array_of_attrvalues,
                                 char *name,
                                 ADCL_Function *fnct);
int ADCL_Function_create_async ( ADCL_work_fnct_ptr *init_fnct,
                                 ADCL_work_fnct_ptr *wait_fnct,
                                 ADCL_Attrset attrset,
                                 int *array_of_attrvalues, char *name,
                                 ADCL_Function *fnct);

int ADCL_Function_free         ( ADCL_Function *fnct );

int ADCL_Fnctset_create ( int maxnum, ADCL_Function *fncts, char *name,
                          ADCL_Fnctset *fnctset );
int ADCL_Fnctset_create_single ( ADCL_work_fnct_ptr *init_fnct,
                                 ADCL_work_fnct_ptr *wait_fnct,
                                 ADCL_Attrset attrset, char *name,
                                 int **without_attribute_combinations,
                                 int num_without_attribute_combinations,
                                 ADCL_Fnctset *fnctset );
/* Function to register history functions to a function set */
int ADCL_Fnctset_reg_hist_fnct ( ADCL_Hist_functions hist_functions, ADCL_Fnctset fnctset );

int ADCL_Fnctset_free ( ADCL_Fnctset *fnctset );

/* ADCL Request functions */
int ADCL_Request_create         ( ADCL_Vector vec, ADCL_Topology topo,
                                  ADCL_Fnctset fnctset,  ADCL_Request *req );
int ADCL_Request_create_generic (ADCL_Vector svec, ADCL_Vector rvec, 
                                 ADCL_Topology topo,
                                 ADCL_Fnctset fnctset,
                                 ADCL_Request *req );
int ADCL_Request_create_generic_rooted (ADCL_Vector svec, ADCL_Vector rvec, 
                                 ADCL_Topology topo,
                                 ADCL_Fnctset fnctset, int root,
                                 ADCL_Request *req );

int ADCL_Request_free   ( ADCL_Request *req );

int ADCL_Request_start  ( ADCL_Request req );
int ADCL_Request_init   ( ADCL_Request req );
int ADCL_Request_wait   ( ADCL_Request req );
int ADCL_Request_update ( ADCL_Request req, TIME_TYPE time );
int ADCL_Request_start_overlap ( ADCL_Request req, ADCL_work_fnct_ptr* midfctn,
                                 ADCL_work_fnct_ptr *endfcnt,
                                 ADCL_work_fnct_ptr *totalfcnt );
/* Function to register creteria function to a given ADCL request */
int ADCL_Request_reg_hist_criteria ( ADCL_Request req, ADCL_Hist_criteria hist_criteria);

int ADCL_Request_get_comm  ( ADCL_Request req, MPI_Comm *comm, int *rank, int *size );
int ADCL_Request_get_curr_function ( ADCL_Request req, char **function_name,
                                     char ***attrs_names, int *attrs_num,
                                     char ***attrs_values_names, int **attrs_values_num );
int ADCL_Request_get_winner_stat ( ADCL_Request req, double *filtered_avg,
                                   double *unfiltered_avg, double *outliers_num );
int ADCL_Request_get_functions_with_average ( ADCL_Request req, 
                                              double filtered_avg,
                                              int *num_functions,
                                              char *** function_name, 
                                              char ***attrs_names, 
                                              int *attrs_num, 
                                              char ****attrs_values_names, 
                                              int ***attrs_values_num);

int ADCL_Request_save_status ( ADCL_Request req, int *tested_num,
                               double **unfiltered_avg,
                               double **filtered_avg,
                               double **outliers, int *winner_so_far );

int ADCL_Request_restore_status ( ADCL_Request req, int tested_num,
                                  double *unfiltered_avg,
                                  double *filtered_avg,
                                  double *outliers );

int ADCL_Request_get_fsname ( ADCL_Request req, char **fsname );
int ADCL_Request_get_tndims ( ADCL_Request req, int *tndims );
int ADCL_Request_get_state ( ADCL_Request req, int *state );

#endif /* __ADCL_H__ */
