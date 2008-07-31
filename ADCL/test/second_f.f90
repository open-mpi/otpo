!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
        program second

        implicit none
        include 'ADCL.inc'

        integer rank, size, ierror, NIT, allocstat
        integer dims(3), cdims(2), periods(2)
        integer vec;
        integer topo;
        integer request;
        integer cart_comm;
        double precision , allocatable, dimension (:,:,:) :: data
        integer, parameter :: NC = 1
        integer, parameter :: DIM1=16
        integer, parameter :: DIM2=32
        
        NIT = 100

        dims(1) = DIM1+2
        dims(2) = DIM2+2
        dims(3) = NC
        cdims(1) = 0
        cdims(2) = 0
        periods(1) = 0
        periods(2) = 0
        
        call MPI_Init ( ierror )
        call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror)
        call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )

        allocate ( data (0:DIM1+1, 0:DIM2+1, NC), &
             stat = allocstat ) 
        if ( allocstat.gt.0 ) then 
           write (*,*) rank, ' : Error allocating memory'
           call MPI_Abort ( MPI_COMM_WORLD, 1, ierror )
        end if

        call ADCL_Init ( ierror )
        call ADCL_Vector_register ( 2, dims, 1, ADCL_VECTOR_HALO, 1, MPI_DOUBLE_PRECISION,&
                                    data, vec, ierror)

        call MPI_Dims_create ( size, 2, cdims, ierror)
        call MPI_Cart_create ( MPI_COMM_WORLD, 2, cdims, periods, 0,  &
                               cart_comm, ierror )
        call ADCL_Topology_create ( cart_comm, topo, ierror )

        call ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &
             request, ierror )

        call init_matrix ( dims, data, cdims, cart_comm )
!        do i=0, NIT 
           call ADCL_Request_start( request, ierror )
!        end do

!        call dump_matrix ( dims, data, "After the communication", cart_comm )

        call ADCL_Request_free ( request, ierror )
        call ADCL_Topology_free ( topo, ierror )
        call ADCL_Vector_deregister ( vec, ierror )
        call MPI_Comm_free ( cart_comm, ierror )
    
        deallocate ( data, stat=allocstat)

        call ADCL_Finalize ( ierror )
        call MPI_Finalize ( ierror )
      end program second


      subroutine init_matrix ( dims, data, cdims, comm )

        implicit none
        include 'ADCL.inc'

        integer, dimension(3):: dims
        double precision, dimension (0:dims(1)-1,0:dims(2)-1, dims(3)) :: data
        integer, dimension(2):: cdims
        integer :: comm

        integer :: rank, ierror
        integer :: i,j, offset1, offset2
        integer, dimension(2):: coords
        integer :: DIM1, DIM2

        DIM1 = dims(1)-2
        DIM2 = dims(2)-2

        call MPI_Comm_rank ( comm, rank, ierror )
        call MPI_Cart_coords(comm, rank, 2, coords, ierror)

        do i=0, dims(1)-1
           do j=0, dims(2)-1
              data(i,j,1) = 0.0
           end do
        end do

        offset2 = coords(2) * DIM2
        offset1 = coords(1) * DIM1 *(DIM2 * cdims(2))
        do i=1, dims(1)-2
           do j= 1, dims(2)-2
              data(i,j,1) = (i-1)*DIM2*cdims(2)+offset1+offset2+(j-1)
           end do
        end do
        
      end subroutine init_matrix

      subroutine dump_matrix  ( dims, data, msg, comm )

        implicit none
        include 'ADCL.inc'

        integer, dimension (3) :: dims
        double precision, dimension (0:dims(1)-1,0:dims(2)-1, dims(3)) :: data
        character :: msg*64
        integer :: comm

        integer :: i,j, k, rank, size, ierror

        call MPI_Comm_rank ( comm, rank, ierror )
        call MPI_Comm_size ( comm, size, ierror ) 

        if ( rank .eq. 0 ) then
           write (*,*) msg
        end if

        do k = 0, size-1
           if ( rank .eq. k) then
              write (*,*) rank
              do i=0, dims(1)-1
                 write (*,99) (data (i,j,1), j=0,dims(2)-1)
              end do
              write (*,*)
           end if
           call MPI_Barrier ( comm, ierror )
        end do

99      format (6F7.2)

      end subroutine dump_matrix
