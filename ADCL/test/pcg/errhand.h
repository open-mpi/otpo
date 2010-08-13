/*
 * file:  errhand.h
 *
 * desc:  Header file for program error-handling code
 *
 */

#ifndef INC_ERRHAND_H
#define INC_ERRHAND_H

#include <assert.h>


/*
 * error codes
 */

#define ERR_NONE         0     /* non error */
#define ERR_ARGS         1     /* bad command-line arguments */
#define ERR_BADMATFILE   2     /* improper sparse matrix file */
#define ERR_BADTOL       3     /* bad error tolerance parameter */

/* etc. ... */


void quit( int errcode, const char* msg );
  /*
   * post :  If errcode != ERR_NONE, aborts program with error
   *         code `errcode' and the message `msg' to stderr.
   */


#endif

/* eof */
