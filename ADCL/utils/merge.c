#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAXLINE 120

FILE *allredfd=NULL, *minmaxfd=NULL, *iterfd=NULL, *mergefd=NULL;
struct lininf {
    int    req;
    int    method;
    double min;
    double max;
    int    minloc;
    int    maxloc;
};


int main (int argc, char **argv )
{
    int i, ret, done=0;
    char line[MAXLINE];
	struct lininf tline, aline, iline;
	
	allredfd = fopen ( "allreduce.out", "r");
	if  ( NULL == allredfd ) {
		printf("Could not find allreduce.out\n");
		return -1;
	}
	
	minmaxfd = fopen ( "minmax.out", "r");
	if  ( NULL == minmaxfd ) {
		printf("Could not find minmax.out\n");
		return -1;
	}
	/* we have to skip the first three entries, since 
	 * they are not in the main loop of the solver */
	for ( i=0; i< 3; i++ ) {
		ret = fscanf ( minmaxfd, "%[^\n]\n", line );
		if ( EOF == ret ) {
			printf("Error reading minmaxfd.out"); 	
			return -1;
		}
	}
	
	iterfd = fopen ( "iterations.out", "r");
	if  ( NULL == iterfd ) {
		printf("Could not find iterations.out\n");
		return -1;
	}	

	mergefd = fopen ( "merge.out", "w");
	if  ( NULL == mergefd ) {
		printf("Could not open merge.out for writing\n");
		return -1;
	}
 
    while ( !done ) {
		fscanf ( iterfd, "%3d %lf %3d %lf %3d\n", 
			&tline.req, &tline.min, &tline.minloc,
			&tline.max, &tline.maxloc);
			
		if ( EOF == ret ) {
			done = 1;	
			break;
		}
		/* Sequence of an iteration is:
		 * 	Allreduce
		 * 	  Matmul
		 * 	  Allreduce
		 * 	  Matmul
		 * 	  Allreduce
		 * 
		 * 	  Matmul
		 *    Allreduce
		 * 	  Matmul
		 * 	  Allreduce
		 *	Allreduce
		 * 	Matmul
		 * 	Matmul
		 */
		 for ( i=0; i< 4; i++ ) {
		 	ret = fscanf (allredfd, "%lf %3d %lf %d", 
		 			&aline.min, &aline.minloc, &aline.max, 
		 			&aline.maxloc);
		 	if	( EOF == ret ) {
		 		done = 1;
		 		break;
		 	}
		 	fprintf (mergefd, "%8.4lf %3d %8.4lf %3d %8.4lf %3d %8.4lf %3d\n",
		  			aline.min, aline.minloc, aline.max, aline.maxloc,
		  			tline.min, tline.minloc, tline.max, tline.maxloc );
		  	
		  	fscanf (minmaxfd, "%3d %3d %lf %3d %lf %d", 
		 			&iline.req, &iline.method, &iline.min, &iline.minloc, 
		 			&iline.max, &iline.maxloc);
		 	if	( EOF == ret ) {
		 		done = 1;
		 		break;
		 	}
		 	fprintf (mergefd, "%8.4lf %3d %8.4lf %3d %8.4lf %3d %8.4lf %3d\n",
		  			iline.min, iline.minloc, iline.max, iline.maxloc,
		  			tline.min, tline.minloc, tline.max, tline.maxloc );
		 }
	
		for ( i=0; i<2; i++ ) {	 
		 	fscanf (allredfd, "%lf %3d %lf %d\n", 
		 			&aline.min, &aline.minloc, &aline.max, 
		 			&aline.maxloc);
		 	if	( EOF == ret ) {
		 		done = 1;
		 		break;
		 	}
		 	fprintf (mergefd, "%8.4lf %3d %8.4lf %3d %8.4lf %3d %8.4lf %3d\n",
		  			aline.min, aline.minloc, aline.max, aline.maxloc,
		  			tline.min, tline.minloc, tline.max, tline.maxloc );
		}
		
		for ( i=0; i<2; i++ ) {
		  	fscanf (minmaxfd, "%3d %3d %lf %3d %lf %d\n", 
		 			&iline.req, &iline.method, &iline.min, &iline.minloc, 
		 			&iline.max, &iline.maxloc);
		 	if	( EOF == ret ) {
		 		done = 1;
		 		break;
		 	}
		 	fprintf (mergefd, "%8.4lf %3d %8.4lf %3d %8.4lf %3d %8.4lf %3d\n",
		  			iline.min, iline.minloc, iline.max, iline.maxloc,
		  			tline.min, tline.minloc, tline.max, tline.maxloc );
		}
	}

	fclose ( allredfd );
	fclose ( minmaxfd );
	fclose ( iterfd );
	fclose ( mergefd );

    return ( 0 );
}


