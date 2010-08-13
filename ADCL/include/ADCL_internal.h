/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef __ADCL_INTERNAL_H__
#define __ADCL_INTERNAL_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>

#include "ADCL.h"
#include "ADCL_array.h"
#include "ADCL_statistics.h"
#include "ADCL_vector.h"
#include "ADCL_topology.h"
#include "ADCL_attribute.h"
#include "ADCL_emethod.h"
#include "ADCL_hist.h"
#include "ADCL_function.h"
#include "ADCL_memory.h"
#include "ADCL_subarray.h"
#include "ADCL_packunpack.h"
#include "ADCL_request.h"
#include "ADCL_change.h"
#include "ADCL_allgatherv.h"
#include "ADCL_allreduce.h"
#include "ADCL_reduce.h"
#include "ADCL_reduce_topo.h"
#include "ADCL_alltoallv.h"
#include "ADCL_alltoall.h"
#include "ADCL_reduce.h"
#include "ADCL_bcast.h"
#include "ADCL_predefined.h"
#include "ADCL_vmap.h"
#include "ADCL_contiguous.h"
#include "ADCL_list.h"
#include "ADCL_hist.h"

#define ADCL_DISPLAY 

#include "ADCL_display.h"

#ifdef ADCL_PAPI
#include "ADCL_papi.h"
#endif

#ifdef ADCL_DUMMY_MPI
#include "ADCL_dummy_mpi.h"
#endif

#include "ADCL_config.h"
#include "ADCL_sysconfig.h"


#define TRUE                           1
#define FALSE                          0

#define ADCL_STATE_REGULAR             -100
#define ADCL_STATE_TESTING             -101
#define ADCL_STATE_DECISION            -102

#define ADCL_EVAL_DONE                 -110
#define ADCL_SOL_FOUND                 -111

#define ADCL_STATISTIC_VOTE            0
#define ADCL_STATISTIC_MAX             1

#define ADCL_UNEQUAL                   0
#define ADCL_IDENT                     1
#define ADCL_SIMILAR                   2

#define ADCL_WORST_CLASS               0
#define ADCL_BEST_CLASS                1

#define ADCL_UNFILTERED                0
#define ADCL_FILTERED                  1

#define ADCL_NO_ROOT		       -1

/* Some prototypes of functions which do not deserve their own header file */
int ADCL_printf_init     ( void );
int ADCL_printf_finalize ( void );
int ADCL_printf          ( const char *format, ...);

int ADCL_readenv( void );

int ADCL_predefined_init ( void );
int ADCL_predefined_finalize ( void );
int ADCL_predefined_alltoallv ( void );

int ADCL_hypothesis_init ( ADCL_emethod_t *e );
int ADCL_hypothesis_shrinklist_byattr ( ADCL_emethod_t *e, int attr_pos,
                    int required_value );
int ADCL_hypothesis_set     ( ADCL_emethod_t *er, int attr, int attrval );
int ADCL_hypothesis_eval_one_attr    ( ADCL_emethod_t *e, int num_attrs,
                       int *attr_values, ADCL_attribute_t * attr,
                       int attr_pos, int max_attr_vals,
                       int *winner_attr_val_pos, int *winner_attr_val,
                       ADCL_statistics_t **tmp_stats,
                       ADCL_function_t **tmp_funcs );
int ADCL_hypothesis_eval_v3 ( ADCL_emethod_t *e );
int ADCL_hypothesis_get_next ( ADCL_emethod_t *e );

int ADCL_fortran_string_f2c(char *fstr, int *len, char **cstr);
int ADCL_fortran_string_c2f(char *cstr, char *fstr, int len);

int ADCL_two_init ( ADCL_emethod_t *e );
int ADCL_twok_get_next( ADCL_emethod_t *e );

#endif /* __ADCL_INTERNAL_H__ */
