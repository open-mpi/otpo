!
! Copyright (c) 2009-2010       HLRS. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
! unit test for 3D Fortran extended neighborhood communication (for Lattice Boltzmann)
!
! set_data: over all processes, all compute cells are numbered continously in x, y and z direction 
! check_data: determines the "direction" in which to check data
! calc_entry: depending on direction and presence of a neighbor, sets loop bounds and offsets and checks entries
!
! ToDo: for nc > 1, program a special set_data and check_data function
!       at the moment all data(x,y,z,1:nc) are set to the number

#undef VERBOSE
 
program testfnctsetextneigh3df
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

   cdims = 0
   periods = .false.

   ! Initiate the MPI environment
   call MPI_Init ( ierror )
   call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror)
   call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )

   ! Describe the neighborhood relations
   call MPI_Dims_create ( size, ndim, cdims, ierror)
   call MPI_Cart_create ( MPI_COMM_WORLD, ndim, cdims, periods, .false., cart_comm, ierror )

   ! Initiate the ADCL library and register a topology object with ADCL
   call ADCL_Init ( ierror )
   call ADCL_Topology_create_extended ( cart_comm, topo, ierror )
  !call ADCL_Topology_create ( cart_comm, topo, ierror )
   call ADCL_Topology_get_cart_neighbors ( nneigh, lneighbors, rneighbors, flip, cart_comm, ierror )
   if ( ierror .ne. ADCL_SUCCESS ) then 
       print *, "failed to get neighbors"; stop
   endif
   neighbors(1) = lneighbors(1);   neighbors(2) = rneighbors(1)
   neighbors(3) = lneighbors(2);   neighbors(4) = rneighbors(2)
   neighbors(5) = lneighbors(3);   neighbors(6) = rneighbors(3)
   !neighbors(7) = lneighbors(4);   neighbors(8) = rneighbors(4)
   !neighbors(9) = lneighbors(5);   neighbors(10) = rneighbors(5)
   !neighbors(11) = lneighbors(6);   neighbors(12) = rneighbors(6)
   !neighbors(13) = lneighbors(7);   neighbors(14) = rneighbors(7)
   !neighbors(15) = lneighbors(8);   neighbors(16) = rneighbors(8)
   !neighbors(17) = lneighbors(9);   neighbors(18) = rneighbors(9)

   ntests_3D = 2
   do itest = 1, ntests_3D
      isok = .false.

      if ( itest .eq. 1 ) then 
         ! **********************************************************************
         ! Test 1: hwidth=1, nc=0
         !dims1(1) = 10
         !dims1(2) = 6
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
         call set_data_3D_cont( data1, rank, dims1, hwidth, cart_comm )
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
         isok = check_data_3D_cont ( data1, rank, dims1, hwidth, neighbors, cart_comm )
         if ( .not. isok ) then
            if ( rank == 0 ) then
                write(*,'(1x,a,i0,a,i0,a,i0)') "2D f90 testsuite failed at iteration", i, ": hwidth = ", hwidth, ", nc =", nc
            end if
            call dump_vector_3D_mpi_dp ( data1, dims1, cart_comm )
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
      if ( rank == 0 )  then
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
         call set_data_4D( data2, rank, dims2(1:3), hwidth, nc, cart_comm )

#ifdef VERBOSE
         call dump_vector_4D_mpi_dp ( data2, dims2, cart_comm )
#endif
         call ADCL_Request_start( request, ierror )
         if ( ADCL_SUCCESS .ne. ierror) then 
            print *, "request_start not successful"
            goto 200
         end if

#ifdef VERBOSE
         call dump_vector_4D_mpi_dp ( data2, dims2, cart_comm )
#endif
         isok = check_data_4D ( data2, rank, dims2(1:3), hwidth, nc, neighbors, cart_comm )
         if ( .not. isok ) then
             if ( rank == 0 ) then
                 write(*,'(1x,a,i0,a,i0,a,i0)') "3D f90 testsuite failed at iteration", i, ": hwidth = ", hwidth, ", nc =", nc
             end if
             call dump_vector_4D_mpi_dp ( data2, dims2, cart_comm )
             exit
         endif
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

      if ( rank == 0 )  then
          write(*,'(1x,a,i0,a,i0,a)') "3D f90 testsuite: hwidth = ", hwidth, ", nc = ", nc, " passed"
      end if
   end do ! test_3D_plus_nc

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


contains

! ****************************************************************************

subroutine set_data_4D ( data, rank, dims, hwidth, nc, cart_comm)

    implicit none
    include 'ADCL.inc'

    integer, intent(in) :: rank, hwidth, nc, cart_comm
    integer, dimension(3), intent(in) :: dims
    double precision, intent(inout) :: data(dims(1),dims(2),dims(3),nc)

    integer :: i, j, k, l, ierr
    integer, dimension(3) :: coords, cart_dims, period, dims_wo_halos

    call MPI_Cart_get(cart_comm, 3, cart_dims, period, coords, ierr)
    ! cube: cart_dims with index coords
    ! inside cube: dims with index (i,j,k)

    do l = 1, nc
        do j=1, dims(2)
           do i=1, dims(1)
              do k=1, hwidth
                 data(i,j,k,l) = -1
              end do 
              do k=dims(3)-hwidth+1, dims(3)
                 data(i,j,k,l) = -1
              end do 
           end do
       end do
   end do

   do l = 1, nc
      do k=1, dims(3)
         do i=1, dims(1)
            do j=1, hwidth
               data(i,j,k,l) = -1
           end do
           do j=dims(2)-hwidth+1, dims(2)
               data(i,j,k,l) = -1
            end do
         end do
      end do
   end do

   do l = 1, nc
      do k=1, dims(3)
         do j=1, dims(2)
            do i=1, hwidth
               data(i,j,k,l) = -1
            end do
            do i=dims(1)-hwidth+1, dims(1)
               data(i,j,k,l) = -1
            end do
         end do
      end do
   end do

    !be aware of the change in i and j in the equation compare to the c code. (i-1) and (j-1)
    ! without hwidth: 
    !    in x direction offset dims(1)*coords(1) + i
    !    in y direction offset dims(1)*cart_dims(1) * ( dims(2)*coords(2) + j-1 )
    !    in z direction offset dims(1)*cart_dims(1) * dims(2)*cart_dims(2) * ( dims(3)*coords(3) + z-1 )
    dims_wo_halos = dims - 2*hwidth 

    do l = 1, nc
       do k=hwidth+1, dims(3)-hwidth
          do j=hwidth+1, dims(2)-hwidth
             do  i=hwidth+1, dims(1)-hwidth
                 !print *, "rank =", rank, ", coords = ( ", coords, " )"
                 data(i,j,k,l) = i-hwidth + dims_wo_halos(1)*coords(1) +                                                   & 
                               dims_wo_halos(1)*cart_dims(1) * ( ( dims_wo_halos(2)*coords(2) + j-hwidth-1 ) +             & 
                                           dims_wo_halos(2)*cart_dims(2) * ( dims_wo_halos(3)*coords(3) + k-hwidth-1 ) )
            end do 
         end do  
      end do
   end do

   return
end subroutine set_data_4D

!****************************************************************************
function check_data_4D ( data, rank, dims, hwidth, nc, neighbors, cart_comm ) result(isok)
   use  auxdata3df
   implicit none

   integer, intent(in) :: rank, hwidth, nc, cart_comm
   integer, dimension(3), intent(in) :: dims
   integer, dimension(6), intent(in) :: neighbors
   double precision, intent(in) :: data(dims(1),dims(2), dims(3), nc)
   logical :: isok

   integer :: i, j, k, l, ierr, lres=1, gres, prod
   integer, dimension(3) :: coords, n_coords, c_coords, cart_size, period
   double precision should_be

   integer :: x_direction, y_direction, z_direction

   ! check for each of the 27 possible locations
   do z_direction = -1, 1
        do y_direction = -1, 1
            do x_direction = -1, 1
                do l = 1, nc
                   prod = x_direction * y_direction * z_direction
#ifdef INCCORNER
                   if ( prod .ne. 0 ) then
                       ! corner
                       lres = check_region_3D (x_direction, y_direction, z_direction, data(:,:,:,l), rank, & 
                              cart_comm, dims, hwidth, neighbors)
                   endif 
#endif
                   if ( prod == 0) then
                       ! edge, face or inside
                       lres = check_region_3D (x_direction, y_direction, z_direction, data(:,:,:,l), rank, & 
                              cart_comm, dims, hwidth, neighbors )
                   end if
                   call MPI_Allreduce ( lres, gres, 1, MPI_INTEGER, MPI_MIN, cart_comm, ierror )
                   if ( gres .ne. 1 ) then 
                      isok = .false.
                      return 
                endif 
                end do
            end do
        end do
    end do


    !if ( gres .eq. 1 ) then
    !    if ( rank == 0 )  then
    !        write(*,'(1x,a,i0,a,i0,a)') "3-D C testsuite: hwidth = ", hwidth, ", nc = ", nc, " passed"
    !    end if
    !else 
    !    if ( rank == 0 ) then
    !        write(*,'(1x,a,i0,a,i0,a)') "3-D C testsuite: hwidth = ", hwidth, ", nc = ", nc, " failed"
    !    end if
    !    call dump_vector_4D_dp ( data, rank, dims );
    !endif

    isok = .true.
    return

end function check_data_4D

end program testfnctsetextneigh3df
