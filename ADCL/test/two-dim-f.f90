!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! Copyright (c) 2009-2010      HLRS. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
!
! unit test for 2D Fortran neighborhood communication


program testfnctsetneigh2df
   use adcl
   use auxdata2df
   use auxdata3df
   implicit none

   integer rank, size, ierror         
   integer nc, hwidth
   integer vmap, vec, topo, request
   integer cart_comm
   integer, parameter :: ndim = 2, nneigh=4
   integer, dimension(ndim) :: cdims
   logical, dimension(ndim) :: periods
   integer, dimension(ndim+1) :: dims
   integer, dimension(nneigh) :: lneighbors, rneighbors, flip
   integer, dimension(2*nneigh) :: neighbors
   double precision, allocatable :: data1(:,:), data2(:,:,:)
   integer, parameter :: niter = 50
   logical :: isok
   integer :: i, itest, ntests_2D, ntests_2D_plus_nc

   cdims   = 0
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
        call ADCL_Topology_create ( cart_comm, topo, ierror )
        call MPI_Cart_shift ( cart_comm, 0, 1, neighbors(1), neighbors(2), ierror )
        call MPI_Cart_shift ( cart_comm, 1, 1, neighbors(3), neighbors(4), ierror )
        

   ntests_2D = 2
   do itest = 1, ntests_2D
      if ( itest .eq. 1 ) then 
         ! **********************************************************************
         ! Test 1: hwidth=1, nc=0 
         nc     = 0
         hwidth = 1

         dims(1) = 8 
         dims(2) = 10 
         dims(3) = 1 ! for reshape 
      else
         ! **********************************************************************
         ! Test 2: hwidth=2, nc=0 
         hwidth = 2
         nc   = 0

         dims(1) = 8 
         dims(2) = 10 
         dims(3) = 1
      end if
      
      allocate ( data1(dims(1),dims(2)) )
      allocate ( data2(dims(1),dims(2),dims(3)) )

      call adcl_vmap_halo_allocate( hwidth, vmap, ierror ) 
      if ( ADCL_SUCCESS .ne. ierror) then 
         print *, "vmap_halo_allocate not successful"
         goto 100
      end if
      call adcl_vector_register_generic ( ndim,  dims(1:ndim), nc, vmap, MPI_DOUBLE_PRECISION, data1, vec, ierror )
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
         isok = .false.

         data2 = reshape( data1, dims ) 
         call set_data_2D_plus_cont( data2, rank, dims, hwidth, cart_comm ) 
         data1 = reshape( data2, dims(1:ndim) ) 
           
#ifdef VERBOSE
         call dump_vector_2D_mpi_dp ( data1, dims(1:ndim), cart_comm )
#endif

         call ADCL_Request_start( request, ierror )

#ifdef VERBOSE
         call dump_vector_2D_mpi_dp ( data1, dims(1:ndim), cart_comm )
#endif
         data2 = reshape( data1, dims ) 
         isok =  check_data_2D_plus_cont ( data2, rank, dims, hwidth, neighbors, cart_comm ) 
         if ( .not. isok ) then
             if ( rank == 0 ) then
                 write(*,'(1x,a,i0,a,i0,a,i0)') "2D f90 testsuite failed at iteration", i, ": hwidth = ", hwidth, ", nc =", nc
             end if
             call dump_vector_2D_mpi_dp ( data1, dims, cart_comm )
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
      deallocate ( data2 )

      if ( rank == 0 .and. isok )  then
          write(*,'(1x,a,i0,a,i0,a)') "2D f90 testsuite: hwidth = ", hwidth, ", nc = ", nc, " passed"
      end if
   end do  ! tests_2D

   ntests_2D_plus_nc = 3
   do itest = 1, ntests_2D_plus_nc
      isok = .false.

      if ( itest .eq. 1 ) then 
         ! **********************************************************************
         ! Test 3: hwidth=1, nc=1 
         hwidth = 1
         nc     = 1
         dims(1) = 8 
         dims(2) = 10 
         dims(3) = nc 
      else if ( itest .eq. 2 ) then
         ! **********************************************************************
         ! Test 4: hwidth=2, nc=1 
         hwidth = 2 
         nc     = 1

         dims(1) = 8 
         dims(2) = 10 
         dims(3) = nc
      else
         ! **********************************************************************
         ! Test 5: hwidth=2, nc=2 
         hwidth = 2
         nc     = 2
         dims(1) = 8
         dims(2) = 10 
         dims(3) = nc
      endif 
 
      allocate ( data2(dims(1),dims(2),nc) )
      call adcl_vmap_halo_allocate( hwidth, vmap, ierror ) 
      if ( ADCL_SUCCESS .ne. ierror) then 
         print *, "vmap_halo_allocate not successful"
         goto 200
      end if

      call adcl_vector_register_generic ( ndim,  dims(1:ndim), nc, vmap, MPI_DOUBLE_PRECISION, & 
                                  data2, vec, ierror)
      if ( ADCL_SUCCESS .ne. ierror) then 
         print *, "vector_register_generic not successful"
         goto 200
      end if

      call ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &
           request, ierror )
      if ( ADCL_SUCCESS .ne. ierror) then 
         print *, "request_create not successful"
         goto 200
      end if

      do i = 1, niter
         call set_data_2D_plus_cont( data2, rank, dims, hwidth, cart_comm ) 
#ifdef VERBOSE
         call dump_vector_3D_mpi_dp ( data2, dims, cart_comm )
#endif
         call ADCL_Request_start( request, ierror )
         if ( ADCL_SUCCESS .ne. ierror) then 
            print *, "request_start not successful"
            goto 200
         end if

         isok = check_data_2D_plus_cont ( data2, rank, dims, hwidth, neighbors, cart_comm )   
#ifdef VERBOSE
         call dump_vector_3D_mpi_dp ( data2, dims, cart_comm )
#endif
         if ( .not. isok ) then
             if ( rank == 0 ) then
                 write(*,'(1x,a,i0,a,i0,a,i0)') "2D f90 testsuite failed at iteration", i, ": hwidth = ", hwidth, ", nc =", nc
             end if
             call dump_vector_2D_mpi_dp ( data1, dims, cart_comm )
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

      if ( rank == 0 .and. isok )  then
          write(*,'(1x,a,i0,a,i0,a)') "2D f90 testsuite: hwidth = ", hwidth, ", nc = ", nc, " passed"
      end if
   end do ! test_2D_plus_nc


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


end program testfnctsetneigh2df

!****************************************************************************
!****************************************************************************
!****************************************************************************

      subroutine set_data_3D ( data, rank, dims, hwidth, nc )

        implicit none
        include 'ADCL.inc'

        integer rank, dims(2), hwidth, nc
        double precision data(dims(1),dims(2), nc)

        integer i, j, k

        do i=1,hwidth
           do j = 1, dims(2)
              do k = 1,nc
                 data(i,j,k) = -1
              end do
           end do
        end do

        do i = dims(1)-hwidth+1, dims(1)
           do j = 1, dims(2)
              do k = 1, nc
                 data(i, j, k) = -1
              end do
           end do
        end do

        do i=1,dims(1)
           do j = 1, hwidth
              do k = 1, nc
                 data(i,j, k) = -1
              end do
           end do
        end do

        do i = 1, dims(1)
           do j = dims(2)-hwidth+1, dims(2)
              do k = 1, nc
                 data(i, j, k) = -1
              end do
           end do
        end do


        do i = hwidth+1, dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k = 1, nc
                 data(i,j, k) = rank
              end do
           end do
        end do

        return
      end subroutine set_data_3D



!****************************************************************************
!****************************************************************************
!****************************************************************************

      subroutine check_data_3D ( data, rank, dims, hwidth, neighbors, nc )

        implicit none
        include 'ADCL.inc'

        integer rank, dims(2), hwidth, nc, neighbors(4)
        double precision data(dims(1),dims(2), nc)
        integer lres, gres, i, j, k, ierr
        double precision should_be

        lres = 1

        if ( neighbors(1) .eq. MPI_PROC_NULL ) then
           should_be = -1
        else
           should_be = neighbors(1)
        endif
        do i=1,hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k=1,nc
                 if ( data(i,j,k) .ne. should_be ) then
                    lres = 0
                    write (*,99) rank, ' : ', i,j,k, 'is ', &
                         data(i,j,k), ' should be ', should_be
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
              do k = 1, nc
                 if ( data(i, j, k) .ne. should_be ) then
                    lres = 0
                    write (*,99) rank, ' : ', i,j,k, 'is ', &
                         data(i,j,k), ' should be ', should_be
                 endif
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
              do k = 1, nc
                 if ( data(i,j,k) .ne. should_be ) then
                    lres = 0
                    write (*,99) rank, ' : ', i,j,k, 'is ', &
                         data(i,j,k), ' should be ', should_be
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
              do k = 1, nc
                 if ( data(i, j, k) .ne. should_be ) then
                    lres = 0
                    write (*,99) rank, ' : ', i,j,k, 'is ', &
                         data(i,j,k), ' should be ', should_be
                 endif
              end do
           end do
        end do


        do i = hwidth+1, dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k = 1, nc
                 if ( data(i,j, k) .ne. rank ) then
                    lres = 0
                    write (*,99) rank, ' : ', i,j,k, 'is ', &
                         data(i,j,k), ' should be ', should_be
                 endif
              end do
           end do
        end do

99      format (i1,a3,3i3,a4,f12.5,a11,f12.5)


        call MPI_Allreduce ( lres, gres, 1, MPI_INTEGER, MPI_MIN, MPI_COMM_WORLD, ierr)
        if ( gres .eq. 0 ) then
!           call dump_vector_2D_dp( data, rank, dims )
           if ( rank .eq. 0 ) then
              write (*,*) '2-D Fortran testsuite hwidth =', hwidth, &
                   'nc = ', nc, ' failed'
           end if
        else
           if ( rank .eq. 0 ) then
              write (*,*) '2-D Fortran testsuite hwidth =', hwidth, &
                   'nc = ', nc, ' passed'
           end if
        end if


        return
      end subroutine check_data_3D

