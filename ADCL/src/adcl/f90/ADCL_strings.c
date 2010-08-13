/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
/* Utility routines to convert a fortran string into a C string.
   The source code is taken from Open MPI, which is allowed according 
   to the Open MPI license. The Copyrights for the file remain as indicated 
   below.

   Please note, that in order to avoid clashes of this file when running ADCL
   on top of Open MPI, I had to rename them to start with ADCL_ 
   Furthermore, the error codes have been translated to ADCL error codes. 
*/

/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2005 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart, 
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * $COPYRIGHT$
 * 
 * Additional copyrights may follow
 * 
 * $HEADER$
 */

#include "ADCL_internal.h"

/*
 * creates a C string from an F77 string
 */
int ADCL_fortran_string_f2c(char *fstr, int *flen, char **cstr)
{
    char *end;
    int i;
    int len = *flen;

    /* Leading and trailing blanks are discarded. */

    end = fstr + len - 1;

    for (i = 0; (i < len) && (' ' == *fstr); ++i, ++fstr) {
        continue;
    }

    if (i >= len) {
        len = 0;
    } else {
        for (; (end > fstr) && (' ' == *end); --end) {
            continue;
        }

        len = end - fstr + 1;
    }

    /* Allocate space for the C string. */

    if (NULL == (*cstr = malloc(len + 1))) {
        return ADCL_NO_MEMORY;
    }

    /* Copy F77 string into C string and NULL terminate it. */

    if (len > 0) {
        strncpy(*cstr, fstr, len);
    }
    (*cstr)[len] = '\0';

    *flen = len;
    return ADCL_SUCCESS;
}


/*
 * Copy a C string into a Fortran string.  Note that when Fortran
 * copies strings, even if it operates on subsets of the strings, it
 * is expected to zero out the rest of the string with spaces.  Hence,
 * when calling this function, the "len" parameter should be the
 * compiler-passed length of the entire string, even if you're copying
 * over less than the full string.  Specifically:
 *
 * http://www.ibiblio.org/pub/languages/fortran/ch2-13.html
 *
 * "Whole operations 'using' only 'part' of it, e.g. assignment of a
 * shorter string, or reading a shorter record, automatically pads the
 * rest of the string with blanks."
 */
int ADCL_fortran_string_c2f(char *cstr, char *fstr, int len)
{
    int i;

    strncpy(fstr, cstr, len);
    for (i = strlen(cstr); i < len; ++i) {
        fstr[i] = ' ';
    }

    return ADCL_SUCCESS;
}


