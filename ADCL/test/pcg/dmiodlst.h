/*
 * $Id: dmiodlst.h,v 1.1 1998/03/19 20:03:10 richie Exp richie $
 *
 * file :  dmiodlst.h
 * desc :  Implementation of a linked-list for the I/O class
 *         Listnodes store DOUBLE PRECISION values
 *
 */

#ifndef INC_DMIODLST_H
#define INC_DMIODLST_H

typedef double ditem_t;
typedef struct tag_dlist_node_t
{
  ditem_t value;
  struct tag_dlist_node_t* next;
} dlist_node_t;

typedef struct tag_dlist_t
{
  dlist_node_t* head;
  dlist_node_t* tail;
  int length;
} dlist_t;



void dlst_init( dlist_t* l );
  /*
   *  post :  initialize an empty list
   */


void dlst_destroy( dlist_t* l );
  /*
   *  post :  removes all the elements in l
   */


int dlst_length( dlist_t* l );
  /*
   *  post :  returns the number of items in the list `l'
   */


void dlst_insert( dlist_t* l, ditem_t i );
  /*
   *  post :  inserts the item `i' at the tail of list `l'
   */

ditem_t dlst_remove( dlist_t* l );
  /*
   *   pre :  dlst_length(l) > 0
   *  post :  removes the item at the head of the list
   */

#endif

/*
 * $Log: dmiodlst.h,v $
 * Revision 1.1  1998/03/19 20:03:10  richie
 * Initial revision
 *
 */
