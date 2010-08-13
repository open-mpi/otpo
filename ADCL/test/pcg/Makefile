#Path of MPI-Library on the Machine

MPI_DIR     = ${HARNESS_ROOT}
MPI_INC_DIR = ${HARNESS_ROOT}/include
MPI_LIB_DIR = ${HARNESS_ROOT}/lib/${HARNESS_ARCH}


#Set some Flags
CC          =   ftmpicc
FC          =   ftmpif77
CFLAGS      =   -I$(MPI_INC_DIR) -g -DFTMPI 
LDFLAGS     =   -lm


EXECUTABLES 	= pcg

all: $(EXECUTABLES)

OBJS            = main.o vector.o iohb.o dmio.o dmiolst.o dmiodlst.o \
	matvect.o mpitimer.o



pcg: $(OBJS)

	$(CC) $(CFLAGS) $(OBJS) -o pcg $(LDFLAGS) 
	mv pcg ${HARNESS_ROOT}/bin/${HARNESS_ARCH}

.c.o:
	$(CC) $(CFLAGS) -c $*.c 

clean:
	/bin/rm -f *.o core $(EXECUTABLES) *~ 
	/bin/rm -f ${HARNESS_ROOT}/bin/${HARNESS_ARCH}/pcg













