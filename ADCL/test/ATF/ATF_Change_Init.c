/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ATF.h"

char ComPatternText[100];

int ATF_Change_init(int npattern)
{

  memset(ComPatternText, 0, 100);

  switch(npattern){
  
    case(patt_fcfs):
      
      strcpy(ComPatternText, "Communication pattern: fcfs");
      break;
   
    default:
      
      return ATF_ERROR;
      break;
      
  }
  
  return ATF_SUCCESS;
} 

