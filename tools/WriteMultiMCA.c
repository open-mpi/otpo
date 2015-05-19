#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define BUFSIZE 80

char *replace_str(char *str, char *orig, char *rep)
{
  static char buffer[BUFSIZE];
  char *p;

  if(!(p = strstr(str, orig)))  
    return str;

  strncpy(buffer, str, p-str); 
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  return buffer;
}


int main(int argc, char **argv)
{ 
  char buf[BUFSIZE];
  char *str_fname = "temp";
  char *filename, *filename1; 
  FILE *fp,*fw;
  char line[BUFSIZE],*ch=NULL;
  char *str = "btl_",  *str1 = "****";
  int i=1;
  if (argc < 2){
    filename = strdup("result1372348552");
  }
  else {
    filename = strdup ( argv[1] );
  }
  fp= fopen (filename,"r");
  loop:;
  sprintf(buf,"mca-param.conf.%d",i);
   fw=fopen(buf,"w");
  while((fgets(line,sizeof(line),fp))!= NULL )
    {
      if((strstr(line, str)) != NULL)
	{
	  fputs((replace_str( line , " ", "=")),fw);
	}
      else if((strstr(line,str1))!= NULL)
	{
	  fprintf(fw, "\n");
	  fclose(fw);
	  i++;
	  // fgets(line,sizeof(line),fp);
	  //	  fseek(fp,55,SEEK_SET);
	  //	  memset ( line, 0, BUFSIZE );
	  goto loop;
	}
    }
  remove(buf);
  fclose(fp);
}
