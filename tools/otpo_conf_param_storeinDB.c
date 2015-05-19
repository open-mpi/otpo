#include <stdio.h>
#include <stdlib.h>
#include </home/shwetajha/postgresql-install/include/libpq-fe.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#define MAX_LINE_LEN 256
#define TOTALSTRINGS 50

int   main(int argc, char **argv) {
  PGconn          *conn;
  PGresult        *res;
  int             rec_count;
  int             row;
  int             col;
  FILE *fp, *fc;  /* file pointer*/
  char line[MAX_LINE_LEN + 1] ;
  char* token ;
  char **column_name_mca=(char **)malloc(TOTALSTRINGS * sizeof(char*));
  char **Table_col_name=(char **)malloc(TOTALSTRINGS * sizeof(char*));
  char **db_column_name_mca=(char **)malloc(TOTALSTRINGS * sizeof(char*));
  char **column_value=(char **)malloc(TOTALSTRINGS * sizeof(char*));
  char **db_column_value=(char **)malloc(TOTALSTRINGS * sizeof(char*)); 
  int i=0,j=0,last_index;
  char str1[MAX_LINE_LEN],str_insert[MAX_LINE_LEN];
  char *app_key;
  char *host_key,*size;
  char *problem_size;
  char *filename;
  char *filename_conn;
  int mca_count,db_count;
  int flag;

  switch (argc)
    {
    case 5:
      filename = strdup (argv[1]);
      filename_conn =strdup (argv[2]);
      app_key=strdup (argv[3]);
      host_key=strdup (argv[4]);
      sprintf(str_insert,"insert into otpo_parameter(App_key,host_key) values ('%s','%s')",app_key,host_key); 
      break;   
    case 6:
      filename = strdup (argv[1]);
      filename_conn =strdup (argv[2]);
      app_key=strdup (argv[3]);
      host_key=strdup (argv[4]);
      size = argv[5];  
      sprintf(str_insert,"insert into otpo_parameter(App_key,host_key,num_proc) values ('%s','%s','%s')",app_key,host_key,size);
      break;
    case 7:
      filename = strdup (argv[1]);
      filename_conn =strdup (argv[2]);
      app_key=strdup (argv[3]);
      host_key=strdup (argv[4]);
      size = argv[5];
      problem_size=argv[6];  
      sprintf(str_insert,"insert into otpo_parameter(App_key,host_key,num_proc,problem_size) values ('%s','%s','%s','%s')",app_key,host_key,size,problem_size);
      break;
    default:
      printf("Error: Enter MCA Parameter File, DB-connection file, Application_key, hostkey ,number of processes and/or problem size.");
      exit(-1);  
    }

    fp = fopen(filename,"r+");
    
    if(NULL == fp){
	printf("\n Error while opening the mca_params.conf file.\n");
      } 
    i=0;     
    while( fgets( line, MAX_LINE_LEN, fp ) != NULL )
      {
	token = strtok( line, "\t =\n\r" ) ;
	if( token != NULL && token[0] != '#' )
	  {
	    column_name_mca[i] = (char *)malloc(strlen(token) + 1);
	    if ( NULL ==  column_name_mca[i]) {
              printf("Malloc failed: Reading column name");
              exit(-1);
            }
	    strcpy(column_name_mca[i], token);	   
	    token = strtok( NULL, "\t =\n\r" ) ;
	    column_value[i] = (char *)malloc(strlen(token) + 1);
	    if ( NULL ==  column_value[i]) {
	      printf("Malloc failed: Reading column value");
	      exit(-1);
            }
	    strcpy(column_value[i], token);
	    i++;
	  }
      }
    mca_count=i;
    fclose(fp);
    fc = fopen(filename_conn,"r+");
    i=0;
    if(NULL == fc){
	printf("\n Error while opening the DB-connection file.\n");
      }
      
      
    while( fgets( line, MAX_LINE_LEN, fc ) != NULL )
      {
	token = strtok( line, "\t =\n\r" ) ;
	if( token != NULL && token[0] != '#' )
	  {
	    db_column_name_mca[i] = (char *)malloc(strlen(token) + 1);
	    if ( NULL ==  db_column_name_mca[i]) {
              printf("Malloc failed: Reading column name");
              exit(-1);
            }
	    strcpy(db_column_name_mca[i], token);
	    token = strtok( NULL, "\t =\n\r" ) ;
	    
	    db_column_value[i] = (char *)malloc(strlen(token) + 1);
	    if ( NULL ==  db_column_value[i]) {
              printf("Malloc failed: Reading column value");
              exit(-1);
            }
	    strcpy(db_column_value[i], token);
	    i++;
	  }
      }
    db_count=i;
    fclose(fc);
    sprintf(str1,"dbname=%s host=%s user=%s password=%s",db_column_value[0],db_column_value[1],db_column_value[2],db_column_value[3]);
    conn = PQconnectdb(str1);
    if ( CONNECTION_BAD == PQstatus(conn)) {
      puts("We were unable to connect to the database");
      exit(0); 
    }
  

    //Create the table, in case table is already present it will just append the data
    res=PQexec(conn,"create table IF NOT EXISTS otpo_parameter(index serial primary key,App_key bytea, host_key bytea, num_proc text,problem_size text, btl_openib_eager_limit integer,btl_openib_eager_rdma_num integer, btl_openib_eager_rdma_threshold integer,btl_openib_use_eager_rdma integer, btl_openib_free_list_num integer, btl_openib_free_list_max integer,btl_openib_use_message_coalescing integer,btl_sm_free_list_num integer, btl_sm_fifo_size integer,btl_sm_num_fifos integer, btl_sm_max_send_size integer)");
    if (PGRES_COMMAND_OK != PQresultStatus(res)){
	fprintf(stderr, "CREATE failed: %s\n", PQerrorMessage(conn));
      }
    
    //Insert dummy data
    res = PQexec(conn,str_insert);
    if (PGRES_COMMAND_OK != PQresultStatus(res)){
        fprintf(stderr, "INSERT failed: %s\n", PQerrorMessage(conn));
      }
    
    //extract the last index value of the table
    res = PQexec(conn,
		 "SELECT max(index) from otpo_parameter");
    if ( PGRES_TUPLES_OK != PQresultStatus(res)){
        fprintf(stderr, "SELECT failed: %s\n", PQerrorMessage(conn));
      }
    rec_count = PQntuples(res);
    for (row=0; row<rec_count; row++) {
      for (col=0; col<1; col++) {
	last_index=atoi(PQgetvalue(res, row, col));
      }
    }

    //Extract column names from the table
    res = PQexec(conn,"SELECT DISTINCT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name='otpo_parameter'");
    if( PGRES_TUPLES_OK != PQresultStatus(res)){
        fprintf(stderr, "SELECT failed: %s\n", PQerrorMessage(conn));
      }
    rec_count = PQntuples(res);
    i=0;

    for (row=0; row<rec_count; row++) {
      for (col=0; col<1; col++) {
	Table_col_name[i] = (char *)malloc(strlen(PQgetvalue(res, row, col)) + 1);
	if ( NULL ==  Table_col_name[i]) {
	  printf("Malloc failed: Reading table column names");
	  exit(-1);
	}
	strncpy(Table_col_name[i],PQgetvalue(res, row, col),strlen(PQgetvalue(res, row, col))+1);
	i++;
      }
    }

    for(i=0;i<mca_count;i++){
      flag=0;
      for(j=0;j<rec_count;j++){
	if(strcmp(Table_col_name[j],column_name_mca[i])==0){
	  sprintf(str1,"update otpo_parameter set %s= %d where index=%d", column_name_mca[i],atoi(column_value[i]), last_index);
	  res=PQexec(conn,str1);
	  if( PGRES_COMMAND_OK != PQresultStatus(res)){
	      fprintf(stderr, "UPDATE failed: %s\n", PQerrorMessage(conn));
	    }
	   flag=1;
	  break;
	}
      }
       if(flag==0){
	 sprintf(str1,"alter table otpo_parameter add column %s integer", column_name_mca[i]);                                                     
	 res=PQexec(conn,str1);        
	 if (PGRES_COMMAND_OK != PQresultStatus(res)){
	   fprintf(stderr, "Alter table failed: %s\n", PQerrorMessage(conn));
	 }                                                                                                                          
	 sprintf(str1,"update otpo_parameter set %s= %d where index=%d", column_name_mca[i],atoi(column_value[i]), last_index);     
	 res=PQexec(conn,str1);                                                                                               
	 if(PGRES_COMMAND_OK != PQresultStatus(res)){
	   fprintf(stderr, "UPDATE failed: %s\n", PQerrorMessage(conn));                                          
	 }                            
       }
    }
    
    PQclear(res);
    PQfinish(conn);
    return 0;
    
}

