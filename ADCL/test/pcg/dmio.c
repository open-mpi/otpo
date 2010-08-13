/*
 * $Id: dmio.c,v 1.3 1998/03/19 20:03:10 richie Exp richie $
 *
 * file:  dmio.c
 * desc:  distributed matrix i/o module
 *
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "distmat.h"
#include "errhand.h"
#include "iohb.h"
#include "dmiolst.h"
#include "dmiodlst.h"

#define NDEBUG_IO


/* private functions */
static int dspio_readHB_mat_double( const char* filename,
                                    int row_i, int row_f,
                                    int** p_rowptr,
                                    int* p_nnz,
                                    int** p_colind,
                                    double** p_val );
  /*
   *   pre :  `filename' is a file containing square symmetric, real, sparse \
   *          matrix data in Harwell-Boeing format
   *
   *  post :  reads the data contained in `filename' into storage
   *          space.  this space is allocated by the routine.
   *          returns pointers to the row pointers, column indices
   *          and values for a row-compressed sparse matrix format.
   */


/* ----------------- code ----------------- */

dsp_matrix_t *
dspmat_newHB( const char* filename )
{
  char  *type;
  int   nrhs;               /* # of right-hand sides in file */
  int   m, n;
  int   nnz;

  dsp_matrix_t* new_mat;
  int min_rows_per_proc;
  int rows_this_proc;
  int leftover;


  readHB_info( filename, &m, &n, &nnz, &type, &nrhs );
#ifndef INSANE
  if( m != n ) {
    fprintf( stderr, "--- Can only operate on square matrices ---\n" );
    return NULL;
  }
#endif

  if( type[0] != 'R' && type[0] != 'r' ) {
    fprintf( stderr, "--- Can only operate on real matrices ---\n" );
    return NULL;
  }

  if( type[1] != 'S' && type[1] != 's' ) {
    fprintf( stderr, "--- Can only operate on symmetric matrices ---\n" );
    return NULL;
  }


  /* alloc matrix */
  new_mat = (dsp_matrix_t *)malloc( sizeof(dsp_matrix_t) );
  if( new_mat == NULL )
    return NULL;
  memset(new_mat, 0, sizeof(dsp_matrix_t));

#ifndef INSANE
  new_mat->N = n;
#else
  new_mat->N = m;
#endif


  /* distribute rows */
  min_rows_per_proc = new_mat->N / g_numprocs;
  rows_this_proc = min_rows_per_proc;
  leftover = new_mat->N % g_numprocs;
  if( g_myproc < leftover ) {
    rows_this_proc++;
    new_mat->row_i = rows_this_proc * g_myproc;
  } else {
    new_mat->row_i = leftover + g_myproc * min_rows_per_proc;
  }
  new_mat->row_f = new_mat->row_i + rows_this_proc - 1;

  new_mat->row_ptr = (int *)malloc( sizeof(int) * (rows_this_proc + 1) );


  /* read in data */
  dspio_readHB_mat_double( filename,
                           new_mat->row_i, new_mat->row_f,
                           &(new_mat->row_ptr),
                           &(new_mat->nnz),
                           &(new_mat->col_ind),
                           &(new_mat->val) );

  /* synchronize A across all processors */
  dspmat_vecmult_sync( new_mat );

  /* clean-up */
  return new_mat;
}



int
dspio_readHB_mat_double( const char* filename,
                         int row_i, int row_f,
                         int** p_rowptr,
                         int* p_nnz,
                         int** p_colind,
                         double** p_val )
{
/****************************************************************************/
/*  This function opens and reads the specified file, interpreting its      */
/*  contents as a sparse matrix stored in the Harwell/Boeing standard       */
/*  format and creating compressed column storage scheme vectors to hold    */
/*  the index and nonzero value information.                                */
/*                                                                          */
/*    ----------                                                            */
/*    **CAVEAT**                                                            */
/*    ----------                                                            */
/*  Parsing real formats from Fortran is tricky, and this file reader       */
/*  does not claim to be foolproof.   It has been tested for cases when     */
/*  the real values are printed consistently and evenly spaced on each      */
/*  line, with Fixed (F), and Exponential (E or D) formats.                 */
/*                                                                          */
/*  **  If the input file does not adhere to the H/B format, the  **        */
/*  **             results will be unpredictable.                 **        */
/*                                                                          */
/****************************************************************************/
  FILE *in_file;
  int i,j,ind,col,offset,count,last,Nrhs;
  int Ptrcrd, Indcrd, Valcrd, Rhscrd;
  int Nrow, Ncol, Nnzero;
  int Ptrperline, Ptrwidth, Indperline, Indwidth;
  int Valperline, Valwidth, Valprec;
  int Valflag;           /* Indicates 'E','D', or 'F' float format */
  char* ThisElement;
  char Title[73], Key[9], Type[4], Rhstype[4];
  char Ptrfmt[17], Indfmt[17], Valfmt[21], Rhsfmt[21];
  char line[BUFSIZ];

  int* rowptr;
  int* colind;
  int  len_colind;
  double* val;
  int  nr = row_f - row_i + 1;
  int  temp_elem;
  int  rownum;
  int  running_offset;
  int  new_insert_pos;

  list_t* symm_entries;    /* store list of symmetric entries to add */
  int* all_rowptr;
  int len_extra_symm;   /* # of extra entries due to symmetry */
  fpos_t save_pos;

  dlist_t* symm_vals;
  fpos_t val_save_pos;
  FILE*  val_in_file;
  char val_line[BUFSIZ];
  int val_col;
  int val_ind;
  int val_i;
  int val_skip_lines;
  char* val_ThisElement;
  double val_temp_elem;

  in_file = fopen( filename, "r");
  assert( in_file != NULL );

  readHB_header(in_file, Title, Key, Type, &Nrow, &Ncol, &Nnzero, &Nrhs,
                Ptrfmt, Indfmt, Valfmt, Rhsfmt,
                &Ptrcrd, &Indcrd, &Valcrd, &Rhscrd, Rhstype);

  assert( Nrow == Ncol );
  assert( Type[0] == 'R' || Type[0] == 'r' );
  assert( Type[1] == 'S' || Type[1] == 's' );
  assert( Type[2] == 'A' || Type[2] == 'a' );

  /*  Parse the array input formats from Line 3 of HB file  */
  ParseIfmt(Ptrfmt,&Ptrperline,&Ptrwidth);
  ParseIfmt(Indfmt,&Indperline,&Indwidth);
  ParseRfmt(Valfmt,&Valperline,&Valwidth,&Valprec,&Valflag);

  /*  Read row pointer array:   */
  offset = 1;

  rowptr = (int *)malloc( sizeof(int)*(nr + 1) );
  *p_rowptr = rowptr;
  if( rowptr == NULL )
    IOHBTerminate( "Out of memory allocating row pointer" );

  all_rowptr = (int *)malloc( sizeof(int)*(row_f+2) );
  if( all_rowptr == NULL )
    IOHBTerminate( "Out of memory allocating all row pointer" );

  symm_entries = (list_t *)malloc( sizeof(list_t)*(row_f+1) );
  symm_vals = (dlist_t *)malloc( sizeof(dlist_t)*(row_f+1) );
  if( symm_entries == NULL || symm_vals == NULL )
    IOHBTerminate( "Out of memory allocating symmetric entries" );
  for( i = 0; i < (row_f+1); i++ ) {
    lst_init( &(symm_entries[i]) );
    dlst_init( &(symm_vals[i]) );
  }

  ThisElement = (char *) malloc(Ptrwidth+1);
  if ( ThisElement == NULL )
    IOHBTerminate("Insufficient memory for ThisElement (1).");
  *(ThisElement+Ptrwidth) = (char) NULL;
  count = 0;
  for( i = 0; i < Ptrcrd; i++ ) {
    fgets( line, BUFSIZ, in_file );
    if ( sscanf(line, "%*s") < 0 ) 
      IOHBTerminate("--- Null (or blank) line in ptr data region ---\n");
    col = 0;
    for( ind = 0; ind < Ptrperline; ind++ ) {

      if( count > Ncol || count > row_f+1 )
        break;

      /* copy and convert index */
      strncpy( ThisElement, line+col, Ptrwidth );
      temp_elem = atoi(ThisElement) - offset;

      if( count >= row_i ) {
        /* only save elements in the proper range for this processor */
        rowptr[ count - row_i ] = temp_elem;
      }

      all_rowptr[ count ] = temp_elem;

      count++;
      col += Ptrwidth;
    }
  }
  free(ThisElement);

#ifndef NDEBUG_IO
fprintf( stderr, "printing rowptr...\n" );
for( i = 0; i <= nr; i++ )
  fprintf( stderr, "rowptr[%d]==%d\n", i, rowptr[i] );
fprintf( stderr, "printing all_rowptr...\n" );
for( i = 0; i <= row_f+1; i++ )
  fprintf( stderr, "all_rowptr[%d]==%d\n", i, all_rowptr[i] );
#endif


  /* alloc space for column indices and values */

  ThisElement = (char *) malloc(Indwidth+1);
  if ( ThisElement == NULL )
    IOHBTerminate("Insufficient memory for ThisElement (2).");
  *(ThisElement+Indwidth) = (char) NULL;
  val_ThisElement = (char *) malloc(Valwidth+2);
  if ( val_ThisElement == NULL )
    IOHBTerminate("Insufficient memory for val_ThisElement (2).");
  *(val_ThisElement+Valwidth) = (char) NULL;
  *(val_ThisElement+Valwidth+1) = (char) NULL;

  len_extra_symm = 0;
  fgetpos( in_file, &save_pos );
  rownum = 0;

  /* position at analogous place in file for values */
  val_in_file = fopen( filename, "r" );
  assert( val_in_file != NULL );
  val_skip_lines = 4 + (Rhscrd > 0) + Ptrcrd + Indcrd;

#ifndef NDEBUG_IO
fprintf( stderr, "header lines: %d (%d), ptr lines: %d, ind lines: %d\n",
         4 + (Rhscrd>0), Rhscrd, Ptrcrd, Indcrd );
fprintf( stderr, "skipping %d lines to get to vals...\n", val_skip_lines );
#endif

  for( i = 0; i < val_skip_lines; i++ ) {
    fgets( line, BUFSIZ, val_in_file );
  }
  fgetpos( val_in_file, &val_save_pos );

  count = 0;
  /*  for( i = 0; i < Indcrd; i++ ) {*/
  i = 0;       /* line number */
  col = 0;     /* column number on a given line */
  ind = 0;     /* item number on a given line */
  val_col = 0; /* analogous */
  val_ind = 0;
  val_i = 0;
  while( i <= Indcrd ) {

    /* read new lines, if necessary */
    if( (ind % Indperline) == 0 ) {
      fgets( line, BUFSIZ, in_file );
      if ( sscanf(line,"%*s") < 0 )
        IOHBTerminate("--- Null (or blank) line in index data region ---\n");
      i++;

      col = 0;
    }

    if( (val_ind % Valperline) == 0 ) {
      fgets( val_line, BUFSIZ, val_in_file );
      if( sscanf(val_line, "%*s") < 0 )
        IOHBTerminate("--- Null (or blank) line in value region ---\n");
      val_i++;

      val_col = 0;

      if( Valflag == 'D' ) {
        while( strchr(val_line, 'D') ) *strchr(val_line,'D') = 'E';
      }
    }

    if (count == Nnzero) break;

    /* read an entry from this line */
    strncpy( ThisElement, line+col, Indwidth );
    col += Indwidth;
    ind++;

    strncpy( val_ThisElement, val_line + val_col, Valwidth );
    *(val_ThisElement+Valwidth) = (char) NULL;
    val_col += Valwidth;
    val_ind++;
    if ( Valflag != 'F' && strchr(val_ThisElement,'E') == NULL ) {
      /* insert a char prefix for exp */
      last = strlen(val_ThisElement);
      for (j=last+1;j>=0;j--) {
        val_ThisElement[j] = val_ThisElement[j-1];
        if ( val_ThisElement[j] == '+' || val_ThisElement[j] == '-' ) {
          val_ThisElement[j-1] = Valflag;
          break;
        }
      }
    }

    /* done */
    if( count >= rowptr[nr] )
      break;

    temp_elem = atoi( ThisElement ) - offset;
    val_temp_elem = atof( val_ThisElement );

#ifndef NDEBUG_IO
    fprintf( stderr, "found: %d (%f)\n", temp_elem, val_temp_elem );
#endif

    if( temp_elem >= row_i && temp_elem <= row_f ) {
      if( temp_elem != rownum ) { /* ignore diagonal entry */

#ifndef NDEBUG_IO
        fprintf( stderr, "new entry: (%d, %d) == %f\n",
                 temp_elem, rownum, val_temp_elem );
#endif

        len_extra_symm++;
        lst_insert( &(symm_entries[temp_elem]), rownum );
        dlst_insert( &(symm_vals[temp_elem]), val_temp_elem );
      }
    }

    count++;
    if( count >= all_rowptr[rownum+1] )
      rownum++;
  }


#ifndef NDEBUG_IO
fprintf( stderr, "extra entries due to symmetry: %d\n", len_extra_symm );
#endif

  len_colind = rowptr[nr] - rowptr[0] + len_extra_symm;
  *p_nnz = len_colind;
  colind = (int *)malloc( sizeof(int)*len_colind );
  *p_colind = colind;

#ifndef NDEBUG_IO
fprintf( stderr, "nnz == %d\n", len_colind );
#endif

  val = (double *)malloc( sizeof(double)*len_colind );
  *p_val = val;

  assert( colind != NULL && val != NULL );

  /*  Read column index array:  */
  fsetpos( in_file, &save_pos );
  i = 0;
  col = 0;
  ind = 0;

  fsetpos( val_in_file, &val_save_pos );
  val_i = 0;
  val_col = 0;
  val_ind = 0;

  count = 0;
  rownum = 0;
  running_offset = 0;

  while( i <= Indcrd ) {

    /* read new lines, if necessary */
    if( (ind % Indperline) == 0 ) {
      fgets( line, BUFSIZ, in_file );
      if ( sscanf(line,"%*s") < 0 )
        IOHBTerminate("--- Null (or blank) line in index data region ---\n");
      i++;

      col = 0;
    }

    if( (val_ind % Valperline) == 0 ) {
      fgets( val_line, BUFSIZ, val_in_file );
      if( sscanf(val_line, "%*s") < 0 )
        IOHBTerminate("--- Null (or blank) line in value region ---\n");
      val_i++;

      val_col = 0;

      if( Valflag == 'D' ) {
        while( strchr(val_line, 'D') ) *strchr(val_line,'D') = 'E';
      }
    }


    if (count == Nnzero) break;


    /* read an entry from this line */
    strncpy( ThisElement, line+col, Indwidth );
    col += Indwidth;
    ind++;

    strncpy( val_ThisElement, val_line + val_col, Valwidth );
    *(val_ThisElement+Valwidth) = (char) NULL;
    val_col += Valwidth;
    val_ind++;
    if ( Valflag != 'F' && strchr(val_ThisElement,'E') == NULL ) {
      /* insert a char prefix for exp */
      last = strlen(val_ThisElement);
      for (j=last+1;j>=0;j--) {
        val_ThisElement[j] = val_ThisElement[j-1];
        if ( val_ThisElement[j] == '+' || val_ThisElement[j] == '-' ) {
          val_ThisElement[j-1] = Valflag;
          break;
        }
      }
    }

    /* done */
    if( count >= all_rowptr[nr + row_i] )
      break;

    temp_elem = atoi( ThisElement ) - offset;
    val_temp_elem = atof( val_ThisElement );

#ifndef NDEBUG_IO
    fprintf( stderr, "entry: (%d, %d) == %f\n", rownum, temp_elem, val_temp_elem );
#endif

    if( count >= all_rowptr[ row_i ] ) {
      new_insert_pos = count - all_rowptr[row_i] + running_offset;

#ifndef NDEBUG_IO
      fprintf( stderr, "inserting into colind[%d]: %d\n",
               new_insert_pos, temp_elem );
#endif
      colind[ new_insert_pos ] = temp_elem;
      val[ new_insert_pos ] = val_temp_elem;
    }

    count++;
    if( count >= all_rowptr[rownum+1] ) {
      rownum++;

      if( rownum >= row_i && rownum <= row_f+1 ) {
#ifndef NDEBUG_IO
        fprintf( stderr, "rowptr[%d] (== %d) += %d\n", rownum - row_i, rowptr[ rownum - row_i ], running_offset );
#endif
        rowptr[ rownum - row_i ] += running_offset;
      }

      if( rownum >= row_i && rownum <= row_f ) {
        /* insert symmetric entries, if any */
        int new_additions = 0;
        list_node_t* trace = symm_entries[rownum].head;
        dlist_node_t* dtrace = symm_vals[rownum].head;

#ifndef NDEBUG_IO
        fprintf( stderr, "symm_entry[%d].length == %d\n", rownum, symm_entries[rownum].length );
#endif

        new_insert_pos = count - all_rowptr[row_i] + running_offset;
        while( trace != NULL ) {
          assert( dtrace != NULL );

#ifndef NDEBUG_IO
          fprintf( stderr, "inserting symmetric entry: colind[%d] = %d\n",
                   new_insert_pos + new_additions, trace->value );
          fprintf( stderr, "inserting symmetric value: val[%d] = %f\n",
                   new_insert_pos + new_additions, dtrace->value );
#endif
          colind[ new_insert_pos + new_additions ] = trace->value;
          val[ new_insert_pos + new_additions ] = dtrace->value;
          trace = trace->next;
          dtrace = dtrace->next;
          new_additions++;
        } /* while */
        assert( new_additions == symm_entries[rownum].length );
        assert( new_additions == symm_vals[rownum].length );
        running_offset += new_additions;
      } /* rownum >= row_i ... */
    } /* count >= ... */
  }

  free(ThisElement);
  free(val_ThisElement);


  for( i = 0; i < row_f+1; i++ ) {
    lst_destroy( &(symm_entries[i]) );
  }
  for( i = 0; i < row_f+1; i++ ) {
    dlst_destroy( &(symm_vals[i]) );
  }
  free( symm_entries );
  free( symm_vals );
  free( all_rowptr );

  fclose( in_file );
  fclose( val_in_file );
  return 1;
}


/*
 * $Log: dmio.c,v $
 * Revision 1.3  1998/03/19 20:03:10  richie
 * Working I/O
 *
 * Revision 1.2  1998/03/19 08:46:57  richie
 * Macro'd debug print statements
 * (pre READ_VALUES)
 *
 * Revision 1.1  1998/03/19 07:23:56  richie
 * Initial revision
 *
 */

