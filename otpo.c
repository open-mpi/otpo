/*
 * Copyright (c) 2007 Cisco, Inc. All rights reserved.
 * Copyright (c) 2010 University of Houston, Inc. All rights reserved.
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "otpo.h"

void handler (int);
void create_skampi_ipfile(void);
static int get_num_combinations ();
static int set_mca_options (int, char**);
static int get_result_files (int, char**);
static void print_usage (void);
int stop_signal;
time_t t1, t2, et1, et2;

int main(int argc , char *argv[]) 
{
    int i, k, num_comb, hr, min, sec, generate_input_file;
    int num_tested, current_winner, resume, num_functions, testing;
    char *input_file = NULL;
    char *interrupt_file = NULL;
    float current;
    double *results = NULL, *unfiltered = NULL, *outliers = NULL;
    ADCL_Attribute *ADCL_param_attributes = NULL;
    ADCL_Attrset ADCL_param_attrset;
    ADCL_Fnctset ADCL_param_fnctset;
    ADCL_Topology ADCL_param_topo;
    ADCL_Request ADCL_param_request;

    /* options */
    static struct option long_opts[] = 
    {
        { "verbose",              no_argument, NULL, 'v' },
        { "status",               no_argument, NULL, 's' },
        { "debug",                no_argument, NULL, 'd' },
        { "silent",               no_argument, NULL, 'n' },

        { "params",               required_argument, NULL, 'p' },
        { "test",                 required_argument, NULL, 't' },
        { "test_path",            required_argument, NULL, 'w' },
        { "message_length",       required_argument, NULL, 'l' }, 
        { "hostfile",             required_argument, NULL, 'h' },
        { "mca",                  required_argument, NULL, 'm' },
        { "format",               required_argument, NULL, 'f' },
        { "out",                  required_argument, NULL, 'o' },
        { "backup",               required_argument, NULL, 'b' },
        { "resume",               required_argument, NULL, 'r' },
        { "collective_operation", required_argument, NULL, 'c' },
        { "number_of_processes",  required_argument, NULL, 'a' },
        { "operation",            required_argument, NULL, 'e' },
        { "generate_input_file",  required_argument, NULL, 'x' },
        { NULL, 0, NULL, 0 }
    };

    et1 = time (NULL);
    stop_signal = 0;

    verbose = 1;
    status = 0;
    debug = 0;
    mca_args_len = 0;
    hostf = NULL;
    test = 0;
    test_path = NULL;
    msg_size = NULL;
    output_dir = NULL;
    resume = 0;
    time(&stamp);
    op_num = -1;
    num_proc = NULL;
    operation = NULL;
    testing = 1;
    generate_input_file = 0;

    tests_names[0] = strdup("Netpipe");
    tests_names[1] = strdup("SKaMPI");
    tests_names[2] = strdup("NPB");
    tests_names[3] = strdup("latency_io");
    tests_names[4] = strdup("noncontig");
    tests_names[5] = strdup("mpi_tile_io");

    if (SUCCESS != set_mca_options (argc, argv)) 
    {
        fprintf(stderr, "Invalid mca options");
        exit(1);
    }

    if (SUCCESS != get_result_files (argc, argv)) 
    {
        fprintf(stderr, "Invalid result files");
        exit(1);
    }

    while (-1 != (i = getopt_long (argc, 
                                   argv, 
                                   "p:t:w:vsndl:m:h:g:o:f:r:b:c:a:e:x:",
                                   long_opts, 
                                   NULL))) 
    {
        switch (i) 
        {
        case 'p':
            input_file = strdup (optarg);
            break;
        case 'r':
            resume = 1;
            interrupt_file = strdup (optarg);
            break;
        case 'b':
            interrupt_file = strdup (optarg);
            break;
        case 'f':
            if (strcasecmp (optarg, "XML")) 
            {
                output = XML;
            }
            else 
            {
                output = TEXT;
            }
            break;
        case 'v':
            verbose = 1;
            break;
        case 's':
            status = 1;
            break;
        case 'n':
            break;
        case 'd':
            debug = 1;
            break;
        case 't':
            if ( !strcasecmp (optarg,"Netpipe")) {
                test = OTPO_TEST_NETPIPE;
            }
            else if ( !strcasecmp (optarg,"skampi")) {
                test = OTPO_TEST_SKAMPI;
            }
            else if ( !strcasecmp (optarg,"NPB")) {
                test = OTPO_TEST_NPB;
            }
            else if ( !strcasecmp (optarg,"latency_io")) {
                test = OTPO_TEST_LATENCY_IO;
            }
            else if ( !strcasecmp (optarg,"noncontig")) {
                test = OTPO_TEST_NONCONTIG;
            }
            else if ( !strcasecmp (optarg,"mpi_tile_io")) {
                test = OTPO_TEST_MPI_TILE_IO;
            }
            else {
                printf ("Invalid Test Name\n");
                exit (1);
            }
            break;
        case 'w':
            test_path = strdup (optarg);
            break;
        case 'l':
            msg_size = strdup (optarg);
            if (optarg[0] != '0' && atoi (optarg) == 0) 
            {
                printf ("Invalid Message Size\n");
                exit (1);
            }
            break;
        case 'm':
            break;
        case 'h':
            hostf = strdup (optarg);
            break;
        case 'o':
            output_dir = strdup (optarg);
            break;
        case 'c':
            op_num =  atoi(optarg);
            break;
        case 'a':
            num_proc = strdup(optarg);
            break;
        case 'e':
            operation = strdup (optarg);
            break;
        case 'x':
            generate_input_file = 1;
            break;
        default: 
            print_usage();
            exit (1);
        }
    }

    if (generate_input_file) {
        otpo_generate_input_file ();
        return 0;
    }

    if (NULL == input_file)
    {
        printf ("You need an input file that contains the parameters\n");
        print_usage();
        exit (1);
    }

    if (NULL == test_path)
    {
        printf ("You need to specify the path to the test u want to execute\n");
        print_usage();
        exit (1);
    }

    if (NULL == output_dir)
    {
        output_dir = strdup ("results");
    }

    if (NULL == interrupt_file)
    {
        interrupt_file = strdup ("interrupt.txt");
    }

    if (NULL == msg_size)
    {
        msg_size = strdup ("1024");
    }

    if (0 == test)
    {
        test = OTPO_TEST_NETPIPE;
    }
    
    if (NULL == num_proc) 
    {
        num_proc = strdup ("2");
    }

    if (OTPO_TEST_SKAMPI == test) 
    {
        if (op_num < 0 || op_num > 11) {
            printf("Invalid Collective Operation number!");
            exit(1);
        }
        if (NULL == operation) {
            operation = strdup ("MPI_MAX");
        }
    }
    else if (OTPO_TEST_LATENCY_IO == test) {
        if (op_num < 1 || op_num > 17) {
            printf("Invalid IO MODE");
            exit(1);
        }
    }

    if (verbose || debug)
    {
        printf ("I will read the Parameter from %s\n", input_file);
        printf ("I will write the intermediate results to %s/result%ld\n", 
                output_dir,stamp);
        printf ("In case I detect an interrupt, I will write my data to %s\n", 
                interrupt_file);
        printf ("The Test case will be using %s, with %s byte messages\n",
                tests_names[test],msg_size); 

        if(OTPO_TEST_SKAMPI == test)
        {
            printf("I will use the operation of number %d with %s number of processes\n", 
                   op_num, num_proc);
        }
    }

    if(OTPO_TEST_SKAMPI == test)
    {
        create_skampi_ipfile();
    }

    /* read file to get number of parameters */
    if (0 == (num_parameters = otpo_get_num_parameters (input_file))) 
    {
        printf ("File contains no Data\n");
        exit (0);
    }
    if (debug) 
    {
        printf ("Read %d parameters\n",num_parameters);
    }

    /* structure that hold all the parameter info */
    list_params = (struct otpo_param_list_t *)
        malloc(sizeof(struct otpo_param_list_t) * num_parameters);
    if (NULL == list_params) 
    {
        fprintf (stderr,"Malloc Failed...\n");
        exit (1);
    }

    /* initialize list_params */
    if (SUCCESS != otpo_initialize_list (input_file))
    {
        exit (1);
    }

    /* fake MPI_init , does nothing*/
    MPI_Init (&argc, &argv);    
    ADCL_Init ();
    
    if (debug)
    {
        otpo_dump_list ();       
    }
    
    /* allocating the list of ADCL attributes */
    ADCL_param_attributes = (ADCL_Attribute *)malloc(sizeof(ADCL_Attribute) 
                                                     * num_parameters);
    if( NULL == ADCL_param_attributes ) 
    {
        fprintf (stderr,"Malloc Failed...\n");
        exit (1);
    }
    
    if (SUCCESS != otpo_populate_attributes (ADCL_param_attributes))
    {
        exit(1);
    }
    ADCL_Attrset_create (num_parameters, ADCL_param_attributes, &ADCL_param_attrset);

    /* total number of combinations */
    num_comb = get_num_combinations ();

    if (FAIL == (num_comb = otpo_populate_function_set (ADCL_param_attrset, num_comb, 
                                                        &ADCL_param_fnctset)))
    {
        exit(1);
    }

    if (debug) 
    {
        printf("Number of possible combinations: %d\n",num_comb);
    }

    ADCL_Topology_create_generic (0, 0, NULL, NULL, NULL, NULL, ADCL_DIRECTION_BOTH,
                                  MPI_COMM_WORLD, &ADCL_param_topo);

    ADCL_Request_create (ADCL_VECTOR_NULL, ADCL_param_topo, ADCL_param_fnctset, 
                         &ADCL_param_request);

    signal(SIGINT, handler);
    signal(SIGHUP, handler);
    signal(SIGTERM, handler);

    /* Start the Tests */
    num_tested=0;
    if (1 == resume) 
    {
        printf("Reading data to Resume...\n");
        if (SUCCESS != otpo_read_interrupt_data (interrupt_file, &num_tested, &results))
        {
            fprintf (stderr, "Can't Restore Data\n");
            exit (1);
        }
        if (debug)
        {
            printf("%d combinations left\n", num_comb-num_tested);
        }
        unfiltered = (double *)malloc(sizeof(double)*num_tested);
        outliers = (double *)malloc(sizeof(double)*num_tested);
        if (NULL == unfiltered || NULL == outliers) 
        {
            fprintf (stderr,"Malloc Failed...\n");
            exit (1);
        }
        for (k=0 ; k<num_tested ; k++) 
        {
            unfiltered[k] = results[k];
            outliers[k] = 0.0;
        }
        ADCL_Request_restore_status (ADCL_param_request, num_tested,
                                     results, unfiltered, outliers);
        if (NULL != results)
        {
            free (results);
        }
        if (NULL != unfiltered)
        {
            free (unfiltered);
        }
        if (NULL != outliers)
        {
            free (outliers);
        }
    }
    current = (float)num_tested;
    if (resume)
    {
        current--;
    }
    /* we execute till num_comb + 1, because ADCL requires to execute one more 
       to switch into the decision state. The last function executed is the 
       winner function 
    */
    for (i=num_tested ; (i<num_comb+1) && testing ; i++) 
    {
        if (status || verbose || debug) 
        {
            if (i >= current)
            {
                printf ("Completed: %d%\r",(int)( (i/(float)num_comb) * 100 ));
                fflush(stdout);
                current += num_comb/100.0; 
            }
        }
        if (0 == stop_signal)
        {
            ADCL_Request_start (ADCL_param_request);
            ADCL_Request_get_state(ADCL_param_request, &testing);
        }
        else if (1 == stop_signal && 1 < i)
        {
            printf ("Backing up Data...\n");
            ADCL_Request_save_status (ADCL_param_request, &num_tested,
                                      &results, &unfiltered, &outliers,
                                      &current_winner);

            if (SUCCESS != otpo_write_interrupt_data (num_tested, results, 
                                                      current_winner, interrupt_file))
            {
                fprintf(stderr,"Couldn't Backup Data, Quiting...\n");
                exit(1);
            }
            if (NULL != results)
            {
                free (results);
            }
            if (NULL != unfiltered)
            {
                free (unfiltered);
            }
            if (NULL != outliers)
            {
                free (outliers);
            }
            exit(1);
        }
        else if (1 == stop_signal)
        {
            exit(1);
        }
    }
    
    /* Output the results */
    if (SUCCESS != otpo_write_results (ADCL_param_request,
                                       &num_functions))
    {
        exit(1);
    }
    
    /* Output the results- seperate*/ 
    if (SUCCESS != otpo_analyze_results (num_functions))
    {
        exit(1);
    }
    
    ADCL_Request_free (&ADCL_param_request);
    ADCL_Topology_free (&ADCL_param_topo);
    ADCL_Fnctset_free (&ADCL_param_fnctset);
    ADCL_Attrset_free (&ADCL_param_attrset);
    for (i=0 ; i<num_parameters ; i++) 
    {
        ADCL_Attribute_free (&ADCL_param_attributes[i]);
    }
    ADCL_Finalize();
    MPI_Finalize();

    if (NULL != input_file) 
    {
        free (input_file);
    }
    if (NULL != output_dir) 
    {
        free (output_dir);
    }
    if (NULL != interrupt_file) 
    {
        free (interrupt_file);
    }
    otpo_free_list_params_objects();
    if (NULL != list_params) 
    {
        free (list_params);
    }
    if (NULL != msg_size) 
    {
        free (msg_size);
    }
    for (i=0 ; i<mca_args_len ; i++)
    {   
        if (NULL != mca_args[i])
            free (mca_args[i]);
    }
    if (NULL != mca_args)
    { 
        free (mca_args);
    }
    if (NULL != hostf) 
    {
        free (hostf);
    }
    if(OTPO_TEST_SKAMPI == test)
    {
        if (NULL != operation) 
        {
            free (operation);
        }
    }    

    if (NULL != num_proc) 
    {
        free (num_proc);
    }

    et2 = time (NULL);
    sec = 0;
    hr = 0;
    min = 0;
    sec = (int)difftime (et2, et1);
    if (3600 <= sec)
    {
        hr = sec/3600;
        sec = sec%3600;
    }
    if (60 <= sec)
    {
        min = sec/60;
        sec = sec%60;
    }
    printf ("Time Elapsed: %d hrs %d min %d sec\n", hr, min, (int)sec);
    return 0;
}

/* 
 * Handler function to detect signals 
 */
void handler (int sig)
{
    t2 = time (NULL);
    if (0 == stop_signal) 
    {
        printf ("Im in the Middle of Quiting, If you can't wait, Press Ctrl-C in the next SECOND to exit forcefully...\n");
        t1 = time (NULL);
        stop_signal = 1;
    }
    else if (1 == stop_signal && (t2-t1) < 1) 
    {
        exit (1);
    }
}

/*
 * Function to get the total number of combinations of attibute values
 */ 
static int get_num_combinations ()
{
    int i;
    int num;
    
    num = list_params[0].num_values;
    for (i=1 ; i<num_parameters ; i++)
    {
        num *= list_params[i].num_values;
    }

    return num;
}

/*
 * Function to set the mca options that the users provide for the mpirun command
 */
static int set_mca_options (int argc, char **argv) 
{
    int i,k;

    for (i=0 ; i<argc ; i++) 
    {
        if (0 == strcmp (argv[i],"--mca") || 0 == strcmp (argv[i], "-m"))
        {
            mca_args_len ++;
            while ('-' != argv[++i][0]) 
            {
                mca_args_len ++;
                if (argc == i+1) 
                    break;
            }
            i --;
        }
    }
    mca_args = (char **)malloc(sizeof(char *) * mca_args_len);
    if (NULL == mca_args) 
    {
        return NO_MEMORY;
    }
    k = 0;
    for (i=0 ; i<argc ; i++)
    {
        if (0 == strcmp (argv[i],"--mca") || 0 == strcmp (argv[i], "-m"))
        {
            mca_args[k++] = strdup("--mca");
            while ('-' != argv[++i][0]) 
            {
                mca_args[k++] = strdup(argv[i]);
                if (argc == i+1) 
                    break;
            }
            i --;
        }
    }

    return SUCCESS;
}

/*
 * Function to get the result files that need to analyzed andtransformed \
 * into input files
 */
static int get_result_files (int argc, char **argv) 
{
    int i,k;

    for (i=0 ; i<argc ; i++) 
    {
        if (0 == strcmp (argv[i],"--generate_input_file") || 
            0 == strcmp (argv[i], "-x"))
        {
            while ('-' != argv[++i][0]) 
            {
                num_result_files ++;
                if (argc == i+1) 
                    break;
            }
            i --;
        }
    }
    result_files = (char **)malloc(sizeof(char *) * num_result_files);
    if (NULL == result_files) 
    {
        return NO_MEMORY;
    }
    k = 0;
    for (i=0 ; i<argc ; i++)
    {
        if (0 == strcmp (argv[i],"--generate_input_file") || 
            0 == strcmp (argv[i], "-x"))
        {
            while ('-' != argv[++i][0]) 
            {
                result_files[k++] = strdup(argv[i]);
                if (argc == i+1) 
                    break;
            }
            i --;
        }
    }

    return SUCCESS;
}

int otpo_dump_list ()
{
    int i,k;

    for (i=0 ; i<num_parameters ; i++) 
    {
        printf ("Name: %s\n",list_params[i].name);
        printf ("Number of Values: %d\n", list_params[i].num_values);
        if (NULL != list_params[i].default_value) 
        {
            printf ("Default: %s\n",list_params[i].default_value);
        }
        for (k=0 ; k<list_params[i].num_values ; k++) 
        {
            if (NULL != list_params[i].string_values[k])
            {
                printf ("%d: %s\n",k,list_params[i].string_values[k]);
            }
            else 
            {
                printf ("%d: %d\n",k,list_params[i].possible_values[k]);
            }
        }
        if (NULL == list_params[i].string_values[0])
        {
            printf ("Start: %d\n",list_params[i].start_value);
            printf ("End: %d\n",list_params[i].end_value);
            printf ("Traversal method: %c\n",list_params[i].traverse);
            printf ("Operation: %c\n",list_params[i].operation);
            printf ("Increment: %d\n",list_params[i].traverse_attr.increment);
            if ('\0' != list_params[i].condition[0]) 
            {
                printf ("Condition: %s\n",list_params[i].condition);
            }
        }
        if (0 != list_params[i].num_rpn_elements)
        {
            printf ("RPN ELEMENTS:\t");
            for (k=0 ; k<list_params[i].num_rpn_elements ; k++)
            {
                if (list_params[i].rpn_elements[k].is_operator)
                {
                    printf("%c\t",list_params[i].rpn_elements[k].type.operator_type);
                }
                else
                {
                    switch (list_params[i].rpn_elements[k].type.operand_type)
                    {
                    case INTEGER:
                        printf("%d\t", list_params[i].rpn_elements[k].
                               value.integer_value);
                        break;
                    case PARAM:
                        printf("%s\t", list_params[list_params[i].rpn_elements[k].
                                                   value.param_index].name);
                        break;
                    case STRING:
                        printf("%s\t", list_params[i].rpn_elements[k].
                               value.string_value);
                        break;
                    }
                }
            }
        }
        printf ("\n");
        printf ("***********DONE*************\n"); 
    }
    return SUCCESS;
}

void create_skampi_ipfile()
{
    FILE *fp;
    char coll_ops[12][15] = {"Bcast", "Barrier", "Reduce", "Allreduce", "Gather", "Allgather", 
                             "Gatherv", "Allgatherv", "Alltoall", "Alltoallv", "Scatter", "Scatterv" };

    fp = fopen("skampi.ski", "w");
    fprintf(fp, "set_min_repetitions(500)\n");
    fprintf(fp, "set_max_repetitions(1000)\n");
    fprintf(fp, "set_max_relative_standard_error(0.03)\n");
    fprintf(fp, "\nset_skampi_buffer(%dKB)\n", atoi(msg_size)*atoi(num_proc)/1000+1);
    fprintf(fp, "\ncomm = MPI_COMM_WORLD\n");
    fprintf(fp, "\nbegin measurement \"MPI %s\" ", coll_ops[op_num]);
    if(op_num == 0)
    {
        fprintf(fp, 
                "\n    measure comm : %s(%s, MPI_BYTE, 0)", 
                coll_ops[op_num], 
                msg_size);
    }
    else if(op_num == 1)
    {
        fprintf(fp, 
                "\n    measure comm : %s()", 
                coll_ops[op_num]);
    }
    else if(op_num == 2)
    {
        fprintf(fp, 
                "\n    measure comm : %s(%d, MPI_INT, %s, 0)", 
                coll_ops[op_num], 
                atoi(msg_size)/4, 
                operation);
    }
    else if(op_num == 3)
    {
        fprintf(fp, 
                "\n    measure comm : %s(%s/4, MPI_INT, %s)", 
                coll_ops[op_num], 
                msg_size, 
                operation);
    }
    else if(op_num == 4 || op_num == 6 || op_num == 10 || op_num ==11)
    {
        fprintf(fp, 
                "\n    measure comm : %s(%s, MPI_BYTE, %s , MPI_BYTE,  0)", 
                coll_ops[op_num], 
                msg_size, 
                msg_size);
    }
    else 
    {
        fprintf(fp, 
                "\n    measure comm : %s(%s, MPI_BYTE, %s , MPI_BYTE)", 
                coll_ops[op_num], 
                msg_size, 
                msg_size);
    }

    fprintf(fp, "\nend measurement");

    fclose(fp);
}

static void print_usage ()
{
    printf ("Usage: \n");
    printf ("-p InputFileName (file that contains the parameters)\n");
    printf ("-d (debug output)\n");
    printf ("-v (verbose output)\n");
    printf ("-s (status output)\n");
    printf ("-n (silent/no output)\n");
    printf ("-t test (name of test, Current supported: Netpipe, Skampi, NPB, latency_io, noncontig, mpi_tile_io)\n");
    printf ("-w test_path (path to the test on your system)\n");
    printf ("-l message_length\n");
    printf ("-h hostfile\n");
    printf ("-m mca_params (mca parameters that you want your mpirun to run with)\n");
    printf ("-f format (format of output, TXT)\n");
    printf ("-o output_dir\n");
    printf ("-b interrupt_file (file to write data to when interrupted)\n");
    printf ("-r interrupt_file (the file where contains the data to resume execution)\n");
    printf ("-c Collective operation number (0- Bcast, 1- Barrier, 2- Reduce, 3- Allreduce, 4- Gather, 5- Allgather,");
    printf (" 6- Gatherv, 7-  Allgatherv, 8- Alltoall, 9- Alltoallv, 10- Scatter, 11- Scatterv)\n");
    printf ("-a Number of processes\n");
    printf ("-e operation for Reduce/Allreduce like MPI_MAX\n");
}
