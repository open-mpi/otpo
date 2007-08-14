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

SAVE         = testsys.tar

#==================================================================
OBJ =   System_modules.o     \
	System_Main.o        \
	System_Init.o        \
	System_Read_config.o 	\
	System_Matchange.o     \
	System_Matmul.o      \
	System_Matmul.blocking.o    \
	System_Matmul.split.o	\
	System_Precon.o   	\
	System_Set.o         	\
	System_Transmatmul.o    \
	System_Transmatmul.blocking.o    \
	System_Solver.o   	\
	System_Solver.tfqmr.o   	\
	System_Solver.qmr.o   	\
	System_objdebug.o \
	System_Display_Result.o 


all: System

.SUFFIXES: .o .f90

compile: $(OBJ)

$(OBJ): $(HEADER)
.f90.o:   $(HEADER) 
	$(FC) -c $(FFLAGS) $<

$(EXENAME):  $(OBJ) 
	$(FC) $(OBJ)  $(FLDFLAGS) -o $(EXENAME) -lefence


clean:
	rm -f $(OBJ)
	rm -f System
	rm -f *.mod *.out
	rm -f *~  *.*#

