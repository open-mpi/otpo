/* switch_steps: switches the pointers in the       */
/*               time structure so the new solution */
/*               becomes the old one and the other  */
/*               way round                          */
/*    input:  struct tstep *solution                */
/*    output: int switch_steps                      */

#include "parheat.h"

int switch_steps( struct tstep *solution )
{
  double *help;

  help = (*solution).old;
  (*solution).old = (*solution).neu;
  (*solution).neu = help;

  return 0;
}
