/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL.h"
#include "ADCL_internal.h"

#ifndef PATH_MAX
#define PATH_MAX 80
#endif

#define MAXLINE 80

extern int ADCL_OUTLIER_FACTOR;
extern int ADCL_OUTLIER_FRACTION;
extern int ADCL_statistic_method;
extern int ADCL_merge_requests;
extern int ADCL_emethod_selection;
extern int ADCL_emethod_numtests;
extern int ADCL_printf_silence;
extern int ADCL_emethod_use_perfhypothesis;

extern int ADCL_method_total_num;
//extern ADCL_method_t * ADCL_method_array;

#define Debug 1

int ADCL_readenv()
{
    int result;
    int len;
    int i;

    char *HomeDir;
    char FilePath[PATH_MAX];
    char buffer[MAXLINE];
    char keyword[MAXLINE];
    char valstring[MAXLINE];
    char *ptr;

    FILE* fp;

    /* New strategy; check first for a configuration
    ** file in the current working directory. If it is not
    ** found there, checkl ${HOME}/.adcl/
    */

    fp = fopen ("config.adcl", "r");
    if ( NULL == fp ) {
    HomeDir = getenv("HOME");
    if (HomeDir == NULL) {
        ADCL_printf("Environment variable HOME not found\n");
        return ADCL_NOT_FOUND;
    }

    memset ( FilePath, 0, PATH_MAX );
    result = snprintf(FilePath, PATH_MAX, "%s/.adcl/config.adcl", HomeDir);

    if (result >= PATH_MAX) {
        ADCL_printf("Warning: path \"%s\" too long. Truncated.\n", FilePath);
    }

    fp = fopen(FilePath, "r");
    if (fp == NULL) {
        ADCL_printf("Could not open %s\n", FilePath);
        return ADCL_NOT_FOUND;
    }

#ifdef Debug
    ADCL_printf("#Outputting contents of %s:\n", FilePath);
#endif
    }


    /*Read lines from configure file*/
    while (fgets(buffer, MAXLINE, fp) != NULL) {

    /* check blank line or # comment */
    if( buffer[0] == '#' ){
        continue;
    }
    len = strlen(buffer);

    /* Parse one single line! */
    i = (int) strcspn (buffer, ":");
    if ( i == len ) {
        continue; /* No colon found */
    }
    strncpy ( keyword, buffer, i);
    ptr = &(buffer[++i]);
#ifdef Debug
    ADCL_printf("#Keyword is %s\n",keyword);
#endif

    /*  ADCL_OUTLIER_FACTOR; */
    if(strncmp(keyword, "ADCL_OUTLIER_FACTOR",
           strlen("ADCL_OUTLIER_FACTOR")) == 0){
        sscanf(ptr, "%d", &ADCL_OUTLIER_FACTOR);
#ifdef  Debug
        ADCL_printf("#ADCL_OUTLIER_FACTOR : %d\n", ADCL_OUTLIER_FACTOR);
#endif
    }

    /*  ADCL_OUTLIER_FRACTION */
    else if(strncmp(keyword, "ADCL_OUTLIER_FRACTION",
            strlen("ADCL_OUTLIER_FRACTION"))==0){
        sscanf(ptr, "%d", &ADCL_OUTLIER_FRACTION);
#ifdef  Debug
        ADCL_printf("#ADCL_OUTLIER_FRACTION : %d\n", ADCL_OUTLIER_FRACTION);
#endif
    }

    /*  ADCL_STATISTIC_METHOD */
    else if ( strncmp(keyword, "ADCL_STATISTIC_METHOD",
              strlen("ADCL_STATISTIC_METHOD"))==0){
        sscanf(ptr,"%s", valstring);
#ifdef  Debug
        ADCL_printf("#ADCL_STATISTIC_METHOD : %s\n", valstring);
#endif
        if ( strncmp(valstring,"ADCL_STATISTIC_MAX",
             strlen("ADCL_STATISTIC_MAX"))==0) {
            ADCL_statistic_method = ADCL_STATISTIC_MAX;
        }
    }

    /*  ADCL_MERGE_REQUESTS  */
    else if ( strncmp(keyword, "ADCL_MERGE_REQUESTS",
              strlen("ADCL_MERGE_REQUESTS"))==0) {
        sscanf(ptr,"%d", &ADCL_merge_requests);
#ifdef  Debug
        ADCL_printf("#ADCL_MERGE_REQUESTS : %d\n", ADCL_merge_requests);
#endif
    }

    /*  ADCL_EMETHOD_NUMTESTS  */
    else if ( strncmp(keyword, "ADCL_EMETHOD_NUMTESTS",
              strlen("ADCL_EMETHOD_NUMTESTS"))==0) {
        sscanf(ptr,"%d", &ADCL_emethod_numtests);
#ifdef  Debug
        ADCL_printf("#ADCL_EMETHOD_NUMTESTS: %d\n",ADCL_emethod_numtests);
#endif
    }

    /*  ADCL_EMETHOD_SELECTION */
    else if (strncmp(keyword,"ADCL_EMETHOD_SELECTION",
             strlen("ADCL_EMETHOD_SELECTION"))==0) {
        sscanf(ptr,"%s", valstring);
#ifdef  Debug
        ADCL_printf("#ADCL_EMETHOD_SELECTION : %s\n", valstring);
#endif
        for ( i=0; i< ADCL_neighborhood_fnctset->fs_maxnum; i++ ) {
        if ( strcmp(valstring, ADCL_neighborhood_fnctset->fs_fptrs[i]->f_name) == 0) {
            ADCL_emethod_selection = i;
            break;
        }
        }
        }

    /*  ADCL_PRINTF_SILENCE  */
    else if ( strncmp(keyword, "ADCL_PRINTF_SILENCE",
              strlen("ADCL_PRINTF_SILENCE"))==0) {
        sscanf(ptr,"%d", &ADCL_printf_silence);
#ifdef  Debug
        ADCL_printf("#ADCL_PRINTF_SILENCE: %d\n",ADCL_printf_silence);
#endif
    }


    /*  ADCL_EMETHOD_USE_PERFHYPOTHESIS  */
    else if ( strncmp(keyword, "ADCL_EMETHOD_USE_PERFHYPOTHESIS",
              strlen("ADCL_EMETHOD_USE_PERFHYPOTHESIS"))==0) {
        sscanf(ptr,"%d ", &ADCL_emethod_use_perfhypothesis);
#ifdef  Debug
        ADCL_printf("#ADCL_EMETHOD_USE_PERFHYPOTHESIS: %d\n",ADCL_emethod_use_perfhypothesis );
#endif
    }


    }

    return ADCL_SUCCESS;
}
