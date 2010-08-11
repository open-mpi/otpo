/*
 * Copyright (c) 2007 Cisco, Inc. All rights reserved.
 * Copyright (c) 2010 University of Houston, Inc. All rights reserved.
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

#include "otpo.h"

struct otpo_list_values_s {
    char *value;
    struct otpo_list_values_s *next;
};
typedef struct otpo_list_values_s otpo_list_values_t;

struct otpo_param_s {
    char *otpo_param_name;
    otpo_list_values_t *otpo_values;
};
typedef struct otpo_param_s otpo_param_t;

static int get_num_parameters (char *file, int *num_params);
static int read_result_file (char *file, 
                             int num_params, 
                             otpo_param_t *params,
                             int *num_values);
static otpo_list_values_t *insert(char *data, otpo_list_values_t *list, int *count);
static void free_list(otpo_list_values_t *list);
static int write_input_file (char *ouput_dir,
                             int num_combined_params,
                             int *num_combined_values,
                             otpo_param_t * combined_params);
int otpo_generate_input_file (char *output_file) {
    int *num_params = NULL;
    int **num_values = NULL;
    otpo_param_t **params = NULL;
    otpo_param_t *combined_params = NULL;
    int *num_combined_values = NULL;
    int i, j, k, num_combined_params;

    num_params = (int *)malloc(sizeof(int) * num_result_files);
    if (NULL == num_params) {
        return NO_MEMORY;
    }

    num_values = (int **)malloc(sizeof(int *) * num_result_files);
    if (NULL == num_values) {
        return NO_MEMORY;
    }

    params = (otpo_param_t **) malloc (sizeof (otpo_param_t *) * 
                                       num_result_files);
    if (NULL == params) {
        return NO_MEMORY;
    }

    for (i=0; i<num_result_files; i++) {
        num_params[i] = 0;
        get_num_parameters (result_files[i],
                            &num_params[i]);
        params[i] = NULL;
        params[i] = (otpo_param_t *) malloc (sizeof (otpo_param_t) *
                                            num_params[i]);
        if (NULL == params[i]) {
            return NO_MEMORY;
        }

        num_values[i] = (int *) malloc (sizeof (int) * num_params[i]);
        if (NULL == num_values[i]) {
            return NO_MEMORY;
        }

        if (SUCCESS != read_result_file (result_files[i], 
                                         num_params[i],
                                         params[i],
                                         num_values[i])) {
            return FAIL;
        }
    }

    if (0 == strcmp ("union", operation)) {
        num_combined_params = 0;

        for (i=0; i<num_result_files; i++) {
            for (j=0; j<num_params[i]; j++) {
                for (k=0 ; k<num_combined_params ; k++) {
                    if (0 == strcmp (params[i][j].otpo_param_name, 
                                     combined_params[k].otpo_param_name)) {
                        break;
                    }
                }
                if (k == num_combined_params) {
                    num_combined_params ++;
                    combined_params = (otpo_param_t *) realloc 
                        (combined_params, 
                         sizeof (otpo_param_t) * num_combined_params);
                    if (NULL == combined_params) {
                        return NO_MEMORY;
                    }
                    combined_params[num_combined_params-1].otpo_param_name = 
                        strdup (params[i][j].otpo_param_name);
                    combined_params[num_combined_params-1].otpo_values = NULL;
                }
            }
        }

        num_combined_values = (int *) malloc (sizeof (int) * num_combined_params);
        if (NULL == num_combined_values) {
            return NO_MEMORY;
        }
        memset (num_combined_values, 0x0, num_combined_params * sizeof (int));

        for (k=0 ; k<num_combined_params ; k++) {
            for (i=0; i<num_result_files; i++) {
                for (j=0; j<num_params[i]; j++) {
                    if (0 == strcmp (params[i][j].otpo_param_name, 
                                     combined_params[k].otpo_param_name)) {
                        otpo_list_values_t *temp = NULL;
                        temp = params[i][j].otpo_values;
                        while (NULL != temp) {
                            combined_params[k].otpo_values = 
                                insert (temp->value, 
                                        combined_params[k].otpo_values, 
                                        &num_combined_values[k]);
                            temp = temp->next;
                        }
                    }
                }
            }
        }
    }

    if (0 == strcmp ("intersection", operation)) {
        int x,y;
        num_combined_params = 0;

        for (i=0; i<num_result_files; i++) {
            for (j=0; j<num_params[i]; j++) {
                for (k=0 ; k<num_combined_params ; k++) {
                    if (0 == strcmp (params[i][j].otpo_param_name, 
                                     combined_params[k].otpo_param_name)) {
                        break;
                    }
                }
                if (k == num_combined_params) {
                    for (x=0; x<num_result_files; x++) {
                        if (x == i) {
                            continue;
                        }
                        for (y=0; y<num_params[x]; y++) {
                            if (0 == strcmp (params[i][j].otpo_param_name,
                                             params[x][j].otpo_param_name)) {
                                break;
                            }
                        }
                        if (y == num_params[x]) {
                            break;
                        }
                    }
                    if (x == num_result_files) {
                        num_combined_params ++;
                        combined_params = (otpo_param_t *) realloc 
                            (combined_params, 
                             sizeof (otpo_param_t) * num_combined_params);
                        if (NULL == combined_params) {
                            return NO_MEMORY;
                        }
                        combined_params[num_combined_params-1].otpo_param_name = 
                            strdup (params[i][j].otpo_param_name);
                        combined_params[num_combined_params-1].otpo_values = NULL;
                    }
                }
            }
        }


        num_combined_values = (int *) malloc (sizeof (int) * num_combined_params);
        if (NULL == num_combined_values) {
            return NO_MEMORY;
        }
        memset (num_combined_values, 0x0, num_combined_params * sizeof (int));

        for (k=0 ; k<num_combined_params ; k++) {
            for (i=0; i<num_result_files; i++) {
                for (j=0; j<num_params[i]; j++) {
                    if (0 == strcmp (params[i][j].otpo_param_name, 
                                     combined_params[k].otpo_param_name)) {
                        otpo_list_values_t *temp1 = NULL;
                        temp1 = params[i][j].otpo_values;
                        while (NULL != temp1) {
                            int temp1_value;
                            int found = 0;
                            temp1_value = atoi (temp1->value);
                            for (x=0; x<num_result_files; x++) {
                                if (x == i) {
                                    continue;
                                }
                                for (y=0; y<num_params[x]; y++) {
                                    if (0 == strcmp (params[i][j].otpo_param_name,
                                                     params[x][y].otpo_param_name)) {
                                        otpo_list_values_t *temp2 = NULL;
                                        temp2 = params[x][y].otpo_values;
                                        while (NULL != temp2) {
                                            int temp2_value;
                                            temp2_value = atoi (temp2->value);
                                            if (temp1_value == temp2_value) {
                                                found = 1;
                                                break;
                                            }
                                            else if (temp1_value < temp2_value) {
                                                break;
                                            }
                                            temp2 = temp2->next;
                                        }
                                        break;
                                    }
                                    
                                }
                                if (found == 0) {
                                    break;
                                }
                            }
                            if (x == num_result_files) {
                                combined_params[k].otpo_values = 
                                    insert (temp1->value, 
                                            combined_params[k].otpo_values, 
                                            &num_combined_values[k]);
                            }
                            temp1 = temp1->next;
                        }
                    }
                }
            }
        }
    }

    if (SUCCESS != write_input_file (output_file,
                                     num_combined_params,
                                     num_combined_values,
                                     combined_params)) {
        return FAIL;
    }

    for (i=0; i<num_result_files; i++) {
        otpo_list_values_t *temp = NULL;
        printf ("FOUND %d parameters in file %s\n", 
                num_params[i], result_files[i]);
        for (j=0 ; j<num_params[i] ; j++) {
            printf ("%s -> %d values\n", params[i][j].otpo_param_name,
                    num_values[i][j]);

            temp = params[i][j].otpo_values;
            while (NULL != temp) {
                printf ("%s\n", temp->value);
                temp = temp->next;
            }
        }
    }

    printf ("FOUND %d Combined parameters\n", num_combined_params);
    for (k=0 ; k<num_combined_params ; k++) {
        otpo_list_values_t *temp = NULL;
        printf ("%s\n", combined_params[k].otpo_param_name);
        
        temp = combined_params[k].otpo_values;
        while (NULL != temp) {
            printf ("%s\n", temp->value);
            temp = temp->next;
        }
    }

    for (i=0; i<num_result_files; i++) {
        for (j=0 ; j<num_params[i] ; j++) {
            if (NULL != params[i][j].otpo_param_name) {
                free (params[i][j].otpo_param_name);
            }
            free_list (params[i][j].otpo_values);
        }
        if (NULL != num_values[i]) {
            free (num_values[i]);
        }
        if (NULL != params[i]) {
            free (params[i]);
        }
    }
    for (k=0 ; k<num_combined_params ; k++) {
        if (NULL != combined_params[k].otpo_param_name) {
            free (combined_params[k].otpo_param_name);
        }
        free_list (combined_params[k].otpo_values);
    }
    if (NULL != params) {
        free (params);
    }
    if (NULL != num_values) {
        free (num_values);
    }
    if (NULL != num_params) {
        free (num_params);
    }
    if (NULL != combined_params) {
        free (combined_params);
    }
    if (NULL != num_combined_values) {
        free (num_combined_values);
    }
}

static int get_num_parameters (char *file, int *num_params) {
    FILE *fp;
    int count = 0;
    char line[LINE_SIZE], *token;

 open1:
    fp = fopen (file, "r");
    if (NULL == fp) { 
        if (EINTR == errno) {
            /* try again */
            goto open1;
        }
        fprintf (stderr,"Unable to open file: %s\n",file);
        return FAIL;
    }

    while (NULL != fgets (line, LINE_SIZE, fp)) {
        if (NULL == line || '\n' == line[0]) {
            continue;
        }
        if ('*' == line[0]) {
            break;
        }
        if ('=' == line[0]) {
            fgets (line, LINE_SIZE, fp);
            continue;
        }
        count ++;
    }
    fclose(fp);

    *num_params = count;
    return SUCCESS;
}

static int read_result_file (char *file, 
                             int num_params, 
                             otpo_param_t *params,
                             int *num_values) {
    FILE *fp;
    char line[LINE_SIZE], *token;
    int i;
    int param_names_set = 0;
    otpo_list_values_t **temp = NULL;
 
 open2:
    fp = fopen (file, "r");
    if (NULL == fp) {
        if (EINTR == errno) {
            /* try again */
            goto open2;
        }
        fprintf (stderr,"Unable to open file: %s\n",file);
        return FAIL;
    }

    while (NULL != fgets (line, LINE_SIZE, fp)) {
        if (NULL == line || '\n' == line[0]) {
            continue;
        }
        if ('=' == line[0] || '*' == line[0]) {
            fgets (line, LINE_SIZE, fp);
            continue;
        }

        if (0 == param_names_set) {
            for (i=0 ; i<num_params ; i++) {
                token = strtok (line," \t\n");
                if (NULL != token) {
                    params[i].otpo_param_name = strdup (token);
                }
                token = strtok (NULL," \t\n");
                params[i].otpo_values = NULL;
                num_values[i] = 0;
                params[i].otpo_values = insert (token , 
                                                params[i].otpo_values,
                                                &num_values[i]);
                fgets (line, LINE_SIZE, fp);
            }
            param_names_set = 1;
        }
        else {
            for (i=0 ; i<num_params ; i++) {
                
                token = strtok (line," \t\n");
                token = strtok (NULL," \t\n");
                params[i].otpo_values = insert (token , 
                                                params[i].otpo_values,
                                                &num_values[i]);
                fgets (line, LINE_SIZE, fp);
            }
        }
    }
    fclose(fp);
    return SUCCESS;
}

static int write_input_file (char *output_file,
                             int num_combined_params,
                             int *num_combined_values,
                             otpo_param_t * combined_params) {
    FILE *fp;
    int k;

 open3:
    fp = fopen (output_file, "wt");
    if (NULL == fp)
    {
        if (EINTR == errno)
        {
            /* try again */
            goto open3;
        }
        fprintf(stderr, "Unable to open file %s\n", output_file);
        return FAIL;
    }

    for (k=0 ; k<num_combined_params ; k++) {
        otpo_list_values_t *temp = NULL;
        if (num_combined_values[k] == 0) {
            continue;
        }
        fprintf (fp,"%s -p ", combined_params[k].otpo_param_name);
        
        temp = combined_params[k].otpo_values;
        while (NULL != temp) {
            fprintf (fp, "%s ", temp->value);
            temp = temp->next;
        }
        fprintf (fp, "\n");
    }
    fclose(fp);
    return SUCCESS;
}

otpo_list_values_t *insert(char *data, otpo_list_values_t *list, int *count) {
    otpo_list_values_t *p;
    otpo_list_values_t *q;

    p = (otpo_list_values_t *)malloc(sizeof(otpo_list_values_t));
    p->value = strdup(data);

    if (list == NULL || atoi(list->value) > atoi(data)) {
        p->next = list;
        *count = *count + 1;
        return p;
    } 
    else {
        q = list;
        if (0 == strcmp (data, q->value)) {
            return list;
        }
        while(NULL != q->next && atoi(q->next->value) <= atoi(data)) {
            if (0 == strcmp (data, q->value)) {
                return list;
            }
            q = q->next;
        }
        if (0 == strcmp (data, q->value)) {
            return list;
        }
        p->next = q->next;
        q->next = p;
        *count = *count + 1;
        return list;
    }
}

static void free_list(otpo_list_values_t *list) {
    otpo_list_values_t *p;

    while(list != NULL) {
        p = list->next;
        free(list);
        list = p;
    }
}
