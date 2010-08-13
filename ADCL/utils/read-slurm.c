#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Constants Definition */
#define MAXLINE               128
#define MAXSIZENAME           12
#define MAXNAMELEN            48
#define NB_OF_IMPL            12
#define BEST_CLASS            0
#define MED_CLASS             1
#define WORST_CLASS           2
#define WORST_CLASS_THRESHOLD 50
//#define VERBOSE               1

/* Static functions declaration */
static int cluster_implementations( double *elapsed_time, int *impl_classes, int p );
static double compute_distance( int i,int j, int **ProblemSizes, int num_sizes, int num_dims );
static double find_dmax( double *distance, double *relation, int num_sizes );

int main (int argc, char **argv )
{
    FILE *infd=NULL;
    char line[MAXLINE];
    char *basestr;
    char infilename[MAXNAMELEN];
    char temp[MAXNAMELEN];
    int i, j, k, p, best_cnt, num_sizes,num_dims, sizes_cnt;
    int *winner;
    double **elapsed_time;
    double *distance;
    double *relation;
    char **Pb_sizes;
    int **ProblemSizes;
    int **impl_classes;
    double *dmax;
    double avg, std_dev;
    int predicted, isok, predicted_winner, closet_pbsize;
    double min_dist, curr_dist;
    double prediction_weight[NB_OF_IMPL];
    double max_weight;

    if ( argc < 2 ) {
	printf(" Usage : read-slurm <infilename> [<num_sizes>][<num_dims>] \n\n");
	printf(" This program takes the slurm output file of PbSizes tests\n"
	       " and summarizes it. It is far from being general\n");
	exit ( 1 ) ;
    }

    strncpy (infilename, argv[1], MAXNAMELEN );

    infd = fopen ( infilename, "r" );
    if ( NULL == infd ) {
	printf("read-slurm: could not open %s for reading\n", infilename );
	exit (-1);
    }

    if ( argc == 4 ) {
	num_sizes = atoi ( argv[2] );
    }

    if ( argc == 4 ) {
	num_dims = atoi ( argv[3] );
    }
    
    winner = (int *)malloc(num_sizes * sizeof(int));
    if ( NULL == winner) {
        printf("read-slurm: could not allocate memory\n");
    }

    Pb_sizes = (char **)malloc(num_sizes * sizeof(char *));
    if ( NULL == Pb_sizes) {
        printf("read-slurm: could not allocate memory\n");
    }
    for ( i=0; i<num_sizes; i++ ) {
        Pb_sizes[i] = (char *) malloc (  MAXNAMELEN * sizeof (char));
	if ( NULL == Pb_sizes[i]) {
	    printf("read-slurm: could not allocate memory\n");
	}
    }

    ProblemSizes = (int **)malloc(num_sizes * sizeof(int *));
    if ( NULL == ProblemSizes ) {
        printf("read-slurm: could not allocate memory\n");
    }
    for ( i=0; i<num_sizes; i++ ) {
        ProblemSizes[i] = (int *) malloc ( num_dims * sizeof(int));
	if ( NULL == ProblemSizes[i] ) {
	    printf("read-slurm: could not allocate memory\n");
	}
    }

    elapsed_time = (double **)malloc(num_sizes * sizeof(double *));
    if ( NULL == Pb_sizes ) {
        printf("read-slurm: could not allocate memory\n");
    }

    for ( i=0; i<num_sizes; i++ ) {
	elapsed_time[i] = (double *) malloc ( NB_OF_IMPL * sizeof (double ));
	if ( NULL == elapsed_time[i] ) {
	    printf("read-slurm: could not allocate memory\n");
	}
    }

    impl_classes = (int **)malloc(num_sizes * sizeof(int *));
    if ( NULL == impl_classes ) {
        printf("read-slurm: could not allocate memory\n");
    }
    for ( i=0; i<num_sizes; i++ ) {
        impl_classes[i] = (int *) malloc ( NB_OF_IMPL * sizeof(int));
	if ( NULL == impl_classes[i]) {
	    printf("read-slurm: could not allocate memory\n");
	}
    }
    distance = (double *) malloc ( num_sizes * sizeof (double ));
    relation = (double *) malloc ( num_sizes * sizeof (double ));
    dmax = (double *) malloc ( num_sizes * sizeof (double ));

    /* Reading the slurm file and saving the data into the allocated structures/tables */
    sizes_cnt =0; /* number of pb sizes read */
    while ( fscanf ( infd, "%[^\n]\n", line ) != EOF ) 
    {
	basestr = strstr ( line, "Size");
	if ( NULL != basestr &&  sizes_cnt<num_sizes  ) {
	    sscanf ( basestr+strlen("Size"), "%s", Pb_sizes[sizes_cnt] );        
            sscanf ( Pb_sizes[sizes_cnt], "%d", &ProblemSizes[sizes_cnt][0] );
            basestr = Pb_sizes[sizes_cnt];
            for(i=1; i<num_dims; i++) {
                basestr = strstr ( basestr , "X");
                basestr = basestr + strlen("X");
                sscanf ( basestr, "%d", &ProblemSizes[sizes_cnt][i] );
	    }
            for (i=0; i<NB_OF_IMPL ;i++) {
		fscanf ( infd, "%[^\n]\n", line );
                basestr = strstr ( line, "is");
                if ( NULL != basestr &&  sizes_cnt<num_sizes  ) {
	            sscanf ( basestr+strlen("is"), "%s", temp );
                    elapsed_time[sizes_cnt][i] = atof(temp);
	        }
	    }
	    fscanf ( infd, "%[^\n]\n", line );
            basestr = strstr ( line, "f");
            if ( NULL != basestr &&  sizes_cnt<num_sizes  ) {
                sscanf ( basestr+strlen("f"), "%s", temp );
                winner[sizes_cnt] = atoi(temp);
	    }
	    sizes_cnt++;
	}

        if ( sizes_cnt == num_sizes ) {
          break; 
        }
    }
    fclose ( infd );

    /* Post processing of the data */
    for (p=1; p<=20; p++) { /* Loop over p: the acceptable performance window */
        /* Clustering implementations according to the value of p */
        best_cnt = 0;
        for(i=0; i<num_sizes; i++) {
            best_cnt += cluster_implementations(elapsed_time[i], impl_classes[i], p);
        }
#ifdef VERBOSE
        printf("p=%d best_cnt avg =%f \n", p,100*(double)best_cnt/(NB_OF_IMPL*(double)num_sizes));
#endif
        /* Comparaison between problem sizes */
        for(i=0; i<num_sizes; i++) {
            for (j=0; j<num_sizes; j++) {
                distance[j] = compute_distance(i, j, ProblemSizes, num_sizes, num_dims);
                relation[j] = 0;
                /* Is the winner of pb size i among the best impl of pb size j ? */
                for(k=0;k<NB_OF_IMPL;k++) {
                    if ( (winner[i] == k) && (impl_classes[j][k] == BEST_CLASS) ) {
                        relation[j] = 1;
                    }
                }
            }
            /* Computing dmax */
            dmax[i] = find_dmax(distance, relation, num_sizes);
#ifdef VERBOSE
            printf( "Max distance for Pb size %dX%dX%d is %f\n",ProblemSizes[i][0],
                    ProblemSizes[i][1], ProblemSizes[i][2], dmax[i]);
#endif
        }
        /* Given a new pb size k, option 1 : let's take the winner of the closest pb size */
        predicted = 0;
        isok = 0;
        for(k=0;k<num_sizes;k++) {
            /* init */
            closet_pbsize = 0;
            min_dist = compute_distance(0, k, ProblemSizes, num_sizes, num_dims);
            /* update */
            for(i=0; (i<num_sizes)&&(i!=k); i++) {
                curr_dist = compute_distance(i, k, ProblemSizes, num_sizes, num_dims);
                if(curr_dist <= min_dist) {
                    min_dist = curr_dist;
                    predicted_winner = winner[i];
                    closet_pbsize = i;
                }
            }
#ifdef VERBOSE
            printf("min dist=%f dmax=%f\n",min_dist, dmax[closet_pbsize]);
#endif
	    if ((winner[k] == predicted_winner)&&(min_dist <= dmax[closet_pbsize])) {
                predicted ++;
            }
            if (impl_classes[k][predicted_winner] ==  BEST_CLASS ) {
                isok++;
            }
        }
        printf("p=%d using the closest problem size technique:\n", p);
        printf("    Exact prediction rate = %f\n    rate within top predictions = %f\n", (double)(predicted*100)/num_sizes,(double)(isok*100)/num_sizes);
        /* Given a new pb size k, option 2 : let's take the majority vote from the pb sizes within dmax */
        predicted = 0;
        isok = 0;
        for(k=0;k<num_sizes;k++) {
            /* init */
            for(i=0; i<num_sizes; i++) {
	    prediction_weight[i]=0;
	    }
            /* update */
            for(i=0; (i<num_sizes)&&(i!=k); i++) {
                curr_dist = compute_distance(i, k, ProblemSizes, num_sizes, num_dims);
                if( curr_dist <= dmax[i] ) {
                    predicted_winner = winner[i];
                    prediction_weight[predicted_winner]+=1/curr_dist;
	        }
            }
            max_weight = prediction_weight[0];
            predicted_winner = 0;
            for(i=1; i<NB_OF_IMPL; i++) {
                if ( prediction_weight[i] > max_weight ) {
                    max_weight = prediction_weight[i];
                    predicted_winner = i;
                }
            }
            if ( winner[k] == predicted_winner ) {
                predicted ++;
            }
            if (impl_classes[k][predicted_winner] ==  BEST_CLASS ) {
                isok++;
            }
        }
        printf("p=%d using the weighted majority vote technique:\n", p);
        printf("    Exact prediction rate = %f\n    rate within top predictions = %f\n", (double)(predicted*100)/num_sizes,(double)(isok*100)/num_sizes);

    }/* p loop */

    /* Free allocated memory */
    free(winner);
    for ( i=0; i<num_sizes; i++ ) {
	free(Pb_sizes[i]);
    }
    free(Pb_sizes);
    for ( i=0; i<num_sizes; i++ ) {
	free(ProblemSizes[i]);
    }
    free(ProblemSizes);
    for ( i=0; i<num_sizes; i++ ) {
	free(elapsed_time[i]);
    }
    free(elapsed_time);
    for ( i=0; i<num_sizes; i++ ) {
	free(impl_classes[i]);
    }
    free(impl_classes);
    free(distance);
    free(relation);

    return 0;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static int cluster_implementations(double *elapsed_time, int *impl_class, int p )
{
    int i, cnt, best_impl;
    double best_threshold, worst_threshold;

    /* Initialization */
    cnt = 0;
    best_impl = 0;
    best_threshold = 1 + (p/100.00);
    worst_threshold = 1 + (WORST_CLASS_THRESHOLD/100.00);
    for(i=1; i<NB_OF_IMPL; i++) {
        if(elapsed_time[i] < elapsed_time[best_impl]) {
            best_impl = i;
	}
    }
    for(i=0; i<NB_OF_IMPL; i++) {
        if(elapsed_time[i]/elapsed_time[best_impl] < best_threshold) {
            impl_class[i] = BEST_CLASS;
            cnt ++;
	}  
        else if(elapsed_time[i]/elapsed_time[best_impl] > worst_threshold) {
            impl_class[i] = WORST_CLASS;
	}
        else {
            impl_class[i] = MED_CLASS;
	}
#ifdef VERBOSE
        printf("Implementation %d is class %d\n",i,impl_class[i]);
#endif
    }
    return cnt;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static double compute_distance(int i,int j, int **ProblemSizes, int num_sizes, int num_dims)
{
    double dist;
    /* The distance uised here is the euclidian distance */
    if((i<0)||(j<0)||(i>=num_sizes)||(j>=num_sizes)) {
	dist =0;
    }
    else {
        if( 3 == num_dims) {
            dist = sqrt( pow((ProblemSizes[i][0]-ProblemSizes[j][0]),2) +
                         pow((ProblemSizes[i][1]-ProblemSizes[j][1]),2) + 
                         pow((ProblemSizes[i][2]-ProblemSizes[j][2]),2));
	}
        if( 2 == num_dims) {
            dist = sqrt( pow((ProblemSizes[i][0]-ProblemSizes[j][0]),2) +
                         pow((ProblemSizes[i][1]-ProblemSizes[j][1]),2) );
	}
        if( 1 == num_dims) {
            dist =abs(ProblemSizes[i][0]-ProblemSizes[j][0]);
	}
    }
#ifdef VERBOSE
    printf("The distance between problems %d and %d is %f\n",i,j,dist);
#endif

    return dist;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static double find_dmax(double *distance, double *relation, int num_sizes )
{
    int i, k, extendable, bound, last_swap;
    double d,r, dmax;

    bound = num_sizes-1;
    dmax = 0;

    /* Sort distance array and move relation array with it */
    while (bound) {
      last_swap = 0;
      for ( k=0; k<bound; k++ ) {
         d = distance[k]; /* t is a maximum of A[0]..A[k] */
         r = relation[k];
         if ( d > distance[k+1] ) {
           distance[k] = distance[k+1];
           distance[k+1] = d; /*swap*/
           relation[k] = relation[k+1];
           relation[k+1] = r; /*swap*/           
           last_swap = k; /* mark the last swap position */
         }
      }
      bound=last_swap;
    }
    extendable = 1;
    /* searching the first 0 in the sorted array */
    for (i=0; i<num_sizes; i++) {
#ifdef VERBOSE
        printf("%f  %f  \n", distance[i],relation[i]);
#endif
        if ((relation[i]==1) && (extendable==1)) {
            dmax = distance[i];
	}
        if(relation[i] == 0) {
            extendable = 0;
	}
    }
    return dmax;     
}
