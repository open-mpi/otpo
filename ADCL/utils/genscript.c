#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_winner ( char *filename);

int main ( int argc, char ** argv ) 
{
    char basefname[50];
    int numprocs, numreqs, i, j;
    char outname[50], curfname[50], curepsname[50], *curwinner=NULL;
    FILE *outfd=NULL;

    if ( argc < 4 ) {
      printf(" Usage : genscript <infilename> <numprocs> <numrequests> \n\n");
      printf(" This program generates a script for \n"
	     " generating gnuplot files from output files\n");
      exit ( 1 ) ;
    }
    
    strcpy ( basefname, argv[1] );
    numprocs = atoi( argv[2] );
    numreqs  = atoi( argv[3] );

    sprintf(outname, "%s.gnuplot", basefname );
    outfd = fopen ( outname, "w" );
    if ( NULL == outfd ) {
	return -1;
    }

    fprintf( outfd, "set term post eps\n");
    for ( i=0; i < numprocs; i++ ) {
	for ( j=0; j< numreqs; j++ ) {
	    sprintf(curfname, "%d.out.%d", i, j);
	    curwinner = get_winner ( curfname );
	    sprintf(curepsname, "%s-%d-%d.eps", basefname, i, j );

	    fprintf(outfd, "set out '%s'\n", curepsname );
	    fprintf(outfd, "set title 'Proc %d req %d winner %s'\n", 
		    i, j, curwinner);
	    fprintf(outfd, "set xlabel 'No. of iterations' \n");
	    fprintf(outfd, "set ylabel 'Execution time [usec]' \n");
	    fprintf(outfd, "plot '%s' u 7 w linespoints\n\n", 
		    curfname );
	    free (curwinner );
	    curwinner = NULL;
	}
    }

    fclose ( outfd );
    return 0;
}

char * get_winner ( char * filename )
{
    FILE *fd;
    char line[120], *basestr=NULL, *winner;
    char reqstr[50];
    double tmp;

    fd = fopen ( filename, "r");
    if ( NULL == fd ) {
	exit ( 1 );
    }


  while ( fscanf ( fd, "%[^\n]\n", line ) != EOF ) 
    {
	/* do nothing, which just need the last value 
	   for line */
    }
  basestr = strstr ( line, "(" );
  sscanf ( basestr, "%s %lf", reqstr, &tmp );
  
  winner = strdup (reqstr);
  return winner;
}
