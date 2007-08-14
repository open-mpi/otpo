/*
 * Copyright (c) 2007 Cisco, Inc. All rights reserved.
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

#include "otpo.h"

static int read_intermediate_results (char *, int, int, char ***, char ****);
static int write_intermediate_results (ADCL_Request, double, char *); 
static int print_output (ADCL_Request, double, FILE*);
char output_file[50];

int otpo_write_results (ADCL_Request req, char *output_dir, int *num_functions) 
{
    double f_avg=0.0, unf_avg=0.0, out_num=0.0, n=0.0;
    
    ADCL_Request_get_winner_stat (req, &f_avg, &unf_avg, &out_num);

    if (FAIL == (*num_functions = 
                    write_intermediate_results (req, f_avg, output_dir)))
    {
        fprintf(stderr,"Unable to write output\n");
        return FAIL;
    }
    return SUCCESS;
}

int otpo_analyze_results (char *output_dir, int num_functions, int num_parameters)
{
    char **names,***values;
    int i,j;

    if (SUCCESS != read_intermediate_results (output_dir, num_functions, 
                                              num_parameters, &names, &values))
    {
        return FAIL;
    }

    /* Free Stuff */
    for (j=0 ; j<num_parameters ; j++) 
    {
        if (NULL != names[j])
        {
            free (names[j]);
        }
    }
    if (NULL != names)
    {
        free (names);
    }
    for (i=0 ; i<num_functions ; i++) 
    {
        for (j=0 ; j<num_parameters ; j++) 
        {
            if (NULL != values[i][j])
            {
                free (values[i][j]);
            }
        }
        if (NULL != values[i]) 
        {
            free (values[i]);
        }
    }
    if (NULL != values)
    {
        free (values);
    }

    return SUCCESS;
}
static int read_intermediate_results (char *output_dir, int num_functions, 
                                      int num_parameters, char ***names, 
                                      char ****values)
{
    FILE *fp;
    char line[LINE_SIZE], *token;
    int i,j,tmp,current,set;

    (*names) = (char **)malloc(sizeof(char *) * num_parameters);
    if (NULL == (*names)) 
    {
        printf("Malloc_Failed...\n");
        return NO_MEMORY;
    }
    (*values) = (char ***)malloc(sizeof(char **) * num_functions);
    if (NULL == (*values)) 
    {
        printf("Malloc_Failed...\n");
        return NO_MEMORY;
    }
    for (j=0 ; j<num_functions ; j++) 
    {
        (*values)[j] = (char **)malloc(sizeof(char *) * num_parameters);
        if (NULL == (*values)[j]) 
        {
            printf("Malloc_Failed...\n");
            return NO_MEMORY;
        }
    }

    set = 0;
    current = 0;
 open1:
    fp = fopen (output_file, "r");
    if (NULL == fp) 
    { 
        if (EINTR == errno)
        {
            /* try again */
            goto open1;
        }
        fprintf (stderr,"Unable to open file: %s\n",output_file);
        return FAIL;
    }

    while (NULL != fgets (line, LINE_SIZE, fp)) 
    {
        if (NULL == line || '\n' == line[0] || 
            '=' == line[0]) 
        {
            continue;
        }
        token = strtok (line," \t\n");
        if (NULL != token)
        {
            token = strtok (NULL," \t\n");
            /* get the number of functions for a specific value */
            tmp = atoi (token);
            for (i=current ; i<current+tmp ; i++) 
            {
                for (j=0 ; j<num_parameters ; j++) 
                {
                    fgets (line, LINE_SIZE, fp);
                    token = strtok (line," \t\n");
                    if (0 == set)
                    {
                        (*names)[j] = strdup (token);
                    }
                    token = strtok (NULL," \t\n");
                    (*values)[i][j] = strdup (token);
                }
                set = 1;
            }
            current += tmp;
        }
    }
    fclose(fp);

    return SUCCESS;
}
static int write_intermediate_results (ADCL_Request req, double f_avg, char *output_dir) 
{
    double n=0.0;
    FILE *fp;
    char *timebuf;
    int num_functions, tmp;
    time_t stamp;

    if (NULL == opendir (output_dir))
    {
        if (0 != mkdir (output_dir, S_IRUSR | S_IWUSR | S_IXUSR))
        {
            fprintf (stderr, "Couldn't create directory %s\n",output_dir);
            return FAIL;
        }
    }
    output_file[0] = '\0';
    strcat (output_file, output_dir);
    strcat (output_file, "/result");
    time(&stamp);
    timebuf = (char *)malloc(sizeof(char) * 26);
    snprintf(timebuf,26,"%ld",stamp);
    strcat (output_file, timebuf);
    
 open2:
    fp = fopen (output_file, "wt");
    if (NULL == fp)
    {
        if (EINTR == errno)
        {
            /* try again */
            goto open2;
        }
        fprintf(stderr, "Unable to open file %s\n", output_file);
        return FAIL;
    }
    num_functions = 0;
    for (n=f_avg ; n<(f_avg+0.05) ; n=n+0.01) 
    {
        fprintf (fp,"\n============================================================\n");
        tmp = print_output (req, n, fp);
        num_functions += tmp;
    }
    fclose (fp);
    if (NULL != timebuf)
    {
        free(timebuf);
    }
    return num_functions;
}
/*
int otpo_write_output_txt (ADCL_Request req, char *output_file) 
int otpo_write_output_xml (ADCL_Request req, char *output_file) 
*/
static int print_output (ADCL_Request req, double n, FILE *fp)
{
    char **a_names=NULL, ***a_values_names=NULL;
    int a_nums=0, i, j, num_functions=0;

    ADCL_Request_get_functions_with_average (req, 
                                             n,
                                             &num_functions,
                                             NULL,
                                             &a_names, 
                                             &a_nums, 
                                             &a_values_names, 
                                             NULL);
    
    fprintf(fp,"%f %d\n", n, num_functions);
    for (i=0 ; i<num_functions ; i++) 
    {
        for (j=0 ; j<a_nums ; j++) 
        {
            fprintf (fp, "%s %s\n", a_names[j], a_values_names[i][j]);
        }
    }
    
    if (debug || verbose) 
    {
        printf ("Found %d Attribute Combinations with result = %f:\n", num_functions, n);
        for (i=0 ; i<num_functions ; i++) 
        {
            for (j=0 ; j<a_nums ; j++) 
            {
                printf ("%s = %s\n", a_names[j], a_values_names[i][j]);
            }
            printf ("******************************************************\n");
        }
    }

    for (j=0 ; j<a_nums ; j++) 
    {
        if (NULL != a_names[j]) 
        {
            free (a_names[j]);
        }
    }
    for (i=0 ; i<num_functions ; i++) 
    {
        for (j=0 ; j<a_nums ; j++) 
        {
            if (NULL != a_values_names[i][j]) 
            {                    
                free (a_values_names[i][j]);
            }
        }
        if (NULL != a_values_names[i]) 
        {                    
            free (a_values_names[i]);
        }
    }
    if (NULL != a_names) 
    {
        free (a_names);
    }
    if (NULL != a_values_names) 
    {                    
        free (a_values_names);
    }
    return num_functions;
}

int otpo_write_interrupt_data (int num_tested, double *results, 
                               int current_winner, char *interrupt_file)
{
    FILE *fp;
    int i;
 open3:
    fp = fopen (interrupt_file, "wt");
    if (NULL == fp)
    {
        if (EINTR == errno)
        {
            /* try again */
            goto open3;
        }
        fprintf(stderr, "Unable to open file %s\n", interrupt_file);
        return FAIL;
    }
    fprintf (fp, "%d\n",num_tested);
    if (debug)
    {
        printf("Tested %d Functions so far\n",num_tested);
    }
    for (i=0 ; i<num_tested ; i++) 
    {
        fprintf (fp, "%lf\n",results[i]);
        if (debug)
        {
            printf("latency = %lf\n",results[i]);
        }
    }
    fclose (fp);
    return SUCCESS;
}

int otpo_read_interrupt_data (char *interrupt_file, int *num_tested, double **results)
{
    FILE *fp;
    int i;
    
    i=0;
 open4:
    fp = fopen (interrupt_file, "r");
    if (NULL == fp) 
    {
        if (EINTR == errno)
        {
            /* try again */
            goto open4;
        }
        fprintf (stderr,"Unable to open file: %s\n",interrupt_file);
        return FAIL;
    }

    fscanf (fp,"%d", &(*num_tested));
    if (debug) 
    {
        printf ("%d combinations have been executed already..\n",*num_tested);
    }
    *results = (double *)malloc((*num_tested) * sizeof(double));
    if (NULL == results)
    {
        fprintf (stderr,"Malloc_Failed...\n");
        exit (1);
    }
    while (!feof(fp) && i<*num_tested)
    {
        fscanf (fp,"%lf", &(*results)[i]);
        if (debug) 
        {
            printf ("latency = %lf\n",(*results)[i]);
        }
        i ++;
    }
    
    fclose (fp);
    return SUCCESS;
}
