/*
 * Copyright (c) 2007 Cisco, Inc. All rights reserved.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "otpo.h"

struct list_elem
{
    int value;
    struct list_elem *next;
};
typedef struct list_elem list;

static int get_possible_values (int);
static int parse (char *, int);
static int get_num_values (char *);
static int set_rpn_stack_elements (int);
static int set_string_values (char *, int);

int otpo_initialize_list (char *file_name, int num_parameters)
{
    int ret;
    
    ret = parse (file_name , num_parameters);
    if (SUCCESS != ret) 
    {
        return ret;
    }
    ret = get_possible_values (num_parameters);
    if (SUCCESS != ret) 
    {
        return ret;
    }
    ret = set_rpn_stack_elements (num_parameters); 
    if (SUCCESS != ret)
    {
        return ret;
    }

    return ret;
}

int otpo_get_num_parameters (char *file_name) 
{
    FILE *fp;
    char line [LINE_SIZE], *token = NULL;
    int num_parameters;
    
    num_parameters = 0;
    fp = fopen (file_name, "r");
    if (NULL == fp) 
    { 
        fprintf (stderr,"Unable to open file: %s\n",file_name);
        exit (1);
    }
    while (NULL != fgets (line, LINE_SIZE, fp)) 
    { 
        if (NULL == line || '\n' == line[0]) 
        {
            continue;
        }
        token = strtok (line," ");
        if (token[0] == '#') 
        {
            continue;
        }
        num_parameters ++;
    }
    fclose (fp);
    return num_parameters;
}

static int parse (char *file_name, int num_parameters) 
{
    FILE *fp;
    char line [LINE_SIZE], strtmp[LINE_SIZE], cond_name[LINE_SIZE];
    char *token = NULL, *tok = NULL, *values_string = NULL;
    int i, j, val, values[50];

    fp = fopen (file_name, "r");
    if (NULL == fp) 
    { 
        fprintf (stderr,"Unable to open file: %s\n",file_name);
        exit (1);
    }
    
    i=0;    
    while (NULL != fgets (line, LINE_SIZE, fp))
    {
        if (NULL == line || '\n' == line[0])
        {
            continue;
        }
        token = strtok (line," \t");
        if ('#' == token[0]) 
        {
            continue;
        }

        list_params[i].otpo_flags = 0;
        list_params[i].name = strdup (token);
	list_params[i].num_values = 0;
        list_params[i].possible_values = NULL;
        list_params[i].condition[0] = '\0';
        list_params[i].num_rpn_elements = 0;
        list_params[i].default_value = NULL;

        for (j=0 ; j<MAX_VALUES ; j++)
        {
            list_params[i].string_values[j] = NULL;
        }

        token = strtok (NULL," \t\n");
        while(1)
        {
            if (NULL == token) 
            {
                /* 
                   If -p option is specified, then we set the sting values after we finish
                   parsing the current line. The reason is that when we have two nested
                   tokenizers, some badness is going on.
                */
                if (NULL != values_string)
                {
                    printf("values string: %s \n",values_string);
                    set_string_values (values_string, i);
                    free(values_string);
                    values_string = NULL;
                }
                break;
	    }
            /* Virtual Param */
            else if (0 == strcmp ("-v",token)) 
            {
                OTPO_PARAM_SET_VIRTUAL (list_params[i]);
                token = strtok (NULL," \t\n");
            }
            /* Default Value */
	    else if (0 == strcmp ("-d",token)) 
            {
	        token = strtok (NULL, " \t\n");
                list_params[i].default_value = strdup (token);
                token = strtok (NULL," \t\n");
            }
            
            /* Possible Values */
	    else if (0 == strcmp ("-p",token)) 
            {
                /* allocate all possible values as strings if they are specified by the user */
                token = strtok (NULL, "{}");
                printf("token string: %s\n",token);
                values_string = strdup (token); 
                token = strtok (NULL, " \t\n");
            }

            else if (0 == strcmp ("-a",token)) 
            {
                OTPO_PARAM_SET_AGGREGATE (list_params[i]);
                token = strtok (NULL, "\"");
                list_params[i].string_values[0] = strdup (token);
                list_params[i].num_values = 1;
                list_params[i].possible_values = (int *)malloc(sizeof(int) * 1);
                if (NULL == list_params[i].possible_values) 
                {
                    fprintf (stderr,"Malloc Failed...\n");
                    return NO_MEMORY;
                }
                list_params[i].possible_values[0] = 0;
                token = strtok (NULL," \t\n");
            }

	    else if (0 == strcmp ("-r",token)) 
            {
                token = strtok (NULL, " \t");
                list_params[i].start_value = atoi (token); 
		if (token[0] != '0' && list_params[i].start_value == 0) 
                {
                    fprintf (stderr,"Invalid Start Value\n");
                    return INVALID_VALUES;
                }
                token = strtok (NULL, " \t");
                list_params[i].end_value = atoi (token); 
                if (token[0] != '0' && list_params[i].end_value == 0) 
                {
                    fprintf (stderr,"Invalid End Value\n");
                    return INVALID_VALUES;
                }
                token = strtok (NULL," \t\n");
            }
	    
	    else if (0 == strcmp ("-t",token)) 
            {
                token = strtok (NULL, " \t\n"); 
                if (0 == strcmp (token,"increment")) 
                {
                    list_params[i].traverse = INCREMENT;
                    token = strtok (NULL, " \t");
                    switch (token [0])
                        {
                        case '+':
                            list_params[i].operation = ADD;
                            break;
                        case '*':
                            list_params[i].operation = MULTIPLY;
                            break;
                        case '-':
                            list_params[i].operation = SUBTRACT;
                            break;
                        case '/':
                            list_params[i].operation = DIVIDE;
                            break;
                        default:
                            fprintf (stderr,"Invalid operation %s\n",token);
                            return INVALID_OPERATION;
                        }
                    token = strtok (NULL, " \t");
                    list_params[i].traverse_attr.increment = atoi (token);
                    if (token[0] != '0' && list_params[i].traverse_attr.increment == 0) 
                    {
                        fprintf (stderr,"Invalid End Value\n");
                        return INVALID_VALUES;
                    }
                }
                else 
                {
                    fprintf (stderr,"Invalid traversal method: %s\n",token);
                    return INVALID_TRAVERSE;
                }
                token = strtok (NULL," \t\n");
            }

	    else if (0 == strcmp ("-i",token)) 
            {
                token = strtok (NULL, " \t\n");
                if ('\"' != token[0]) 
                {
                    list_params[i].condition[0] = '\0';
                    strcat (list_params[i].condition,token);
                    list_params[i].condition[strlen(token)] = '\0';
                }
                else 
                {
                    for (j=0 ; j<strlen(token)-1 ; j++) 
                    {
                        cond_name[j] = token[j+1];
                    }
                    cond_name[j] = '\0';
                    token = strtok (NULL, "\"");
                    if (CONDITION_LENGTH < (strlen(token) + j + 1))
                    {
                        printf ("condition for %s is too long and will not be evaluated\n",
                                list_params[i].name);
                    }
                    else 
                    {
                        list_params[i].condition[0] = '\0';
                        strncat (list_params[i].condition,cond_name,j);
                        strcat (list_params[i].condition, " ");
                        strcat (list_params[i].condition, token);
                        list_params[i].condition[strlen(token) + j + 1] = '\0';
                    }
                }
                token = strtok (NULL," \t\n");
            }
            else
            {
                fprintf(stderr, "Invalid Option %s in Param file...\n",token);
                exit(1);
            }
        }
        i++;
    }
    fclose (fp);
    return SUCCESS;
}

static int get_num_values (char *str) 
{
    char *tok = NULL;
    int num, tmp;
    
    num = 0;
    tok = strtok (str, " \t");
    while(NULL != tok) 
    {
        tmp = atoi (tok); 
        if (tok[0] != '0' && tmp == 0) 
        {
            break;
        }
        num++;
        tok = strtok (NULL," \t");
    }
    return num;
}

static int get_possible_values (int num_parameters) 
{
    int i, tmp, num, j;
    list *curr, *head;

    for (i=0 ; i<num_parameters ; i++) 
    {
        if (0 != list_params[i].num_values) 
        {
            continue;
        }
        
        num = 0;
        head = NULL;
        curr = (list*)malloc(sizeof (list));
        curr->value = list_params[i].start_value;
        curr->next = head;
        head = curr;
        num ++;
        tmp = list_params[i].start_value;
        while (tmp <= list_params[i].end_value) 
        {
            switch (list_params[i].operation) 
            {
            case '+':
                tmp += list_params[i].traverse_attr.increment;
                break;
            case '-':
                tmp -= list_params[i].traverse_attr.increment;
                break;
            case '*':
                if (0 == tmp) 
                { 
                    tmp = 1;
                    num --;
                    goto out;
                }
                else 
                {
                    tmp *= list_params[i].traverse_attr.increment;
                }
                break;
            case '/':
                if (0 == tmp) 
                {
                    tmp = 1;
                    num --;
                    goto out;
                }
                else 
                {
                    tmp /= list_params[i].traverse_attr.increment;
                }
                break;
            default:
                break;
            }
            if (tmp > list_params[i].end_value)
            {
                continue;
            }
            curr = (list*)malloc (sizeof (list));
            curr->value = tmp;
            curr->next = head;
            head = curr;
        out:
            num ++;
        }
        curr = head;
        
        /* allocating the array of possible arrays in the param struct */
        list_params[i].num_values = num;
        list_params[i].possible_values = (int *)malloc(sizeof(int)*list_params[i].num_values);
        if (NULL == list_params[i].possible_values) 
        {
            printf ("Malloc Failed...\n");
            return NO_MEMORY;
        }

        /* Adding the values in the list to param_list */
        j=num;
        while (curr)
        {
            list_params[i].possible_values[--j] = curr->value;
            curr = curr->next;
        }

        /* Freeing the Linked List */
        curr = head;
        while (curr)
        {
            head = curr;
            curr = curr->next;
            free(head);
        }
    }
    return SUCCESS;
}

static int set_rpn_stack_elements (int num_parameters)
{
    int i, j, count;
    char *token=NULL, *condition_string=NULL;
    
    for (i=0 ; i<num_parameters ; i++)
    {
        if ('\0' == list_params[i].condition[0])
        {
            list_params[i].num_rpn_elements = 0;
            continue;
        }
        count = 0;
        condition_string = strdup (list_params[i].condition);
        token = strtok (condition_string," \t\n");
        
        while (NULL != token)
        {
            list_params[i].rpn_elements[count].is_operator = 1;
            if (strcmp (token,"+") == 0) 
            { 
                list_params[i].rpn_elements[count].type.operator_type = ADD;
                goto set;
            }
            if (strcmp (token,"-") == 0) 
            {
                list_params[i].rpn_elements[count].type.operator_type = SUBTRACT;
                goto set;
            }
            if (strcmp (token,"*") == 0) 
            {
                list_params[i].rpn_elements[count].type.operator_type = MULTIPLY;
                goto set;
            }
            if (strcmp (token,"/") == 0) 
            {
                list_params[i].rpn_elements[count].type.operator_type = DIVIDE;
                goto set;
            }
            if (strcmp (token,"=") == 0) 
            {
                list_params[i].rpn_elements[count].type.operator_type = EQUAL;
                goto set;
            }
            if (strcmp (token,"!=") == 0) 
            {
                list_params[i].rpn_elements[count].type.operator_type = NEQUAL;
                goto set;
            }
            if (strcmp (token,">") == 0) 
            {
                list_params[i].rpn_elements[count].type.operator_type = GREATER;
                goto set;
            }
            if (strcmp (token,"<") == 0) 
            {
                list_params[i].rpn_elements[count].type.operator_type = LESS;
                goto set;
            }
            if (strcmp (token,">=") == 0) 
            {
                list_params[i].rpn_elements[count].type.operator_type = GEQUAL;
                goto set;
            }
            if (strcmp (token,"<=") == 0) 
            {
                list_params[i].rpn_elements[count].type.operator_type = LEQUAL;
                goto set;
            }
            if (strcmp (token,"&&") == 0) 
            {
                list_params[i].rpn_elements[count].type.operator_type = AND;
                goto set;
            }
            if (strcmp (token,"||") == 0) 
            {
                list_params[i].rpn_elements[count].type.operator_type = OR;
                goto set;
            }

            list_params[i].rpn_elements[count].is_operator = 0;
            
            for (j=0 ; j<strlen(token) ; j++)
            {
                if (!isdigit(token[j]))
                {
                    break;
                }
            }
            if (strlen(token) == j)
            {
                list_params[i].rpn_elements[count].is_operator = 0;
                list_params[i].rpn_elements[count].type.operand_type = INTEGER; 
                list_params[i].rpn_elements[count].value.integer_value = atoi (token);
                if (token[0] != '0' && list_params[i].rpn_elements[count].value.integer_value == 0) 
                {
                    fprintf (stderr,"Invalid value for comparison\n");
                    return INVALID_VALUES;
                }
                goto set;
            }
            for (j=0 ; j<num_parameters ; j++) 
            {
                if (strcmp(token,list_params[j].name) == 0) 
                {
                    list_params[i].rpn_elements[count].is_operator = 0;
                    list_params[i].rpn_elements[count].type.operand_type = PARAM; 
                    list_params[i].rpn_elements[count].value.param_index = j;
                    goto set;
                }
            }
            fprintf (stderr, "I can't evaluate the rpn of %s\n", list_params[i].name);
            return FAIL;
        set:
            if (RPN_MAX_ELEMENTS == count)
            {
                fprintf (stderr, 
                         "You have reached the limit of rpn elements (%d)\n", 
                         RPN_MAX_ELEMENTS);
                exit (1);
            }
            count ++;
            token = strtok (NULL," \t\n");
        }
        list_params[i].num_rpn_elements = count;
        if (NULL != condition_string)
        {
            free (condition_string);
        }
    }
    return SUCCESS;
}

static int set_string_values (char *values, int index)
{
    char *tok;
    int j;

    tok = strtok (values, " ,\t\n");
    while (1)
    {
        if (NULL == tok)
        {
            break;
        }
        if (MAX_VALUES == list_params[index].num_values)
        {
            fprintf (stderr,
                     "You have reached the limit of possible string values (%d)\n",
                     MAX_VALUES);
            exit (1);
        }
        list_params[index].string_values[list_params[index].num_values] = 
            strdup(tok);
        list_params[index].num_values ++;
        tok = strtok (NULL," ,\t\n");
    }

    /*
      the int array in list_params will be the same as the strings, if they can 
      be converted into integers, otherwise it will be from 0->num_values 
    */
    list_params[index].possible_values = (int *)malloc(sizeof(int) * 
                                                   list_params[index].num_values);
    if (NULL == list_params[index].possible_values) 
    {
        fprintf (stderr,"Malloc Failed...\n");
        return NO_MEMORY;
    }
    for (j=0 ; j<list_params[index].num_values ; j++) 
    {
        if (list_params[index].string_values[j][0] != '0' && 
            atoi(list_params[index].string_values[j]) == 0) 
        {
            break;
        }
        list_params[index].possible_values[j] = 
            atoi(list_params[index].string_values[j]);
    }
    if (j != list_params[index].num_values)
    {
        for (j=0 ; j<list_params[index].num_values ; j++) 
        {
            list_params[index].possible_values[j] = j;
        }
    }
}
int otpo_free_list_params_objects (int num_parameters) 
{
    int i, j;
    for (i=0 ; i<num_parameters ; i++) 
    { 
        if (NULL != list_params[i].name) 
        {
            free (list_params[i].name);
        }
        if (NULL != list_params[i].default_value)
        {
            free (list_params[i].default_value);
        }
        if (NULL != list_params[i].possible_values) 
        {
            free (list_params[i].possible_values);
        }
        for (j=0 ; j<list_params[i].num_values ; j++)
        {
            if (NULL != list_params[i].string_values[j])
            {
                free (list_params[i].string_values[j]);
            }
        }
    }
    return SUCCESS;
}
