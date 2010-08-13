#
# Copyright (c) 2006-2007      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
include Makefile.defs

HEADER = $(ADCL_INC_DIR)/ADCL_allgatherv.h  \
         $(ADCL_INC_DIR)/ADCL_allreduce.h   \
         $(ADCL_INC_DIR)/ADCL_alltoall.h    \
         $(ADCL_INC_DIR)/ADCL_alltoallv.h   \
         $(ADCL_INC_DIR)/ADCL_array.h       \
         $(ADCL_INC_DIR)/ADCL_attribute.h   \
         $(ADCL_INC_DIR)/ADCL_bcast.h       \
	 $(ADCL_INC_DIR)/ADCL_change.h      \
	 $(ADCL_INC_DIR)/ADCL_config.h	    \
	 $(ADCL_INC_DIR)/ADCL_contiguous.h  \
	 $(ADCL_INC_DIR)/ADCL_emethod.h     \
         $(ADCL_INC_DIR)/ADCL_function.h    \
	 $(ADCL_INC_DIR)/ADCL_hist.h        \
	 $(ADCL_INC_DIR)/ADCL_fprototypes.h \
	 $(ADCL_INC_DIR)/ADCL.h             \
	 $(ADCL_INC_DIR)/ADCL_internal.h    \
	 $(ADCL_INC_DIR)/ADCL_list.h        \
	 $(ADCL_INC_DIR)/ADCL_memory.h      \
	 $(ADCL_INC_DIR)/ADCL_method.h      \
	 $(ADCL_INC_DIR)/ADCL_packunpack.h  \
         $(ADCL_INC_DIR)/ADCL_papi.h        \
         $(ADCL_INC_DIR)/ADCL_predefined.h  \
         $(ADCL_INC_DIR)/ADCL_reduce.h      \
	 $(ADCL_INC_DIR)/ADCL_request.h     \
         $(ADCL_INC_DIR)/ADCL_statistics.h  \
	 $(ADCL_INC_DIR)/ADCL_subarray.h    \
         $(ADCL_INC_DIR)/ADCL_sysconfig.h   \
         $(ADCL_INC_DIR)/ADCL_topology.h    \
	 $(ADCL_INC_DIR)/ADCL_vector.h      \
	 $(ADCL_INC_DIR)/ADCL_vmap.h        \
	 /home/mschaara/tmp/otpo/ADCL/include/ADCL_dummy_mpi.h \


all:
	cd src/framework ; make 
	cd src/memory ; make
	cd src/communication ; make 
	cd src/adcl/C ; make all 
	
	cd utils ; make all 
	cd src/dummympi ; make all

clean:
	cd src/framework ; make clean
	cd src/memory ; make clean
	cd src/communication ; make clean
	cd src/adcl/C ; make clean
	
	cd utils ; make clean
	cd src/dummympi ; make clean
	rm -rf *~ ./include/*~ 
	rm -rf ./lib/libadcl.a

install:
	test -z "/home/mschaara/ADCL/lib" || mkdir -p "/home/mschaara/ADCL/lib"	
	install -c -m 644 $(ADCL_LIB_DIR)/libadcl.a  /home/mschaara/ADCL/lib/
	test -z "/home/mschaara/ADCL/include" || mkdir -p "/home/mschaara/ADCL/include"	
	install -c -m 644 $(ADCL_INC_DIR)/ADCL.h  /home/mschaara/ADCL/include/	
	install -c -m 644 $(ADCL_INC_DIR)/ADCL.inc /home/mschaara/ADCL/include/
