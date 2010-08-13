!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! Copyright (c) 2009-2010       HLRS. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
! unit test for 3D Fortran extended neighborhood communication (for Lattice Boltzmann)
!
#undef VERBOSE

program testfnctsetneigh3df
    use adcl
    use auxdata3df
    use auxdata4df
    implicit none
 
    integer rank, size, ierror 
    integer nc, hwidth
    integer vmap, vec, topo, request
    integer cart_comm
    integer, parameter :: ndim = 3
    integer, dimension(ndim) :: dims1, cdims
    logical, dimension(ndim) :: periods
    integer, dimension(ndim+1) :: dims2
    integer, parameter :: nneigh=9
    integer, dimension(nneigh) :: lneighbors, rneighbors, flip
    integer, dimension(2*nneigh) :: neighbors
    double precision, allocatable :: data1(:,:,:), data2(:,:,:,:)
    integer, parameter :: niter = 50
    logical :: isok
    integer :: i, itest, ntests_3D, ntests_3D_plus_nc
    
    ! integer, parameter :: DIM0=4, DIM1=5, DIM2=6
    integer, parameter :: DIM0=4, DIM1=3, DIM2=2

    logical :: check_data_3D, check_data_4D

    cdims = 0
    periods = .false.

    ! Initiate the MPI environment
    call MPI_Init ( ierror )
    call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror)
    call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )

    ! Describe the neighborhood relations
    call MPI_Dims_create ( size, 3, cdims, ierror)
    call MPI_Cart_create ( MPI_COMM_WORLD, 3, cdims, periods, 0,  &
            cart_comm, ierror )
    call MPI_Cart_shift ( cart_comm, 0, 1, neighbors(1), neighbors(2), ierror )
    call MPI_Cart_shift ( cart_comm, 1, 1, neighbors(3), neighbors(4), ierror )
    call MPI_Cart_shift ( cart_comm, 2, 1, neighbors(5), neighbors(6), ierror )

    ! Initiate the ADCL library and register a topology object with ADCL
    call ADCL_Init ( ierror )
    call ADCL_Topology_create ( cart_comm, topo, ierror )

    ntests_3D = 2
    do itest = 1, ntests_3D
        isok = .false.

        if ( itest .eq. 1 ) then 
        ! **********************************************************************
        ! Test 1: hwidth=1, nc=0
        hwidth = 1
        nc = 0
        dims1(1) = DIM0 + 2*hwidth;
        dims1(2) = DIM1 + 2*hwidth;
        dims1(3) = DIM2 + 2*hwidth;
        else
        ! **********************************************************************
        ! Test 2: hwidth=2, nc=0
        hwidth = 2
        nc = 0
        dims1(1) = DIM0 + 2*hwidth;
        dims1(2) = DIM1 + 2*hwidth;
        dims1(3) = DIM2 + 2*hwidth;
        end if 

        allocate ( data1(dims1(1),dims1(2), dims1(3)) )
        call adcl_vmap_halo_allocate( hwidth, vmap, ierror )
        if ( ADCL_SUCCESS .ne. ierror) then 
        print *, "vmap_halo_allocate not successful"
        goto 100
        end if

        call adcl_vector_register_generic ( ndim,  dims1, nc, vmap, MPI_DOUBLE_PRECISION, data1, vec, ierror )
        if ( ADCL_SUCCESS .ne. ierror) then 
        print *, "vector_register_generic not successful"
        goto 100
        end if

        call ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, request, ierror )
        if ( ADCL_SUCCESS .ne. ierror) then 
        print *, "request_create not successful"
        goto 100
        end if

        do i = 1, niter

             call set_data_3D ( data1, rank, dims1, hwidth ) 
#ifdef VERBOSE
             call dump_vector_3D_mpi_dp ( data1, dims1, cart_comm )
#endif
            call ADCL_Request_start( request, ierror )
            if ( ADCL_SUCCESS .ne. ierror) then 
                print *, "request_start not successful"
                goto 100
            end if
#ifdef VERBOSE
            call dump_vector_3D_mpi_dp ( data1, dims1, cart_comm )
#endif
            isok = check_data_3D ( data1, rank, dims1, hwidth, neighbors )
            if ( .not. isok ) then
                if ( rank == 0 ) then
                   write(*,'(1x,a,i0,a,i0,a,i0)') "2D f90 testsuite failed at iteration", i, ": hwidth = ", hwidth, ", nc =", nc
                   call dump_vector_3D_mpi_dp ( data1, dims1, cart_comm )
                end if
                exit
            endif
         end do  ! niter

100   continue
         call ADCL_Request_free ( request, ierror )
         if ( ADCL_SUCCESS .ne. ierror) then 
             print *, "request_free not successful"
         end if

         call ADCL_Vector_deregister ( vec, ierror )
         if ( ADCL_SUCCESS .ne. ierror) then 
             print *, "vector_deregister not successful"
         end if

         call ADCL_Vmap_free ( vmap, ierror )
         if ( ADCL_SUCCESS .ne. ierror) then 
             print *, "vmap_free not successful"
         end if

         deallocate ( data1 )
         if ( rank == 0 .and. isok )  then
             write(*,'(1x,a,i0,a,i0,a)') "3D f90 testsuite: hwidth = ", hwidth, ", nc = ", nc, " passed"
         end if
    end do  ! tests_3D


    ntests_3D_plus_nc = 3
    do itest = 1, ntests_3D_plus_nc
        isok = .false.

        if ( itest .eq. 1 ) then 
            ! **********************************************************************
            ! Test 3: hwidth=1, nc=1
            hwidth = 1
            nc = 1
            dims2(1) = DIM0 + 2*hwidth;
            dims2(2) = DIM1 + 2*hwidth;
            dims2(3) = DIM2 + 2*hwidth;
            dims2(4) = nc
        else if ( itest .eq. 2 ) then
            ! **********************************************************************
            ! Test 4: hwidth=2, nc=1
            hwidth = 2
            nc = 1
            dims2(1) = DIM0 + 2*hwidth;
            dims2(2) = DIM1 + 2*hwidth;
            dims2(3) = DIM2 + 2*hwidth;
            dims2(4) = nc
        else
            ! **********************************************************************
            ! Test 5: hwidth=2, nc=2
            hwidth = 2
            nc = 2
            dims2(1) = DIM0 + 2*hwidth;
            dims2(2) = DIM1 + 2*hwidth;
            dims2(3) = DIM2 + 2*hwidth;
            dims2(4) = nc
        end if

        allocate ( data2(dims2(1),dims2(2), dims2(3), dims2(4)) )
        call adcl_vmap_halo_allocate( hwidth, vmap, ierror )
        if ( ADCL_SUCCESS .ne. ierror) then 
            print *, "vmap_halo_allocate not successful"
            goto 200
        end if

        call adcl_vector_register_generic ( ndim,  dims2(1:3), nc, vmap, MPI_DOUBLE_PRECISION, data2, vec, ierror )
        if ( ADCL_SUCCESS .ne. ierror) then 
            print *, "vector_register_generic not successful"
            goto 200
        end if

        call ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, request, ierror )
        if ( ADCL_SUCCESS .ne. ierror) then 
            print *, "request_create not successful"
            goto 200
        end if

        do i = 1, niter
            call set_data_4D ( data2, rank, dims2, hwidth, nc ) 
            call ADCL_Request_start( request, ierror )

            if ( ADCL_SUCCESS .ne. ierror) then 
                print *, "request_start not successful"
                goto 200
            end if

            isok = check_data_4D ( data2, rank, dims2, hwidth, neighbors, nc )
            if ( .not. isok ) then
                if ( rank == 0 ) then
                   write(*,'(1x,a,i0,a,i0,a,i0)') "3D f90 testsuite failed at iteration", i, ": hwidth = ", hwidth, ", nc =", nc
                   call dump_vector_4D_mpi_dp ( data2, dims2, cart_comm )
                end if
                exit
            end if
        end do  ! niter

200   continue
        call ADCL_Request_free ( request, ierror )
        if ( ADCL_SUCCESS .ne. ierror) then 
            print *, "request_free not successful"
        end if

        call ADCL_Vector_deregister ( vec, ierror )
        if ( ADCL_SUCCESS .ne. ierror) then 
            print *, "vector_deregister not successful"
        end if

        call ADCL_Vmap_free ( vmap, ierror )
        if ( ADCL_SUCCESS .ne. ierror) then 
            print *, "vmap_free not successful"
        end if

        deallocate ( data2 )

        if ( rank == 0 .and. isok )  then
            write(*,'(1x,a,i0,a,i0,a)') "3D f90 testsuite: hwidth = ", hwidth, ", nc = ", nc, " passed"
        end if

    end do ! itest

    ! **********************************************************************
    ! done
    call ADCL_Topology_free ( topo, ierror )
    if ( ADCL_SUCCESS .ne. ierror) then 
    print *, "topology_free not successful"
    end if
    call MPI_Comm_free ( cart_comm, ierror )

    call ADCL_Finalize ( ierror )
    if ( ADCL_SUCCESS .ne. ierror) then 
    print *, "adcl_finalize not successful"
    end if
    call MPI_Finalize ( ierror )


end program testfnctsetneigh3df

    !****************************************************************************
    !****************************************************************************
    !****************************************************************************

subroutine set_data_4D ( data, rank, dims, hwidth, nc )

    implicit none
    include 'ADCL.inc'

    integer rank, dims(3), hwidth, nc
    double precision data(dims(1),dims(2), dims(3),nc)

    integer i, j, k, l

        do i=1,hwidth
           do j = 1, dims(2)
              do k = 1, dims(3)
                 do l = 1, nc
                    data(i,j,k,l) = -1
                 end do
              end do
           end do
        end do

        do i = dims(1)-hwidth+1, dims(1)
           do j = 1, dims(2)
              do k = 1, dims(3)
                 do l = 1, nc
                    data(i, j, k, l) = -1
                 end do
              end do
           end do
        end do

        do i=1,dims(1)
           do j = 1, hwidth
              do k = 1, dims(3)
                 do l = 1, nc
                    data(i, j, k, l) = -1
                 end do
              end do
           end do
        end do


        do i = 1, dims(1)
           do j = dims(2)-hwidth+1, dims(2)
              do k = 1, dims(3)
                 do l = 1, nc
                    data(i, j, k, l) = -1
                 end do
              end do
           end do
        end do


        do i = 1, dims(1)
           do j = 1, dims(2)
              do k = 1, hwidth
                 do l = 1, nc
                    data(i, j, k, l) = -1
                 end do
              end do
           end do
        end do

        do i = 1,dims(1)
           do j = 1, dims(2)
              do k = dims(3)-hwidth+1, dims(3)
                 do l = 1, nc
                    data(i, j, k, l) = -1
                 end do
              end do
           end do
        end do


        do i = hwidth+1, dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k = hwidth+1, dims(3)-hwidth
                 do l = 1, nc
                    data(i,j,k,l) = rank
                 end do
              end do
           end do
        end do

       return
    end subroutine set_data_4D

    !****************************************************************************

function  check_data_4D ( data, rank, dims, hwidth, neighbors, nc ) result(isok)
    implicit none
    include 'ADCL.inc'

    integer, intent(in) :: rank, hwidth, nc
    integer, dimension(3), intent(in) :: dims
    integer, dimension(6), intent(in) :: neighbors
    double precision, intent(in) :: data(dims(1),dims(2), dims(3), nc)
    logical :: isok

    integer :: i, j, k, l, ierr, lres=1, gres
    double precision :: should_be
        lres = 1

        if ( neighbors(1) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(1)
        endif
        do i=1,hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k = hwidth+1, dims(3)-hwidth
                 do l = 1, nc 
                    if ( data(i,j,k, l) .ne. should_be ) then                       
                       lres = 0 
                       write (*,99) rank, ' : ', i,j,k,l, 'is ', &
                            data(i,j,k,l), ' should be ', should_be
                    endif
                 end do
              end do
           end do
        end do

        if ( neighbors(2) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(2)
        endif
        do i = dims(1)-hwidth+1, dims(1)
           do j = hwidth+1, dims(2)-hwidth
              do k = hwidth+1, dims(3)-hwidth
                 do l = 1, nc 
                    if ( data(i, j, k, l) .ne. should_be ) then 
                       lres = 0
                       write (*,99) rank, ' : ', i,j,k,l, 'is ', &
                            data(i,j,k,l), ' should be ', should_be
                    end if
                 end do
              end do
           end do
        end do


        if ( neighbors(3) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(3)
        endif
        do i=hwidth+1,dims(1)-hwidth
           do j = 1, hwidth
              do k = hwidth+1, dims(3)-hwidth
                 do l = 1, nc 
                    if ( data(i,j, k, l) .ne. should_be ) then
                       lres = 0                    
                       write (*,99) rank, ' : ', i,j,k,l, 'is ', &
                            data(i,j,k,l), ' should be ', should_be
                    endif
                 end do
              end do
           end do
        end do

        if ( neighbors(4) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(4)
        endif
        do i = hwidth+1, dims(1)-hwidth
           do j = dims(2)-hwidth+1, dims(2)
              do k = hwidth+1, dims(3)-hwidth
                 do l = 1, nc 
                    if ( data(i, j, k, l) .ne. should_be ) then 
                       lres = 0
                       write (*,99) rank, ' : ', i,j,k,l, 'is ', &
                            data(i,j,k,l), ' should be ', should_be
                    endif
                 end do
              end do
           end do
        end do

        if ( neighbors(5) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(5)
        endif
        do i=hwidth+1,dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k = 1, hwidth
                 do l = 1, nc 
                    if ( data(i,j, k, l) .ne. should_be ) then
                       lres = 0                    
                       write (*,99) rank, ' : ', i,j,k,l, 'is ', &
                            data(i,j,k,l), ' should be ', should_be
                    endif
                 end do
              end do
           end do
        end do

        if ( neighbors(6) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(6)
        endif
        do i = hwidth+1, dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k = dims(3)-hwidth+1, dims(3)
                 do l = 1, nc 
                    if ( data(i, j, k, l) .ne. should_be ) then 
                       lres = 0
                       write (*,99) rank, ' : ', i,j,k,l, ' is ', &
                            data(i,j,k,l), ' should be ', should_be
                    endif
                 end do
              end do
           end do
        end do


        do i = hwidth+1, dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k = hwidth+1, dims(3)-hwidth
                 do l = 1, nc 
                    if ( data(i,j,k,l) .ne. rank ) then 
                       lres = 0
                       write (*,99) rank, ' : ', i,j,k,l, 'is ', &
                            data(i,j,k,l), ' should be ', should_be
                    endif
                 end do
              end do
           end do
        end do


        call MPI_Allreduce ( lres, gres, 1, MPI_INTEGER, MPI_MIN, MPI_COMM_WORLD, ierr)

        if ( gres .eq. 0 ) then
            isok = .false.
        else
            isok = .true. 
        end if

99      format (i1,a3,4i3,a4,f12.5,a11,f12.5)

        return
    end function check_data_4D

    !****************************************************************************
    !****************************************************************************
    !****************************************************************************

    subroutine set_data_3D ( data, rank, dims, hwidth )

        implicit none
        include 'ADCL.inc'

        integer rank, dims(3), hwidth
        double precision data(dims(1),dims(2),dims(3))

        integer i, j, k

        do i=1,hwidth
           do j = 1, dims(2)
              do k = 1, dims(3)
                 data(i,j,k) = -1
              end do
           end do
        end do

        do i = dims(1)-hwidth+1, dims(1)
           do j = 1, dims(2)
              do k = 1, dims(3)
                 data(i, j, k) = -1
              end do
           end do
        end do

        do i=1,dims(1)
           do j = 1, hwidth
              do k = 1, dims(3)
                 data(i, j, k) = -1
              end do
           end do
        end do


        do i = 1, dims(1)
           do j = dims(2)-hwidth+1, dims(2)
              do k = 1, dims(3)
                 data(i, j, k) = -1
              end do
           end do
        end do


        do i = 1, dims(1)
           do j = 1, dims(2)
              do k = 1, hwidth
                 data(i, j, k) = -1
              end do
           end do
        end do

        do i = 1,dims(1)
           do j = 1, dims(2)
              do k = dims(3)-hwidth+1, dims(3)
                 data(i, j, k) = -1
              end do
           end do
        end do


        do i = hwidth+1, dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k = hwidth+1, dims(3)-hwidth
                 data(i,j,k) = rank
              end do
           end do
        end do

    return
    end subroutine set_data_3D

    !****************************************************************************
    !****************************************************************************
    !****************************************************************************

    function check_data_3D ( data, rank, dims, hwidth, neighbors ) result (isok)

    implicit none
    include 'ADCL.inc'

    integer, intent(in) :: rank, hwidth
    integer, dimension(3), intent(in) :: dims
    integer, dimension(6), intent(in) :: neighbors
    double precision, intent(in) :: data(dims(1),dims(2), dims(3))
    logical :: isok

    integer :: i, j, k, l, ierr, lres=1, gres
    double precision ::  should_be

        lres = 1
      
        if ( neighbors(1) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(1)
        endif
        do i=1,hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k = hwidth+1, dims(3)-hwidth
                 if ( data(i,j,k) .ne. should_be ) then
                    lres = 0 
                 endif
              end do
           end do
        end do

        if ( neighbors(2) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(2)
        endif
        do i = dims(1)-hwidth+1, dims(1)
           do j = hwidth+1, dims(2)-hwidth
              do k = hwidth+1, dims(3)-hwidth
                 if ( data(i, j, k) .ne. should_be ) then 
                    lres = 0
                 end if
              end do
           end do
        end do


        if ( neighbors(3) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(3)
        endif
        do i=hwidth+1,dims(1)-hwidth
           do j = 1, hwidth
              do k = hwidth+1, dims(3)-hwidth
                 if ( data(i,j, k) .ne. should_be ) then
                    lres = 0                    
                 endif
              end do
           end do
        end do

        if ( neighbors(4) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(4)
        endif
        do i = hwidth+1, dims(1)-hwidth
           do j = dims(2)-hwidth+1, dims(2)
              do k = hwidth+1, dims(3)-hwidth
                 if ( data(i, j, k) .ne. should_be ) then 
                    lres = 0
                 endif
              end do
           end do
        end do

        if ( neighbors(5) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(5)
        endif
        do i=hwidth+1,dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k = 1, hwidth
                 if ( data(i,j, k) .ne. should_be ) then
                    lres = 0                    
                 endif
              end do
           end do
        end do

        if ( neighbors(6) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(6)
        endif
        do i = hwidth+1, dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k = dims(3)-hwidth+1, dims(3)
                 if ( data(i, j, k) .ne. should_be ) then 
                    lres = 0
                 endif
              end do
           end do
        end do


        do i = hwidth+1, dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k = hwidth+1, dims(3)-hwidth
                 if ( data(i,j,k) .ne. rank ) then 
                    lres = 0
                 endif
              end do
           end do
        end do

        call MPI_Allreduce ( lres, gres, 1, MPI_INTEGER, MPI_MIN, MPI_COMM_WORLD, ierr)
        
        if ( gres .eq. 0 ) then
            isok = .false.
        else
            isok = .true. 
        end if
        
        return
    end function check_data_3D
