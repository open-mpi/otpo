#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_winner ( char *filename);

int main ( int argc, char ** argv ) 
{
    char basefname[50];
    int numprocs, numreqs, i, j;
    char outname[50], curepsname[50];
    FILE *outfd=NULL;

    if ( argc < 4 ) {
      printf(" Usage : gentex <infilename> <numprocs> <numrequests> \n\n");
      printf(" This program generates a tex file which  \n"
	     " includes all gnuplot eps-files from the previous\n"
	     " genscript step");
      exit ( 1 ) ;
    }
    
    strcpy ( basefname, argv[1] );
    numprocs = atoi( argv[2] );
    numreqs  = atoi( argv[3] );

    sprintf(outname, "%s.tex", basefname );
    outfd = fopen ( outname, "w" );
    if ( NULL == outfd ) {
	exit (-1);
    }

    fprintf( outfd, "\\documentclass{article}\n");
    fprintf( outfd, "\\usepackage{epsf}\n");
    fprintf( outfd, "\\begin{document}\n\n");

    for ( i=0; i < numprocs; i++ ) {
	for ( j=0; j< numreqs; j++ ) {
	    if ( j == 1 )  continue;
	    sprintf(curepsname, "%s-%d-%d.eps", basefname, i, j );

//	    fprintf( outfd, "\\begin{figure}[tbh]\n");
	    fprintf( outfd, "\\begin{center}\n");
	    fprintf( outfd, "\\epsfxsize=125mm\n");
	    fprintf( outfd, "\\epsfbox{%s}\n", curepsname );
	    fprintf( outfd, "\\end{center}\n");
//	    fprintf( outfd, "\\end{figure}\n\n");
	}
    }

    fprintf( outfd, "\\end{document}\n");

    fclose ( outfd );
    return 0;
}

