/* include/ADCL_config.h.  Generated from ADCL_config.h.in by configure.  */
/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2009           HLRS 
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef __ADCL_CONFIG_H__
#define __ADCL_CONFIG_H__

/* Choose type of non-contiguous message handling */
#define DERIVED_TYPES 1
#define PACK_UNPACK   1

/* Choose different communication pattern */
#define CHANGE_AAO    1
#define CHANGE_PAIR   1

/* Choose data transfer primitive */
#define ISEND_IRECV   1
#define SEND_IRECV    1
#define SEND_RECV     1
#define SENDRECV      1

/* #undef MPI_WIN        */
#define MPI_SUBARRAY 1
/* #undef FENCE_PUT      */
/* #undef FENCE_GET      */
/* #undef POSTSTART_PUT  */
/* #undef POSTSTART_GET  */

/* Dump printf statements to a file? */
#define ADCL_FILE_PER_PROC 1

/* Use MPI_Barrier to synchronize process before performance measurements ? */
#define ADCL_USE_BARRIER 1

/* Disable ADCL internal timings and let the user provide the timing values */
#define ADCL_USERLEVEL_TIMINGS 1

/* Use MPI_Wtime instead of gettimeofday */
/* #undef ADCL_USE_MPIWTIME */

/* Enable dumping the ADCL knowledge to an XML file  */
/* #undef ADCL_KNOWLEDGE_TOFILE */

/* Enable new output format ( emethod winner instead of request winner )   */
/* #undef ADCL_NEW_OUTPUT_FORMAT */

/* Enable saving the winner of a request  */
#define ADCL_SAVE_REQUEST_WINNER 1

/* Enable the usage of the GNU scientific libraries */
/* #undef ADCL_GSL */


/* enable PAPI module? */
/* #undef ADCL_PAPI  */

/* Timer to be used */
#define TIMER TIMER_GETTIMEOFDAY

/* enable Dummy MPI library? */
#define ADCL_DUMMY_MPI 1

/* Acceptable performance window for historic learning */
#define ADCL_PERF_WIN 10

/* Minimum number of history entries to try to make a prediction */
#define ADCL_MIN_HIST 5

/* The use of a smoothing operation on the history data */
/* #undef ADCL_SMOOTH_HIST */

/* Define the size of the smoothing window */
#define ADCL_SMOOTH_WIN 3

/* Prediction algorithm to be used for historic learning */
#define ADCL_PRED_ALGO ADCL_WMV

#endif
