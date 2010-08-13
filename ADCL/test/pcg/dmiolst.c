/*
 * $Id: dmiolst.c,v 1.1 1998/03/19 07:23:56 richie Exp richie $
 *
 * file :  dmiolst.c
 * desc :  Implementation of a linked-list for the I/O class
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "dmiolst.h"

void
lst_init( list_t* l )
{
  assert( l != NULL );
  l->head = NULL;
  l->tail = NULL;
  l->length = 0;
}


void
lst_destroy( list_t* l )
{
  assert( l != NULL );
  while( l->length > 0 )
    lst_remove( l );
}


int
lst_length( list_t* l )
{
  assert( l != NULL );
  return l->length;
}


void
lst_insert( list_t* l, item_t i )
{
  list_node_t* n;

  assert( l != NULL );

  n = (list_node_t *)malloc( sizeof(list_node_t) );
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

item_t
lst_remove( list_t* l )
{
  list_node_t* n;
  item_t i;

  assert( lst_length(l) > 0 );
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
 * $Log: dmiolst.c,v $
 * Revision 1.1  1998/03/19 07:23:56  richie
 * Initial revision
 *
 */
