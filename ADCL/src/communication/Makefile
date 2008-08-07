#
# Copyright (c) 2006-2007      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
include ../../Makefile.defs

OBJ =	ADCL_change_sb_aao_debug.o \
	ADCL_change_sb_aao_IsendIrecv.o \
	ADCL_change_sb_aao_SendIrecv.o \
	ADCL_change_sb_aao_IsendIrecv_pack.o \
	ADCL_change_sb_pair_debug.o \
	ADCL_change_sb_pair_IsendIrecv.o \
	ADCL_change_sb_pair_Send_Recv.o \
	ADCL_change_sb_pair_SendIrecv.o \
	ADCL_change_sb_pair_IsendIrecv_pack.o\
	ADCL_change_sb_pair_Send_Recv_pack.o\
	ADCL_change_sb_pair_SendIrecv_pack.o \
	ADCL_change_sb_aao_SendIrecv_pack.o \
	ADCL_change_sb_pair_Sendrecv_pack.o \
	ADCL_change_sb_pair_Sendrecv.o \
	ADCL_change_sb_pair_win_fence_put.o \
	ADCL_change_sb_pair_win_fence_get.o \
	ADCL_change_sb_pair_post_start_put.o \
	ADCL_change_sb_pair_post_start_get.o \
        ADCL_change_sb_aao_win_fence_put.o \
        ADCL_change_sb_aao_win_fence_get.o \
        ADCL_change_sb_aao_post_start_put.o \
        ADCL_change_sb_aao_post_start_get.o


.SUFFIXES: .o.c

all: $(OBJ)
	$(AR) $(AR_FLAGS) $(ADCL_LIB_DIR)/$(ADCL_LIB) $(OBJ)

ADCL_change_sb_aao_debug.o :$(HEADER) ADCL_change_sb_aao.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_aao.c -DCOMMMODE=0 \
			-o ADCL_change_sb_aao_debug.o 

ADCL_change_sb_aao_IsendIrecv.o :$(HEADER) ADCL_change_sb_aao.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_aao.c -DCOMMMODE=1 \
			-o ADCL_change_sb_aao_IsendIrecv.o 

ADCL_change_sb_aao_SendIrecv.o :$(HEADER) ADCL_change_sb_aao.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_aao.c -DCOMMMODE=3 \
			-o ADCL_change_sb_aao_SendIrecv.o 

ADCL_change_sb_aao_IsendIrecv_pack.o :$(HEADER) ADCL_change_sb_aao.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_aao.c -DCOMMMODE=4 \
			-o ADCL_change_sb_aao_IsendIrecv_pack.o 

ADCL_change_sb_aao_SendIrecv_pack.o :$(HEADER) ADCL_change_sb_aao.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_aao.c -DCOMMMODE=6 \
			-o ADCL_change_sb_aao_SendIrecv_pack.o	

ADCL_change_sb_pair_debug.o :$(HEADER) ADCL_change_sb_pair.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_pair.c -DCOMMMODE=0 \
			-o ADCL_change_sb_pair_debug.o 

ADCL_change_sb_pair_IsendIrecv.o :$(HEADER) ADCL_change_sb_pair.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_pair.c -DCOMMMODE=1 \
			-o ADCL_change_sb_pair_IsendIrecv.o 

ADCL_change_sb_pair_Send_Recv.o :$(HEADER) ADCL_change_sb_pair.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_pair.c -DCOMMMODE=2 \
			-o ADCL_change_sb_pair_Send_Recv.o 

ADCL_change_sb_pair_SendIrecv.o :$(HEADER) ADCL_change_sb_pair.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_pair.c -DCOMMMODE=3 \
			-o ADCL_change_sb_pair_SendIrecv.o 

ADCL_change_sb_pair_IsendIrecv_pack.o :$(HEADER) ADCL_change_sb_pair.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_pair.c -DCOMMMODE=4 \
			-o ADCL_change_sb_pair_IsendIrecv_pack.o

ADCL_change_sb_pair_Send_Recv_pack.o :$(HEADER) ADCL_change_sb_pair.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_pair.c -DCOMMMODE=5 \
			-o ADCL_change_sb_pair_Send_Recv_pack.o	

ADCL_change_sb_pair_SendIrecv_pack.o :$(HEADER) ADCL_change_sb_pair.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_pair.c -DCOMMMODE=6 \
			-o ADCL_change_sb_pair_SendIrecv_pack.o	

ADCL_change_sb_pair_Sendrecv_pack.o :$(HEADER) ADCL_change_sb_pair.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_pair.c -DCOMMMODE=7 \
	                -o ADCL_change_sb_pair_Sendrecv_pack.o

ADCL_change_sb_pair_Sendrecv.o :$(HEADER) ADCL_change_sb_pair.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_pair.c -DCOMMMODE=8 \
	                -o ADCL_change_sb_pair_Sendrecv.o

ADCL_change_sb_pair_win_fence_put.o :$(HEADER) ADCL_change_sb_pair.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_pair.c -DCOMMMODE=9 \
		        -o ADCL_change_sb_pair_win_fence_put.o

ADCL_change_sb_pair_win_fence_get.o :$(HEADER) ADCL_change_sb_pair.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_pair.c -DCOMMMODE=10 \
	                -o ADCL_change_sb_pair_win_fence_get.o

ADCL_change_sb_pair_post_start_put.o :$(HEADER) ADCL_change_sb_pair.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_pair.c -DCOMMMODE=11 \
	                -o ADCL_change_sb_pair_post_start_put.o

ADCL_change_sb_pair_post_start_get.o :$(HEADER) ADCL_change_sb_pair.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_pair.c -DCOMMMODE=12 \
			-o ADCL_change_sb_pair_post_start_get.o	

ADCL_change_sb_aao_win_fence_put.o :$(HEADER) ADCL_change_sb_aao.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_aao.c -DCOMMMODE=9 \
			-o ADCL_change_sb_aao_win_fence_put.o

ADCL_change_sb_aao_win_fence_get.o :$(HEADER) ADCL_change_sb_aao.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_aao.c -DCOMMMODE=10 \
			-o ADCL_change_sb_aao_win_fence_get.o

ADCL_change_sb_aao_post_start_put.o :$(HEADER) ADCL_change_sb_aao.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_aao.c -DCOMMMODE=11 \
			-o ADCL_change_sb_aao_post_start_put.o

ADCL_change_sb_aao_post_start_get.o :$(HEADER) ADCL_change_sb_aao.c
	$(CC) $(CFLAGS) -c ADCL_change_sb_aao.c -DCOMMMODE=12 \
			-o ADCL_change_sb_aao_post_start_get.o




$(OBJ):	$(HEADER)
.c.o:	$(HEADER)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o *~
