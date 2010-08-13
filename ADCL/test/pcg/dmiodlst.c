/*
 * $Id: dmiodlst.c,v 1.1 1998/03/19 20:03:10 richie Exp richie $
 *
 * file :  dmiodlst.c
 * desc :  Implementation of a linked-list for the I/O class
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "dmiodlst.h"

void
dlst_init( dlist_t* l )
{
  assert( l != NULL );
  l->head = NULL;
  l->tail = NULL;
  l->length = 0;
}


void
dlst_destroy( dlist_t* l )
{
  assert( l != NULL );
  while( l->length > 0 )
    dlst_remove( l );
}


int
dlst_length( dlist_t* l )
{
  assert( l != NULL );
  return l->length;
}


void
dlst_insert( dlist_t* l, ditem_t i )
{
  dlist_node_t* n;

  assert( l != NULL );

  n = (dlist_node_t *)malloc( sizeof(dlist_node_t) );
  assert( n != NULL );
  n->value = i;
  n->next = NULL;

  if( l->tail == NULL ) {
    l->head = n;
    l->tail = n;
  } else {
    l->tail->next = n;
    l->tail = n;
  }

  l->length++;
}

ditem_t
dlst_remove( dlist_t* l )
{
  dlist_node_t* n;
  ditem_t i;

  assert( dlst_length(l) > 0 );
  assert( l->head != NULL && l->tail != NULL );

  n = l->head;
  i = n->value;

  l->head = n->next;
  if( l->head == NULL ) {
    assert( l->length == 1 );
    l->tail = NULL;
  }

  free( n );
  l->length--;

  return i;
}

/*
 * $Log: dmiodlst.c,v $
 * Revision 1.1  1998/03/19 20:03:10  richie
 * Initial revision
 *
 *
 */
