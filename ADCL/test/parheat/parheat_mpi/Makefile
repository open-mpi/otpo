include ../../../Makefile.defs

OBJ  = get_dimlist.o read_input.o int_sort.o get_domain.o \
       get_mesh_mem.o get_coords.o write_step.o \
       get_time_mem.o set_initial.o get_bcnode_mem.o \
       find_bcnodes.o apply_bc.o get_offset.o update_interior.o \
       update_faces.o update_solution.o switch_steps.o \
       central_diff.o get_datatypes.o check_done.o parheat.o

HEADER = parheat.h

.SUFFIXES: .o.c

all: $(HEADER) $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o parheat

$(OBJ):	$(HEADER)
.c.o:	$(HEADER)
	$(CC) -I. $(CFLAGS) -c $<

clean:
	rm -rf *.o *~
	rm -ff parheat

