/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ATF.h"

#define MAXLINE 100
#define MAXKEYLEN 100

/* The global variables: */

int ATF_maxnproblem;
int ATF_maxnsolver;
/* int ATF_maxnpattern;*/

int ATF_nhosts ;
int ATF_verbose;
int ATF_autos ;

int *ATF_firstranks;
int *ATF_problemsx;
int *ATF_problemsy;
int *ATF_problemsz;
int *ATF_solvarr;
/* int *ATF_patternarr;*/

float ATF_tol;
int ATF_nit;

int ATF_Read_config(int *nPr, int *nSol)
{
    int i, j, flag, flag2;
    int rank, size;
    int iarray[7];		/* Store data into this array */
    float rarray[2];
    
    /* bool isallocated; */
    
    /* The variables below should be defined in Global_Modules */
    
    /*    float tol */
    float subtol;
    int franks;
    
    FILE *Conf_File;
    
    char keyword[MAXKEYLEN];
    char temp[MAXKEYLEN];
    char buffer[MAXLINE];
    char *ptr;
    
    
    
    
/*********************************************************/
    
    MPI_Comm_size( MPI_COMM_WORLD, &size);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank);
    
    /* Initiate: */
    
    ATF_nit=300;
    ATF_nhosts = 1;
    ATF_verbose =1;
    
    ATF_autos = 0;
    ATF_tol = 1.0e-06;
    subtol= 1.0e-06;
    
    ptr = NULL;
    flag =0;
    flag2=0;
    
    ATF_firstranks = NULL;
    ATF_problemsx = NULL;
    ATF_problemsy = NULL;
    ATF_problemsz = NULL;
    ATF_solvarr = NULL;
    /* ATF_patternarr = NULL;*/
    
    memset(buffer, 0, MAXLINE);
    memset(keyword, 0, MAXKEYLEN);
    
    
    if (rank == 0){
	
	/* read data into iarray and rarray */
	if ( (Conf_File =fopen("System.config", "r")) == NULL){
	    
	    fprintf(stderr, "%d @ %s, open file error !\n",  __LINE__,__FILE__);
	    return ATF_ERROR;
	}	
	
	while (fgets(buffer, MAXLINE, Conf_File) != NULL) {
	    
	    
	    /* check blank line or # comment */
	    if( buffer[0] != '#' ){
		
		/* printf("This is a line!, %s\n", buffer); */
		
		/* Parse one single line! */
		i = 0;
		flag =0;
		while(i<strlen(buffer)) {
		    temp[i]= buffer[i];
		    if(buffer[i] == ':'){
			strncpy(keyword, temp, i);
			/*	printf("Keyword is %s\n",keyword); */
			i++;
			flag =1;	/* this means this line has keywords; */
			ptr = &buffer[i];
			break;		
		    }
		    /* 	else{ */
		    /* 		printf("This is a NULL line!\n"); */
		    /* 	} */
		    i++;
		}
		
		/*  If this is a keyworkd...... */
		if(flag ==1){
		    
		    /*  Nit; */
		    if(strncmp(keyword, "nit", strlen("nit")) == 0){
			
			sscanf(ptr, "%d", &ATF_nit);
#ifdef Debug
			printf("nit : %d\n", ATF_nit);	
#endif
			ptr = NULL;						
		    }
		    
		    /*  tol */
		    if(strncmp(keyword, "tol", strlen("tol")) == 0){
			
			sscanf(ptr, "%f", &ATF_tol);
#ifdef Debug
			printf("tol : %f\n", ATF_tol);	
#endif
			ptr = NULL;						
		    }
		    
		    /* subtol */
		    if(strncmp(keyword, "subtol", strlen("subtol")) == 0){
			
			sscanf(ptr, "%f", &subtol);
#ifdef Debug
			printf("subtol : %f\n", subtol);	
#endif
			ptr = NULL;						
		    }
		    
		    
		    /* verbos */
		    if(strncmp(keyword, "verbos", strlen("verbos")) == 0){
			
			sscanf(ptr, "%d", &ATF_verbose);
#ifdef Debug
			printf("verbos : %d\n", ATF_verbose);	
#endif
			ptr = NULL;						
		    }
		    
		    
		    /* nprobs */
		    if(strncmp(keyword, "nprobs", strlen("nprobs")) == 0){
			
			sscanf(ptr, "%d", &ATF_maxnproblem);
#ifdef Debug
			printf("nprobs : %d\n", ATF_maxnproblem);	
#endif			
			/*  allocate space to problems; */
			ATF_problemsx =  (int *)malloc(sizeof(int) * ATF_maxnproblem);
			ATF_problemsy =  (int *)malloc(sizeof(int) * ATF_maxnproblem);
			ATF_problemsz =  (int *)malloc(sizeof(int) * ATF_maxnproblem);
			
			if((ATF_problemsx == NULL) ||(ATF_problemsy == NULL)||(ATF_problemsy == NULL)){
			    
			    printf("Memory malloc error at %d of %s!\n", __LINE__, __FILE__);
			}
			/* read line from the next nprobs lines */
			
			/* Read problem x */
			memset(buffer, 0 ,strlen(buffer));
			fgets(buffer, MAXLINE, Conf_File);
			ptr = buffer;
			
			/* printf("The buffer for x is %s, %d\n", ptr,k); */
			for(j=0;j<ATF_maxnproblem;j++){
			    sscanf(ptr, "%d", &ATF_problemsx[j]);
			    while( *ptr!=' '){
				ptr ++;
			    }
			    ptr ++;
			}	
			
			/* Read problem y */
			memset(buffer, 0 ,strlen(buffer));
			fgets(buffer, MAXLINE, Conf_File);
			ptr = buffer;
			/* printf("The buffer for y is %s, %d\n", ptr,k); */
			for(j=0;j<ATF_maxnproblem;j++){
			    sscanf(ptr, "%d", &ATF_problemsy[j]);
			    while( *ptr!=' '){
				ptr ++;
			    }
			    ptr ++;
			}	
			
			/* Read problem z */
			memset(buffer, 0 ,strlen(buffer));
			fgets(buffer, MAXLINE, Conf_File);
			ptr = buffer;
			/* printf("The buffer for z is %s, %d\n", ptr,k); */
			for(j=0;j<ATF_maxnproblem;j++){
			    sscanf(ptr, "%d", &ATF_problemsz[j]);
			    while( *ptr!=' '){
				ptr ++;
			    }
			    ptr ++;
			}
			
			for(j=0;j<ATF_maxnproblem;j++){

			    printf("The %d dimension of x, y, z are %d,%d,%d\n",j+1,ATF_problemsx[j], ATF_problemsy[j],ATF_problemsz[j]);
			}
			
			
			ptr = NULL;	
			/* do what things? */
		    }
		    
		    /* nsolv */
		    if(strncmp(keyword, "nsolv", strlen("nsolv")) == 0){
			
			sscanf(ptr, "%d", &ATF_maxnsolver);
#ifdef Debug
			printf("nsolv: %d \n", ATF_maxnsolver);	
#endif
			ptr = NULL;			
			
			ATF_solvarr =  (int *)malloc(sizeof(int) * ATF_maxnsolver);
			
			if( ATF_solvarr == NULL ){
			    
			    printf("Memory malloc error at %d of %s!\n", __LINE__, __FILE__);
			}
			memset(buffer, 0 ,strlen(buffer));
			fgets(buffer, MAXLINE, Conf_File);
			ptr = buffer;
			
			for(j=0; j<ATF_maxnsolver; j++){
			    sscanf(ptr, "%d", &ATF_solvarr[j]);
			    while( *ptr!=' '){
				ptr ++;
			    }
			    ptr ++;
			}
			
			for(j=0;j<ATF_maxnsolver;j++){
			    
			    printf("%d ",ATF_solvarr[j]);
			}
			printf("\n");
			
		    }
		    /*
		     npatt 
		    if(strncmp(keyword, "npatt", strlen("npatt")) == 0){
			
			sscanf(ptr, "%d", &ATF_maxnpattern);
#ifdef Debug
			printf("There are %d patterns \n", ATF_maxnpattern);	
#endif
			ptr = NULL;	
			
			ATF_patternarr =  (int *)malloc(sizeof(int) * ATF_maxnpattern);
			
			if( ATF_patternarr == NULL ){
			    
			    printf("Memory malloc error at %d of %s!\n", __LINE__, __FILE__);
			}
			
			memset(buffer, 0 ,strlen(buffer));
			fgets(buffer, MAXLINE, Conf_File);
			ptr = buffer;
			
			for(j=0; j<ATF_maxnpattern; j++){
			    sscanf(ptr, "%d", &ATF_patternarr[j]);
			    while( *ptr!=' '){
				ptr ++;
			    }
			    ptr ++;
			}
			
			for(j=0;j<ATF_maxnpattern;j++){
			    
			    printf("%d ",ATF_patternarr[j]);
			}
			printf("\n");
			
		    }
		    */
                      
                    
		    /* ATF_nhosts */
		    if(strncmp(keyword, "ATF_nhosts", strlen("ATF_nhosts")) == 0){
			
			sscanf(ptr, "%d", &ATF_nhosts);
#ifdef Debug 
			fprintf(stderr, "ATF_nhosts : %d\n", ATF_nhosts);	
#endif
			ptr = NULL;						
			
			ATF_firstranks = (int *)malloc(sizeof(int) * ATF_nhosts);
			flag =1;
		    }
		    
		    /*  ATF_autos */
		    if(strncmp(keyword, "ATF_autos", strlen("ATF_autos")) == 0){
			
			sscanf(ptr, "%d", &ATF_autos);
#ifdef Debug
			fprintf(stderr, "ATF_autos : %d\n", ATF_autos);	
#endif
			ptr = NULL;						
		    }
		    /* franks */
		    if(strncmp(keyword, "franks", strlen("franks")) == 0){
			
			sscanf(ptr, "%d", &franks);
#ifdef Debug
			fprintf(stderr, "franks : %d\n", franks);	
#endif
			ptr = NULL;						
		    }
		    
		    
		}
		/* printf("The keyword is %s\n", keyword);				 */
		
	    }
	    
	    else{
		
		/* 	printf("This is a comment line, continue!\n"); */
		continue;
		
	    }
	    
	    memset(buffer, 0, strlen(buffer));
	    memset(keyword, 0, strlen(keyword));
	    
	}
	
	
	fclose(Conf_File);
	
	/* Distributed data */
	iarray[0] = ATF_nit;
	iarray[1] = ATF_nhosts;
	iarray[2] = ATF_autos;
	iarray[3] = ATF_maxnproblem;
	iarray[4] = ATF_maxnsolver;
	/* iarray[5] = ATF_maxnpattern;*/
	iarray[6] = ATF_verbose;
	
	rarray[0] = ATF_tol;
	rarray[1] = subtol;
	
    }
    
    MPI_Bcast(iarray, 7, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(rarray, 2, MPI_FLOAT, 0, MPI_COMM_WORLD);
    
    /* rank !=0 */
    if (rank !=0 ){
	
	ATF_nit = iarray[0];
	ATF_nhosts =iarray[1];
	ATF_autos = iarray[2];
        ATF_maxnproblem = iarray[3];
	ATF_maxnsolver = iarray[4];
	ATF_verbose = iarray[6];
	
	ATF_tol = rarray[0];
	subtol = rarray[1];
	
	ATF_firstranks = (int *) malloc ( ATF_nhosts * sizeof(int) );
	/* ATF_patternarr = (int *) malloc ( ATF_maxnpattern* sizeof(int) ); */
	ATF_solvarr = (int *) malloc ( ATF_maxnsolver* sizeof(int) );
	ATF_problemsx = (int *) malloc ( ATF_maxnproblem *sizeof(int) );
	ATF_problemsy = (int *) malloc ( ATF_maxnproblem * sizeof(int) );
	ATF_problemsz = (int *) malloc ( ATF_maxnproblem* sizeof(int) );
	
	if((ATF_problemsx == NULL) ||(ATF_problemsy == NULL)||
	   (ATF_problemsy == NULL)||(ATF_solvarr == NULL) ||(ATF_firstranks== NULL)){
	    
	    printf("Memory malloc error at %d of %s!\n", __LINE__, __FILE__);
	}	
	
    }
    
    MPI_Bcast (ATF_problemsx, ATF_maxnproblem, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast (ATF_problemsy, ATF_maxnproblem, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast (ATF_problemsz, ATF_maxnproblem, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast (ATF_solvarr, ATF_maxnsolver, MPI_INT, 0, MPI_COMM_WORLD);
   /*  MPI_Bcast (ATF_patternarr, ATF_maxnpattern, MPI_INT, 0,
   **  MPI_COMM_WORLD);*/
    
    
    if((ATF_nhosts>1) &&(ATF_autos == 0)){
	
        MPI_Bcast ( ATF_firstranks, ATF_nhosts, MPI_INT, 0, MPI_COMM_WORLD);
    }
    
    *nPr  = ATF_maxnproblem;
    *nSol = ATF_maxnsolver;
    
    /* printf("Read config_correct !\n"); */
    
    return ATF_SUCCESS ;
	
}
