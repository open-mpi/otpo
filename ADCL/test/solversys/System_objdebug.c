/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include <stdio.h>
#include <unistd.h>
#include "mpi.h"

void system_gethostname_ (int *rank )
{
  char hostname[1024];
  int len=1024;

  gethostname ( hostname, len );
  printf ( "Proces %d  is on host %s\n", *rank, hostname );

  return;
}

void system_sleep_ ( int * time ) 
{
    sleep ( (unsigned) *time);
    return;
   
}

void system_loc_ ( void * vec, int *addr )
{
  addr = (int*) vec;
  return;
}

#ifdef HAVE_PACX
void SYSTEM_DEBUG_REQUEST( int * req)
{
  printf(" debugging request no. %d\n", *req );
  printf(" in_use = %d, cmd = %d count = %d type = %d \n", PACX_rarray[*req].in_use,
	 PACX_rarray[*req].cmd_packet.cmd, PACX_rarray[*req].cmd_packet.count, 
	 PACX_rarray[*req].cmd_packet.datatype);
  printf(" comm = %d, mode = %d done = %d\n", PACX_rarray[*req].cmd_packet.comm_id,
	 PACX_rarray[*req].mode, PACX_rarray[*req].done );
}


void SYSTEM_DEBUG_DATATYPE ( int *type )
{
  printf(" debugging datatype %d\n", *type );
  printf(" id = %d in_use = %d active = %d datatype = %d sendinfo = %d \n",
	 PACX_darray[*type].id, PACX_darray[*type].in_use, PACX_darray[*type].active,
	 PACX_darray[*type].datatype, PACX_darray[*type].sendinfo );
  printf(" size = %d extent = %d counter = %d\n", PACX_darray[*type].size,
	 PACX_darray[*type].extent, PACX_darray[*type].counter );

}
#endif
