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
	$(FC) $(FFLAGS) -o $(ADCL_BIN_DIR)/matmul_userfuncs matmul_userfuncs.f90 $(FLDFLAGS)

clean:
	rm -rf *.o *~ *.out *.mod
	rm -f $(ADCL_BIN_DIR)/matmul_userfuncs
