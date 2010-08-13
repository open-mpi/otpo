/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "ADCL.h"
#include "mpi.h"

/* Constants Definition */
#define NB_OF_ITERATIONS      5000
#define BEST_CLASS            0
#define BEST_CLASS_THRESHOLD  5
#define MED_CLASS             1
#define WORST_CLASS           2
#define WORST_CLASS_THRESHOLD 50
#define NB_OF_IMPL            12
#define NB_OF_SIZES           16
#define VERBOSE               1

/* Implementations names */
char *impl_names[NB_OF_IMPL] = {"IsendIrecv_aao","SendIrecv_aao","IsendIrecv_aao_pack","SendIrecv_aao_pack",
                               "IsendIrecv_pair","IsendIrecv_pair_pack","SendIrecv_pair","SendIrecv_pair_pack",
                               "Sendrecv_pair","Send_Recv_pair","Sendrecv_pair_pack","Send_Recv_pair_pack"};

/* Dimensions of the data matrix per process */
int ProblemSizes[NB_OF_SIZES] = { 32, 36, 40, 44, 48, 52, 56, 60, 
                                  64, 68, 72, 76, 80, 84, 88, 92};

static void cluster_implementations(double *g_elapsed_time, int *impl_classes );
static double compute_distance(int i,int j);
static int winner_name2num(char *winner_name);
static double find_dmax(double *distance, double *relation);

int main ( int argc, char ** argv ) 
{
    /* General variables */
    int hwidth, rank, size, err;

    int i, j, k,it;
    char *winner_name;
    int impl_classes[NB_OF_SIZES][NB_OF_IMPL];
    int winners[NB_OF_SIZES][2];
    double t_start, dmax, distance[NB_OF_SIZES], relation[NB_OF_SIZES], elapsed_time[14], g_elapsed_time[NB_OF_SIZES][14];

    /* Definition of the 1-D vector */
    int dims;
    double *data, **data2;
    ADCL_Vmap vmap;
    ADCL_Vector vec;

    /* Variables for the process topology information */
    int cdims=0, periods=0;
    MPI_Comm cart_comm;
    ADCL_Topology topo;
    ADCL_Request request;

    /* Initiate the MPI environment */
    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    /* Describe the neighborhood relations */
    MPI_Dims_create ( size, 1, &cdims );
    MPI_Cart_create ( MPI_COMM_WORLD, 1, &cdims, &periods, 0, &cart_comm);

    /* Initiate the ADCL library and register a topology object with ADCL */
    ADCL_Init ();
    ADCL_Topology_create ( cart_comm, &topo );

    /**********************************************************************/
    /* hwidth=1, nc=0 */
    hwidth=1;
    for (i=0; i<NB_OF_SIZES; i++) {

        dims = ProblemSizes[i] + 2*hwidth;

        if(rank == 0) {
            printf("Explored Problem Size %d\n", ProblemSizes[i]);
        }

        err = ADCL_Vmap_halo_allocate ( hwidth, &vmap );
        if ( ADCL_SUCCESS != err) goto exit;
        err = ADCL_Vector_allocate_generic ( 1,  &dims, 0, vmap, MPI_DOUBLE, &data, &vec );
        ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &request );
        
        /* Evaluate implementation 1 */
        t_start = MPI_Wtime();
        for ( it=0; it<NB_OF_ITERATIONS; it++ ) {
             ADCL_change_sb_aao_IsendIrecv( request );
        }
        elapsed_time[0] = MPI_Wtime()-t_start;

        /* Evaluate implementation 2 */
        t_start = MPI_Wtime();
        for ( it=0; it<NB_OF_ITERATIONS; it++ ) {
             ADCL_change_sb_aao_SendIrecv( request );
        }
        elapsed_time[1] = MPI_Wtime()-t_start;

        /* Evaluate implementation 3 */
        t_start = MPI_Wtime();
        for ( it=0; it<NB_OF_ITERATIONS; it++ ) {
             ADCL_change_sb_aao_IsendIrecv_pack( request );
        }
        elapsed_time[2] = MPI_Wtime()-t_start;

        /* Evaluate implementation 4 */
        t_start = MPI_Wtime();
        for ( it=0; it<NB_OF_ITERATIONS; it++ ) {
             ADCL_change_sb_aao_SendIrecv_pack( request );
        }
        elapsed_time[3] = MPI_Wtime()-t_start;

        /* Evaluate implementation 5 */
        t_start = MPI_Wtime();
        for ( it=0; it<NB_OF_ITERATIONS; it++ ) {
             ADCL_change_sb_pair_IsendIrecv( request );
        }
        elapsed_time[4] = MPI_Wtime()-t_start;

        /* Evaluate implementation 6 */
        t_start = MPI_Wtime();
        for ( it=0; it<NB_OF_ITERATIONS; it++ ) {
             ADCL_change_sb_pair_IsendIrecv_pack( request );
        }
        elapsed_time[5] = MPI_Wtime()-t_start;

       /* Evaluate implementation 7 */
        t_start = MPI_Wtime();
        for ( it=0; it<NB_OF_ITERATIONS; it++ ) {
             ADCL_change_sb_pair_SendIrecv( request );
        }
        elapsed_time[6] = MPI_Wtime()-t_start;

        /* Evaluate implementation 8 */
        t_start = MPI_Wtime();
        for ( it=0; it<NB_OF_ITERATIONS; it++ ) {
             ADCL_change_sb_pair_SendIrecv_pack( request );
        }
        elapsed_time[7] = MPI_Wtime()-t_start;

        /* Evaluate implementation 9 */
        t_start = MPI_Wtime();
        for ( it=0; it<NB_OF_ITERATIONS; it++ ) {
             ADCL_change_sb_pair_Sendrecv( request );
        }
        elapsed_time[8] = MPI_Wtime()-t_start;

        /* Evaluate implementation 10 */
        t_start = MPI_Wtime();
        for ( it=0; it<NB_OF_ITERATIONS; it++ ) {
             ADCL_change_sb_pair_Send_Recv( request );
        }
        elapsed_time[9] = MPI_Wtime()-t_start;

        /* Evaluate implementation 11 */
        t_start = MPI_Wtime();
        for ( it=0; it<NB_OF_ITERATIONS; it++ ) {
             ADCL_change_sb_pair_Sendrecv_pack( request );
        }
        elapsed_time[10] = MPI_Wtime()-t_start;

        /* Evaluate implementation 12 */
        t_start = MPI_Wtime();
        for ( it=0; it<NB_OF_ITERATIONS; it++ ) {
             ADCL_change_sb_pair_Send_Recv_pack( request );
        }
        elapsed_time[11] = MPI_Wtime()-t_start;


        /* Evaluate ADCL B.F amd P.H. */
        /* So far, ADCL will behace according to the config.adcl file */
        t_start = MPI_Wtime();
        for ( it=0; it<NB_OF_ITERATIONS; it++ ) {
             ADCL_Request_start( request );
        }
        elapsed_time[12] = MPI_Wtime()-t_start;
        /* Get winning implementation name */
        ADCL_Request_get_curr_function ( request, &winner_name, NULL, NULL, NULL, NULL);
        /* Get winning implementation number */
        winners[i][0] = winner_name2num(winner_name);

        /* Evaluate ADCL P.H */
        t_start = MPI_Wtime();
        for ( it=0; it<NB_OF_ITERATIONS; it++ ) {
             ADCL_Request_start( request );
        }
        elapsed_time[13] = MPI_Wtime()-t_start;
        /* Get winning implementation name */
        ADCL_Request_get_curr_function ( request, &winner_name, NULL, NULL, NULL, NULL);
        /* Get winning implementation number */
        winners[i][1] = winner_name2num(winner_name);

        MPI_Allreduce ( &elapsed_time, &g_elapsed_time[i], 14 , MPI_DOUBLE, MPI_MAX , MPI_COMM_WORLD);
        if(rank == 0) {
#ifdef VERBOSE
            /* Printing the timing for each implementation and ADCL */
            for(j=0;j<12;j++) {
                printf("Elapsed time using f%d:%s is %f\n",j,impl_names[j],g_elapsed_time[i][j]);
	    }
#endif
            printf("Winner using ADCL B.F. is f%d:%s\nWinner using ADCL P.H. is f%d:%s\n", 
                   winners[i][0], impl_names[winners[i][0]], winners[i][1], impl_names[winners[i][1]] );
            /* Cluster implementations */
            cluster_implementations(g_elapsed_time[i], impl_classes[i]);
	}

        ADCL_Request_free ( &request );
        ADCL_Vector_free ( &vec );
	ADCL_Vmap_free ( &vmap );
    }

    ADCL_Topology_free ( &topo );
    MPI_Comm_free ( &cart_comm );

    /* Processing the results */
    if(rank == 0) {
        /* Comparaison between problem sizes */
        for(i=0; i<NB_OF_SIZES; i++) {
            for (j=0; j<NB_OF_SIZES; j++) {
                distance[j] = compute_distance(i,j);
#ifdef VERBOSE
                printf("The distance between problems %d and %d is %f\n",i,j,distance[2*j]);
#endif
                relation[j] = 0;
                /* Is the winner of pb size i among the best impl of pb size j ? */
                for(k=0;k<NB_OF_IMPL;k++) {
                    if ( (winners[i][0]==k) && (impl_classes[j][k] == BEST_CLASS) ) {
                        relation[j] = 1;
#ifdef VERBOSE
                        printf("similar pb sizes: %d can help optimizing %d\n",i,j);
#endif
		    }
		}
            }
            dmax = find_dmax(distance,relation);
            printf( "Max distance for Pb size %d is %f\n",ProblemSizes[i], dmax);
        }
    }
    
exit:
    ADCL_Finalize ();
    MPI_Finalize ();
    return 0;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static void cluster_implementations(double *g_elapsed_time, int *impl_class )
{
    int i, best_impl;
    double best_threshold, worst_threshold;

    best_impl = 0;
    best_threshold = 1 + (BEST_CLASS_THRESHOLD/100.00);
    worst_threshold = 1 + (WORST_CLASS_THRESHOLD/100.00);

    for(i=1; i<NB_OF_IMPL; i++) {
        if(g_elapsed_time[i] < g_elapsed_time[best_impl]) {
            best_impl = i;
	}
    }
    printf("Best implementation: %d\n",best_impl);
    for(i=0; i<NB_OF_IMPL; i++) {
        if(g_elapsed_time[i]/g_elapsed_time[best_impl] < best_threshold) {
            impl_class[i] = BEST_CLASS;
	}  
        else if(g_elapsed_time[i]/g_elapsed_time[best_impl] > worst_threshold) {
            impl_class[i] = WORST_CLASS;
	}
        else {
            impl_class[i] = MED_CLASS;
	}
#ifdef VERBOSE
        printf("Implementation %d is class %d\n",i,impl_class[i]);
#endif
    }
    return;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static double compute_distance(int i,int j)
{
    double dist;

    if((i<0)||(j<0)||(i>=NB_OF_SIZES)||(j>=NB_OF_SIZES)) {
	dist =0;
    }
    else {
        dist = abs(ProblemSizes[i]-ProblemSizes[j]);
    }
#ifdef VERBOSE
    printf("The distance between probems %d and %d is %f\n",i,j,dist);
#endif
    return dist;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static int winner_name2num(char *winner_name)
{
    int i, winner_num;
    for(i=0;i<NB_OF_IMPL;i++) {
        if( strcmp(winner_name, impl_names[i])==0 ) {
            return i;
	}
    }
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static double find_dmax(double *distance, double *relation)
{
    int i, k, extendable, bound, last_swap;
    double d,r, dmax;

    bound = NB_OF_SIZES-1;
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
    for (i=0; i<NB_OF_SIZES; i++) {
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
