/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef __ADCL_HIST_H__
#define __ADCL_HIST_H__

#ifdef ADCL_DUMMY_MPI
#include "ADCL_dummy_mpi.h"
#else
#include "mpi.h"
#endif

extern ADCL_array_t *ADCL_hist_array;

int  ADCL_hist_create ( ADCL_emethod_t *e );
void ADCL_hist_free   ( void );
int  ADCL_hist_find   ( ADCL_emethod_t *e, ADCL_hist_t **hist );
void ADCL_hist_read_from_file ( ADCL_emethod_t *e );

int get_int_data_from_xml (char *str, int *res);
int get_str_data_from_xml (char *str, char **dest);

#endif /* __ADCL_HIST_H__ */
