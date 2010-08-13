/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"


/* An outlier defines as number of times larger
   than the minimal value */
int ADCL_OUTLIER_FACTOR=3;

/* Percentage of outliers allowed such that we really
   treat them as outliers */
int ADCL_OUTLIER_FRACTION=20;

/* what measure shall be used ? */
int ADCL_statistic_method=ADCL_STATISTIC_MAX;

extern int ADCL_emethod_numtests;

struct lininf {
    double min;
    double max;
    int    minloc;
    int    maxloc;
};

#define TLINE_INIT(_t) { _t.min=9999999999.99; _t.max=0.0; \
       _t.minloc=-1; _t.maxloc=-1;}
#define TLINE_MIN(_t, _time, _i){ \
           if ( _time < _t.min ) { \
               _t.min    = _time;  \
               _t.minloc = _i;}}
#define TLINE_MIN_NZ(_t, _time, _i){ \
	if ( (_time<_t.min) && (0!=_time) ) {	\
	    _t.min    = _time;			\
	    _t.minloc = _i;}}
#define TLINE_MAX(_t, _time, _i) { \
            if ( _time > _t.max ) { \
                _t.max = _time;     \
                _t.maxloc =_i;}}

/**********************************************************************/
 int ADCL_statistics_create ( ADCL_statistics_t*** stats, int fs_maxnum ) {
/**********************************************************************/
    ADCL_statistics_t** tstats;
    int ret = ADCL_SUCCESS;
    int i;

    tstats = (ADCL_statistics_t **)calloc(fs_maxnum, sizeof(ADCL_statistics_t *));
    if ( NULL == tstats ) {
        ret = ADCL_NO_MEMORY;
        goto exit;
    }
    for ( i=0; i< fs_maxnum; i++ ) {
        tstats[i] = (ADCL_statistics_t *)calloc(1, sizeof(ADCL_statistics_t));
        if ( NULL == tstats[i] ) {
            ret = ADCL_NO_MEMORY;
            goto exit;
        }

        /* Allocate the measurements arrays */
	tstats[i]->s_time = (TIME_TYPE *)calloc(ADCL_emethod_numtests, sizeof(TIME_TYPE));
        if ( NULL == tstats[i]->s_time ) {
            ret = ADCL_NO_MEMORY;
            goto exit;
        }
    }

 exit:
    if ( ret != ADCL_SUCCESS  ) {
	ADCL_statistics_free ( &(tstats) , fs_maxnum );
    }
    else {
	*stats = tstats;
    }
    return ret;
}

/**********************************************************************/
int ADCL_statistics_free ( ADCL_statistics_t ***stats, int fs_maxnum ) {
/**********************************************************************/
    ADCL_statistics_t** tstats = *stats;
    int i;

    if ( NULL != tstats  ) {
        for ( i=0; i< fs_maxnum; i++ ) {
            if ( NULL != tstats[i]) {
                if ( NULL != tstats[i]->s_time ) {
                    free ( tstats[i]->s_time );
                }
                free ( tstats[i] );
            }
        }
        free ( tstats );
    }
    stats = NULL;

    return ADCL_SUCCESS;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_statistics_filter_timings (ADCL_statistics_t **statistics, int count,
				    int rank )
{
    int i, j;
    int numoutl;
    TIME_TYPE min;
    double sum, sum_filtered;

    for ( i=0; i < count; i++ ) {
        sum          = 0.0;
        sum_filtered = 0.0;
        if ( !(ADCL_STAT_IS_FILTERED(statistics[i])) && (0!=statistics[i]->s_rescount)) {
            /* Determine the min  value for method i */
            for ( min=999999, j=0; j<statistics[i]->s_rescount; j++ ) {
                if ( statistics[i]->s_time[j] < min ) {
                    min = statistics[i]->s_time[j];
                }
            }

            /* Count how many values are N times larger than the min. */
            for ( numoutl=0, j=0; j<statistics[i]->s_rescount; j++ ) {
                sum += statistics[i]->s_time[j];
                if ( statistics[i]->s_time[j] >= (ADCL_OUTLIER_FACTOR * min) ) {
#if 0
                    ADCL_printf("#%d: stat %d meas. %d is outlier %lf min "
                        "%lf\n", rank, i, j, statistics[i]->s_time[j], min );
#endif
                    numoutl++;
                }
                else {
                    sum_filtered += statistics[i]->s_time[j];
                }
            }
            /* unfiltered avg. */
            statistics[i]->s_lpts[0] = sum / statistics[i]->s_rescount;

            /* filtered avg. */
	    statistics[i]->s_lpts[1] = sum_filtered/(statistics[i]->s_rescount-
						     numoutl );

            /* percentage of outliers */
            statistics[i]->s_lpts[2] = 100*numoutl/statistics[i]->s_rescount;

#if 0
            ADCL_printf("#%d: stat %d num. of outliers %d min %lf avg. %lf "
                "filtered avg. %lf perc. %lf \n", rank,
                i, numoutl, min, statistics[i]->s_lpts[0],
                statistics[i]->s_lpts[1], statistics[i]->s_lpts[2]);
#endif

            ADCL_STAT_SET_FILTERED (statistics[i]);
        }
    }


    return ADCL_SUCCESS;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
double ADCL_statistics_time (void)
{
    struct timeval tp;
    double retval;

#if TIMER == TIMER_MPI_WTIME
    /* Use MPI_wtime as timer */
    retval = MPI_Wtime();
#elif TIMER == TIMER_GETTIMEOFDAY
    /* Use gettime of day as timer */
    gettimeofday (&tp, NULL);
    retval = tp.tv_usec;
#elif TIMER == TIMER_PAPI_REAL_USEC
    /* Use PAPI functions as timer */
    retval = (double)PAPI_get_real_usec();
#else
    retval = 0;
#endif

    return retval;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_statistics_global_max_v3 ( ADCL_statistics_t **statistics, int count,
                    MPI_Comm comm, int rank )
{
    int i;
    double *lpts, *gpts;
    
    lpts = (double *) malloc ( 2 * 3 * count * sizeof(double) );
    if ( NULL == lpts ) {
        return ADCL_NO_MEMORY;
    }
    gpts = &(lpts[3 * count]);

    for ( i = 0; i < count; i++ ) {
        lpts[3*i]   = statistics[i]->s_lpts[0];
        lpts[3*i+1] = statistics[i]->s_lpts[1];
        lpts[3*i+2] = statistics[i]->s_lpts[2];
    }

    if  ( ADCL_STATISTIC_MAX == ADCL_statistic_method ) {
        MPI_Allreduce ( lpts, gpts, 3 * count, MPI_DOUBLE, MPI_MAX, comm);
    }

    for ( i = 0; i < count; i++ ) {
	statistics[i]->s_gpts[0] = gpts[3*i];
	statistics[i]->s_gpts[1] = gpts[3*i+1];
	statistics[i]->s_gpts[2] = gpts[3*i+2];
    }

    free ( lpts );
    return ADCL_SUCCESS;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int ADCL_statistics_get_winner_v3 ( ADCL_statistics_t **statistics, int count,
                    int *winner )
{
    int i, retval;
    struct lininf tline_filtered, tline_unfiltered;
    TLINE_INIT ( tline_unfiltered );
    TLINE_INIT ( tline_filtered );

    for ( i = 0; i < count; i++) {
#if 0
        ADCL_printf("#%d %lf %lf %lf\n", i, statistics[i]->s_gpts[0],
		    statistics[i]->s_gpts[1], statistics[i]->s_gpts[2]);
#endif
        TLINE_MIN_NZ ( tline_unfiltered, statistics[i]->s_gpts[0], i );
        TLINE_MIN_NZ ( tline_filtered, statistics[i]->s_gpts[1], i );
    }

    if ( statistics[tline_filtered.minloc]->s_gpts[2] < ADCL_OUTLIER_FRACTION){
        *winner = tline_filtered.minloc;
        ADCL_printf("# heurstic win is %d (filtered) \n", *winner);
        retval = ADCL_FILTERED;
    }
    else {
        *winner = tline_unfiltered.minloc;
        ADCL_printf("# heuristic win is %d (unfiltered) \n",   *winner );
        retval = ADCL_UNFILTERED;
    }
    return retval;
}
