/* int_sort: sorts integers in decreasing order */
/* input:  int num_int                          */
/*         int int_list                         */
/* output: int int_sort                         */

int int_sort( int num_int, int *int_list )
{
  int i, j;
  int help;

  for( i=num_int-1 ; i>0 ; i-- )
  {
    for( j=0 ; j<i ; j++ )
    {
      if( int_list[j] < int_list[j+1] )
      {
        help = int_list[j];
        int_list[j] = int_list[j+1];
        int_list[j+1] = help;
      }
    }
  }
  return 0;
}
