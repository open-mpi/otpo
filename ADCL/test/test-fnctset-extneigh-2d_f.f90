!
! Copyright (c) 2009-2010       HLRS. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!

! unit test for 2D Fortran extended neighborhood communication (for Lattice Boltzmann)
!

#define INCCORNER

program testfnctsetextneigh2df
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
   call ADCL_Topology_create_extended ( cart_comm, topo, ierror )
   call ADCL_Topology_get_cart_neighbors ( nneigh, lneighbors, rneighbors, flip, cart_comm, ierror )
   if ( ierror .ne. ADCL_SUCCESS ) then 
       print *, "failed to get neighbors"; stop
   endif
   neighbors(1) = lneighbors(1);   neighbors(2) = rneighbors(1)
   neighbors(3) = lneighbors(2);   neighbors(4) = rneighbors(2)
   neighbors(5) = lneighbors(3);   neighbors(6) = rneighbors(3)
   neighbors(7) = lneighbors(4);   neighbors(8) = rneighbors(4)

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

      do i = 1, 1 !niter
         call set_data_2D_plus_cont( data2, rank, dims, hwidth, cart_comm ) 
#ifdef VERBOSE
         call dump_vector_3D_mpi_dp ( data2, dims, cart_comm )
#endif
         call ADCL_Request_start( request, ierror )
         if ( ADCL_SUCCESS .ne. ierror) then 
            print *, "request_start not successful"
            goto 200
         end if

         !call dump_vector_3D_mpi_dp ( data2, dims, cart_comm )
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


end program testfnctsetextneigh2df
