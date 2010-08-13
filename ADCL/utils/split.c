#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAXLINE 120

int main (int argc, char **argv )
{
    FILE *infd=NULL,  **outfd=NULL;
    int numprocs=-1, numrequests=-1;

    int reqid, i, j;
    char inname[50], outname[50];
    char line[MAXLINE];
    char *basestr, reqstr[50];

    if (argc < 3 )
    {
	printf(" Usage : split <numprocs> <numrequests> \n\n");
	printf(" This program splits ADCL output files \n"
	       " into multiple output files per proc and request.\n");
	exit ( 1 ) ;
    }
    
    numprocs = atoi( argv[1] );
    numrequests = atoi ( argv[2] );

    outfd = (FILE **) malloc ( numrequests * sizeof(FILE *) );
    if ( NULL == outfd ) {
	exit (-1);
    }

    for ( j = 0; j < numprocs; j++ ) {
	sprintf(inname, "%d.out", j);
	infd = fopen ( inname, "r" );
	
	for (i=0; i < numrequests; i++ ) {
	    sprintf(outname, "%s.%d", inname, i );
	    outfd[i] = fopen ( outname, "w" );
	    if ( NULL == outfd[i] ) {
		exit (-1);
	    }
	}

	/* Read infile and store the values in the according list */
	while ( fscanf ( infd, "%[^\n]\n", line ) != EOF ) 
	{
	    if ( line[0] == '#' ) {
		/* Skip comment lines */
		continue;
	    }
	    
	    /*read the parameters of the run */
	    basestr = strstr ( line, "request" );
	    sscanf ( basestr, "%7s %d", reqstr, &reqid );
	    
	    fprintf ( outfd[reqid], "%s\n", line );
	}
	
	for ( i=0; i< numrequests; i++ ) {
	    fclose ( outfd[i]);
	}
    }

  free ( outfd );

  return ( 0 );
}
