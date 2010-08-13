#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAXLINE 128
#define MAXNAMELEN 48

int main (int argc, char **argv )
{
    FILE *infd=NULL,  *outfd=NULL;
    int nummethods=14;
    int numrep=3;
    char line[MAXLINE], methstr[MAXNAMELEN];
    char *basestr;
    double **results=NULL;
    char infilename[MAXNAMELEN], outfilename[MAXNAMELEN];
    char **methname=NULL;
    int i, j, namecnt = 0, rescnt = 0, methcnt=0;
    char brutestr[3][MAXLINE], hypostr[3][MAXLINE];
    int deccnt=0, hypoflag=0, meth;
    double min, max, sum ;

    if ( argc < 2 ) {
	printf(" Usage : eval-slurm <infilename> [<nummethods> "
	       "<numrepetitions>] \n\n");
	printf(" This program takes the slurm output file \n"
	       " and summarizes it. It is far from being general\n");
	exit ( 1 ) ;
    }	

    strncpy (infilename, argv[1], MAXNAMELEN );
    strncpy ( outfilename, infilename+6, MAXNAMELEN);

    outfd = fopen ( outfilename, "w" );
    if ( NULL == outfd ) {
	printf("eval-slurm: could not open %s for writing\n", outfilename );
	exit (-1);
    }

    infd = fopen ( infilename, "r" );
    if ( NULL == infd ) {
	printf("eval-slurm: could not open %s for reading\n", infilename );
	exit (-1);
    }

    if ( argc == 3 ) {
	nummethods = atoi ( argv[2] );
    }
    if ( argc == 4 ) {
	nummethods = atoi ( argv[2] );
	numrep     = atoi ( argv[3] );

    }

    results = (double **) malloc ( nummethods * sizeof ( double *));
    methname = (char **) malloc (  nummethods * sizeof (char *));
    if ( NULL == results || NULL == methname) {
	printf("eval-slurm: could not allocate memory\n");
    }
    for ( i=0; i<nummethods; i++ ) {
	results[i] = (double *) malloc ( numrep * sizeof (double ));
	methname[i] = (char *) calloc ( 1, MAXNAMELEN );
	if ( NULL == results[i] || NULL == methname[i]) {
	    printf("eval-slurm: could not allocate memory\n");
	}
    }


    while ( fscanf ( infd, "%[^\n]\n", line ) != EOF ) 
    {
	basestr = strstr ( line, "using");
	if ( NULL != basestr && namecnt < nummethods ) {
	    sscanf ( basestr, "using %s", methname[namecnt] );
	    namecnt++;
	}

	basestr = strstr ( line, "equations");
	if ( NULL != basestr ) {
	    sscanf ( basestr, "equations   : %lf", &(results[methcnt][rescnt]));
	    methcnt++; 
	    if ( methcnt == nummethods ) {
		methcnt = 0;
		rescnt++;
	    }
	}

	basestr = strstr ( line, " winner ");
	if ( NULL != basestr ) {
	    if ( hypoflag ) {
		do {
		    fscanf ( infd, "%[^\n]\n", line );
		    basestr = strstr ( line, " req ");
		} while ( basestr == NULL );

		basestr = strstr ( line, " winner is ");
		sscanf ( basestr, " winner is %d %s\n", &meth, methstr );
		sprintf( hypostr[deccnt], "perf. hypothesis: winner is %d %s \n",
			     meth, methstr );
		deccnt++;
		hypoflag = 0;
	    }
	    else {
		char *filtstr;

		filtstr = strstr ( line, " (filtered)");
		fscanf ( infd, "%[^\n]\n", line );
		basestr = strstr ( line, " winner is " ) ;
		sscanf ( basestr, " winner is %d %s\n", &meth, methstr );
		if ( NULL != filtstr ) {
		    sprintf( brutestr[deccnt], "brute force search: winner is %d %s (filtered)\n",
			     meth, methstr );
		}
		else {
		    sprintf( brutestr[deccnt], "brute force search: winner is %d %s (unfiltered)\n",
			     meth, methstr );
		}
		hypoflag = 1;
	    }
	}

    }

    for ( i=0; i<nummethods; i++ ) {
	min = max = sum = results[i][0];
	for ( j=1; j<numrep; j++ ) {
	    sum+= results[i][j];
	    if ( results[i][j] < min ) min = results[i][j];
	    if ( results[i][j] > max ) max = results[i][j];
	}

	fprintf ( outfd, "%16s  ", methname[i] );
	for ( j=0; j<numrep; j++ ) {
	    fprintf( outfd, " %16.2lf ", results[i][j] );
	}
	fprintf ( outfd, " | %16.2lf  %16.2lf %16.2lf\n", 
		  min, max, sum/numrep );
    }

    for ( i=0; i<3; i++ ) {
	fprintf(outfd, "%s", brutestr[i]);
    }

    for ( i=0; i<3; i++ ) {
	fprintf(outfd, "%s", hypostr[i]);
    }

    fclose ( infd );
    fclose ( outfd );

    return 0;
}
    
