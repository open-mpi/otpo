#
# Copyright (c) 2006-2007      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
include ../../Makefile.defs

OBJ =  	ATF_Main.o 		\
	ATF_Read_config.o 	\
	ATF_Init.o 		\
	ATF_Init_matrix.o 	\
	ATF_Memory.o 		\
	ATF_Set_Datatypes.o     \
	ATF_Get.o               \
	ATF_Set.bsp2.o          \
	ATF_Precon.o            \
	ATF_Change_fcfs.o       \
	ATF_Matmul.o            \
	ATF_Solver.o            \
	ATF_Solver_tfqmr.o      \
	ATF_tfqmr.Cal.o         \
	ATF_Display_result.o    \
	ATF_Free_matrix.o       \

HEADER = ATF.h ATF_Adcl_global.h 

.SUFFIXES: .o.c

all: $(HEADER) $(OBJ)
	    $(CC) $(OBJ) $(LDFLAGS) -o ATF

$(OBJ):	$(HEADER)
.c.o:	$(HEADER)
	$(CC) -I. $(CFLAGS) -c $<

clean:
	rm -rf *.o *~
	rm -f ATF
	rm -rf $(ATF_LIB)
