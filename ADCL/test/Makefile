#
# Copyright (c) 2006-2007      University of Houston. All rights reserved.
# Copyright (c) 2009           HLRS. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
include ../Makefile.defs

CEXECS = first second third fourth \
	 one-dim two-dim three-dim \
	 test-fnctset-noattrs test-fnctset-single test-fnctset-single-2 \
	 test-fnctset-allgatherv-1d  test-fnctset-alltoallv-1d \
	 test-fnctset-alltoall test-fnctset-allreduce test-fnctset-reduce \
	 attrs_1 attrs_2 attrs_3 attrs_3a attrs_4 \
	 data_1 twok_1 PbSizes_1D PbSizes_2D PbSizes_3D \
	 test-fnctset-extneigh-2d test-fnctset-extneigh-3d 

FEXECS = first_f second_f \
	 one-dim-f two-dim-f three-dim-f three-dim-large \
	 test-fnctset-noattrs_f test-fnctset-single_f     \
	 test-fnctset-allgatherv_f \
	 test-fnctset-alltoall_f test-fnctset-allreduce_f  \
	 test-fnctset-extneigh-2d_f test-fnctset-extneigh-3d_f \
	 test-prometheus

ADCL_LIB_ = $(ADCL_LIB_DIR)/$(ADCL_LIB)

.SUFFIXES: .c .h .f90 

###################################################################################################

all: ctests ftests

ctests: $(CEXECS)

ftests:	$(FEXECS)


###################################################################################################
### CTESTS                                                                                      ###
###################################################################################################

first: $(ADCL_BIN_DIR)/first
	
$(ADCL_BIN_DIR)/first: first.o
	$(CC) $^ -o $@ $(LDFLAGS)
#
second: $(ADCL_BIN_DIR)/second
	
$(ADCL_BIN_DIR)/second:	second.o 
	$(CC) $^ -o $@ $(LDFLAGS)
#
third: $(ADCL_BIN_DIR)/third
	
$(ADCL_BIN_DIR)/third:	third.o 
	$(CC) $^ -o $@ $(LDFLAGS)
#
fourth: $(ADCL_BIN_DIR)/fourth
	
$(ADCL_BIN_DIR)/fourth:	fourth.o 
	$(CC) $^ -o $@ $(LDFLAGS)

###################################################################################################

one-dim: $(ADCL_BIN_DIR)/one-dim
	
$(ADCL_BIN_DIR)/one-dim: one-dim.o 
	$(CC) $^ -o $@ $(LDFLAGS)
#
two-dim: $(ADCL_BIN_DIR)/two-dim
	
$(ADCL_BIN_DIR)/two-dim: aux-data-2D.o two-dim.o  
	$(CC) $^ -o $@ $(LDFLAGS)
#
three-dim: $(ADCL_BIN_DIR)/three-dim
	
$(ADCL_BIN_DIR)/three-dim: aux-data-3D.o aux-data-4D.o three-dim.o 
	$(CC) $^ -o $@ $(LDFLAGS)

###################################################################################################

test-fnctset-noattrs: $(ADCL_BIN_DIR)/test-fnctset-noattrs
	
$(ADCL_BIN_DIR)/test-fnctset-noattrs: test-fnctset-noattrs.o 
	$(CC) $^ -o $@ $(LDFLAGS)
#
test-fnctset-single: $(ADCL_BIN_DIR)/test-fnctset-single
	
$(ADCL_BIN_DIR)/test-fnctset-single: test-fnctset-single.o 
	$(CC) $^ -o $@ $(LDFLAGS)
#
test-fnctset-single-2: $(ADCL_BIN_DIR)/test-fnctset-single-2
	
$(ADCL_BIN_DIR)/test-fnctset-single-2: test-fnctset-single-2.o
	$(CC) $^ -o $@ $(LDFLAGS)
#
test-fnctset-allgatherv-1d: $(ADCL_BIN_DIR)/test-fnctset-allgatherv-1d
	
$(ADCL_BIN_DIR)/test-fnctset-allgatherv-1d: aux-data-1D.o test-fnctset-allgatherv-1d.o 
	$(CC) $^ -o $@ $(LDFLAGS)
#
test-fnctset-alltoallv-1d: $(ADCL_BIN_DIR)/test-fnctset-alltoallv-1d
	
$(ADCL_BIN_DIR)/test-fnctset-alltoallv-1d: aux-data-1D.o test-fnctset-alltoallv-1d.o 
	$(CC) $^ -o $@ $(LDFLAGS)
#
test-fnctset-alltoall: $(ADCL_BIN_DIR)/test-fnctset-alltoall
	
$(ADCL_BIN_DIR)/test-fnctset-alltoall: aux-data-1D.o test-fnctset-alltoall.o 
	$(CC) $^ -o $@ $(LDFLAGS) 
#
test-fnctset-allreduce: $(ADCL_BIN_DIR)/test-fnctset-allreduce
	
$(ADCL_BIN_DIR)/test-fnctset-allreduce: test-fnctset-allreduce.o
	$(CC) $^ -o $@ $(LDFLAGS)

#
test-fnctset-reduce: $(ADCL_BIN_DIR)/test-fnctset-reduce
	
$(ADCL_BIN_DIR)/test-fnctset-reduce: test-fnctset-reduce.o
	$(CC) $^ -o $@ $(LDFLAGS)
###################################################################################################

attrs_1: $(ADCL_BIN_DIR)/attrs_1
	
$(ADCL_BIN_DIR)/attrs_1: attrs_1.o
	$(CC) $^ -o $@ $(LDFLAGS)
#
attrs_2: $(ADCL_BIN_DIR)/attrs_2
	
$(ADCL_BIN_DIR)/attrs_2: attrs_2.o
	$(CC) $^ -o $@ $(LDFLAGS)
#
attrs_3: $(ADCL_BIN_DIR)/attrs_3
	
$(ADCL_BIN_DIR)/attrs_3: attrs_3.o
	$(CC) $^ -o $@ $(LDFLAGS)
#
attrs_3a: $(ADCL_BIN_DIR)/attrs_3a
	
$(ADCL_BIN_DIR)/attrs_3a: attrs_3a.o
	$(CC) $^ -o $@ $(LDFLAGS)
#
attrs_4: $(ADCL_BIN_DIR)/attrs_4
	
$(ADCL_BIN_DIR)/attrs_4: attrs_4.o
	$(CC) $^ -o $@ $(LDFLAGS)

###################################################################################################

twok_1: $(ADCL_BIN_DIR)/twok_1
	
$(ADCL_BIN_DIR)/twok_1: twok_1.o
	$(CC) $^ -o $@ $(LDFLAGS)

###################################################################################################

data_1: $(ADCL_BIN_DIR)/data_1
	
$(ADCL_BIN_DIR)/data_1: data_1.o
	$(CC) $^ -o $@ $(LDFLAGS)

###################################################################################################

PbSizes_3D: $(ADCL_BIN_DIR)/PbSizes_3D
	
$(ADCL_BIN_DIR)/PbSizes_3D: PbSizes_3D.o
	$(CC) $^ -o $@ $(LDFLAGS)
#
PbSizes_2D: $(ADCL_BIN_DIR)/PbSizes_2D
	
$(ADCL_BIN_DIR)/PbSizes_2D: PbSizes_2D.o
	$(CC) $^ -o $@ $(LDFLAGS)
#
PbSizes_1D: $(ADCL_BIN_DIR)/PbSizes_1D
	
$(ADCL_BIN_DIR)/PbSizes_1D: PbSizes_1D.o
	$(CC) $^ -o $@ $(LDFLAGS)

###################################################################################################

test-fnctset-extneigh-2d: $(ADCL_BIN_DIR)/test-fnctset-extneigh-2d
	
$(ADCL_BIN_DIR)/test-fnctset-extneigh-2d: aux-data-2D.o aux-data-3D.o test-fnctset-extneigh-2d.o 
	$(CC) $^ $(LDFLAGS) -o $@

test-fnctset-extneigh-3d: $(ADCL_BIN_DIR)/test-fnctset-extneigh-3d 
	
$(ADCL_BIN_DIR)/test-fnctset-extneigh-3d: aux-data-3D.o aux-data-4D.o test-fnctset-extneigh-3d.o 
	$(CC) $^ $(LDFLAGS) -o $@

###################################################################################################
### FORTRAN TESTS                                                                               ###
###################################################################################################

first_f: $(ADCL_BIN_DIR)/first_f
	
$(ADCL_BIN_DIR)/first_f: first_f.o
	$(FC) $^ -o $@ $(LDFLAGS)
#
second_f: $(ADCL_BIN_DIR)/second_f
	
$(ADCL_BIN_DIR)/second_f: second_f.o
	$(FC) $^ -o $@ $(LDFLAGS)

###################################################################################################

one-dim-f: $(ADCL_BIN_DIR)/one-dim-f
	
$(ADCL_BIN_DIR)/one-dim-f: one-dim-f.o
	$(FC) $^ -o $@ $(FLDFLAGS)
#
two-dim-f: $(ADCL_BIN_DIR)/two-dim-f
	
$(ADCL_BIN_DIR)/two-dim-f: aux-data-2D-dp-f.o aux-data-3D-dp_f.o two-dim-f.o 
	$(FC) $^ -o $@ $(FLDFLAGS)
#
three-dim-f: $(ADCL_BIN_DIR)/three-dim-f
	
$(ADCL_BIN_DIR)/three-dim-f: aux-data-3D-dp_f.o aux-data-4D-dp_f.o three-dim-f.o
	$(FC) $^ -o $@ $(FLDFLAGS)
#
three-dim-large: $(ADCL_BIN_DIR)/three-dim-large
	
$(ADCL_BIN_DIR)/three-dim-large: three-dim-large.o
	$(FC) $^ -o $@ $(FLDFLAGS)

###################################################################################################

test-fnctset-noattrs_f: $(ADCL_BIN_DIR)/test-fnctset-noattrs_f
	
$(ADCL_BIN_DIR)/test-fnctset-noattrs_f:  test-fnctset-noattrs_f.o
	$(FC) $^ -o $@ $(FLDFLAGS)
#
#
test-fnctset-single_f: $(ADCL_BIN_DIR)/test-fnctset-single_f
	
$(ADCL_BIN_DIR)/test-fnctset-single_f:  test-fnctset-single_f.o
	$(FC) $^ -o $@ $(FLDFLAGS)
#
test-fnctset-allgatherv_f: $(ADCL_BIN_DIR)/test-fnctset-allgatherv_f
	
$(ADCL_BIN_DIR)/test-fnctset-allgatherv_f: aux-data-1D_f.o test-fnctset-allgatherv_f.o 
	$(FC) $^ -o $@ $(FLDFLAGS)
#
test-fnctset-alltoall_f: $(ADCL_BIN_DIR)/test-fnctset-alltoall_f
	 
$(ADCL_BIN_DIR)/test-fnctset-alltoall_f: aux-data-1D_f.o test-fnctset-alltoall_f.o 
	$(FC) $^ -o $@ $(FLDFLAGS)
#
test-fnctset-allreduce_f: $(ADCL_BIN_DIR)/test-fnctset-allreduce_f
	
$(ADCL_BIN_DIR)/test-fnctset-allreduce_f:  test-fnctset-allreduce_f.o
	$(FC) $^ -o $@ $(FLDFLAGS)
#
test-fnctset-extneigh-2d_f: $(ADCL_BIN_DIR)/test-fnctset-extneigh-2d_f 
	
$(ADCL_BIN_DIR)/test-fnctset-extneigh-2d_f: adcl_f.o aux-data-2D-dp-f.o aux-data-3D-dp_f.o test-fnctset-extneigh-2d_f.o 
	$(FC) $^ -o $@ $(FLDFLAGS)
#
test-fnctset-extneigh-3d_f: $(ADCL_BIN_DIR)/test-fnctset-extneigh-3d_f
	
$(ADCL_BIN_DIR)/test-fnctset-extneigh-3d_f: adcl_f.o aux-data-3D-dp_f.o aux-data-4D-dp_f.o test-fnctset-extneigh-3d_f.o  
	$(FC) $^ -o $@ $(FLDFLAGS)
#
test-prometheus: $(ADCL_BIN_DIR)/test-prometheus 
	
$(ADCL_BIN_DIR)/test-prometheus:  test-prometheus.o
	$(FC) $^ -o $@ $(FLDFLAGS)

###################################################################################################
###################################################################################################

aux-data-2D-dp-f.o: adcl_f.o aux-data-2D-f.f90
	$(FC) $(FFLAGS) -I../include -c -DDP -DSET_DATA=set_data_2D_dp -DDUMP_VECTOR=dump_vector_2D_dp \
	      -DDUMP_VECTOR_MPI=dump_vector_2D_mpi_dp \
	      -DCHECK_DATA=check_data_2D_dp -DDATATYPE="doubleprecision" -o aux-data-2D-dp-f.o aux-data-2D-f.f90

aux-data-2D-int-f.o: adcl_f.o aux-data-2D-f.f90
	$(FC) $(FFLAGS) -I../include -c -DINT -DSET_DATA=set_data_2D_int -DDUMP_VECTOR=dump_vector_2D_int \
	      -DDUMP_VECTOR_MPI=dump_vector_2D_mpi_dp \
	      -DCHECK_DATA=check_data_2D_int -DDATATYPE="integer" -o aux-data-2D-int-f.o aux-data-2D-f.f90

aux-data-3D-dp_f.o: adcl_f.o aux-data-3D_f.f90
	$(FC) $(FFLAGS) -I../include -c -DSET_DATA=set_data_3D_dp -DDUMP_VECTOR=dump_vector_3D_dp \
	      -DDUMP_VECTOR_MPI=dump_vector_3D_mpi_dp \
	      -DCHECK_DATA=check_data_3D_dp -DDATATYPE="doubleprecision" -o aux-data-3D-dp_f.o aux-data-3D_f.f90

aux-data-4D-dp_f.o: adcl_f.o aux-data-4D_f.f90
	$(FC) $(FFLAGS) -I../include -c -DSET_DATA=set_data_4D_dp -DDUMP_VECTOR=dump_vector_4D_dp \
	      -DDUMP_VECTOR_MPI=dump_vector_4D_mpi_dp \
	      -DCHECK_DATA=check_data_4D_dp -DDATATYPE="doubleprecision" -o aux-data-4D-dp_f.o aux-data-4D_f.f90 

%.o: %.c $(ADCL_LIB_)
	$(CC) -c $(CFLAGS) $<

%.o: %.f90 $(ADCL_LIB_)
	$(FC) -c $(FFLAGS) -I../include $<

###################################################################################################

clean:
	rm -rf *.o *~ *.out *.mod
	rm -f $(ADCL_BIN_DIR)/*.out

	rm -f $(ADCL_BIN_DIR)/first $(ADCL_BIN_DIR)/second $(ADCL_BIN_DIR)/third $(ADCL_BIN_DIR)/fourth \
	      $(ADCL_BIN_DIR)/one-dim $(ADCL_BIN_DIR)/two-dim $(ADCL_BIN_DIR)/three-dim \
	      $(ADCL_BIN_DIR)/test-fnctset-noattrs $(ADCL_BIN_DIR)/test-fnctset-single $(ADCL_BIN_DIR)/test-fnctset-single-2 \
	      $(ADCL_BIN_DIR)/test-fnctset-allgatherv-1d $(ADCL_BIN_DIR)/test-fnctset-alltoallv-1d \
	      $(ADCL_BIN_DIR)/test-fnctset-alltoall $(ADCL_BIN_DIR)/test-fnctset-allreduce \
              $(ADCL_BIN_DIR)/test-fnctset-extneigh-2d $(ADCL_BIN_DIR)/test-fnctset-extneigh-3d \
	      $(ADCL_BIN_DIR)/attrs_1 $(ADCL_BIN_DIR)/attrs_2 $(ADCL_BIN_DIR)/attrs_3 $(ADCL_BIN_DIR)/attrs_3a $(ADCL_BIN_DIR)/attrs_4 \
	      $(ADCL_BIN_DIR)/data_1 $(ADCL_BIN_DIR)/twok_1 $(ADCL_BIN_DIR)/PbSizes_1D $(ADCL_BIN_DIR)/PbSizes_2D $(ADCL_BIN_DIR)/PbSizes_3D

	rm -f $(ADCL_BIN_DIR)/= $(ADCL_BIN_DIR)/first_f $(ADCL_BIN_DIR)/second_f \
	      $(ADCL_BIN_DIR)/one-dim-f $(ADCL_BIN_DIR)/two-dim-f $(ADCL_BIN_DIR)/three-dim-f 	$(ADCL_BIN_DIR)/three-dim-large \
	      $(ADCL_BIN_DIR)/test-fnctset-noattrs_f $(ADCL_BIN_DIR)/test-fnctset-single_f \
	      $(ADCL_BIN_DIR)/test-fnctset-allgatherv_f \
              $(ADCL_BIN_DIR)/test-fnctset-extneigh-2d_f $(ADCL_BIN_DIR)/test-fnctset-extneigh-3d_f \
	      $(ADCL_BIN_DIR)/test-fnctset-alltoall_f  $(ADCL_BIN_DIR)/test-fnctset-allreduce_f \
	      $(ADCL_BIN_DIR)/test-prometheus


