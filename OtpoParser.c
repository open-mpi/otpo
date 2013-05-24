#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>

#define MAXLINE 180
#define MAX_NUM_PARAMETERS 140
#define MAX_VALUES 140
#define MAX_PARAM_NAME_LEN 80

int check_and_add ( char *);
void add_value ( int, int );
void dump_parameters (void);


struct parameters
{
  char parameter_name[MAX_PARAM_NAME_LEN];
  int in_use;
  int  parameter_value[MAX_VALUES];
  int parameter_count[MAX_VALUES];
};

struct parameters param[MAX_NUM_PARAMETERS]; 

int main(int argc, char **argv)
{
  FILE *fp;
  int i=1, pos;
  char buffer[MAXLINE];
  char *token;
  char *str = "btl_";
  char *filename;

  if ( argc < 2 ) {
    filename = strdup ( "result1314230494");
  }
  else {
    filename = strdup ( argv[1] );
  }

  memset ( param, 0, MAX_NUM_PARAMETERS * sizeof ( struct parameters ));

  fp= fopen (filename,"r");
  
  while (fgets(buffer, MAXLINE, fp) != NULL)
    {
      if((strstr(buffer, str)) != NULL){
	token = strtok(buffer," ");
	i=1; 
	while(token != NULL){
	  if (i==1){
	    pos = check_and_add ( token );
	    if ( pos == -1 ) {
	      printf("need an additional slot for parameters !\n");
	      return -1;
	    }
	  }
	  else {
	    add_value ( pos, atoi (token));
	  }
	  token = strtok(NULL," "); 
	  i++;
	}
      }
    }
  
  fclose(fp);
  dump_parameters ();
  

  return 0;
}
    
void dump_parameters ( void )
{
  int m,n;

  for(m=0;m<MAX_NUM_PARAMETERS; m++){
    if ( param[m].in_use == 0 ) {
      break;
    }
    for (n=0; n<MAX_VALUES; n++ ) {
      if ( param[m].parameter_count[n] > 0 ) {
	printf("%s\t%d\t%d\n",param[m].parameter_name,param[m].parameter_value[n],param[m].parameter_count[n]);
      }
    }
  }

  return;
}

void add_value ( int pos, int value )
{
  int n;
  int found=0;
  int lastval=0;

  for ( n=0; n< MAX_VALUES; n++ ) {
    if ( param[pos].parameter_value[n] == value ) {
      found = 1;
      param[pos].parameter_count[n]++;
      break;
    }
    if ( param[pos].parameter_count[n] == 0 ) {
      lastval=n;
      break;
    }
  }
  
  if ( found == 0 ) {
    /* parameter value not yet know, lets add it */
    param[pos].parameter_value[lastval] = value;
    param[pos].parameter_count[lastval] = 1;
  }

  return;
}

int  check_and_add ( char *param_name )
{
  int m;
  int pos=0;

  for ( m=0; m<MAX_NUM_PARAMETERS; m++) {
    if ( !strcmp (param[m].parameter_name, param_name )) {
      pos = m;
      return pos;
    }

    if ( param[m].in_use == 0 ) {
      /* this element is not in use any more */
      strncpy ( param[m].parameter_name, param_name, MAX_PARAM_NAME_LEN );
      param[m].in_use = 1;
      return m;
    }
  }

  return -1;
}
  


  
