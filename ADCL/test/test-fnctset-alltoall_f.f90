!
! Copyright (c) 2009           HLRS. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!

program test_fnctset_alltoall
   implicit none
   include 'ADCL.inc'
 
   integer :: cnt, dims, ierror, rank, size
   integer :: cdims = 0
   integer :: periods = 0
 
   integer topo
   external dump_vector_1D, set_data_1D, check_data_1D
 
   call MPI_Init ( ierror )
   call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror )
   call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )
 
   call adcl_init ( ierror )
 
   call adcl_topology_create ( MPI_COMM_WORLD, topo, ierror )
   if ( ADCL_SUCCESS .ne. ierror) then 
       print *, "Topology creation failed"
       goto 100
   end if 
 
   cnt = 15 !200 
   dims = 13
   dims=3
 
   call alltoall_test(cnt, dims, rank, size, topo)

100 if ( ADCL_TOPOLOGY_NULL .ne. topo)   call adcl_topology_free ( topo, ierror )
   call adcl_finalize ( ierror )
   call MPI_Finalize ( ierror )

end program test_fnctset_alltoall

!*******************************************************************************
subroutine alltoall_test(cnt, dims, rank, size, topo)
!*******************************************************************************
    implicit none
    include 'ADCL.inc'
    integer, intent(in) :: cnt, dims, rank, size, topo 
    double precision, allocatable, dimension(:) :: sdata, rdata
    integer dim, i
    integer cnts(size), displ(size)
    integer ierror
    integer svec, rvec
    integer vmap
    integer request

    ! set up arrays for verification
    do i=1, size
        cnts(i)  = dims
        displ(i) = dims*(i-1)
    end do

    dim = dims*size
    allocate(sdata(dim), rdata(dim))

    call set_data_1D ( sdata, -5, dim)
    call set_data_1D ( rdata, -10, dim)

    call adcl_vmap_alltoall_allocate( cnts(1), cnts(1), vmap, ierror ); 
    if ( ADCL_SUCCESS .ne. ierror) then 
        print *, "Vmap creation failed"
        goto 100
    end if 

    call adcl_vector_register_generic ( 1,  dim, 0, vmap, MPI_DOUBLE_PRECISION, sdata, svec, ierror )
    if ( ADCL_SUCCESS .ne. ierror) then 
        print *, "Send vector registration failed"
        goto 100
    end if 
    call adcl_vector_register_generic ( 1,  dim, 0, vmap, MPI_DOUBLE_PRECISION, rdata, rvec, ierror )
    if ( ADCL_SUCCESS .ne. ierror) then 
        print *, "Receive vector registration failed"
        goto 100
    end if 

    call adcl_request_create_generic ( svec, rvec, topo, ADCL_FNCTSET_ALLTOALL, request, ierror )
    if ( ADCL_SUCCESS .ne. ierror) then 
        print *, "request_create not successful"
        goto 100
    end if 


    do i = 1, cnt
       call set_data_1D ( sdata, rank, dim)
       call set_data_1D ( rdata, -1,   dim)

!#ifdef VERBOSE
!       call dump_vector_1D ( sdata, rank, dim )
!       call dump_vector_1D ( rdata, rank, dim )
!#endif

       call adcl_request_start( request, ierror )
       if ( ADCL_SUCCESS .ne. ierror) then
           print *, "request_start not successful"   
           goto 100
       endif 

       call check_data_1D ( rdata, cnts, displ, rank, size, ierror )
       if ( ADCL_SUCCESS .ne. ierror) then 
           print *, "check_data_1D not successful"   
           goto 100
       endif 

    end do

    call MPI_Barrier ( MPI_COMM_WORLD, ierror );

100 if ( ADCL_SUCCESS .ne. ierror) print *, "ADCL ierror nr.", ierror

    call adcl_vector_deregister( svec, ierror )
    call adcl_vector_deregister( rvec, ierror )

    if ( ADCL_REQUEST_NULL .ne. request) call adcl_request_free ( request, ierror )
    if ( ADCL_VMAP_NULL    .ne. vmap)    call adcl_vmap_free (vmap, ierror)

    deallocate (sdata, rdata)

    return
end subroutine alltoall_test
