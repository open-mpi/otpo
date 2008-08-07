/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef __ADCL_CHANGE_H__
#define __ADCL_CHANGE_H__

#include "ADCL.h"


#define TOPO req->r_emethod->em_topo


#if COMMMODE == 0
  #define COMMTEXT  "DebugNoCommunication"

  #define ADCL_CHANGE_SB_AAO  ADCL_change_sb_aao_debug
  #define ADCL_CHANGE_SB_PAIR ADCL_change_sb_pair_debug

  #define PREPARE_COMMUNICATION(req)
  #define STOP_COMMUNICATION(req)

  #define SEND_START(req,i,tag) printf("%d: want to send to %d\n",    \
                TOPO->t_rank, TOPO->t_neighbors[i]);
  #define RECV_START(req,i,tag) printf("%d: want to recv from %d\n",   \
                TOPO->t_rank, TOPO->t_neighbors[i]);
  #define SEND_WAITALL(req)
  #define RECV_WAITALL(req)
  #define SEND_WAIT(req,i)
  #define RECV_WAIT(req,i)

#elif COMMMODE == 1
  #define COMMTEXT  "IsendIrecv"

  #define ADCL_CHANGE_SB_AAO  ADCL_change_sb_aao_IsendIrecv
  #define ADCL_CHANGE_SB_PAIR ADCL_change_sb_pair_IsendIrecv

  #define PREPARE_COMMUNICATION(req)
  #define STOP_COMMUNICATION(req)


  #define SEND_START(req,i,tag) MPI_Isend(req->r_svecs[i]->v_data, 1,\
         req->r_sdats[i], TOPO->t_neighbors[i], tag, TOPO->t_comm,\
         &req->r_sreqs[i])
 #define RECV_START(req,i,tag) MPI_Irecv(req->r_rvecs[i]->v_data, 1,\
         req->r_rdats[i], TOPO->t_neighbors[i], tag, TOPO->t_comm,\
         &req->r_rreqs[i])
  #define SEND_WAITALL(req) MPI_Waitall(2*TOPO->t_ndims, req->r_sreqs, \
         MPI_STATUSES_IGNORE )
  #define RECV_WAITALL(req) MPI_Waitall(2*TOPO->t_ndims, req->r_rreqs, \
         MPI_STATUSES_IGNORE)
  #define SEND_WAIT(req,i) MPI_Wait(&req->r_sreqs[i], MPI_STATUS_IGNORE)
  #define RECV_WAIT(req,i) MPI_Wait(&req->r_rreqs[i], MPI_STATUS_IGNORE)

#elif COMMMODE == 2
  #define COMMTEXT  "Send_Recv"
  #define ADCL_CHANGE_SB_PAIR ADCL_change_sb_pair_Send_Recv

  #define PREPARE_COMMUNICATION(req)
  #define STOP_COMMUNICATION(req)

  #define SEND_START(req,i,tag) MPI_Send(req->r_svecs[i]->v_data, 1, \
                                req->r_sdats[i], TOPO->t_neighbors[i], tag, TOPO->t_comm )
  #define RECV_START(req,i,tag) MPI_Recv(req->r_rvecs[i]->v_data, 1,  \
         req->r_rdats[i], TOPO->t_neighbors[i], tag, TOPO->t_comm, \
         MPI_STATUS_IGNORE )
  #define SEND_WAITALL(req)
  #define RECV_WAITALL(req)
  #define SEND_WAIT(req,i)
  #define RECV_WAIT(req,i)

#elif COMMMODE == 3
  #define COMMTEXT "SendIrecv"
  #define ADCL_CHANGE_SB_AAO  ADCL_change_sb_aao_SendIrecv

  #define PREPARE_COMMUNICATION(req)
  #define STOP_COMMUNICATION(req)

  #define ADCL_CHANGE_SB_PAIR ADCL_change_sb_pair_SendIrecv
  #define SEND_START(req,i,tag) MPI_Send(req->r_svecs[i]->v_data, 1,\
         req->r_sdats[i], TOPO->t_neighbors[i], tag, TOPO->t_comm)
  #define RECV_START(req,i,tag) MPI_Irecv(req->r_rvecs[i]->v_data, 1,\
         req->r_rdats[i], TOPO->t_neighbors[i], tag, TOPO->t_comm,\
         &req->r_rreqs[i])
  #define SEND_WAITALL(req)
  #define RECV_WAITALL(req) MPI_Waitall(2*TOPO->t_ndims, req->r_rreqs, \
         MPI_STATUSES_IGNORE)
  #define SEND_WAIT(req,i)
  #define RECV_WAIT(req,i) MPI_Wait(&req->r_rreqs[i], MPI_STATUS_IGNORE)

#elif COMMMODE == 4
  #define COMMTEXT  "IsendIrecvPack"

  #define ADCL_CHANGE_SB_AAO  ADCL_change_sb_aao_IsendIrecv_pack
  #define ADCL_CHANGE_SB_PAIR ADCL_change_sb_pair_IsendIrecv_pack

  #define PREPARE_COMMUNICATION(req)
  #define STOP_COMMUNICATION(req)

  #define SEND_START(req,i,tag) { int _pos=0;                          \
     MPI_Pack ( req->r_svecs[i]->v_data, 1, req->r_sdats[i],                \
                req->r_sbuf[i], req->r_spsize[i], &_pos, TOPO->t_comm);\
     MPI_Isend(req->r_sbuf[i], _pos, MPI_PACKED, TOPO->t_neighbors[i],  \
           tag, TOPO->t_comm, &req->r_sreqs[i]);}

  #define RECV_START(req,i,tag) MPI_Irecv(req->r_rbuf[i], req->r_rpsize[i],\
         MPI_PACKED, TOPO->t_neighbors[i], tag, TOPO->t_comm,&req->r_rreqs[i])
  #define SEND_WAITALL(req) MPI_Waitall(2*TOPO->t_ndims, req->r_sreqs,    \
         MPI_STATUSES_IGNORE )
  #define RECV_WAITALL(req) { int _i, _pos=0;                            \
     MPI_Waitall(2*TOPO->t_ndims, req->r_rreqs, MPI_STATUSES_IGNORE);  \
     for (_i=0; _i< 2*TOPO->t_ndims; _i++, _pos=0 ) {                  \
         if ( TOPO->t_neighbors[_i] == MPI_PROC_NULL ) continue;          \
         MPI_Unpack(req->r_rbuf[_i], req->r_rpsize[_i], &_pos,           \
              req->r_rvecs[_i]->v_data, 1, req->r_rdats[_i], TOPO->t_comm ); } }

  #define SEND_WAIT(req,i) MPI_Wait(&req->r_sreqs[i], MPI_STATUS_IGNORE)
  #define RECV_WAIT(req,i) { int _pos=0;                         \
     MPI_Wait(&req->r_rreqs[i], MPI_STATUS_IGNORE);              \
     MPI_Unpack(req->r_rbuf[i], req->r_rpsize[i], &_pos,         \
         req->r_rvecs[i]->v_data, 1, req->r_rdats[i], TOPO->t_comm ); }

#elif COMMMODE == 5

  #define COMMTEXT "Send_RecvPack"

  #define ADCL_CHANGE_SB_PAIR ADCL_change_sb_pair_Send_Recv_pack

  #define PREPARE_COMMUNICATION(req)
  #define STOP_COMMUNICATION(req)

   #define SEND_START(req, i, tag) {  int _pos=0;                      \
     MPI_Pack( req->r_svecs[i]->v_data, 1, req->r_sdats[i],                 \
              req->r_sbuf[i], req->r_spsize[i], &_pos, TOPO->t_comm);   \
     MPI_Send( req->r_sbuf[i], _pos, MPI_PACKED,                       \
               TOPO->t_neighbors[i], tag, TOPO->t_comm ); }

  #define RECV_START(req,i,tag) { int _pos =0;                                   \
    MPI_Recv( req->r_rbuf[i], req->r_rpsize[i],                                    \
              MPI_PACKED, TOPO->t_neighbors[i], tag, TOPO->t_comm, MPI_STATUS_IGNORE );   \
    MPI_Unpack( req->r_rbuf[i], req->r_rpsize[i], &_pos,              \
                req->r_rvecs[i]->v_data, 1, req->r_rdats[i], TOPO->t_comm ); }

  #define SEND_WAITALL(req)
  #define RECV_WAITALL(req)
  #define SEND_WAIT(req,i)
  #define RECV_WAIT(req,i)

#elif COMMMODE == 6

  #define COMMTEXT "SendIrecvPack"

  #define ADCL_CHANGE_SB_AAO  ADCL_change_sb_aao_SendIrecv_pack
  #define ADCL_CHANGE_SB_PAIR ADCL_change_sb_pair_SendIrecv_pack

  #define PREPARE_COMMUNICATION(req)
  #define STOP_COMMUNICATION(req)

  #define SEND_START(req, i, tag) {  int _pos=0;                      \
     MPI_Pack( req->r_svecs[i]->v_data, 1, req->r_sdats[i],                 \
              req->r_sbuf[i], req->r_spsize[i], &_pos, TOPO->t_comm);   \
     MPI_Send( req->r_sbuf[i], _pos, MPI_PACKED,                       \
               TOPO->t_neighbors[i], tag, TOPO->t_comm ); }

  #define RECV_START(req,i,tag)  MPI_Irecv(req->r_rbuf[i], \
      req->r_rpsize[i],MPI_PACKED,  TOPO->t_neighbors[i], tag,\
      TOPO->t_comm,&req->r_rreqs[i])

  #define SEND_WAITALL(req)
  #define RECV_WAITALL(req) { int _i, _pos=0;   MPI_Waitall(2*TOPO->t_ndims,\
      req->r_rreqs, MPI_STATUSES_IGNORE);  \
    for (_i=0; _i< 2*TOPO->t_ndims; _i++, _pos=0 ) {                 \
        if ( TOPO->t_neighbors[_i] == MPI_PROC_NULL ) continue;       \
            MPI_Unpack(req->r_rbuf[_i], req->r_rpsize[_i], &_pos,       \
            req->r_rvecs[_i]->v_data, 1, req->r_rdats[_i], TOPO->t_comm );     \
    } \
   }

  #define SEND_WAIT(req,i)
  #define RECV_WAIT(req,i) { int _pos=0;                                \
    MPI_Wait(&req->r_rreqs[i], MPI_STATUS_IGNORE);                      \
    MPI_Unpack(req->r_rbuf[i], req->r_rpsize[i], &_pos,                 \
               req->r_rvecs[i]->v_data, 1, req->r_rdats[i], TOPO->t_comm ); }

#elif COMMMODE == 7

  #define COMMTEXT "SendrecvPack"
  #define ADCL_CHANGE_SB_PAIR ADCL_change_sb_pair_Sendrecv_pack

  #define PREPARE_COMMUNICATION(req)
  #define STOP_COMMUNICATION(req)

  #define SEND_START(req, i, tag) {  int _pos=0;                            \
         MPI_Pack( req->r_svecs[i]->v_data, 1, req->r_sdats[i],                  \
                   req->r_sbuf[i], req->r_spsize[i], &_pos, TOPO->t_comm);   \
         MPI_Sendrecv(req->r_sbuf[i], _pos, MPI_PACKED, TOPO->t_neighbors[i],\
              tag, req->r_rbuf[i], req->r_rpsize[i], MPI_PACKED,    \
                      TOPO->t_neighbors[i], tag, TOPO->t_comm, MPI_STATUS_IGNORE ); \
         _pos=0;                                                            \
         MPI_Unpack( req->r_rbuf[i], req->r_rpsize[i], &_pos,               \
                     req->r_rvecs[i]->v_data, 1, req->r_rdats[i], TOPO->t_comm ); }

  #define RECV_START(req,i,tag)
  #define SEND_WAITALL(req)
  #define RECV_WAITALL(req)
  #define SEND_WAIT(req,i)
  #define RECV_WAIT(req,i)

#elif COMMMODE == 8

  #define COMMTEXT "Sendrecv"

  #define ADCL_CHANGE_SB_PAIR ADCL_change_sb_pair_Sendrecv

  #define PREPARE_COMMUNICATION(req)
  #define STOP_COMMUNICATION(req)

  #define SEND_START(req,i,tag) MPI_Sendrecv(req->r_svecs[i]->v_data, 1,req->r_sdats[i],\
                    TOPO->t_neighbors[i], tag, req->r_rvecs[i]->v_data,1, req->r_rdats[i],\
                    TOPO->t_neighbors[i], tag, TOPO->t_comm, MPI_STATUS_IGNORE )

  #define RECV_START(req,i,tag)
  #define SEND_WAITALL(req)
  #define RECV_WAITALL(req)
  #define SEND_WAIT(req,i)
  #define RECV_WAIT(req,i)

#elif COMMMODE == 9


  #define COMMTEXT  "WinFencePut"

  #define ADCL_CHANGE_SB_PAIR ADCL_change_sb_pair_win_fence_put
  #define ADCL_CHANGE_SB_AAO  ADCL_change_sb_aao_win_fence_put

  #define PREPARE_COMMUNICATION(req) MPI_Win_fence (0, req->r_win);
  #define STOP_COMMUNICATION(req)    MPI_Win_fence (0, req->r_win);

  #define SEND_START( req, i, tag )  MPI_Put ( req->r_svecs[i]->v_data, 1, req->r_sdats[i], \
                                               TOPO->t_neighbors[i], 0, 1, \
                           ( (i%2 == 0) ? req->r_rdats[i+1]:req->r_rdats[i-1]), \
                                               req->r_win)

  #define RECV_START(req,i,tag)
  #define SEND_WAITALL(req)
  #define RECV_WAITALL(req)
  #define SEND_WAIT(req,i)
  #define RECV_WAIT(req,i)


#elif COMMMODE == 10

  #define COMMTEXT  "WinFenceGet"
  #define ADCL_CHANGE_SB_PAIR ADCL_change_sb_pair_win_fence_get
  #define ADCL_CHANGE_SB_AAO  ADCL_change_sb_aao_win_fence_get

  #define PREPARE_COMMUNICATION(req) MPI_Win_fence (0, req->r_win);
  #define STOP_COMMUNICATION(req)    MPI_Win_fence (0, req->r_win);

  #define SEND_START(req,i,tag)

  #define RECV_START( req, i, tag )  MPI_Get ( req->r_rvecs[i]->v_data, 1, req->r_rdats[i], \
                                               TOPO->t_neighbors[i], 0, 1, \
                                               ((i%2 == 0)?req->r_sdats[i+1]:req->r_sdats[i-1]), \
                                               req->r_win)

  #define SEND_WAITALL(req)
  #define RECV_WAITALL(req)
  #define SEND_WAIT(req,i)
  #define RECV_WAIT(req,i)



#elif COMMMODE == 11

  #define COMMTEXT  "PostStartPut"
  #define ADCL_CHANGE_SB_PAIR ADCL_change_sb_pair_post_start_put
  #define ADCL_CHANGE_SB_AAO  ADCL_change_sb_aao_post_start_put

  #define PREPARE_COMMUNICATION(req) {MPI_Win_post(req->r_group, 0, req->r_win);\
                                      MPI_Win_start(req->r_group, 0, req->r_win); }
  #define STOP_COMMUNICATION(req)   {MPI_Win_complete(req->r_win); \
                     MPI_Win_wait(req->r_win);}

  #define SEND_START( req, i, tag )  MPI_Put ( req->r_svecs[i]->v_data, 1, req->r_sdats[i], \
                                               TOPO->t_neighbors[i], 0, 1, \
                           ((i%2 == 0)? req->r_rdats[i+1]:req->r_rdats[i-1]), \
                           req->r_win)

  #define RECV_START(req,i,tag)
  #define SEND_WAITALL(req)
  #define RECV_WAITALL(req)
  #define SEND_WAIT(req,i)
  #define RECV_WAIT(req,i)

#elif COMMMODE == 12

  #define COMMTEXT  "PostStartGet"
  #define ADCL_CHANGE_SB_PAIR ADCL_change_sb_pair_post_start_get
  #define ADCL_CHANGE_SB_AAO  ADCL_change_sb_aao_post_start_get

  #define PREPARE_COMMUNICATION(req) { MPI_Win_post(req->r_group, 0, req->r_win);\
                                       MPI_Win_start(req->r_group, 0, req->r_win); }

  #define STOP_COMMUNICATION(req)    { MPI_Win_complete(req->r_win); \
                                       MPI_Win_wait(req->r_win); }

  #define SEND_START(req,i,tag)

  #define RECV_START( req, i, tag )  MPI_Get( req->r_rvecs[i]->v_data, 1, req->r_rdats[i],  \
                                              TOPO->t_neighbors[i], 0, 1, ((i%2 ==0)?req->r_sdats[i+1]:req->r_sdats[i-1]), req->r_win)
  #define SEND_WAITALL(req)
  #define RECV_WAITALL(req)
  #define SEND_WAIT(req,i)
  #define RECV_WAIT(req,i)

#endif

/*COMM 1*/
void ADCL_change_sb_aao_IsendIrecv  (ADCL_request_t *req );
void ADCL_change_sb_pair_IsendIrecv (ADCL_request_t *req );

/*COMM 2*/
void ADCL_change_sb_pair_Send_Recv (ADCL_request_t *req );


/*COMM 3*/
void ADCL_change_sb_aao_SendIrecv  (ADCL_request_t *req );
void ADCL_change_sb_pair_SendIrecv (ADCL_request_t *req );

/*COMM 4*/
void ADCL_change_sb_aao_IsendIrecv_pack  (ADCL_request_t *req );
void ADCL_change_sb_pair_IsendIrecv_pack (ADCL_request_t *req );

/*COMM 5*/
void ADCL_change_sb_pair_Send_Recv_pack (ADCL_request_t *req );

/*COMM 6*/
void ADCL_change_sb_aao_SendIrecv_pack  (ADCL_request_t *req );
void ADCL_change_sb_pair_SendIrecv_pack (ADCL_request_t *req );

/* COMM 7: Sendrecv-Packed datatype*/

void ADCL_change_sb_pair_Sendrecv_pack (ADCL_request_t *req );

/* COMM 8: Sendrecv-Derived datatype*/
void ADCL_change_sb_pair_Sendrecv(ADCL_request_t *req );

/* COMM 9: Win_fence_put-Derived datatype*/
void ADCL_change_sb_pair_win_fence_put(ADCL_request_t *req );
void ADCL_change_sb_aao_win_fence_put(ADCL_request_t *req );

/* COMM 10:Win_fence_get-Derived datatype */
void ADCL_change_sb_pair_win_fence_get( ADCL_request_t *req );
void ADCL_change_sb_aao_win_fence_get(ADCL_request_t *req );

/*COMM 11: Post_start_put-Derived datatype*/
void ADCL_change_sb_pair_post_start_put( ADCL_request_t *req );
void ADCL_change_sb_aao_post_start_put (ADCL_request_t *req );


/*COMM 12: Post_start_get-Derived datatype*/
void ADCL_change_sb_pair_post_start_get( ADCL_request_t *req );
void ADCL_change_sb_aao_post_start_get( ADCL_request_t *req );

#endif /* __ADCL_CHANGE_H__ */

