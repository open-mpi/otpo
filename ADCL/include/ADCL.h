/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2007           Cisco, Inc. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef __ADCL_H__
#define __ADCL_H__

#include "ADCL_config.h"

#if ADCL_DUMMY_MPI
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
#define ADCL_INVALID_COMTYPE   16
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

#ifdef ADCL_PAPI
#define ADCL_INVALID_PAPI      30
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

#ifdef ADCL_PAPI
#define ADCL_PAPI_NULL      (void*) -10
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

/* define the object types visible to the user */
typedef struct ADCL_vector_s*    ADCL_Vector;
typedef struct ADCL_vectset_s*   ADCL_Vectset;
typedef struct ADCL_request_s*   ADCL_Request;
typedef struct ADCL_topology_s*  ADCL_Topology;
typedef struct ADCL_attribute_s* ADCL_Attribute;
typedef struct ADCL_attrset_s*   ADCL_Attrset;
typedef struct ADCL_function_s*  ADCL_Function;
typedef struct ADCL_fnctset_s*   ADCL_Fnctset;
#ifdef ADCL_PAPI
typedef struct ADCL_papi_s*      ADCL_Papi;
#endif


/* define predefined functionsets */
extern struct ADCL_fnctset_s *ADCL_neighborhood_fnctset;
extern struct ADCL_fnctset_s *ADCL_fnctset_rtol;
extern struct ADCL_fnctset_s *ADCL_fnctset_ltor;

#define ADCL_FNCTSET_NEIGHBORHOOD ADCL_neighborhood_fnctset
#define ADCL_FNCTSET_SHIFT_LTOR   ADCL_fnctset_shift_ltor
#define ADCL_FNCTSET_SHIFT_RTOL   ADCL_fnctset_shift_rtol

#define TIME_TYPE double

/* Prototypes of the User level interface functions */

/* ADCL environment functions */
int ADCL_Init (void );
int ADCL_Finalize (void );

/* ADCL Vector functions and ADCL Vectorset functions */
int ADCL_Vector_allocate   ( int ndims, int *dims, int nc, int comtype, int hwidth,
                             MPI_Datatype dat, void *data, ADCL_Vector *vec );
int ADCL_Vector_free       ( ADCL_Vector *vec );
int ADCL_Vector_register   ( int ndims, int *dims, int nc, int comtype, int hwidth,
                             MPI_Datatype dat, void *data, ADCL_Vector *vec );
int ADCL_Vector_deregister ( ADCL_Vector *vec );

int ADCL_Vectset_create ( int maxnum,
                          ADCL_Vector  *svecs,
                          ADCL_Vector  *rvecs,
                          ADCL_Vectset *vectset );

int ADCL_Vectset_free   ( ADCL_Vectset *vectset );

/* ADCL Topology functions */
int ADCL_Topology_create  ( MPI_Comm cart_comm, ADCL_Topology *topo);
int ADCL_Topology_free    ( ADCL_Topology *topo );
int ADCL_Topology_create_generic ( int ndims, int *lneighbors,
                                   int *rneighbors, int *coords, int direction,
                                   MPI_Comm comm, ADCL_Topology *topo);

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
int ADCL_Fnctset_create_single_fnct ( ADCL_work_fnct_ptr *iptr, ADCL_Attrset attrset,
                                      char *name, int **without_attribute_combinations,
                                      int num_without_attribute_combinations, 
                                      ADCL_Fnctset *fnctset );
int ADCL_Fnctset_create_single_fnct_async ( ADCL_work_fnct_ptr *init_fnct,
                                            ADCL_work_fnct_ptr *wait_fnct,
                                            ADCL_Attrset attrset, char *name,
                                            int **without_attribute_combinations,
                                            int num_without_attribute_combinations,
                                            ADCL_Fnctset *fnctset );
int ADCL_Fnctset_free   ( ADCL_Fnctset *fnctset );


/* ADCL Request functions */
int ADCL_Request_create         ( ADCL_Vector vec, ADCL_Topology topo,
                                  ADCL_Fnctset fnctset,  ADCL_Request *req );
int ADCL_Request_create_generic (ADCL_Vectset vectset,
                                 ADCL_Topology topo,
                                 ADCL_Fnctset fnctset,
                                 ADCL_Request *req );

int ADCL_Request_free   ( ADCL_Request *req );

int ADCL_Request_start  ( ADCL_Request req );
int ADCL_Request_init   ( ADCL_Request req );
int ADCL_Request_wait   ( ADCL_Request req );
int ADCL_Request_update ( ADCL_Request req, TIME_TYPE time );
int ADCL_Request_start_overlap ( ADCL_Request req, ADCL_work_fnct_ptr* midfctn,
                                 ADCL_work_fnct_ptr *endfcnt,
                                 ADCL_work_fnct_ptr *totalfcnt );

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

#endif /* __ADCL_H__ */
