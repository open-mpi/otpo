#
# Copyright (c) 2006-2007      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
include ../../Makefile.defs

EXENAME      = System


OBJ =   System_modules.o              \
        System_objdebug.o             \
        System_Display_Result.o       \
        System_Init.o                 \
        System_Main.o                 \
        System_Matchange.o            \
        System_Matmul.blocking.o      \
        System_Matmul.o               \
        System_Matmul.split.o         \
        System_Precon.o               \
        System_Read_config.o          \
        System_Set.o                  \
        System_Solver.o               \
        System_Solver.qmr.o           \
        System_Solver.tfqmr.o         \
        System_Transmatmul.blocking.o \
        System_Transmatmul.o

#        ADCL_request.o
.SUFFIXES: .c .f90

all: System


compile: $(OBJ)

$(EXENAME):  $(OBJ) $(HEADER) $(ADCL_LIB_DIR)/$(ADCL_LIB) 
	$(FC) -o $@ $(OBJ) $(LDFLAGS)

.f90.o: $(HEADER)
	$(FC) -c $(FFLAGS) $<

System_objdebug.o: System_objdebug.c
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f $(OBJ)
	rm -f System
	rm -f *.mod 
	rm -f *~ i.* 

