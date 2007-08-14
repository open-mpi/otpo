!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
  module Matrix
    implicit none
    integer :: Pattern, NIT, nDims, nProSize, nXSize, OneDirection_Row=2, MPI_CART_ERROR = 1
    integer, dimension(2)::Dims
    double precision, dimension(:,:), pointer:: colA, colB, colC, C
  end module Matrix
    
    
  module InitMatrix
    use Matrix
  contains

    subroutine ReadConf()
    use Matrix   
    integer :: anfang, ende, line, i,err,end, punkt
    integer, dimension (7) :: iarraz
    logical :: ex, belegt
    logical, save :: isallocated
    character *80 :: s 
    character*11 :: s_chen

    inquire ( file = 'Values.conf', exist = ex )
    if ( ex ) then
      open ( unit=10, file = 'Values.conf' )
100   continue
      anfang = 0
      ende   = 0
      belegt = .false.

99 format(a)

      read ( 10, 99, err=130, end=120 ) s
      line = line + 1

      if ( s(1:1).eq.'#') then
        goto 100
      end if
    
      punkt = index ( s, ':' ) 

      i = punkt 
110   continue
      
      i = i + 1
      if ( s(i:i).ne.' '.and.( s(i-1:i-1).eq.' '.or. &
           s(i-1:i-1).eq.':')) then 
           anfang = i
      end if

      if ( s(i:i).eq.' '.and.s(i-1:i-1).ne.' '.and.&
           s(i-1:i-1).ne.':' ) then
           ende = i
           belegt = .true.
      end if

      if ( .not.belegt ) goto 110
        read( s(1:punkt), 991 ) s_chen
      
     ! write(*,*) s_chen   

      select case ( s_chen )
        case ( 'MatrixSize:' )
          read ( s(anfang:ende), 992 ) nProSize
         ! write(*,*) nProSize
        case ( 'IterNumber:' )
          read ( s(anfang:ende), 992 ) NIT
         ! write(*,*) NIT
         case ('PatternNO :')
          read( s(anfang:ende), 992) Pattern 

        case default
          write (*,*) 'Unknown keqword in System.config, line :',line
      end select

      goto 100
130 continue
      write (*,*) 'Error reading the configuration file'
120 continue
    close ( 10 )

  else
    write (*,*) 'No File Values.conf in this directory'
  end if
!end if
991 format(a11)
992 format(i5)
    end subroutine ReadConf 
    
    

    subroutine SetMatrix(nProcs, MyRank)
      use Matrix
      implicit none
      
      integer, intent(IN) :: nProcs, MyRank
      integer :: i,j,dimy

      ! Allocate memory to matrix
      if( mod(nProSize,nProcs) .NE. 0) then
         write(*,*) "Error:Process number must be devided by Problem size"
         return
      end if
     
    
      nXSize = nProSize/nProcs
      
      !write(*, *) nXSize
      
      allocate(colA(nProSize, nXSize))
      allocate(colB(nProSize, nXSize))
      allocate(colC(nProSize, nXSize))
      allocate(C(nProSize, nProSize))
      
      do i=1, nProSize, 1
         dimy = mod(MyRank, nProSize)*nXSize
         do j=1, nXSize, 1
            ColC(i,j) = 0.0D0
            ColA(i,j) = (10.0D0*i + dimy+j)/1000.0d0
            ColB(i,j) = (10.0D0*i + dimy+j)/1000.0d0
         end do
      end do
      
    end subroutine SetMatrix
  end module InitMatrix
  
  
  module MMultiply
  contains
    
    subroutine Multiply(C, A, B, Ny, Nx, RankNumber)
      implicit none
      
      integer, intent(IN) :: RankNumber,Ny, Nx
      double precision, dimension(Ny,Nx) :: C, B
      double precision, dimension(:,:),pointer :: A
      integer :: i, j, k
      
      do j=1, Nx, 1
         do k=1, Nx, 1
            do i=1, Ny, 1
               C(i,j)= C(i,j) + A(i,k) * B((RankNumber*Nx+k), j)
            end do
         end do
      end do
    end subroutine Multiply
  end module MMultiply


program FirstTest
    use Matrix
    use InitMatrix
    implicit none
    include 'ADCL.inc'
    
    integer :: front, endl
    integer :: topo, request, fnctset
    integer funcs(3)
    integer :: i, rank, size, NewComm, ierror
    logical :: Periods(1), Reorder(1), EndConf
    
    external PMatmulSynch, PMatmulOverLap, PMatmulBcast
    
    NIT = 200
    
    call MPI_Init ( ierror )
    call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror)
    call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )
    
    call ADCL_Init ( ierror )
    
    call ADCL_Function_create ( PMatmulSynch, ADCL_ATTRSET_NULL, 0, &
         "PMatmulSynch", funcs(1), ierror ) 
    call ADCL_Function_create ( PMatmulOverLap, ADCL_ATTRSET_NULL, 0, &
         "PMatmulOverLap", funcs(2), ierror ) 
    call ADCL_Function_create ( PMatmulBcast, ADCL_ATTRSET_NULL, 0, &
         "PMatmulBcast", funcs(3), ierror ) 
    call ReadConf()

    write(*,*) "ProSie=", nProSize, "iter=", NIT
    call SetMatrix( size, rank )
    
    nDims = 2
    Dims(1) = 1
    Dims(2) = size
    
    Periods(1) = .true.
    Reorder(1) = .false.
    
    call MPI_Cart_create ( MPI_COMM_WORLD, nDims, Dims, Periods, Reorder, NewComm, ierror )
   

    select case ( Pattern )
      case(-1)
        call ADCL_Fnctset_create ( 3, funcs, "trivial fortran funcs", fnctset, &
           ierror)
      case(1)
        call ADCL_Fnctset_create (1, funcs(1), "Synchronize", fnctset, &
           ierror)
      case(2)
        call ADCL_Fnctset_create(1, funcs(2), "Overlap", fnctset, &
           ierror)
      case(3)
        call ADCL_Fnctset_create(1, funcs(3), "Broadcast", fnctset, &
           ierror)    
      case default
        write(*,*) "Unknown pattern number,error! " 
    end select     
    
    call ADCL_Topology_create_generic ( 0, 0, 0, 0, ADCL_DIRECTION_BOTH, &
         NewComm, topo, ierror )
    
    call ADCL_Request_create ( ADCL_VECTOR_NULL, topo, fnctset, request, ierror )


    do i=0, NIT 
       call ADCL_Request_start( request, ierror )
    end do
    
    call ADCL_Request_free ( request, ierror )
    call ADCL_Topology_free ( topo, ierror )
    call ADCL_Fnctset_free ( fnctset, ierror )
    
    do i=1, 3
       call ADCL_Function_free ( funcs(i), ierror )
    end do
    
    call ADCL_Finalize ( ierror )
    call MPI_Finalize ( ierror )
    
  end program FirstTest

 
  subroutine PMatmulSynch ( request ) 
    use Matrix
    use MMultiply
    implicit none
    include 'ADCL.inc'
    
    integer request
    integer comm, tag, i, rank, size, ierror, TopoType
    integer :: direction, disp, right,left
    integer :: rowreqs(2), rowstatus (MPI_STATUS_SIZE, 2)
    
    double precision, dimension(nProSize,nXSize),target :: ColTemp
    double precision, dimension(:,:), pointer:: pColA, pColTemp
    
    
    call ADCL_Request_get_comm ( request, comm, rank, size, ierror )
    ! write (*,*) rank, ": In PMatmulSynch, size = ", size
    
    call MPI_Topo_test (comm, TopoType, ierror)
    
    if(TopoType .ne. MPI_CART) then
       write(*,*) "Error! Communicator topology is not set!"
       ierror = 1
    end if
    
    direction = 1
    disp = 1
    
    call MPI_Cart_shift ( comm, direction, disp, left, right, ierror )
    
    pColA => ColA
    pColTemp => ColTemp
    
    tag = 100
    
    do i=size, 1, -1
       
       call MPI_Irecv ( pColTemp(1,1), nProSize*nXSize, MPI_DOUBLE_PRECISION, left, tag, comm, rowreqs(1), ierror )
       call MPI_ISend ( pColA(1,1), nProSize*nXSize, MPI_DOUBLE_PRECISION, right, tag, comm, rowreqs(2),ierror )
       call MPI_Waitall(2, rowreqs, rowstatus, ierror)
       call Multiply(ColC, pColA, ColB, nProSize, nXSize, mod((i+rank),size))
       
       if ( mod(i,2) .eq. 0 )then 
          pColA => ColTemp
          pColTemp => ColA
       else
          
          pColA => ColA
          pColTemp => ColTemp
       endif
    enddo
    
    pColA => ColA
    pColTemp => ColTemp
    
    call MPI_Irecv ( pColTemp(1,1), nProSize*nXSize, MPI_DOUBLE_PRECISION, left, tag, comm, rowreqs(1), ierror )
    call MPI_ISend ( pColA(1,1), nProSize*nXSize, MPI_DOUBLE_PRECISION, right, tag, comm, rowreqs(2),ierror )
    call MPI_Waitall (2, rowreqs, rowstatus, ierror)
    
    
  end subroutine PMatmulSynch
  
  
  subroutine PMatmulOverLap ( request ) 
    use Matrix
    use MMultiply
    implicit none
    include 'ADCL.inc'
    
    integer :: request, comm, tag, i, rank, size, ierror, TopoType
    integer :: direction, disp, right,left
    integer :: rowreqs(2), rowstatus (MPI_STATUS_SIZE, 2)
    double precision, dimension(nProSize,nXSize),target :: ColTemp
    double precision, dimension(:,:), pointer:: pColA, pColTemp
    
    call ADCL_Request_get_comm ( request, comm, rank, size, ierror )
   ! write (*,*) rank, ": In PMatmulOverLap, size = ", size
    
    call MPI_Topo_test (comm, TopoType, ierror)
    
    call ADCL_Request_get_comm ( request, comm, rank, size, ierror )
    ! write (*,*) rank, ": In PMatmulOverLap, size = ", size
    
    direction = 1
    disp = 1
    
    call MPI_Cart_shift ( comm, direction, disp, left, right, ierror )
    
    pColA => ColA
    pColTemp => ColTemp
    
    tag = 100
    do i=size, 1, -1
       
       call MPI_Irecv ( pColTemp(1,1), nProSize*nXSize, MPI_DOUBLE_PRECISION, left, tag, Comm, rowreqs(1), ierror )
       call MPI_ISend ( pColA(1,1), nProSize*nXSize, MPI_DOUBLE_PRECISION, right, tag, Comm, rowreqs(2),ierror )
       call Multiply(ColC, pColA, ColB, nProSize, nXSize, mod((i+rank),size))
       call MPI_Waitall(2, rowreqs, rowstatus, ierror)
       
       if ( mod(i,2) .eq. 0 )then 
          pColA => ColTemp
          pColTemp => ColA
       else
          pColA => ColA
          pColTemp => ColTemp
       endif
    enddo
    
    pColA => ColA
    pColTemp => ColTemp
    
    call MPI_Irecv ( pColTemp(1,1), nProSize*nXSize, MPI_DOUBLE_PRECISION, left, tag, comm, rowreqs(1), ierror )
    call MPI_ISend ( pColA(1,1), nProSize*nXSize, MPI_DOUBLE_PRECISION, right, tag, comm, rowreqs(2),ierror )
    call MPI_Waitall (2, rowreqs, rowstatus, ierror)
    
    
  end subroutine PMatmulOverLap
  
  subroutine PMatmulBcast ( request ) 
    use Matrix
    use MMultiply
    implicit none
    include 'ADCL.inc'
    
    integer :: request, comm, tag, i, rank, size, ierror, TopoType
    double precision, dimension(nProSize,nXSize),target :: ColTemp
    double precision, dimension(:,:), pointer:: pColA, pColTemp
    
    call ADCL_Request_get_comm ( request, comm, rank, size, ierror )
    ! write (*,*) rank, ": In PMatmulBcast, size = ", size
    
    call MPI_Topo_test (Comm, TopoType, ierror)
    
    if(TopoType .ne. MPI_CART) then
       write(*,*) "Error! Communicator topology is not set!"
       ierror = 1
    end if
    
    !Each process broadcast it's part of A to others.
    do i=size-1, 0, -1
       
       if (rank .eq. i ) then 
          call MPI_Bcast(ColA, nXSize*nProSize, MPI_DOUBLE_PRECISION, i, comm, ierror)
          pColA => ColA
          call Multiply(ColC, pColA, ColB, nProSize, nXSize, i)
       else
          call MPI_Bcast (ColTemp, nXSize*nProSize, MPI_DOUBLE_PRECISION, i, comm, ierror )
          pColA => ColTemp
          call Multiply(ColC, pColA, ColB, nProSize, nXSize, i)
       endif
    enddo
    
    
  end subroutine PMatmulBcast
  
