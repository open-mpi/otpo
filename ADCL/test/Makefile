#
# Copyright (c) 2006-2007      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
include ../Makefile.defs

all: ctests ftests

ctests: first second third fourth \
	one-dim two-dim three-dim \
	test-fnctset-noattrs test-fnctset-single test-fnctset-single-2 \
	attrs_1 attrs_2 attrs_3 attrs_3a attrs_4 \
	data_1 PbSizes_1D PbSizes_2D PbSizes_3D

ftests: one-dim-f two-dim-f three-dim-f three-dim-large  \
	first_f second_f                                 \
	test-fnctset-noattrs_f test-fnctset-single_f

first: 	first.c
	$(CC) $(CFLAGS) -o $(ADCL_BIN_DIR)/first first.c $(LDFLAGS) 

second:	second.c
	$(CC) $(CFLAGS) -o $(ADCL_BIN_DIR)/second second.c $(LDFLAGS)

third:	third.c
	$(CC) $(CFLAGS) -o $(ADCL_BIN_DIR)/third third.c $(LDFLAGS)

fourth:	fourth.c
	$(CC) $(CFLAGS) -o $(ADCL_BIN_DIR)/fourth fourth.c $(LDFLAGS)

one-dim: one-dim.c
	$(CC) $(CFLAGS) -o $(ADCL_BIN_DIR)/one-dim one-dim.c $(LDFLAGS)

two-dim: two-dim.c
	$(CC) $(CFLAGS) -o $(ADCL_BIN_DIR)/two-dim two-dim.c $(LDFLAGS)

three-dim: three-dim.c
	$(CC) $(CFLAGS) -o $(ADCL_BIN_DIR)/three-dim three-dim.c $(LDFLAGS)

test-fnctset-noattrs: test-fnctset-noattrs.c
	$(CC) $(CFLAGS) -o $(ADCL_BIN_DIR)/test-fnctset-noattrs test-fnctset-noattrs.c \
	$(LDFLAGS)

test-fnctset-single: test-fnctset-single.c
	$(CC) $(CFLAGS) -o $(ADCL_BIN_DIR)/test-fnctset-single test-fnctset-single.c \
	$(LDFLAGS)

test-fnctset-single-2: test-fnctset-single-2.c
	$(CC) $(CFLAGS) -o $(ADCL_BIN_DIR)/test-fnctset-single-2 test-fnctset-single-2.c \
	$(LDFLAGS)

attrs_1: attrs_1.c
	$(CC) $(CFLAGS) -o $(ADCL_BIN_DIR)/attrs_1 attrs_1.c $(LDFLAGS)

attrs_2: attrs_2.c
	$(CC) $(CFLAGS) -o $(ADCL_BIN_DIR)/attrs_2 attrs_2.c $(LDFLAGS)

attrs_3: attrs_3.c
	$(CC) $(CFLAGS) -o $(ADCL_BIN_DIR)/attrs_3 attrs_3.c $(LDFLAGS)

attrs_3a: attrs_3a.c
	$(CC) $(CFLAGS) -o $(ADCL_BIN_DIR)/attrs_3a attrs_3a.c $(LDFLAGS)

attrs_4: attrs_4.c
	$(CC) $(CFLAGS) -o $(ADCL_BIN_DIR)/attrs_4 attrs_4.c $(LDFLAGS)

data_1: data_1.c
	$(CC) $(CFLAGS) -o $(ADCL_BIN_DIR)/data_1 data_1.c $(LDFLAGS)

PbSizes_3D: PbSizes_3D.c
	$(CC) $(CFLAGS) -o $(ADCL_BIN_DIR)/PbSizes_3D PbSizes_3D.c $(LDFLAGS)

PbSizes_2D: PbSizes_2D.c
	$(CC) $(CFLAGS) -o $(ADCL_BIN_DIR)/PbSizes_2D PbSizes_2D.c $(LDFLAGS)

PbSizes_1D: PbSizes_1D.c
	$(CC) $(CFLAGS) -o $(ADCL_BIN_DIR)/PbSizes_1D PbSizes_1D.c $(LDFLAGS)

first_f: first_f.f90
	$(FC) $(FFLAGS) -o $(ADCL_BIN_DIR)/first_f first_f.f90 $(FLDFLAGS)

second_f: second_f.f90
	$(FC) $(FFLAGS) -o $(ADCL_BIN_DIR)/second_f second_f.f90 $(FLDFLAGS)

one-dim-f: one-dim-f.f90
	$(FC) $(FFLAGS) -o $(ADCL_BIN_DIR)/one-dim-f one-dim-f.f90 $(FLDFLAGS)

two-dim-f: two-dim-f.f90
	$(FC) $(FFLAGS) -o $(ADCL_BIN_DIR)/two-dim-f two-dim-f.f90 $(FLDFLAGS)

three-dim-f: three-dim-f.f90
	$(FC) $(FFLAGS) -o $(ADCL_BIN_DIR)/three-dim-f three-dim-f.f90 $(FLDFLAGS)

three-dim-large: three-dim-large.f90
	$(FC) $(FFLAGS) -o $(ADCL_BIN_DIR)/three-dim-large three-dim-large.f90 $(FLDFLAGS)

test-fnctset-noattrs_f:  test-fnctset-noattrs_f.f90 	
	$(FC) $(FFLAGS) -o $(ADCL_BIN_DIR)/test-fnctset-noattrs_f test-fnctset-noattrs_f.f90 $(FLDFLAGS)

test-fnctset-single_f:  test-fnctset-single_f.f90 	
	$(FC) $(FFLAGS) -o $(ADCL_BIN_DIR)/test-fnctset-single_f test-fnctset-single_f.f90 $(FLDFLAGS)

clean:
	rm -rf *.o *~ *.out
	rm -rf $(ADCL_BIN_DIR)/*.out
	rm -f $(ADCL_BIN_DIR)/attrs_1 $(ADCL_BIN_DIR)/attrs_2
	rm -f $(ADCL_BIN_DIR)/attrs_3 $(ADCL_BIN_DIR)/attrs_3a
	rm -f $(ADCL_BIN_DIR)/attrs_4 $(ADCL_BIN_DIR)/data_1
	rm -f $(ADCL_BIN_DIR)/first $(ADCL_BIN_DIR)/first_f
	rm -f $(ADCL_BIN_DIR)/second $(ADCL_BIN_DIR)/second_f
	rm -f $(ADCL_BIN_DIR)/third $(ADCL_BIN_DIR)/fourth
	rm -f $(ADCL_BIN_DIR)/one-dim $(ADCL_BIN_DIR)/one-dim-f
	rm -f $(ADCL_BIN_DIR)/two-dim $(ADCL_BIN_DIR)/two-dim-f
	rm -f $(ADCL_BIN_DIR)/three-dim $(ADCL_BIN_DIR)/three-dim-f
	rm -f $(ADCL_BIN_DIR)/three-dim-large
	rm -f $(ADCL_BIN_DIR)/test-fnctset-noattrs $(ADCL_BIN_DIR)/test-fnctset-noattrs_f
	rm -f $(ADCL_BIN_DIR)/test-fnctset-single $(ADCL_BIN_DIR)/test-fnctset-single_f	
	rm -f $(ADCL_BIN_DIR)/test-fnctset-single-2 $(ADCL_BIN_DIR)/PbSizes_1D
	rm -f $(ADCL_BIN_DIR)/Pbsizes_2D $(ADCL_BIN_DIR)/PbSizes_3D
