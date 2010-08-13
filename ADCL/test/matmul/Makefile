#
# Copyright (c) 2006-2007      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
include ../../Makefile.defs

all: matmul_userfuncs

matmul_userfuncs: matmul_userfuncs.f90 
	$(FC) -c $(FFLAGS) matmul_userfuncs.f90 
	$(FC) matmul_userfuncs.o -o matmul_userfuncs $(FLDFLAGS) 

clean:
	rm -rf *.o *~ *.out *.mod
#	rm -f $(ADCL_BIN_DIR)/matmul_userfuncs

	rm -f matmul_userfuncs


