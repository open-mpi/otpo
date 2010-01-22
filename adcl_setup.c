/*
 * Copyright (c) 2007 Cisco, Inc. All rights reserved.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "otpo.h"

static int check_child_died (pid_t);
static int kill_child (pid_t);
static int get_next_combination (int *);
static int get_next_val (int, int);
static int check_rpn (int *);
static int pop ();
static int push (int);
static int set_mpi_arguments (char ***);
static int update_adcl_request (ADCL_Request);

unsigned int  TIMEOUT = 200;
int rpn_stack [RPN_MAX_ELEMENTS];
int rpn_stack_position;

/* Funtion that forks the mpirun */
void otpo_test_func (ADCL_Request req) 
{
    int attrs_num, *attrs_values_num;
    int i,k,m,j,code,rc;
    int virtual, aggregate;
    char *function_name;
    char **attrs_names;
    char **attrs_values_names;
    pid_t pid;
    int pipefd[2];
    char c;
    
    if (pipe (pipefd) < 0) 
    {
        if (errno == EMFILE)
            printf ("EMFILE caught\n");
        perror ("Pipe creation failed");
        exit (errno);
    }
    
    code = 0;
    rc = 0;
    pid = fork();
    if (0 > pid) 
    {
        perror ("Fork Failed\n");
        exit (errno);
    }
    else if (0 == pid) /* Child Process */
    {
        char ** mpi_args; /* argument list to execvp */
        /* environment variables (parameters set) */
        char env[LINE_SIZE];
        char env_value[LINE_SIZE]; 

        pid = setsid();
        if (!debug) 
        {
            close (1);
            close (2);
        }

        set_mpi_arguments (&mpi_args);
        
        ADCL_Request_get_curr_function (req, &function_name, &attrs_names, &attrs_num,
                                        &attrs_values_names, &attrs_values_num);
        if (debug) 
        {
            printf ("***********************************\n");
            printf ("Exporting...\n");
        }

        for (i=0 ; i<attrs_num ; i++) 
        {
            strcpy (env, "OMPI_MCA_");
            strcat (env, attrs_names[i]);
            for (j=0 ; j<attrs_num ; j++)
            {
                virtual = 0;
                aggregate = 0;
                if (0 == strcmp (list_params[j].name, attrs_names[i]))
                {
                    if (OTPO_PARAM_IS_VIRTUAL (list_params[j])) 
                    {
                        virtual = 1;
                        break;
                    }
                    else if (OTPO_PARAM_IS_AGGREGATE (list_params[j]))
                    {
                        aggregate = 1;
                        break;
                    }
                }
            }
            if (virtual) 
            {
                continue;
            }

            if (aggregate)
            {
                char *token;
                
                token = strtok (attrs_values_names[i],"$");
                if ('$' != attrs_values_names[i][0])
                {
                    strcat(env_value,token);
                    token = strtok (NULL, "$");
                }
                while (NULL != token)
                {
                    for (m=0 ; m<attrs_num ; m++) 
                    {
                        if (0 == strcmp(attrs_names[m],token))
                        {
                            strcat(env_value,attrs_values_names[m]);
                            break;
                        }
                    }
                    if (m == attrs_num) 
                    {
                        fprintf (stderr, "Can't resolve %s value\n",attrs_names[i]);
                        exit(1);
                    }
                    token = strtok (NULL, "$");
                    if (NULL == token)
                    {
                        break;
                    }
                    strcat(env_value,token);
                    token = strtok (NULL, "$");
                }
            }
            else 
            {
                strcpy (env_value, attrs_values_names[i]);
            }
            
            if (debug) 
            {
                printf ("%d:  %s=%s\n",i,env, env_value);
            }
	    
            if (0>setenv(env,env_value,1))
            {
                perror ("SETENV Failed");
                exit (errno);
            }
        }
        for (k=0 ; k<attrs_num ; k++) 
        {
            if (NULL != attrs_names[k])
            {
                free (attrs_names[k]);
            }
            if (NULL != attrs_values_names[k]) 
            {
                free (attrs_values_names[k]);
            }
        }
        if (NULL != function_name) 
        {
            free (function_name);
        }
        if (NULL != attrs_names)
        {
            free (attrs_names);
        }
        if (NULL != attrs_values_names)
        {
            free (attrs_values_names);
        }
        if (NULL != attrs_values_num)
        {
            free (attrs_values_num);
        }
        
        if (debug) 
        {
            printf ("Executing now...\n");
	}

        close (pipefd[0]); /* close reading end of pipe */
        fcntl (pipefd[1], F_SETFD, FD_CLOEXEC);
	

        if (-1 == (code = execvp ("mpirun", mpi_args))) 
        {
            perror ("EXECVP failed");
            write (pipefd[1], &code, sizeof(int));
            exit (1);
        }
    }
    else /* Parent Process */
    {    
        close (pipefd[1]); /* close write end of pipe */        
        while (1)
        {
            rc = read(pipefd[0], &code, sizeof(int));
            /* execvp was successfull, break */
            if (0 == rc)
            {
                break;
            }
            else if (rc < 0)
            {
                /* Signal Safety */
                if (EINTR == errno)
                {
                    continue;
                }
                /* other signals are bad */
                else
                {
                    if (debug)
                    {
                        printf ("Error %d, Case Ignored..\n",errno);
                    }
                    ADCL_Request_update (req, 1000);
                    exit (1);
                }
            }
            else 
            {
                /* execvp failed !! */
                if (debug)
                {
                    printf ("Execvp failed, Case Ignored..\n");
                }
                ADCL_Request_update (req, 1000);
                exit (1);
            }
        }
        close (pipefd[0]);

        if (check_child_died (pid))
        {   
            if (SUCCESS != update_adcl_request (req))
            {
                fprintf (stderr,"Unable to update request\n");
                exit (1);
            }
        }
        else 
        {
            if (debug)
            {
                printf ("CHILD did not die after timeout, I will TERMINATE him...\n");
            }
            if (SUCCESS == kill_child (pid)) 
            {
                if (debug) 
                { 
                    printf ("Child is dead, this case will be ignored...\n");
                }
                ADCL_Request_update (req, 1000);
            }
            else 
            {
                if (debug)
                {
                    printf ("Child would just won't die.. Quitting ..\n");
                }
                exit(1);
            }
        }
    }
    return;    
}

/* populate attributes for ADCL objects */
int otpo_populate_attributes (ADCL_Attribute *ADCL_param_attributes) {
    int i, j, digits,k;
    char **attr_val_names=NULL;
    
    k=0;
    for (i=0 ; i<num_parameters ; i++) 
    {
        attr_val_names = (char **)malloc(sizeof(char *) * list_params[i].num_values);
        if (NULL == attr_val_names) 
        {
            printf("Malloc Failed...\n");
            return NO_MEMORY;
        }

        /* generate the attribute values as strings */ 
        for (j=0 ; j<list_params[i].num_values ; j++) 
        {
            if (NULL != list_params[i].string_values[j]) 
            {
                attr_val_names[j] = strdup (list_params[i].string_values[j]);
                continue;
            }
            if (0 < list_params[i].possible_values[j]) 
            {
                digits = (floor (log10 (abs (list_params[i].possible_values[j]))) + 1);
            }
            else if (0 == list_params[i].possible_values[j]) 
            {
                digits = 1;
            }
            else
            {
                digits = (floor (log10 (abs (list_params[i].possible_values[j]))) + 2);
            }
            
            attr_val_names[j] = NULL;
            attr_val_names[j] = (char *)malloc(sizeof(char) * digits);
            if (NULL == attr_val_names[j]) 
            {
                printf ("Malloc Failed... %d\n",j);
                return NO_MEMORY;
            }

            snprintf (attr_val_names[j], digits+1, "%d", list_params[i].possible_values[j]);  
        }
        /* create the attribute */
        ADCL_Attribute_create (list_params[i].num_values, list_params[i].possible_values, 
                               attr_val_names , list_params[i].name , &ADCL_param_attributes[k++]);
        
        for (j=0 ; j<list_params[i].num_values ; j++)
        {
            if (NULL != attr_val_names[j]) 
            {
                free (attr_val_names[j]);
            }
        }
        if (NULL != attr_val_names)
        {
            free (attr_val_names);
        }
    }
    return SUCCESS;
} 

/* 
 * Populate the functions, excluding unneeded combinations
 * Create the function set
 * Return the number of functions 
 */
int otpo_populate_function_set (ADCL_Attrset attrset, int num_functions, 
                                ADCL_Fnctset *fnctset)
{
    int i, count, more, attr_vals[num_parameters];
    ADCL_Function functions[num_functions];

    for (i=0 ; i<num_parameters ; i++) 
    {
        attr_vals[i] = list_params[i].possible_values[0];
    }
    
    more = 1;
    count = 0;
    
    while (more)
    {
        if (check_rpn (attr_vals))
        {
            ADCL_Function_create ((ADCL_work_fnct_ptr *)otpo_test_func, attrset, 
                                  attr_vals, test, &functions[count++]);
        }
        else {
            if (debug)
            {
                printf("Removing the Combination:\t");
                for (i=0 ; i<num_parameters ; i++) 
                {
                    printf("%d\t",attr_vals[i]);
                }
                printf("\n");
            }
        }
        more = get_next_combination (attr_vals);
    }    
    ADCL_Fnctset_create (count, functions, "NETPIPE FNCTSET", fnctset);

    return count;
}

/* Check that the combination attr_vals, satisfies all rpns */
static int check_rpn (int *attr_vals)
{
    int i, j, value1, value2, ret;
        
    ret = 1; 
    
    for (i=0 ; i<num_parameters; i++)
    {
        if (0 == ret) 
        {
            break;
        }
        if (0 == list_params[i].num_rpn_elements)
        {
            continue;
        }
        rpn_stack_position = 0;
        j = 0;
        while (1)
        {
            if (list_params[i].num_rpn_elements == j)
            {
                break;
            }
            if (list_params[i].rpn_elements[j].is_operator)
            {
                value1 = pop();
                value2 = pop();
                switch (list_params[i].rpn_elements[j].type.operator_type)
                {
                case ADD:
                    push (value2+value1);
                    break;
                case SUBTRACT:
                    push (value2-value1);
                    break;
                case MULTIPLY:
                    push (value2*value1);
                    break;
                case DIVIDE:
                    push (value2/value1);
                    break;
                case EQUAL:
                    push ((value2 == value1) ? 1 : 0); 
                    break;
                case NEQUAL:
                    push ((value2 != value1) ? 1 : 0);
                    break;
                case GREATER:
                    push ((value2 > value1) ? 1 : 0);
                    break;
                case LESS:
                    push ((value2 < value1) ? 1 : 0);
                    break;
                case GEQUAL:
                    push ((value2 >= value1) ? 1 : 0);
                    break;
                case LEQUAL:
                    push ((value2 <= value1) ? 1 : 0);
                    break;
                case AND:
                    push ((value2==1 && value1==1) ? 1 : 0);
                    break;
                case OR:
                    push ((value2==1 || value1==1) ? 1 : 0);
                    break;
                default:
                    fprintf (stderr, "Invalid operator\n");
                    exit(1);
                }
            }
            else 
            {
                if (PARAM == list_params[i].rpn_elements[j].type.operand_type)
                {
                    push (attr_vals[list_params[i].rpn_elements[j].value.param_index]);
                }
                else if (INTEGER == list_params[i].rpn_elements[j].type.operand_type)
                {
                    push (list_params[i].rpn_elements[j].value.integer_value);
                }
            }
            j ++;
        }
        if (1 == rpn_stack_position)
            /*&& attr_vals[i] != list_params[i].possible_values[0])*/
        {
            ret = rpn_stack[0];
        }
        else if (1 != rpn_stack_position) 
        {
            fprintf (stderr, "RPN Condition is not Valid\n");
            exit (1);
        }
    }
    return ret;
}

static int pop ()
{
    if (0 < rpn_stack_position)
    {
        return rpn_stack[--rpn_stack_position];
    }
    else 
    {
        fprintf (stderr, "RPN Stack is EMPTY\n");
        exit (1);
    }
}

static int push (int value)
{
    if (RPN_MAX_ELEMENTS > rpn_stack_position)
    {
        rpn_stack[rpn_stack_position++] = value;
    }
    else 
    {
        fprintf (stderr, "RPN Stack is FULL\n");
        exit (1);
    }
    return SUCCESS;
}

static int get_next_combination (int *attr_vals)
{
    int i, current;
    
    for (i=0 ; i<num_parameters ; i++)
    {
        current = attr_vals[i];

        if (current < list_params[i].possible_values[list_params[i].num_values-1])
        {
            attr_vals[i] = get_next_val (i, attr_vals[i]);
            return 1;
        }
        else if (current == list_params[i].possible_values[list_params[i].num_values-1])
        {
            attr_vals[i] = list_params[i].possible_values[0];
        }
    }
    return 0;
}

static int get_next_val (int index, int val)
{
    int i, next_val=-1;
    
    for (i=0 ; i<list_params[index].num_values ; i++)
    {
        if (list_params[index].possible_values[i] == val) 
        {
            next_val = i;
            break;
        }
    }
    if (-1 != next_val && next_val != (list_params[index].num_values-1)) 
    {
        next_val = list_params[index].possible_values[i+1];
    }
    return next_val;
}

static int check_child_died (pid_t pid)
{
    time_t end;
    pid_t ret;

    end = time (NULL) + TIMEOUT; 
    do
    {
        ret = waitpid (pid, 0, WNOHANG);
        if (pid == ret) 
        {
            /* it died */
            return 1;
        }
        else if (-1 == pid && ECHILD == errno)
        {
            /* good enough */
            return 1;
        }
    } while (time(NULL) < end);
    
    /* still alive */
    return 0;
}

static int kill_child (pid_t pid) 
{
    if (debug)
    {
        printf ("Killing the child with SIGTREM...\n");
    }
    if (0 == kill (pid, SIGTERM))
    { 
        return SUCCESS;
    }
    if (debug)
    {
        printf ("SIGTREM didn't kill the child, will try SIGINT...\n");
    }
    if (0 == kill (pid, SIGINT))
    {
        return SUCCESS;
    }
    if (debug)
    {
        printf ("SIGINT didn't kill the child, I will be violent...\n");
    }
    if (0 == kill (pid, SIGKILL))
    {
        return SUCCESS;
    }
    return FAIL;
}

static int set_mpi_arguments (char *** mpi_args)
{
    int i;
    i = 0;
    
    /* JMS: do not use a hard-coded 20 here */
    (*mpi_args) = (char **)malloc(sizeof(char*) * (mca_args_len + 20));
    (*mpi_args)[0] = "mpirun";
    (*mpi_args)[1] = "-np";
    (*mpi_args)[2] = num_proc;
    for (i=3 ; i<mca_args_len+3 ; i++)
    {
        (*mpi_args)[i] = mca_args[i-3];
    }
    if (NULL != hostf) 
    {
        (*mpi_args)[i++] = "-hostfile";
        (*mpi_args)[i++] = hostf;
    }
    else 
    {
        (*mpi_args)[i++] = "--bynode";
    }

#if LEAVE_PINNED
    (*mpi_args)[i++] = "--mca";
    (*mpi_args)[i++] = "mpi_leave_pinned";
    (*mpi_args)[i++] = "1";
#endif
#if PAFFINITY_ALONE
    (*mpi_args)[i++] = "--mca";
    (*mpi_args)[i++] = "mpi_paffinity_alone";
    (*mpi_args)[i++] = "1";
#endif 
    
    (*mpi_args)[i++] = test_path;
    if (!strcasecmp(test,"Netpipe"))
    {
        (*mpi_args)[i++] = "-l";
        (*mpi_args)[i++] = msg_size;
        (*mpi_args)[i++] = "-u";
        (*mpi_args)[i++] = msg_size;
        (*mpi_args)[i++] = "-p 0";
    }
    else if (!strcasecmp(test,"skampi"))
    {
        (*mpi_args)[i++] = "-i";
        (*mpi_args)[i++] = "skampi.ski"; 
        (*mpi_args)[i++] = "-o";
        (*mpi_args)[i++] = "skampi.sko";
    }

    (*mpi_args)[i++] = NULL;
    
    return SUCCESS;
}

static int update_adcl_request (ADCL_Request req)
{
    char line[LINE_SIZE];
    char *tok;
    double latency;
    FILE *fp;
   
    if(!strcasecmp(test,"skampi"))
    {
      open:
        fp = fopen ("skampi.sko","r");
        if (NULL == fp)
        {
            if (EINTR == errno)
            {
                /* try again */
                goto open;
            }
            printf ("--> Can't open skampi.sko");
            return FAIL;
        }

        fgets (line, LINE_SIZE, fp);
        fgets (line, LINE_SIZE, fp);
        fgets (line, LINE_SIZE, fp);
	
        
        fgets (line, LINE_SIZE, fp);
        tok = strtok (line," \t");
        tok = strtok (NULL," \t");
        latency = strtod (tok,NULL);
        if (debug)
        {
	    printf ("Time taken = %f usec\n",latency);
        }
        
        ADCL_Request_update (req, latency);
        remove("skampi.sko");
    } 
    else if(!strcasecmp(test,"netpipe"))
    {
      open1: 
        fp = fopen ("np.out", "r");
        if (NULL == fp) 
        {
            if (EINTR == errno)
            {
                /* try again */
                goto open1;
            }
            printf ("Can't open file np.out");
            return FAIL;
        }
        while (NULL != fgets (line, LINE_SIZE, fp)) 
        {
            tok = strtok (line," \t");
            tok = strtok (NULL," \t");
            tok = strtok (NULL," \t");
            latency = strtod (tok,NULL);
            if (debug) 
            {
                printf ("latency = %f usec\n",latency*1000000);
            }
            ADCL_Request_update (req, latency*1000000);
        }
    }   
    
    fclose(fp);
    return SUCCESS;
}
