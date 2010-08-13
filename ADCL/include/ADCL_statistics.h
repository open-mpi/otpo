/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef __ADCL_STATISTICS_H__
#define __ADCL_STATISTICS_H__

#include <sys/time.h>
#ifdef ADCL_DUMMY_MPI
#include "ADCL_dummy_mpi.h"
#else
#include "mpi.h"
#endif
#include "ADCL_attribute.h"
#include "ADCL_function.h"

#define TIME ADCL_statistics_time()

#define ADCL_STAT_TIMEDIFF(_tstart,_tend,_exec) {         \
    if ( _tend > _tstart ) _exec = (double) (_tend-_tstart); \
    else _exec = (1000000.0 - _tstart) + _tend; }

#define ADCL_FLAG_PERF     -100
#define ADCL_FLAG_NOPERF   -101

/* Possible values for STAT_flags */
#define ADCL_STAT_TESTED    0x00000001
#define ADCL_STAT_FILTERED  0x00000002
#define ADCL_STAT_EVAL      0x00000004

#define ADCL_STAT_IS_TESTED(STAT)   ((STAT)->s_flags & ADCL_STAT_TESTED)
#define ADCL_STAT_IS_FILTERED(STAT) ((STAT)->s_flags & ADCL_STAT_FILTERED)
#define ADCL_STAT_IS_EVAL(STAT)     ((STAT)->s_flags & ADCL_STAT_EVAL)

#define ADCL_STAT_SET_TESTED(STAT)   ((STAT)->s_flags |= ADCL_STAT_TESTED)
#define ADCL_STAT_SET_FILTERED(STAT) ((STAT)->s_flags |= ADCL_STAT_FILTERED)
#define ADCL_STAT_SET_EVAL(STAT)     ((STAT)->s_flags |= ADCL_STAT_EVAL)

struct ADCL_statistics_s{
    short         s_count; /* how often has this routine already been called */
    short      s_rescount; /* how often has this routine already reported back */
    TIME_TYPE    *s_time;  /* measurements */
    int          s_flags;  /* Has this data set already been filtered? */
    double      s_lpts[3]; /* local no. of pts by this function */
    double      s_gpts[3]; /* global no. of pts for this function */
};
typedef struct ADCL_statistics_s ADCL_statistics_t;

int ADCL_statistics_create ( ADCL_statistics_t*** stats, int fs_maxnum );
int ADCL_statistics_free   ( ADCL_statistics_t*** stats, int fs_maxnum );

int ADCL_statistics_filter_timings  ( ADCL_statistics_t **stats, int count,
                    int rank );
double ADCL_statistics_time(void);
int ADCL_statistics_global_max_v3 ( ADCL_statistics_t **statistics, int count,
                    MPI_Comm comm, int rank );
int ADCL_statistics_get_winner_v3 ( ADCL_statistics_t **statistics, int count,
                    int *winner );

/* Data structure for performance hypothesis search algorithm */
struct ADCL_hypothesis_s {
    int                *h_attr_hypothesis; /* List of performance hypothesis*/
    int                *h_attr_confidence; /* List of confidence values */
    int                  h_num_avail_meas; /* Counter keeping track of how many
                          methods have already been tested*/
    int                  *h_curr_attrvals; /* list of attribute values currently
                          being evaluated */
    ADCL_attribute_t       *h_active_attr; /* attribute currently being optimized */
    int                  h_active_attrpos; /* position of the current actively
                          investigated attribute in the attrset */
};
typedef struct ADCL_hypothesis_s ADCL_hypothesis_t;

/* Data structure for performance hypothesis search algorithm */
struct ADCL_twok_factorial_s {
    int twok_num;
    ADCL_fnctset_t           *twok_fnctset; /* 2k function set */
    int                    *twok_fncts_pos; /* 2k functions positions */
    int                      twok_next_pos; /* Next function to be tested */
    int                          twok_best; /* Best function in the twok functionset */
    int                      **twok_labels; /* Sign table header: Attribute Combination Labels */
    int                  **twok_sign_table; /* Sign Table */
    double                       *twok_sst; /* SST's */
    double                         *twok_q; /* Q's */
};
typedef struct ADCL_twok_factorial_s ADCL_twok_factorial_t;

#endif
