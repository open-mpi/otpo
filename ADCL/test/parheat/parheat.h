#ifndef __PARHEAT_H__
#define __PARHEAD_H__

/* parheat.h: header file for the parheat */
/*            program                     */

#include "mpi.h"

struct point { double x;
               double y;
               double z;
             };

struct tstep { double *start;
               double *old;
               double *neu;
             };

struct timing { double comp;
                double comm_start;
                double recv_end;
                double send_end;
                double sync;
                double total;
              };

int int_sort( int num_int, int *int_list );

int central_diff( double c_fact, double delta_t, double delta_x, \
                  int *grid, int index, double lambda, \
                  struct tstep *solution );

int get_offset( int *grid, int *start, int *end, \
               int *neighbor, int *s_off, int *r_off );

int get_datatypes( int *grid, int *start, int *end, \
                    MPI_Datatype *faces, int msg_fac );

int update_interior( double c_fact, double delta_t, double delta_x, \
            int *grid, int *start, int *end, \
            double lambda, struct tstep *solution );

int update_faces( double c_fact, double delta_t, double delta_x,  \
                  int *grid, int *start, int *end, int *neighbor, \
                  double lambda, struct tstep *solution );

int check_done( double eps, int *flag, int *grid, int *start, \
                int *end, struct tstep *solution, struct point *set );

int switch_steps( struct tstep *solution );

int apply_bc( int num_bnodes, struct point *set, \
              double *step, int *bnode );

int get_domain( int *grid, int *coords, int *dimlist, \
                int ndim, double *min, double *max,   \
                int *ppnode, double *lmin, double *lmax );

int get_dimlist( int num_nodes, int *ndim, int *dim );

int get_coords( int *grid, int *start, int *end, \
                double *min, double *max, struct point *set );

int read_input( int *grid, \
                int *mem_fac, int *msg_fac, int *cpt_fac, \
                double *accuracy, double *tstep_fac, double *c_fact,  \
                double *min, double *max );

int get_mesh_mem( int k, int *grid, struct point ***setlist );

int get_time_mem( int *grid, struct tstep *solution );

int get_bcnode_mem( int num_bnodes, int **mem );

int find_bcnodes( int *grid, int *start, int *end, \
                  int *neighbor, int *bnode, int *num_bcnodes );

int set_initial( int *grid, double *mem );

int update_solution( double c_fact, double delta_t, double delta_x, \
              int *grid, int *start, int *end, int *neighbor, \
              double tstep_fac, double accuracy, int msg_fac, \
              int cpt_fac, int *num_iter, struct point *set, struct timing *data, \
              struct tstep *solution, MPI_Comm newcomm );

int write_step( int *grid, struct point *set, \
                double *step, char *filename );


#endif /* __PARHEAT_H__ */
