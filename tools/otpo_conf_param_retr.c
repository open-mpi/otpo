#include <stdio.h>
#include <stdlib.h>
#include </home/shwetajha/postgresql-install/include/libpq-fe.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
//#include "mpi.h"
#define MAX_LINE_LEN 256

static void exit_nicely(PGconn *conn)
{
  PQfinish(conn);
  exit(1);
}


int   main(int argc, char **argv) {
  PGconn          *conn;
  PGresult        *res,*res1,*res2;
  int             rec_count,rec_count1,rec_count2;
  int             row;
  int             col;
  FILE *fp,*fc;  /* file pointer*/
  char line[MAX_LINE_LEN + 1] ;
  char* token ;
  char *column_name[15];
  char *col_name[15], *db_column_name[15],*index[10];
  char *column_value[10], *db_column_value[10]; 
  int i=0,j=0,last_index;
  char col_variable[30];
  char str1[150];
  char *app_key;
  char *host_key;
  char *problem_size;
  char filename[50];
  char *filename_conn;
  char *size;  
  int file_no=1;
  int column_count;
  int cnt_ind;

  //MPI_Init ( &argc, &argv );
  // MPI_Comm_size ( MPI_COMM_WORLD, &size );
  //MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
  
  
  if (argc < 4){
    printf("Error: Enter DB-connection file, Application_key,  hostkey, number of proc and/or problem size.");
    exit(-1);
  }
  else {
    filename_conn =strdup (argv[1]);
    app_key=strdup (argv[2]);
    host_key=strdup (argv[3]);
    size =argv[4];
    problem_size= argv[5];
  }
  
  //  if(rank==0){
    fc = fopen(filename_conn,"r+");
    if(fc == NULL)
      {
	printf("\n Error while opening the file.\n");
      }
    while( fgets( line, MAX_LINE_LEN, fc ) != NULL )
      {
	token = strtok( line, "\t =\n\r" ) ;
	if( token != NULL && token[0] != '#' )
	  {
	    //printf("i= %d",i);
	    db_column_name[i] = malloc(strlen(token) + 1);
	    strcpy(db_column_name[i], token);
	    //printf( "col:\t%s\n", db_column_name[i] ) ;
	    
	    token = strtok( NULL, "\t =\n\r" ) ;
	    db_column_value[i] = malloc(strlen(token) + 1);
	    strcpy(db_column_value[i], token);
	    //printf("value: %s\n",db_column_value[i]);
	    i++;
	  }
      }
    
    
    sprintf(str1,"dbname=%s host=%s user=%s password=%s",db_column_value[0],db_column_value[1],db_column_value[2],db_column_value[3]);
    conn = PQconnectdb(str1);
    if (PQstatus(conn) == CONNECTION_BAD) {                                                                                                                                               puts("We were unable to connect to the database");                                                                                                                                  exit(0);                                                                                                                                                                        
    }  
    i=0;  
    char *substr1="index", *substr2="app_key",*substr3="host_key",*substr4="num_proc",*substr5="problem_size";
    res = PQexec(conn,"SELECT DISTINCT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name='otpo_parameter'");
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
      {
        fprintf(stderr, "SELECT failed: %s\n", PQerrorMessage(conn));
	exit_nicely(conn);
      }
    rec_count = PQntuples(res);
    column_count=rec_count;

    i=0;
    for (row=0; row<rec_count; row++) {
      for (col=0; col<1; col++) {
	//if(strncmp(PQgetvalue(res, row, col), substr, strlen(substr))==0){
	if((strncmp(PQgetvalue(res, row, col), substr1, strlen(substr1))!=0) && (strncmp(PQgetvalue(res, row, col), substr2, strlen(substr2))!=0) && (strncmp(PQgetvalue(res, row, col), substr3, strlen(substr3))!=0) && (strncmp(PQgetvalue(res, row, col), substr4, strlen(substr4))!=0) && (strncmp(PQgetvalue(res, row, col), substr5, strlen(substr5))!=0)){  
	col_name[i] = malloc(strlen(PQgetvalue(res, row, col)) + 1);
	  strcpy(col_name[i],PQgetvalue(res, row, col));
	  //printf("%s\n", col_name[i]);
	  i++;
	}
      }
    }
    
    column_count=i--;
    
    //     printf("\nhere here here here problem_size = %s and size is %d\n", problem_size, strlen(problem_size));
    if (argc ==4){
      sprintf(str1,"select index from otpo_parameter where app_key ='%s' and host_key='%s'", app_key,host_key);
    }
   else if (argc ==5){
      sprintf(str1,"select index from otpo_parameter where app_key ='%s' and host_key='%s' and num_proc = '%s' ", app_key,host_key,size);                            
    }
   else if(argc ==6){
	 sprintf(str1,"select index from otpo_parameter where app_key ='%s' and host_key='%s' and num_proc ='%s' and problem_size='%s' ", app_key,host_key,size,problem_size);
	 }
   else{
     printf("Please enter the required number of arguments\n");
     exit(-1);
   }
    res = PQexec(conn,str1);                                                                                                                                                      
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
      {
	fprintf(stderr, "SELECT failed: %s\n", PQerrorMessage(conn));
	exit_nicely(conn);
	}
    rec_count1 = PQntuples(res);                                                                                                                                                         if(rec_count1 == 0){
      printf("There is no entry for the entered application, hostkey and number of procs\n");
      exit_nicely(conn);
    }
    //printf("%d\n",rec_count1);                                                                                                                                                       
    for (row=0; row<rec_count1; row++) {                                                                                                                                             
      for (col=0; col<1; col++) {                                                                                                                                                    
	index[row] = malloc(strlen(PQgetvalue(res, row, col)) + 1);                                                                                                                  
	strcpy(index[row],PQgetvalue(res, row, col));                                                                                                                                
	//printf("index for row %d is %s\n",row, index[row]);
	//	index_count++;                                                                                                                          
      }                                                                                                                                                                              
      }   
    cnt_ind=rec_count1;
    //printf("index count is %d\n",cnt_ind);
    int k;     
    for(k=0;k< cnt_ind;k++){

      //printf("index: %d\n",atoi(index[k]));
      //sprintf(filename,"mca-param.conf.%d",k);
      sprintf(filename,"mca-param.conf.%d",atoi(index[k]));
	  fp = fopen(filename,"w");
	 if(fp == NULL) 
	{
	  printf("Failed to open file for writing\n");
	  return -1;
	}
      
      for(j=0;j<column_count;j++){
	sprintf(str1,"select %s from otpo_parameter where index = %d",col_name[j], atoi(index[k]));
	res = PQexec(conn,str1);
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
	  {
	    fprintf(stderr, "SELECT failed: %s\n", PQerrorMessage(conn));
	    exit_nicely(conn);
	  }
	rec_count1 = PQntuples(res);
	for (row=0; row<rec_count1; row++) {
	  for (col=0; col<1; col++) {
	    //printf("%s\n",PQgetvalue(res, row, col));
	    if(atoi(PQgetvalue(res, row, col)) != 0)
	      fprintf(fp,"%s = %s\n",col_name[j],PQgetvalue(res, row, col));
	  }
	}  
      
    }

    fclose(fp);
    }
    
    //      }
    PQclear(res);
    PQfinish(conn);
    //     }//rank
    //MPI_Finalize ();
  return 0;  
}
