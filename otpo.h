/*
 * Copyright (c) 2007 Cisco, Inc. All rights reserved.
 * Copyright (c) 2010 University of Houston, Inc. All rights reserved.
 */
#ifndef __OTPO_H__
#define __OTPO_H__

#include "ADCL.h"
#include "otpo_config.h"
#include "time.h"

#define LINE_SIZE 1000
#define CONDITION_LENGTH 500
#define RPN_MAX_ELEMENTS 100
#define MAX_VALUES 100
#define RANGE 1.05
int num_parameters;
int debug;
int verbose;
int status;
char *msg_size;
int test;
char *test_path;
char **mca_args;
int mca_args_len;
char **result_files;
int num_result_files;
char *hostf;
time_t stamp;
int op_num;
char *num_proc;
char *operation;
char *tests_names[3];
char *output_dir;

/* enum to the error codes that can be returned */
enum otpo_error_codes
{
    NO_MEMORY = -100,
    INVALID_VALUES = -200,
    INVALID_OPERATION = -300,
    INVALID_TRAVERSE = -400,
    SUCCESS = 0,
    FAIL = -1
};

/* Enum to all operands that can be specified in the RPN */
enum otpo_operator
{
    ADD = '+',
    MULTIPLY = '*',
    DIVIDE = '/',
    SUBTRACT = '-',
    EQUAL = '=',
    GREATER = '>',
    LESS = '<',
    GEQUAL = 1,
    LEQUAL = 2,
    NEQUAL = 3,
    AND = 4,
    OR = 5
};

/* enum to the operand types available in the RPN */
/* currently u can only have PARAM or INTEGER */
enum otpo_operand
{
    PARAM = 1,
    INTEGER = 2,
    FLOAT = 3,
    DOUBLE = 4,
    STRING = 5
};

enum otpo_traverse_method 
{
    INCREMENT = 'i'
};

enum otpo_ouput_method 
{
    TEXT = 1,
    XML = 2
}output;

/* struct that hold the rpn elements after being evaluated from a string */
struct otpo_rpn_stack_element_t
{
    int is_operator;
    union type_t
    {
        enum otpo_operator operator_type;
        enum otpo_operand operand_type;
    } type;
    union value_t
    {
        int param_index;
        int integer_value;
        float float_value;
        double double_value;
        char string_value[CONDITION_LENGTH];
    } value;
};
typedef struct otpo_rpn_stack_element_t otpo_rpn_stack_element;

/* main struct that holds all the info necessary for a parameter */
struct otpo_param_list_t 
{
    char                           *name; /* name of the parameter */
    char                           *string_values[MAX_VALUES]; /* possible values in strings */
    char                           *default_value; /* default value for the parameter */
    int                            *possible_values; /* possible values for the parameter */
    int                            num_values; /* number of values possible for that parameter */
    int                            start_value; /* start value (in case of range) */
    int                            end_value; /* end value (in case of range) */
    enum otpo_traverse_method      traverse; /* traversal method of the possible attribute values */
    enum otpo_operator             operation; /* operation on how to apply the increment */
    char                           condition[CONDITION_LENGTH]; /* rpn condition that the parameter has to satisfy */
    otpo_rpn_stack_element         rpn_elements[RPN_MAX_ELEMENTS]; /* stack that holds all the rpn elements */
    int                            num_rpn_elements;
    unsigned int                   otpo_flags; /* virtual - aggregate etc. */
    /* The following is a union with one variable, because later, other traversal methods might be added */
    union traverse_attr_t /* attributes to the traversal method */
    {
        int increment;
    }traverse_attr;
}*list_params;

#define OTPO_PARAM_VIRTUAL     0x00000001
#define OTPO_PARAM_AGGREGATE   0x00000002

#define OTPO_PARAM_IS_VIRTUAL(_param) ((_param).otpo_flags&OTPO_PARAM_VIRTUAL)
#define OTPO_PARAM_IS_AGGREGATE(_param) ((_param).otpo_flags&OTPO_PARAM_AGGREGATE)

#define OTPO_PARAM_SET_VIRTUAL(_param) ( (_param).otpo_flags |= OTPO_PARAM_VIRTUAL)
#define OTPO_PARAM_SET_AGGREGATE(_param) ( (_param).otpo_flags |= OTPO_PARAM_AGGREGATE)

#define OTPO_TEST_NETPIPE      0
#define OTPO_TEST_SKAMPI       1
#define OTPO_TEST_NPB          2
#define OTPO_TEST_LATENCY_IO   3
#define OTPO_TEST_NONCONTIG    4
#define OTPO_TEST_MPI_TILE_IO  5

int otpo_initialize_list (char *file_name);
int otpo_free_list_params_objects ();
int otpo_get_num_parameters (char *file_name);
int otpo_populate_attributes (ADCL_Attribute *ADCL_param_attributes);
int otpo_populate_function_set (ADCL_Attrset attrset, int num_functions, 
                                ADCL_Fnctset *fnctset);
int otpo_write_results (ADCL_Request req, int *num_functions);
int otpo_analyze_results (int num_functions); 
int otpo_write_interrupt_data (int num_tested, double *results, 
                               int current_winner, char *backup_file);
int otpo_read_interrupt_data (char *backup_file, int *num_tested, double **results);
int otpo_dump_list (void);
void otpo_test_func (ADCL_Request req);
int otpo_generate_input_file ();
#endif /* __OTPO_H__*/
