/*
 * Copyright (c) 2007 Cisco, Inc. All rights reserved.
 * Copyright (c) 2010 University of Houston, Inc. All rights reserved.
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

#include <unistd.h>

#include <sys/stat.h>

#include "otpo.h"

static int check_child_died (pid_t);
static int kill_child (pid_t);
static int get_next_combination (int *);
static int get_next_val (int, int);
static int check_rpn (int *);
static otpo_rpn_stack_element pop ();
static int push (otpo_rpn_stack_element);
static int set_mpi_arguments (char ***);
static int update_adcl_request (ADCL_Request);

unsigned int  TIMEOUT = 200;
otpo_rpn_stack_element rpn_stack [RPN_MAX_ELEMENTS];
int rpn_stack_position;

/* Funtion that forks the mpirun */
void otpo_test_func (ADCL_Request req) 
{
    int attrs_num, *attrs_values_num;
    int i,k,m,j,code,rc, fd;
    int virtual, aggregate;
    char *function_name;
    char **attrs_names;
    char **attrs_values_names;
    pid_t pid;
    int pipefd[2];
    char c;
    char cmd[100];
    FILE *pfp = NULL;

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
        
        ADCL_Request_get_curr_function (req, &function_name, 
                                        &attrs_names, &attrs_num,
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
                char *token = NULL;
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

        /* Put here benchmarks with no output file option */
        switch (test) {
        case OTPO_TEST_NPB:
            if((fd = open("npb.out", O_RDWR | O_CREAT, S_IRWXU ))==-1){
                perror("open");
                return ;
            }
            break;
        case OTPO_TEST_LATENCY_IO:
            if((fd = open("latency_io.out", O_RDWR | O_CREAT, S_IRWXU ))==-1){
                perror("open");
                return ;
            }
            break;
        case OTPO_TEST_NONCONTIG:
            if((fd = open("noncontig.out", O_RDWR | O_CREAT, S_IRWXU ))==-1){
                perror("open");
                return ;
            }
            break;
        case OTPO_TEST_MPI_TILE_IO:
            if((fd = open("mpi_tile_io.out", O_RDWR | O_CREAT, S_IRWXU ))==-1){
                perror("open");
                return ;
            }
            break;
        default:
            break;
        }

        if (OTPO_TEST_NPB == test ||
            OTPO_TEST_LATENCY_IO == test ||
            OTPO_TEST_NONCONTIG == test ||
            OTPO_TEST_MPI_TILE_IO == test) {

            dup2(fd,STDOUT_FILENO); /*copy the file descriptor fd into standard output*/
            dup2(fd,STDERR_FILENO); /* same, for the standard error */
            close(fd); /* close the file descriptor as we don't need it more  */
        }

        if (-1 == (code = execvp ("mpirun",  mpi_args))) {
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
    char fnctset_name[20];
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
                                  attr_vals, (char *)tests_names[test], &functions[count++]);
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
    sprintf(fnctset_name, "%s FNCTSET",  tests_names[test]);
    ADCL_Fnctset_create (count, functions, fnctset_name, fnctset);

    return count;
}

/* Check that the combination attr_vals, satisfies all rpns */
static int check_rpn (int *attr_vals)
{
    int i, j, ret;
    otpo_rpn_stack_element e1, e2, e3;
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
                char *temp1=NULL,*temp2=NULL;
                e1 = pop();
                if (STRING == e1.type.operand_type) {
                    temp1 = strdup (e1.value.string_value);
                }
                else if (PARAM == e1.type.operand_type) {
                    int k = 0;
                    while (1) {
                        if (attr_vals[e1.value.param_index] == 
                            list_params[e1.value.param_index].possible_values[k]) {
                            temp1 = strdup 
                                (list_params[e1.value.param_index].string_values[k]);
                            break;
                        }
                        k++;
                    }
                }
                e2 = pop();
                if (STRING == e2.type.operand_type) {
                    temp2 = strdup (e2.value.string_value);
                }
                else if (PARAM == e2.type.operand_type) {
                    int k = 0;
                    while (1) {
                        if (attr_vals[e2.value.param_index] == 
                            list_params[e2.value.param_index].possible_values[k]) {
                            temp2 = strdup 
                                (list_params[e2.value.param_index].string_values[k]);
                            break;
                        }
                        k++;
                    }
                }
                e3.type.operand_type = INTEGER;
                e3.is_operator = 0;
                switch (list_params[i].rpn_elements[j].type.operator_type)
                {
                case EQUAL:
                    if (NULL != temp1 && NULL != temp2)
                        e3.value.integer_value = 
                            ((strcmp(temp2,temp1)) ? 0 : 1);
                    if (INTEGER == e2.type.operand_type && INTEGER == e1.type.operand_type)
                        e3.value.integer_value = 
                            ((e2.value.integer_value == e1.value.integer_value) ? 1 : 0);
                    push (e3);
                    break;
                case NEQUAL:
                    if (NULL != temp1 && NULL != temp2)
                        e3.value.integer_value = 
                            ((strcmp(temp1,temp2)) ? 1 : 0);
                    if (INTEGER == e2.type.operand_type && INTEGER == e1.type.operand_type)
                        e3.value.integer_value = 
                            ((e2.value.integer_value == e1.value.integer_value) ? 0 : 1);
                    push (e3);
                    break;
                case ADD:
                    e3.value.integer_value = 
                        e1.value.integer_value + e2.value.integer_value;
                    push (e3);
                    break;
                case SUBTRACT:
                    e3.value.integer_value = 
                        e2.value.integer_value - e1.value.integer_value;
                    push (e3);
                    break;
                case MULTIPLY:
                    e3.value.integer_value = 
                        e2.value.integer_value * e1.value.integer_value;
                    push (e3);
                    break;
                case DIVIDE:
                    e3.value.integer_value = 
                        e2.value.integer_value / e1.value.integer_value;
                    push (e3);
                    break;
                case GREATER:
                        e3.value.integer_value = 
                            ((e2.value.integer_value > e1.value.integer_value) ? 1 : 0);
                    push (e3);
                    break;
                case LESS:
                        e3.value.integer_value = 
                            ((e2.value.integer_value < e1.value.integer_value) ? 1 : 0);
                    push (e3);
                    break;
                case GEQUAL:
                        e3.value.integer_value = 
                            ((e2.value.integer_value >= e1.value.integer_value) ? 1 : 0);
                    push (e3);
                    break;
                case LEQUAL:
                        e3.value.integer_value = 
                            ((e2.value.integer_value <= e1.value.integer_value) ? 1 : 0);
                    push (e3);
                    break;
                case AND:
                        e3.value.integer_value = 
                            ((e2.value.integer_value==1 && e1.value.integer_value==1) ? 1 : 0);
                    push (e3);
                    break;
                case OR:
                        e3.value.integer_value = 
                            ((e2.value.integer_value==1 || e1.value.integer_value==1) ? 1 : 0);
                    push (e3);
                    break;
                default:
                    fprintf (stderr, "Invalid operator\n");
                    exit(1);
                }
                if (NULL != temp1) {
                    free (temp1);
                    temp1 = NULL;
                }
                if (NULL != temp2) {
                    free (temp2);
                    temp2 = NULL;
                }
            }
            else 
            {
                push (list_params[i].rpn_elements[j]);
            }
            j ++;
        }
        if (1 == rpn_stack_position)
        {
            ret = rpn_stack[0].value.integer_value;
            if (!ret && attr_vals[i] == list_params[i].possible_values[0]) {
                ret = 1;
            }
        }
        else if (1 != rpn_stack_position) 
        {
            fprintf (stderr, "RPN Condition is not Valid\n");
            exit (1);
        }
    }
    return ret;
}

static otpo_rpn_stack_element pop ()
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

static int push (otpo_rpn_stack_element element)
{
    if (RPN_MAX_ELEMENTS > rpn_stack_position)
    {
        rpn_stack[rpn_stack_position++] = element;
        /*
        if (STRING == element.type.operand_type) {
            printf ("PUSHING %s\n", element.value.string_value);
        }
        else if (PARAM == element.type.operand_type) {
            printf ("PUSHING %s\n", list_params[element.value.param_index].name);
        }
        else if (INTEGER == element.type.operand_type) {
            printf ("PUSHING %d\n", element.value.integer_value);
        }
        */
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
    int i=0,k;
    char temp[2];
   
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
    else if (OTPO_TEST_NETPIPE == test)
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
     switch(test) {
     case OTPO_TEST_NETPIPE:
         (*mpi_args)[i++] = "-l";
         (*mpi_args)[i++] = msg_size;
         (*mpi_args)[i++] = "-u";
         (*mpi_args)[i++] = msg_size;
         (*mpi_args)[i++] = "-p 0";
         break;
     case OTPO_TEST_SKAMPI:
         (*mpi_args)[i++] = "-i";
         (*mpi_args)[i++] = "skampi.ski"; 
         (*mpi_args)[i++] = "-o";
         (*mpi_args)[i++] = "skampi.sko";
         break;
     case OTPO_TEST_NPB: /* No argument so far */
         break;
     case OTPO_TEST_LATENCY_IO:
         sprintf (temp,"%d",op_num);
         (*mpi_args)[i++] = "-m";
         (*mpi_args)[i++] = "12";
         (*mpi_args)[i++] = "-s";
         (*mpi_args)[i++] = msg_size;
         (*mpi_args)[i++] = "-p";
         (*mpi_args)[i++] = output_dir;
         (*mpi_args)[i++] = "-f";
         (*mpi_args)[i++] = "io_test";
         break;
     case OTPO_TEST_NONCONTIG:
         (*mpi_args)[i++] = "-fname";
         (*mpi_args)[i++] = "io_test";
         (*mpi_args)[i++] = "-fsize";
         (*mpi_args)[i++] = "20";
         (*mpi_args)[i++] = "-veclen";
         (*mpi_args)[i++] = num_proc;
         (*mpi_args)[i++] = "-timing";
         (*mpi_args)[i++] = "-coll";
         break;
     case OTPO_TEST_MPI_TILE_IO:
         (*mpi_args)[i++] = "--filename";
         (*mpi_args)[i++] = "io_test";
         (*mpi_args)[i++] = "--write_file";
         (*mpi_args)[i++] = "--collective";
         (*mpi_args)[i++] = "--nr_tiles_x";

         (*mpi_args)[i++] = "--nr_tiles_y";
         
         (*mpi_args)[i++] = "--sz_tile_x";

         (*mpi_args)[i++] = "--sz_tile_y";
         
         (*mpi_args)[i++] = "--sz_element";
         (*mpi_args)[i++] = msg_size;
         break;
    }

    (*mpi_args)[i++] = NULL;
    for (k=0 ; k<i-1 ; k++) {
        printf ("%s ", (*mpi_args)[k]);
    }
    printf ("\n");

    return SUCCESS;
}

static int update_adcl_request (ADCL_Request req)
{
    char line[LINE_SIZE];
    char *tok;
    double latency, bandwidth;
    FILE *fp;
    switch(test) {

    case OTPO_TEST_NETPIPE:
    open: 
        fp = fopen ("np.out", "r");
        if (NULL == fp) 
        {
            if (EINTR == errno)
            {
                /* try again */
                goto open;
            }
            printf ("Can't open file np.out\n");
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
        break;

    case OTPO_TEST_SKAMPI:
    open1:
        fp = fopen ("skampi.sko","rwq");
        if (NULL == fp)
        {
            if (EINTR == errno)
            {
                /* try again */
                goto open1;
            }
            printf ("--> Can't open skampi.sko\n");
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
        break;

    case OTPO_TEST_NPB:
    open2:
        fp = fopen ("npb.out", "r");
        if (NULL == fp)
        {
            if (EINTR == errno)
            {
                /* try again */
		goto open2;
            }
            printf ("--> Can't open npb.out\n");
            return FAIL;
        }
        fseek ( fp , -875 , SEEK_END );
        fgets (line, LINE_SIZE, fp);
        tok = strtok (line," \t");
        latency = strtod (tok,NULL);
        ADCL_Request_update (req, latency);
        remove("npb.out");
        break;

    case OTPO_TEST_LATENCY_IO:
    open3:
        fp = fopen ("latency_io.out", "r");
        if (NULL == fp)
        {
            if (EINTR == errno)
            {
                /* try again */
		goto open3;
            }
            printf ("--> Can't open noncontig.out\n");
            return FAIL;
        }
        while (1) {
            fgets (line, LINE_SIZE, fp);
            if (line[0] == 'T') {
                break;
            }
        }
        tok = strtok (line," \t");
        while (1) {
            if (!strcmp ("bandwidth", tok)) {
                break;
            }
            tok = strtok (NULL," \t");
        }
        tok = strtok (NULL," \t");
        bandwidth = strtod (tok, NULL);
        bandwidth = 1/bandwidth;
        if (debug || verbose) {
            printf ("bandwidth: %s\n", tok); 
        }
        ADCL_Request_update (req, bandwidth);
        remove ("latency_io.out");
        remove ("io_test");
        break;

    case OTPO_TEST_NONCONTIG:
    open4:
        fp = fopen ("noncontig.out", "r");
        if (NULL == fp)
        {
            if (EINTR == errno)
            {
                /* try again */
		goto open4;
            }
            printf ("--> Can't open noncontig.out\n");
            return FAIL;
        }
        while (1) {
            fgets (line, LINE_SIZE, fp);
            if (line[0] == ' ') {
                break;
            }
        }
        tok = strtok (line," \t");
        while (1) {
            if (tok[0] == ':') {
                break;
            }
            tok = strtok (NULL," \t");
        }
        tok = strtok (NULL," \t/"); /*min*/
        tok = strtok (NULL," \t/"); /*max*/
        bandwidth = strtod (tok,NULL);
        ADCL_Request_update (req, (-1 * bandwidth));
        remove ("noncontig.out");
        break;

    case OTPO_TEST_MPI_TILE_IO:
    open5:
        fp = fopen ("mpi_tile_io.out", "r");
        if (NULL == fp)
        {
            if (EINTR == errno)
            {
                /* try again */
		goto open4;
            }
            printf ("--> Can't open mpi_tile_io.out\n");
            return FAIL;
        }
        while (1) {
            fgets (line, LINE_SIZE, fp);
            if (line[0] != '#') {
                break;
            }
        }
        tok = strtok (line," \t");
        tok = strtok (NULL," \t");
        tok = strtok (NULL," \t");
        tok = strtok (NULL," \t");
        bandwidth = strtod (tok,NULL);
        ADCL_Request_update (req, (-1 * bandwidth));
        remove ("mpi_tile_io.out");
        break;
    }
    
    fclose(fp);
    return SUCCESS;
}
