#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "minmax_generic.h"

/* Parameters of the application */
int outlier_factor=3, outlier_fraction=20;
int output_files=1; /* true */
int numprocs=-1, numreqs=-1, nummeas=-1;
int *nummethods=NULL;
int *idx_methodstart=NULL;
char **fnctsets=NULL;
int deconly=1;
int filter=0;             

/* Prototypes */
void minmax_init     (int argc, char ** argv, struct emethod ****em );
void minmax_read_input ( struct emethod ***em ); //  em[request][proc][implementation]
void minmax_read_params ( char* parfile );
void minmax_read_perfline( char line[MAXLINE], int* req, int* method, double* time);
void minmax_finalize ( struct emethod ****em );

void minmax_filter_timings     ( int r, struct emethod **em, int ofac, int nmeas );
void minmax_calc_per_iteration ( int r, struct emethod **em, char *filename );
void minmax_calc_statistics    ( int r, struct emethod **em, char *filename );
void minmax_clear_poison_field ( int r, struct emethod **em);
void minmax_calc_decision      ( int r, struct emethod **em, int outlier_fraction ); 
#if defined(GSL) || defined(NR)
void minmax_calc_robust        ( int r, struct emethod **em, char *filename );
#endif

int HierarchicalClusterAnalysis(char metric, int transpose, char method, 
double* avg, int* nfilt);
void init_cluster_vars(const int ndata, double *data);
void minmax_calc_cluster ( int r, struct emethod **em, char *filename ); 
void free_cluster_vars();



static int tcompare ( const void*, const void* );
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int main (int argc, char **argv )
{
    struct emethod ***emethods;
    int r, i;

    /* Aquire the required memory and read input files */
    minmax_init ( argc, argv, &emethods );
    minmax_read_input ( emethods );

//    /* Early stopping Criterion: this does not work as expected */
//    for (r=0; r<numreqs; r++){
//       printf("\n\n********************** Request %d *************************\n", r);
//       for (i=0; i<nummethods[r]; i++){
//          minmax_early_stopping ( r, i, emethods[r]); 
//       }
//    }

    /* Second step: calculate statistics, filter data etc. */
    for (r=0; r<numreqs; r++){
       printf("\n\n********************** Request %d *************************\n", r);
       printf("\nHEURISTIC\n\n");
       minmax_calc_decision    ( r, emethods[r], outlier_fraction );
       minmax_filter_timings   ( r, emethods[r], outlier_factor, -1);

       if ( output_files ) {
          minmax_calc_per_iteration ( r, emethods[r], "minmax.out" );
       }

       printf("\nMEDIAN\n\n");
       minmax_calc_statistics  ( r, emethods[r], NULL );

#if defined(GSL) || defined(NR)
       printf("\nROBUST STATISTICS\n\n");
       minmax_calc_robust      ( r, emethods[r], NULL );
#endif

       printf("\nCLUSTER ANALYSIS\n\n");
       minmax_calc_cluster     ( r, emethods[r], NULL );
       if ( output_files ) {
	minmax_calc_per_iteration ( r, emethods[r], "minmax-filtered.out" );
       }
    }

    /* Free the aquired memory */
    minmax_finalize (&emethods); 
    return ( 0 );
}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
void minmax_read_input ( struct emethod ***emethods ) 
/* reads *.out files and inits em_time and em_avg */
{
    char line[MAXLINE], *basestr;
    char inname[64],  reqstr[64], colon[1];
    struct lininf tline;
    int ret, req, i, no_line, all_done=0;
    int method, pos, count, pos_r, count_r;
    double time;
    FILE **infd=NULL;
    int r, j, t;
    int done; 
    int *decreq; // capture requests which have winners

    /* Open the input files */
    infd = (FILE **) malloc ( numprocs * sizeof(FILE *) );
    if ( NULL == infd ) {
	exit (-1);
    }

    for ( i = 0; i < numprocs; i++ ) {
	sprintf( inname, "%d.out", i);
	infd[i] = fopen ( inname, "r" );
	if (NULL == infd[i] ) {
	    printf("Could not open input file %s for reading\n", inname );
	    exit (-1);
	}
    }	

    if ( deconly ) {
       decreq = (int*) malloc ( numreqs * sizeof(int) );
       for ( r=0; r< numreqs; r++){
           decreq[r] = 0;
       }
    }


    /* Read simultaneously all infile and store the values in the according emethod structures */
    while ( all_done < numprocs ) {
	TLINE_INIT(tline);
	
        /* read next "meaningful" line out of infiles */
	for ( i=0; i< numprocs; i++ ) {
	    no_line = 0;
	    while ( !no_line ) {
		ret = fscanf ( infd[i], "%[^\n]\n", line );
		if ( EOF == ret ) {
		    all_done++;
		    break;
		}
		if ( NULL != strstr ( line, "winner is") &&
		     deconly ){
		   basestr = strstr ( line, ":" );
		   sscanf ( basestr, "%1s %s %d", colon, reqstr, &tline.req );
		   decreq[tline.req] = 1;
                   /* XXX: here was an exit */
                   /* this makes some trouble for verification runs */
		   continue;
		}
		if ( line[0] == '#' ) {
		    /* Skip other comment lines */
		    continue;
		}
		no_line = 1;
	    } /* end while */	    
	    
	    if ( i == 0 ) {
		/*read the parameters of the run out of first infile */
		minmax_read_perfline(line, &tline.req, &tline.method, &time);
	        tline.method -= idx_methodstart[tline.req];
		//printf("%s %d method %d time %lf\n", "timer", tline.req, tline.method, time);
	    }
	    else {
		/*read the parameters of the run and do consitency checks with first infile */
		minmax_read_perfline(line, &req, &method, &time);

		if ( req != tline.req ) {
		    printf("Request mismatch at process %d \n", i);
		}
	        method -= idx_methodstart[req];

		if ( method != tline.method ) {
		    printf ("Method mismatch at process %d\n", i);
		}		
	    }	
	    if ( deconly && decreq[tline.req]) /* this does not work for verification runs */
	        continue; 

            //printf("%d: request %d method %d %lf\n", i, tline.req, tline.method, time);

	    pos = emethods[tline.req][i][tline.method].em_rescount;
	    count = emethods[tline.req][i][tline.method].em_count;
	    if ( pos < count ) { /* safety net for verification runs */
               emethods[tline.req][i][tline.method].em_time[pos] = time;
	       emethods[tline.req][i][tline.method].em_rescount++;
	       emethods[tline.req][i][tline.method].em_avg += time/count;
            } 
            //else if ( pos == count )  {
            //   /* see if all (other) requests are finished, too */ 
            //   done = 1;
            //   for (r=0; r<numreqs; r++){
            //      pos_r   = emethods[r][i][tline.method].em_rescount;
	    //      count_r = emethods[r][i][tline.method].em_count;
	    //      if ( pos_r < count_r ) {
            //         done = 0; 
            //         break; 
            //      }
	    //   }
            //   if ( done == 1 ) goto exit;
	    //}

	} /* end for */
    }

//     /* For Debugging */
//     for ( r=0; r<numreqs; r++){
//        for ( i=0; i< numprocs; i++){
//           for ( j=0; j< nummethods[r]; j++){
//              printf("emethods[%d][%d][%d]: count %d, rescount %d \n", r, i, j, 
// 	       emethods[r][i][j].em_count, emethods[r][i][j].em_rescount);
// 	     for ( t=0; t<emethods[r][i][j].em_count; t++){
// 	         printf("  time[%d] %lf\n", t, emethods[r][i][j].em_time[t]);
//              }
// 	     printf("\n");
// 	  }
//        }
// 
//     }


 exit:

    for ( i=0; i< numprocs; i++ ) {
	fclose ( infd[i]);
    }

    free ( infd );

    return;
}


/**********************************************************************/
void minmax_read_perfline( char line[MAXLINE], int* req, int* method, double* time){
/* reads one line of out-file                                         */
/**********************************************************************/
   char reqstr[64], str[64], colon[1];
   char *basestr;

   basestr = strstr ( line, ":" );
   sscanf ( basestr, "%s %s %d", colon, reqstr, req );

   basestr = strstr ( line, " method" );
   sscanf ( basestr, " %6s %d", str, method );

   basestr = strstr ( line, ")" );
   sscanf ( basestr, "%1s %lf\n", str, time );

   //printf("%s %d method %d time %lf\n", reqstr, *req, *method, *time);
}


/**************************************************************************************************/
void minmax_filter_timings ( int r_id, struct emethod **em, int outlier_factor, int nmeas ) {
/**************************************************************************************************/
/* determines em_cnt_outliers, em_cnt_filtered, em_sum_filtered, em_average_filtered              */
/* and em_perc_filtered                                                                           */ 
/* r_id           - request number                                                                */
/* em             - emethod object                                                                */
/* outlier_factor - measurements larger than outlier_factor * min are considered as outliers      */
/* nmeas          - #measurements (optional, for early stopping criterion)                        */  
/*                  if <=0 use em[i][j].em_rescount, else use nmeas                                */
/**************************************************************************************************/
   int i, j, k, rescount;
   double min;

   for (i=0; i < numprocs; i++ ) {
      for ( j=0; j< nummethods[r_id]; j++ ) {
         if ( 0 < nmeas ) {
            rescount = nmeas;
         }
         else {
            rescount = em[i][j].em_rescount;
         }
 
         em[i][j].em_sum_filtered = 0.0;
         em[i][j].em_cnt_outliers= 0;
         em[i][j].em_avg_filtered = 0.0;
       
         /* Determine the min  value for method [i][j]*/
         for ( min=999999, k=0; k<rescount; k++ ) {
            if ( em[i][j].em_time[k] < min ) {
                min = em[i][j].em_time[k];
            }
         }
       
         /* Count how many values are N times larger than the min and
            mark those as outliers, sum up execution times of other values */
         for ( k=0; k<rescount; k++ ) {
            if ( em[i][j].em_time[k] >= (outlier_factor * min) ) {
                em[i][j].em_poison[k] = 1;   // set to use minmax_calc_per_iteration */
                em[i][j].em_cnt_outliers++;
#ifdef DEBUG
            printf("#%d: request %d method %d meas. %d is outlier %lf min %lf\n",
		   i, r_id, j, k,  em[i][j].em_time[j], min );
#endif
            }
            else {
               em[i][j].em_sum_filtered += em[i][j].em_time[k];
            }
         }
         em[i][j].em_cnt_filtered = rescount - em[i][j].em_cnt_outliers; 

         /* calculate average (filtered) time and outlier percentage */
         em[i][j].em_avg_filtered  = em[i][j].em_sum_filtered / em[i][j].em_cnt_filtered; 
         em[i][j].em_perc_filtered = 100 * em[i][j].em_cnt_outliers / rescount;
      }
   }

   return;
}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
void minmax_calc_per_iteration ( int r, struct emethod **em, char *filename )
{
   int i, j, k;
   FILE *outf;
   struct lininf tline;

   outf = fopen(filename, "w");
   if ( NULL == outf ) {
       printf("calc_perit_filtered: could not open %s for writing\n", filename);
       exit (-1);
   }

   for (j=0; j< nummethods[r]; j++ ) {
      for ( k=0; k<nummeas; k++ ) {
         TLINE_INIT(tline);
         for (i=0; i< numprocs; i++ ) {
      	    if ( !em[i][j].em_poison[k] ) { 
      	        TLINE_MIN(tline, em[i][j].em_time[k], i);
      	        TLINE_MAX(tline, em[i][j].em_time[k], i);
      	    }
      	    else {
      	        em[i][j].em_cnt_outliers++;
      	    }
         }
         fprintf (outf, "%3d %8.4lf %3d %8.4lf %3d\n", 
      	    j, tline.min, tline.minloc, tline.max, 
      	    tline.maxloc );
      }
   
   }


  for ( i=0; i< numprocs; i++ ) {
     fprintf(outf, "# %d: ", i);
     for ( j=0; j< nummethods[r]; j++ ) {
          fprintf(outf, "%d ", em[i][j].em_cnt_outliers);
     }
     fprintf(outf, "\n");
     
  }

  fclose (outf);
  return;
}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static int tcompare ( const void *p, const void* q )
{
    double *a, *b;
    a = (double *) p;
    b = (double *) q;

    if ( *a < *b ) {
	return -1;
    }
    if ( *a == *b ) {
	return 0;
    }

    return 1;
}


void minmax_calc_statistics ( int r, struct emethod **em, char *filename ) 
{
    FILE *outf=NULL;
    struct lininf *tline, *tline_perc, tline2;
    int i, j, k;

    tline  = (struct lininf *) malloc ( sizeof(struct lininf) * nummethods[r] ); 
    tline_perc  = (struct lininf *) malloc ( sizeof(struct lininf) * nummethods[r] ); 
    if ( NULL == tline || NULL == tline_perc ) {
	printf("calc_statistics: could not allocate memory\n");
    }
    
    if ( NULL != filename ) {
	outf = fopen(filename, "a");
	if ( NULL == outf ) {
	    printf("calc_statistics: could not open %s for writing\n", filename );
	    exit (-1);
	}
	fprintf(outf, "\n");
    }
    
    minmax_clear_poison_field (r, em);
	
    /* Calculate the median of all measurement series */
    for (i=0; i < numprocs; i++ ) {
	for ( j=0; j< nummethods[r]; j++ ) {
	    qsort ( em[i][j].em_time, em[i][j].em_rescount, sizeof(double), 
		    tcompare );
	    if ( em[i][j].em_rescount % 2 == 0 ) {
		em[i][j].em_median = ( em[i][j].em_time[ em[i][j].em_rescount/2 ]  + 
				       em[i][j].em_time[(em[i][j].em_rescount/2) + 1 ] ) /2; 
	    }
	    else {
		em[i][j].em_median = em[i][j].em_time[ (em[i][j].em_rescount/2)+1];
	    }
	    
	    em[i][j].em_1stquartile = em[i][j].em_time[ em[i][j].em_rescount / 4 ];
	    em[i][j].em_3rdquartile = em[i][j].em_time[ em[i][j].em_rescount * 3 / 4 ];
	    em[i][j].em_iqr = em[i][j].em_3rdquartile - em[i][j].em_1stquartile; 
	    em[i][j].em_llimit = em[i][j].em_1stquartile  - (1.5 * em[i][j].em_iqr);
	    em[i][j].em_ulimit = em[i][j].em_3rdquartile  + (1.5 * em[i][j].em_iqr);
	    
	    em[i][j].em_cnt_outliers = 0;
	    em[i][j].em_sum_filtered = 0;
	    em[i][j].em_cnt_filtered = 0;
	    em[i][j].em_avg_filtered = 0;
	    em[i][j].em_perc_filtered = 0;
	    
	    for ( k=0; k< em[i][j].em_1stquartile; k++ ) {
		if ( em[i][j].em_time[k] < em[i][j].em_llimit ) {
		    em[i][j].em_poison[k] = 1;
		}
		else {
		    break;
		}
	    }
	    
	    for ( k=em[i][j].em_3rdquartile;  k< em[i][j].em_rescount;  k++ ) {
		if ( em[i][j].em_time[k] > em[i][j].em_ulimit ) {
		    em[i][j].em_poison[k] = 1;
		}
		else {
		    break;
		}
	    }
	    
	    for ( k=0; k < em[i][j].em_rescount; k++ ) {
		if ( em[i][j].em_poison[k] == 0 ) {
		    em[i][j].em_sum_filtered += em[i][j].em_time[k];
		    em[i][j].em_cnt_filtered ++;
		}
		else {
		    em[i][j].em_cnt_outliers++;
		}
	    }
	    em[i][j].em_perc_filtered = 100 * em[i][j].em_cnt_outliers/em[i][j].em_rescount;	    
	    em[i][j].em_avg_filtered  = em[i][j].em_sum_filtered / em[i][j].em_cnt_filtered;
	}	
    }
    
    TLINE_INIT ( tline2 );
    for (j=0; j< nummethods[r]; j++ ) {
	TLINE_INIT(tline[j]);
	TLINE_INIT(tline_perc[j]);
	for (i=0; i< numprocs; i++ ) {
	    TLINE_MAX(tline[j], em[i][j].em_avg_filtered, i);
	    TLINE_MAX(tline_perc[j], em[i][j].em_perc_filtered, i);
	}
	TLINE_MIN (tline2, tline[j].max, j );
	printf("%d : avg. filtered %lf perc. filtered %lf\n", j, tline[j].max, tline_perc[j].max );
    }
    
    if ( NULL != filename ) {
	fprintf(outf, "Standard formula winner is %d\n", tline2.minloc );
	fclose (outf);
    }
    else {
	printf("Standard formula winner is %d\n", tline2.minloc );
    }

    free ( tline );
    free ( tline_perc );
    return;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
void minmax_calc_decision ( int r, struct emethod **em, int outlier_fraction ) 
/* based on em_avg, em_avg_filtered, and em_perc_filtered, the maximum on each proc and the  */
/* total minimum are computed */
{
    struct lininf *tline_perc, *tline_unf, *tline_filt;
    int *meth;
    struct lininf tline_avg, tline_filtered_avg, tline_new;
    int i, j;

    tline_perc  = (struct lininf *) malloc ( sizeof(struct lininf) * nummethods[r] ); 
    tline_unf   = (struct lininf *) malloc ( sizeof(struct lininf) * nummethods[r] ); 
    tline_filt  = (struct lininf *) malloc ( sizeof(struct lininf) * nummethods[r] ); 
    meth  = (int *) malloc ( sizeof(int) * nummethods[r] ); 
    if ( NULL == tline_perc || NULL == tline_unf || NULL == tline_filt || NULL == meth) {
	printf("minmax_calc_decision: could not allocate memory\n");
	exit (-1);
    }

    /* get maximum of unfiltered, filtered and filtered percentage together with location on each proc */
    for (j=0; j< nummethods[r]; j++ ) {
        TLINE_INIT(tline_perc[j]);
        TLINE_INIT(tline_unf[j]);
        TLINE_INIT(tline_filt[j]);
        for (i=0; i< numprocs; i++ ) {
            TLINE_MAX(tline_unf[j], em[i][j].em_avg, i);
            TLINE_MAX(tline_filt[j], em[i][j].em_avg_filtered, i);
            TLINE_MAX(tline_perc[j], em[i][j].em_perc_filtered, i);
        }
    }

    /* The final decision and output */
    TLINE_INIT (tline_avg);
    TLINE_INIT (tline_filtered_avg);
    TLINE_INIT (tline_new);
    for ( j=0; j<nummethods[r]; j++  ) {
        TLINE_MIN (tline_avg, tline_unf[j].max, j);
        TLINE_MIN (tline_filtered_avg, tline_filt[j].max, j);

        printf("%d: result: ", j);
        if ( tline_perc[j].max > outlier_fraction ) {
            meth[j] = tline_unf[j].max;
            printf("%lf :uf: ", tline_unf[j].max );
        }
        else {
            meth[j] = tline_filt[j].max;
            printf("%lf : f: ", tline_filt[j].max );
        }
        TLINE_MIN(tline_new, meth[j], j);
        printf (" unfiltered: %lf filtered: %lf perc: %lf\n", 
               tline_unf[j].max, tline_filt[j].max, tline_perc[j].max );
    }

    printf("New decision winner is %d \n", tline_new.minloc + idx_methodstart[r] );
    if ( tline_perc[tline_filtered_avg.minloc].max < outlier_fraction ) 
       printf ("Prev. decision winner is %d (filtered)\n", tline_filtered_avg.minloc + idx_methodstart[r]);
    else
       printf ("Prev. decision winner is %d (unfiltered)\n", tline_avg.minloc + idx_methodstart[r] );

    free ( tline_perc );
    free ( tline_unf);
    free ( tline_filt);
    free ( meth);

    return;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
void minmax_read_params ( char* parfile ) 
{
    char line[MAXLINE], *basestr;
    //char inname[64],  reqstr[64];
    char reqstr[64];
    struct linepar tline;
    int ret, no_line, done=0;
    //int ret, req, i, no_line, done=0;
    //int method, pos, count;
    //double time;
    int nwinners = 0; 
    FILE *infd=NULL;

    /* Open the input files */
    printf( "Reading %s\n", parfile );
    infd = fopen ( parfile, "r" );
    if (NULL == infd ) {
        printf("Could not open input file %s for reading\n", parfile );
        exit (-1);
    }

    /* Read infile and store the values in the according emethod structures */
    while ( !done ) {
	TLINEPAR_INIT(tline);
	
	no_line = 0;
	while ( !no_line ) {
	    ret = fscanf ( infd, "%[^\n]\n", line );
	    if ( EOF == ret ) {
	        done++;
	        break;
	    }
	    if ( line[0] == '#' ) {
	        /* Skip comment lines */
	        continue;
	    }
	    no_line = 1;
	}	    
	
	/*read the parameters of the run */
	basestr = strstr ( line, "request" );
	sscanf ( basestr, "%7s %d", reqstr, &tline.req );
	
	basestr = strstr ( line, "fnctset" );
	sscanf ( basestr, "%7s %*s", reqstr, &tline.fnctset );
	
	basestr = strstr ( line, "nummeth" );
	sscanf ( basestr, "%7s %d\n", reqstr, &tline.nummeth );

	basestr = strstr ( line, "start" );
	sscanf ( basestr, "%7s %d\n", reqstr, &tline.idxmeth );

        nummethods[tline.req] = tline.nummeth;
	idx_methodstart[tline.req] = tline.idxmeth;
        //fcntset[tline.req] = tline.fnctset;
    }

 exit:

    fclose ( infd);

    return;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
void minmax_init (int argc, char ** argv, struct emethod ****emethods) 
{
    struct emethod ***em;
    char* parfile;
    int i, j, r, tmpargc=5;

    if (argc < 5 )
    {
	printf(" Usage : minmax <numprocs> <numreq> <nummeas> <parfile> <options>\n\n");
	//printf(" Usage : minmax <parfile> "<numprocs> <numreqs> <nummeas>"
	printf("   minmax takes the output files generated by ADCL and \n");
	printf("   determines for each individual measurement the minimal and \n");
	printf("   maximual value across all processes and the according locations\n");
	printf(" Options: \n");
	printf("   <numprocs>   : number of processes \n");
	printf("   <numreqs>    : number of requests \n");
	printf("   <nummeas>    : number of measurements per implementation \n");
	printf("   <parfile>    : name of parameter file \n");
	printf("   -deconly     : stop after the decision procedure\n");
	printf("   -ofiles      : write output files \n");
	printf("   -ofraction   : outlier fraction to be used \n");
	printf("   -ofactor     : outlier factor to be used \n");

	exit ( 1 ) ;
    }
    
    numprocs    = atoi ( argv[1] );
    numreqs     = atoi ( argv[2] );
    nummeas     = atoi ( argv[3]);
    parfile     = (char*) malloc (strlen(argv[4])+1); 
    strcpy ( parfile, argv[4]);
    
    while ( tmpargc < argc ) {
	if ( strncmp ( argv[tmpargc], "-deconly", strlen("-deconly") )== 0 ) {
	    deconly = 1;
	}
	else if ( strncmp(argv[tmpargc], "-ofiles", strlen("-ofiles") )== 0 ) {
	    deconly=1;
	    output_files = 1;
	}
	else if ( strncmp(argv[tmpargc], "-ofactor", strlen("-ofactor") )== 0 ) {
	    tmpargc++;
	    outlier_factor = atoi ( argv[tmpargc] );
	}
	else if ( strncmp(argv[tmpargc], "-ofraction", strlen("-ofraction") )== 0 ) {
	    tmpargc++;
	    outlier_fraction = atoi ( argv[tmpargc] );
	}

	tmpargc++;
    } 

    nummethods = (int *) malloc ( numreqs * sizeof(int) );
    idx_methodstart = (int *) malloc ( numreqs * sizeof(int) );

    minmax_read_params ( parfile );

    em = (struct emethod ***) malloc ( numreqs * sizeof(struct emethod**) ); 
    if ( NULL != emethods ) {
        for ( r=0; r<numreqs; r++){
	   /* Allocate the required emethods array to hold the overall data */
	   em[r] = ( struct emethod **) malloc ( numprocs * sizeof ( struct emethod *));
	   if ( NULL == em[r] ) {
	       exit (-1);
	   }
	   
	   for ( i=0; i< numprocs; i++ ) {
	       em[r][i] = (struct emethod *) calloc (1, nummethods[r]*sizeof(struct emethod));
	       if  ( NULL == em[r][i] ) {
	   	exit (-1);
	       }
	       
	       for (j=0; j< nummethods[r]; j++ ) {
	   	em[r][i][j].em_time = (double *) calloc (1, nummeas * sizeof(double));
	   	if ( NULL == em[r][i][j].em_time ) {
	   	    exit (-1);
	   	}
	   	em[r][i][j].em_poison = (int *) calloc (1, nummeas * sizeof(int));
	   	if ( NULL == em[r][i][j].em_poison ) {
	   	    exit (-1);
	   	}
                em[r][i][j].em_count    = nummeas;
	       }
	   }
	}
    }
    *emethods = em;

    return;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
void minmax_clear_poison_field ( int r, struct emethod **em)
{
    int i, j, k;

    for ( i=0; i<numprocs; i++ ) {
        for (j=0; j< nummethods[r]; j++ ) {
            for ( k=0; k<nummeas; k++ ) {
                em[i][j].em_poison[k] = 0;
            }
        }
    }

    return;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
void minmax_finalize ( struct emethod ****emethods )
{
    int i, j, r;
    struct emethod ***em = *emethods;

    for ( i=0; i< numprocs; i++ ) {
       for ( r=0; r< numreqs; r++) {
          for (j=0; j< nummethods[r]; j++ ) {
             if ( NULL != em[r][i][j].em_time ) {
               free ( em[r][i][j].em_time );
             }
             
             if ( NULL != em[r][i][j].em_poison ) {
               free ( em[r][i][j].em_poison );
             }
          }
          if ( NULL != em[r][i] ) {
              free ( em[r][i] );
          }
       }
    }

    if ( NULL != em ) {
	free ( em );
    }

    *emethods = NULL;
    return;
}
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
#if defined(GSL) || defined(NR)
void minmax_calc_robust ( int r, struct emethod **em, char *filename ) 
{
    FILE *outf=NULL;
    struct lininf *tline, tline2;
    int i, j;
    double nu, sigma, val;

    tline  = (struct lininf *) malloc ( sizeof(struct lininf) * nummethods[r] ); 
    if ( NULL == tline ) {
	printf("calc_robust: could not allocate memory\n");
    }
    
    if ( NULL != filename ) {
	outf = fopen(filename, "a");
	if ( NULL == outf ) {
	    printf("calc_robust: could not open %s for writing\n", filename );
	    exit (-1);
	}
	fprintf(outf, "\n");
    }

    /* Calculate the median of all measurement series */
    /* 30 measurements are not enough to estimate nu from set (nu=0 as input),
       so choose some reasonable value for nu, which is 4.0 or 6.0 */
#ifdef NU6
    nu = 6.0;
    for (i=0; i < numprocs; i++ ) {
	for ( j=0; j< nummethods[r]; j++ ) {
	    ml ( em[i][j].em_rescount, em[i][j].em_time, &nu, &(em[i][j].em_avg_filtered) , 
		 &sigma, &val );
	    
	}
    }
#endif    
    /* Caculate ML-estimate of mu and sigma for t model */
    /* 30 measurements are not enough to estimate nu from set (nu=0 as input),
       so choose some reasonable value for nu, which is 4.0 or 6.0 */
    nu = 4.0;
    double avg, avgmin = 1e10, romin, romax ;
    for ( j=0; j< nummethods[r]; j++ ) {
        avg = 0.0; romin=1e10; romax=0;
        for (i=0; i < numprocs; i++ ) {
            /* brute force 2 */
            /*  if (i==23 && j == 7) {
                em[i][j].em_avg_filtered=40837.784755;  continue;
              }
              else if (i==61 && j==7){
                 em[i][j].em_avg_filtered=38184.112130; continue;
              }*/

            /*if (i==24 && j == 3) {
                em[i][j].em_avg_filtered = 88684.864138;
                continue;
            }*/
	    ml ( em[i][j].em_rescount, em[i][j].em_time, &nu, &(em[i][j].em_avg_filtered) , 
		 &sigma, &val );
            avg = avg + em[i][j].em_avg_filtered;
            if (em[i][j].em_avg_filtered < romin) romin = em[i][j].em_avg_filtered;
            if (em[i][j].em_avg_filtered > romax) romax = em[i][j].em_avg_filtered;
            // printf("proc=%d, method=%d, mu=%lf, sigma=%lf\n", i, j, em[i][j].em_avg_filtered, sigma);
            
	}
        //printf("min: %lf avg %lf max %lf\n", romin, avg/numprocs, romax);
        if (avg < avgmin) avgmin=avg;
    }

	//printf("Robust formula winner has minimum avg %fl\n", avgmin / nummeas);
    
    TLINE_INIT ( tline2 );
    for (j=0; j< nummethods[r]; j++ ) {
	TLINE_INIT(tline[j]);
	for (i=0; i< numprocs; i++ ) {
	    TLINE_MAX(tline[j], em[i][j].em_avg_filtered, i);
	}
	printf("%d: average: %lf \n", j, tline[j].max);
	TLINE_MIN (tline2, tline[j].max, j );
    }
    
    if ( NULL != filename ) {
	fprintf(outf, "Robust formula nu = 4 winner is %d\n", tline2.minloc );
	fclose (outf);
    }
    else {
	printf("Robust formula nu = 4 winner is %d\n", tline2.minloc );
    }



    
    free ( tline );
    return;
}
#endif

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
void minmax_calc_cluster ( int r, struct emethod **em, char *filename ) 
{
    FILE *outf=NULL;
    struct lininf *tline, *tline_perc, tline2;
    int i, j, nfilt;
    char  genemetric = 'e'; 
    //arraymetric = getmetric(e);
    //jobname = setjobname(argv[i],0);
    char method = 'a';

    tline  = (struct lininf *) malloc ( sizeof(struct lininf) * nummethods[r] ); 
    tline_perc  = (struct lininf *) malloc ( sizeof(struct lininf) * nummethods[r] ); 
    if ( NULL == tline || NULL == tline_perc ) {
	printf("calc_cluster: could not allocate memory\n");
    }
    
    if ( NULL != filename ) {
	outf = fopen(filename, "a");
	if ( NULL == outf ) {
	    printf("calc_cluster: could not open %s for writing\n", filename );
	    exit (-1);
	}
	fprintf(outf, "\n");
    }
    

    for ( j=0; j< nummethods[r]; j++ ) {
        TLINE_INIT ( tline_perc[j] );
        for (i=0; i < numprocs; i++ ) {
            //printf("proc %d, method=%d\n", i,j);
 
            init_cluster_vars(em[i][j].em_rescount, em[i][j].em_time);
	    HierarchicalClusterAnalysis(genemetric, 0, method, &(em[i][j].em_avg_filtered), &nfilt);
	    TLINE_MAX ( tline_perc[j], nfilt, i );
            free_cluster_vars();
	}
    }
    
    TLINE_INIT ( tline2 );
    for (j=0; j< nummethods[r]; j++ ) {
	TLINE_INIT(tline[j]);
	for (i=0; i< numprocs; i++ ) {
	    TLINE_MAX(tline[j], em[i][j].em_avg_filtered, i);
	}
	printf("%d: filtered: %lf  perc filtered %lf \n", j, tline[j].max, 
                                                          (tline_perc[j].max/nummeas)*100) ;
	TLINE_MIN (tline2, tline[j].max, j );
    }
    
    if ( NULL != filename ) {
	fprintf(outf, "Cluster analysis winner is %d\n", tline2.minloc );
	fclose (outf);
    }
    else {
	printf("Cluster analysis winner is %d\n", tline2.minloc );
    }
    
    free ( tline );
    free ( tline_perc );
    return;
}
