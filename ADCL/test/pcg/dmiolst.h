/*
 * $Id: dmiolst.h,v 1.1 1998/03/19 07:23:56 richie Exp richie $
 *
 * file :  dmiolst.h
 * desc :  Implementation of a linked-list for the I/O class
 *
 */

#ifndef INC_DMIOLST_H
#define INC_DMIOLST_H

typedef int item_t;
typedef struct tag_list_node_t
{
  item_t value;
  struct tag_list_node_t* next;
} list_node_t;

typedef struct tag_list_t
{
  list_node_t* head;
  list_node_t* tail;
  int length;
} list_t;



void lst_init( list_t* l );
  /*
   *  post :  initialize an empty list
   */


void lst_destroy( list_t* l );
  /*
   *  post :  removes all elements in l
   */


int lst_length( list_t* l );
  /*
   *  post :  returns the number of items in the list `l'
   */


void lst_insert( list_t* l, item_t i );
  /*
   *  post :  inserts the item `i' at the tail of list `l'
   */

item_t lst_remove( list_t* l );
  /*
   *   pre :  lst_length(l) > 0
   *  post :  removes the item at the head of the list
   */

#endif

/*
 * $Log: dmiolst.h,v $
 * Revision 1.1  1998/03/19 07:23:56  richie
 * Initial revision
 *
 */
