!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
      subroutine System_Matchange_init ( ierror )
        
      USE globale_daten
      USE trans_mat

      implicit none
      include 'mpif.h'

      integer :: ierror, status
      integer :: rank, size

      call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )
      call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror )

      allocate (rmb00temp ( 1, dmju:dmjo, dmku:dmko, 1, 1 ), &
           rmf00temp ( 1, dmju:dmjo, dmku:dmko, 1, 1 ), &
           rm0b0temp ( dmiu:dmio, 1, dmku:dmko, 1, 1 ), &
           rm0f0temp ( dmiu:dmio, 1, dmku:dmko, 1, 1 ), &
           rm00btemp ( dmiu:dmio, dmju:dmjo, 1, 1, 1 ), &
           rm00ftemp ( dmiu:dmio, dmju:dmjo, 1, 1, 1 ), stat=status )
      if (status.gt.0 ) then 
         write (*,*) rank, ' : Error allocating memory'
      end if        

      allocate ( sendbuf1((n2+2)*(n3+2)*nc*nc), sendbuf2((n2+2)*(n3+2)*nc*nc), &
           recvbuf1((n2*2)*(n3+2)*nc*nc), recvbuf2((n2*2)*(n3+2)*nc*nc),       &
           sendbuf3((n1+2)*(n3+2)*nc*nc), sendbuf4((n1+2)*(n3+2)*nc*nc),       & 
           recvbuf3((n1+2)*(n3+2)*nc*nc), recvbuf4((n1+2)*(n3+2)*nc*nc),       &
           sendbuf5((n1+2)*(n2+2)*nc*nc), sendbuf6((n1+2)*(n2+2)*nc*nc),       &
           recvbuf5((n1+2)*(n2+2)*nc*nc), recvbuf6((n1+2)*(n2+2)*nc*nc),       &
           stat = status)
      if (status.gt.0 ) then 
         write (*,*) rank, ' : Error allocating memory'
      end if        
        

      ierror = 0

      end subroutine System_Matchange_init
!******************************************************************************
!******************************************************************************
!******************************************************************************
!******************************************************************************
      subroutine System_Matchange( ierror )

!------------------------------------------------------------------------------
!
!     Unterprogramm fuer den Randaustausch der Matrix an allen 6 Raendern
!
!------------------------------------------------------------------------------
 
      USE globale_daten
      USE matrix
      USE trans_mat

      implicit none

      include 'mpif.h'

!...lokale Variablen

      integer :: handnum, ierror, info
      integer :: i, j, k, n, l, ni, position
      integer, dimension ( 6) :: msendhandle, mrecvhandle
      integer :: size2, size4, size6

      integer, dimension (MPI_STATUS_SIZE,6) :: sendstatusfeld
      integer, dimension (MPI_STATUS_SIZE) :: status
      integer :: rank, size

      call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )
      call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror )

      size2 = (n2+2)*(n3+2)*nc*nc * SIZE_OF_REALx
      size4 = (n1+2)*(n3+2)*nc*nc * SIZE_OF_REALx
      size6 = (n1+2)*(n2+2)*nc*nc * SIZE_OF_REALx

!- - - - - - - - - - - - - - - - - - - -  - - - - - - - - - - - - - - - - - -
!...Asynchrones Empfangen der Matrizenteile der Nachbarprozessoren

      if (.not. rand_ab) then
         call MPI_IRECV(recvbuf1, size2, MPI_PACKED, tid_io, 5000,  &
             MPI_COMM_WORLD, mrecvhandle(1), info)
      else
         mrecvhandle(1) = MPI_REQUEST_NULL
      end if

      if (.not. rand_sing) then
         call MPI_IRECV(recvbuf2, size2, MPI_PACKED, tid_iu, 5001,  &
             MPI_COMM_WORLD, mrecvhandle(2), info)
      else
         mrecvhandle(2) = MPI_REQUEST_NULL
      end if

      if (.not. rand_zu) then
         call MPI_IRECV(recvbuf3, size4, MPI_PACKED, tid_jo, 5002, &
             MPI_COMM_WORLD, mrecvhandle(3), info)
      else
         mrecvhandle(3) = MPI_REQUEST_NULL
      end if

      if (.not. rand_festk) then
         call MPI_IRECV(recvbuf4, size4, MPI_PACKED, tid_ju, 5003,  &
             MPI_COMM_WORLD, mrecvhandle(4), info)
      else
         mrecvhandle(4) = MPI_REQUEST_NULL
      end if
         
      if (.not. rand_symo) then
         call MPI_IRECV(recvbuf5, size6, MPI_PACKED, tid_ko, 5004,  &
             MPI_COMM_WORLD, mrecvhandle(5), info)
      else
         mrecvhandle(5) = MPI_REQUEST_NULL
      end if

      if (.not. rand_symu) then
         call MPI_IRECV(recvbuf6, size6, MPI_PACKED, tid_ku, 5005,  &
             MPI_COMM_WORLD, mrecvhandle(6), info)
      else
         mrecvhandle(6) = MPI_REQUEST_NULL
      end if

!- - - - - - - - - - - - - - - - - - - -  - - - - - - - - - - - - - - - - - -
!...Asynchrones Senden der Matrizenteile an die Nachbarprozessoren

      if (.not. rand_sing) then
         position = 0
         do l = 1, nc
            do n = 1, nc
               do k = dmku, dmko
                  do j = dmju, dmjo
                     call MPI_PACK(rmb00(1,j,k,n,l), 1, MPI_DOUBLE_PRECISION, sendbuf1,   &
                         size2, position, MPI_COMM_WORLD, info)
                  end do
               end do
            end do
         end do
         call MPI_ISEND(sendbuf1, position, MPI_PACKED, tid_iu, 5000, &
                       MPI_COMM_WORLD, msendhandle(1), info)
      else
         msendhandle(1) = MPI_REQUEST_NULL
      end if

      if (.not. rand_ab) then
         position = 0
         do l = 1, nc
            do n = 1, nc
               do  k = dmku, dmko
                  do  j = dmju, dmjo
                     call MPI_PACK(rmf00(dmio,j,k,n, l), 1, MPI_DOUBLE_PRECISION, sendbuf2,  &
                         size2, position,  MPI_COMM_WORLD, info)
                  end do
               end do
            end do
         end do
        
      call MPI_ISEND(sendbuf2, position, MPI_PACKED, tid_io, 5001,  &
                    MPI_COMM_WORLD, msendhandle(2), info)
      else
         msendhandle(2) = MPI_REQUEST_NULL
      end if

      if (.not. rand_festk) then
         position = 0
         do l = 1, nc
            do  n = 1, nc
               do i = dmiu, dmio
                  do k = dmku, dmko
                     call MPI_PACK(rm0b0(i,1,k,n,l), 1, MPI_DOUBLE_PRECISION, sendbuf3,   &
                          size4, position,MPI_COMM_WORLD, info)
                  end do
               end do
            end do
         end do
         
         call MPI_ISEND(sendbuf3, position, MPI_PACKED, tid_ju, 5002, &
                       MPI_COMM_WORLD, msendhandle(3), info)
      else
         msendhandle(3) = MPI_REQUEST_NULL
      end if

      if (.not. rand_zu) then
         position = 0
         do l = 1, nc
            do n = 1, nc
               do i = dmiu, dmio
                  do k = dmku, dmko
                     call MPI_PACK(rm0f0(i,dmjo,k,n,l), 1, MPI_DOUBLE_PRECISION, sendbuf4,  &
                          size4, position, MPI_COMM_WORLD, info)
                  end do
               end do
            end do
         end do
         
         call MPI_ISEND(sendbuf4, position, MPI_PACKED, tid_jo, 5003, &
                       MPI_COMM_WORLD, msendhandle(4), info)
      else
         msendhandle(4) = MPI_REQUEST_NULL
      end if

      
      if (.not. rand_symu) then
        position = 0
         do l = 1, nc
            do  n = 1, nc
               do i = dmiu, dmio
                  do j = dmju, dmjo
                     call MPI_PACK(rm00b(i,j,1,n,l), 1, MPI_DOUBLE_PRECISION, sendbuf5,  &
                          size6, position, MPI_COMM_WORLD, info)
                  end do
               end do
            end do
         end do
         
         call MPI_ISEND(sendbuf5, position, MPI_PACKED, tid_ku, 5004, &
                       MPI_COMM_WORLD, msendhandle(5), info)
      else
         msendhandle(5) = MPI_REQUEST_NULL
      end if
      

      if (.not. rand_symo) then
         position = 0
         do l = 1, nc
            do n = 1, nc
               do i = dmiu, dmio
                  do j = dmju, dmjo
                     call MPI_PACK(rm00f(i,j,dmko,n,l), 1, MPI_DOUBLE_PRECISION, sendbuf6,  &
                          size6, position, MPI_COMM_WORLD, info)
                  end do
               end do
            end do
         end do
         
         call MPI_ISEND(sendbuf6, position, MPI_PACKED, tid_ko, 5005, &
                       MPI_COMM_WORLD, msendhandle(6), info)
      else
         msendhandle(6) = MPI_REQUEST_NULL
      end if
 
!- - - - - - - - - - - - - - - - - - - -  - - - - - - - - - - - - - - - - - -      
!...Auspacken der empfangenen Nachrichten
      
      do ni = 1, 6
         
         call MPI_WAITANY(6, mrecvhandle, handnum, status, info)
         
         position = 0

         select case (handnum)

         case (1)
            do l = 1, nc
               do n = 1, nc
                  do k = dmku, dmko
                     do j = dmju, dmjo
                        call MPI_UNPACK(recvbuf1, size2, position,  &
                            rmb00temp(1,j,k,n,l),1, MPI_DOUBLE_PRECISION,  &
                            MPI_COMM_WORLD, info)
                     end do
                  end do
               end do
            end do

         case (2)
            do l = 1, nc
               do n = 1, nc
                  do k = dmku, dmko
                     do j = dmju, dmjo
                        call MPI_UNPACK(recvbuf2, size2, position,  &
                            rmf00temp(1,j,k,n,l), 1, MPI_DOUBLE_PRECISION,  &
                            MPI_COMM_WORLD, info)
                     end do
                  end do
               end do
            end do

         case (3)
            do l = 1, nc
               do n = 1, nc
                  do i = dmiu, dmio
                     do k = dmku, dmko
                        call MPI_UNPACK(recvbuf3, size4, position,  &
                            rm0b0temp(i,1,k,n,l), 1, MPI_DOUBLE_PRECISION, &
                            MPI_COMM_WORLD, info)
                     end do
                  end do
               end do
            end do

         case (4)
            do l = 1, nc
               do n = 1, nc
                  do i = dmiu, dmio
                     do k = dmku, dmko
                        call MPI_UNPACK(recvbuf4, size4, position,  &
                            rm0f0temp(i,1,k,n,l), 1, MPI_DOUBLE_PRECISION, &
                            MPI_COMM_WORLD, info)
                     end do
                  end do
               end do
            end do

         case (5)
            do l = 1, nc
               do n = 1, nc
                  do i = dmiu, dmio
                     do j = dmju, dmjo
                        call MPI_UNPACK(recvbuf5, size6, position,  &
                            rm00btemp(i,j,1,n,l), 1, MPI_DOUBLE_PRECISION, &
                            MPI_COMM_WORLD, info)
                     end do
                  end do
               end do
            end do

         case (6)
            do l = 1, nc
               do n = 1, nc
                  do i = dmiu, dmio
                     do j = dmju, dmjo
                        call MPI_UNPACK(recvbuf6, size6, position,  &
                            rm00ftemp(i,j,1,n,l), 1, MPI_DOUBLE_PRECISION, &
                            MPI_COMM_WORLD, info)
                     end do
                  end do
               end do
            end do

         case (MPI_UNDEFINED)
            goto 400

         end select

      end do

 400  continue
      
!- - - - - - - - - - - - - - - - - - - -  - - - - - - - - - - - - - - - - - -
!...Kommt vorerst mal hierher, bei der Optimierung kann man sich dann 
!   ueberlegen, wie man es am besten macht.

      call MPI_WAITALL(6, msendhandle, sendstatusfeld, info)

      ierror = 0

      return
      end
