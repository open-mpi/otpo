#ifndef __ADCL_BCAST__
#define __ADCL_BCAST__

#define ADCL_TAG_BCAST 126

int ADCL_ddt_copy_content_same_ddt(MPI_Datatype dtype, int count, char* dest, char* src);
int ADCL_op_reduce( MPI_Op op, char *source, char *target,
                                  int count, MPI_Datatype dtype); 
#endif
