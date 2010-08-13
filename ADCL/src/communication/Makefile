#
# Copyright (c) 2006-2007      University of Houston. All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
include ../../Makefile.defs

OBJ =	ADCL_change_sb_aao_debug.o           \
	ADCL_change_sb_aao_IsendIrecv.o      \
	ADCL_change_sb_aao_SendIrecv.o       \
	ADCL_change_sb_aao_IsendIrecv_pack.o \
	ADCL_change_sb_pair_debug.o          \
	ADCL_change_sb_pair_IsendIrecv.o     \
	ADCL_change_sb_pair_Send_Recv.o      \
	ADCL_change_sb_pair_SendIrecv.o      \
	ADCL_change_sb_pair_IsendIrecv_pack.o\
	ADCL_change_sb_pair_Send_Recv_pack.o \
	ADCL_change_sb_pair_SendIrecv_pack.o \
	ADCL_change_sb_aao_SendIrecv_pack.o  \
	ADCL_change_sb_pair_Sendrecv_pack.o  \
	ADCL_change_sb_pair_Sendrecv.o       \
	ADCL_change_sb_pair_win_fence_put.o  \
	ADCL_change_sb_pair_win_fence_get.o  \
	ADCL_change_sb_pair_post_start_put.o \
	ADCL_change_sb_pair_post_start_get.o \
        ADCL_change_sb_aao_win_fence_put.o   \
        ADCL_change_sb_aao_win_fence_get.o   \
        ADCL_change_sb_aao_post_start_put.o  \
        ADCL_change_sb_aao_post_start_get.o  \
        ADCL_allgatherv_native.o             \
        ADCL_allgatherv_linear.o             \
        ADCL_allgatherv_bruck.o              \
        ADCL_allgatherv_neighborexchange.o   \
        ADCL_allgatherv_ring.o               \
        ADCL_allgatherv_two_procs.o          \
        ADCL_allgatherv_recursivedoubling.o  \
        ADCL_allreduce_linear.o              \
        ADCL_allreduce_nonoverlapping.o      \
        ADCL_allreduce_ring.o                \
        ADCL_allreduce_native.o              \
        ADCL_allreduce_recursivedoubling.o   \
        ADCL_alltoallv_native.o              \
        ADCL_alltoallv_pairwise.o            \
        ADCL_alltoallv_linear.o              \
        ADCL_alltoall_native.o               \
        ADCL_alltoall_pairwise.o             \
        ADCL_alltoall_pairwise_excl.o        \
        ADCL_alltoall_linear.o               \
        ADCL_alltoall_bruck.o                \
        ADCL_alltoall_linear_sync.o          \
        ADCL_alltoall_ladd_2.o               \
        ADCL_alltoall_ladd_4.o               \
        ADCL_alltoall_ladd_8.o               \
        ADCL_bcast_linear.o                  \
        ADCL_reduce_linear.o                 \
        ADCL_reduce_binary_segsize4k.o                 \
        ADCL_reduce_binomial_segsize4k.o               \
        ADCL_reduce_in_order_binary_segsize4k.o        \
        ADCL_reduce_chain_segsize4k.o                  \
        ADCL_reduce_pipeline_segsize4k.o               \
        ADCL_reduce_native.o                 \
        ADCL_reduce_binary_segsize32k.o                 \
        ADCL_reduce_binomial_segsize32k.o               \
        ADCL_reduce_in_order_binary_segsize32k.o        \
        ADCL_reduce_chain_segsize32k.o                  \
        ADCL_reduce_pipeline_segsize32k.o               \
        ADCL_reduce_binary_segsize128k.o                 \
        ADCL_reduce_binomial_segsize128k.o               \
        ADCL_reduce_in_order_binary_segsize128k.o        \
        ADCL_reduce_chain_segsize128k.o                  \
        ADCL_reduce_pipeline_segsize128k.o               \
        ADCL_reduce_binary_segsize0.o                 \
        ADCL_reduce_binomial_segsize0.o               \
        ADCL_reduce_in_order_binary_segsize0.o        \
        ADCL_reduce_chain_segsize0.o                  \
        ADCL_reduce_pipeline_segsize0.o               \
        ADCL_reduce_generic.o                \
        ADCL_reduce_topo.o


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

ADCL_alltoall_ladd_2.o :$(HEADER) ADCL_alltoall_ladd.c
	$(CC) $(CFLAGS) -c ADCL_alltoall_ladd.c -DCOMMMODE=1 -o $@

ADCL_alltoall_ladd_4.o :$(HEADER) ADCL_alltoall_ladd.c
	$(CC) $(CFLAGS) -c ADCL_alltoall_ladd.c -DCOMMMODE=2 -o $@

ADCL_alltoall_ladd_8.o :$(HEADER) ADCL_alltoall_ladd.c
	$(CC) $(CFLAGS) -c ADCL_alltoall_ladd.c -DCOMMMODE=3 -o $@



ADCL_reduce_binary_segsize4k.o :$(HEADER) ADCL_reduce_binary.c
	$(CC) $(CFLAGS) -c ADCL_reduce_binary.c -DCOMMMODE=1 \
			-o ADCL_reduce_binary_segsize4k.o 
ADCL_reduce_binary_segsize32k.o :$(HEADER) ADCL_reduce_binary.c
	$(CC) $(CFLAGS) -c ADCL_reduce_binary.c -DCOMMMODE=2 \
			-o ADCL_reduce_binary_segsize32k.o 
ADCL_reduce_binary_segsize128k.o :$(HEADER) ADCL_reduce_binary.c
	$(CC) $(CFLAGS) -c ADCL_reduce_binary.c -DCOMMMODE=3 \
			-o ADCL_reduce_binary_segsize128k.o 
ADCL_reduce_binary_segsize0.o :$(HEADER) ADCL_reduce_binary.c
	$(CC) $(CFLAGS) -c ADCL_reduce_binary.c -DCOMMMODE=4 \
			-o ADCL_reduce_binary_segsize0.o 

ADCL_reduce_binomial_segsize4k.o :$(HEADER) ADCL_reduce_binomial.c
	$(CC) $(CFLAGS) -c ADCL_reduce_binomial.c -DCOMMMODE=1 \
			-o ADCL_reduce_binomial_segsize4k.o 
ADCL_reduce_binomial_segsize32k.o :$(HEADER) ADCL_reduce_binomial.c
	$(CC) $(CFLAGS) -c ADCL_reduce_binomial.c -DCOMMMODE=2 \
			-o ADCL_reduce_binomial_segsize32k.o 
ADCL_reduce_binomial_segsize128k.o :$(HEADER) ADCL_reduce_binomial.c
	$(CC) $(CFLAGS) -c ADCL_reduce_binomial.c -DCOMMMODE=3 \
			-o ADCL_reduce_binomial_segsize128k.o 
ADCL_reduce_binomial_segsize0.o :$(HEADER) ADCL_reduce_binomial.c
	$(CC) $(CFLAGS) -c ADCL_reduce_binomial.c -DCOMMMODE=4 \
			-o ADCL_reduce_binomial_segsize0.o 

ADCL_reduce_chain_segsize4k.o :$(HEADER) ADCL_reduce_chain.c
	$(CC) $(CFLAGS) -c ADCL_reduce_chain.c -DCOMMMODE=1 \
			-o ADCL_reduce_chain_segsize4k.o 
ADCL_reduce_chain_segsize32k.o :$(HEADER) ADCL_reduce_chain.c
	$(CC) $(CFLAGS) -c ADCL_reduce_chain.c -DCOMMMODE=2 \
			-o ADCL_reduce_chain_segsize32k.o 
ADCL_reduce_chain_segsize128k.o :$(HEADER) ADCL_reduce_chain.c
	$(CC) $(CFLAGS) -c ADCL_reduce_chain.c -DCOMMMODE=3 \
			-o ADCL_reduce_chain_segsize128k.o 
ADCL_reduce_chain_segsize0.o :$(HEADER) ADCL_reduce_chain.c
	$(CC) $(CFLAGS) -c ADCL_reduce_chain.c -DCOMMMODE=4 \
			-o ADCL_reduce_chain_segsize0.o 

ADCL_reduce_pipeline_segsize4k.o :$(HEADER) ADCL_reduce_pipeline.c
	$(CC) $(CFLAGS) -c ADCL_reduce_pipeline.c -DCOMMMODE=1 \
			-o ADCL_reduce_pipeline_segsize4k.o 
ADCL_reduce_pipeline_segsize32k.o :$(HEADER) ADCL_reduce_pipeline.c
	$(CC) $(CFLAGS) -c ADCL_reduce_pipeline.c -DCOMMMODE=2 \
			-o ADCL_reduce_pipeline_segsize32k.o 
ADCL_reduce_pipeline_segsize128k.o :$(HEADER) ADCL_reduce_pipeline.c
	$(CC) $(CFLAGS) -c ADCL_reduce_pipeline.c -DCOMMMODE=3 \
			-o ADCL_reduce_pipeline_segsize128k.o 
ADCL_reduce_pipeline_segsize0.o :$(HEADER) ADCL_reduce_pipeline.c
	$(CC) $(CFLAGS) -c ADCL_reduce_pipeline.c -DCOMMMODE=4 \
			-o ADCL_reduce_pipeline_segsize0.o 

ADCL_reduce_in_order_binary_segsize4k.o :$(HEADER) ADCL_reduce_in_order_binary.c
	$(CC) $(CFLAGS) -c ADCL_reduce_in_order_binary.c -DCOMMMODE=1 \
			-o ADCL_reduce_in_order_binary_segsize4k.o 
ADCL_reduce_in_order_binary_segsize32k.o :$(HEADER) ADCL_reduce_in_order_binary.c
	$(CC) $(CFLAGS) -c ADCL_reduce_in_order_binary.c -DCOMMMODE=2 \
			-o ADCL_reduce_in_order_binary_segsize32k.o 
ADCL_reduce_in_order_binary_segsize128k.o :$(HEADER) ADCL_reduce_in_order_binary.c
	$(CC) $(CFLAGS) -c ADCL_reduce_in_order_binary.c -DCOMMMODE=3 \
			-o ADCL_reduce_in_order_binary_segsize128k.o 
ADCL_reduce_in_order_binary_segsize0.o :$(HEADER) ADCL_reduce_in_order_binary.c
	$(CC) $(CFLAGS) -c ADCL_reduce_in_order_binary.c -DCOMMMODE=4 \
			-o ADCL_reduce_in_order_binary_segsize0.o 


$(OBJ):	$(HEADER)
.c.o:	$(HEADER)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o *~
