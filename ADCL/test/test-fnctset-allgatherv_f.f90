!
! Copyright (c) 2008           HLRS. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!

program test_fnctset_allgatherv 
   implicit none
   include 'ADCL.inc'
 
   integer :: cnt, dims, ierror, rank, size
   integer :: cdims = 0
   integer :: periods = 0
 
   integer topo
   integer cart_comm
   external allgatherv_test1
 
   call MPI_Init ( ierror )
   call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror )
   call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )
 
   call adcl_init ( ierror )
 
   call MPI_Dims_create ( size, 1, cdims, ierror )
   call MPI_Cart_create ( MPI_COMM_WORLD, 1, cdims, periods, 0, cart_comm, ierror)
 
   call adcl_topology_create ( cart_comm, topo, ierror )
 
   cnt = 20 !200 
   dims = 3
 
   ! AllGather
   !call allgatherv_test1(cnt, dims, rank, size, topo)
 
   ! true AllGatherV 
   !call allgatherv_test2(cnt, dims, rank, size, topo)
 
   ! true AllGatherV with MPI_IN_PLACE 
   call allgatherv_test3(cnt, dims, rank, size, topo)

   if ( ADCL_TOPOLOGY_NULL .ne. topo)   call adcl_topology_free ( topo, ierror )
   call MPI_Comm_free ( cart_comm, ierror )
   call adcl_finalize ( ierror )
   call MPI_Finalize ( ierror )

end program test_fnctset_allgatherv 
      
!*******************************************************************************
subroutine allgatherv_test1(cnt, dims, rank, size, topo)
!*******************************************************************************
    implicit none
    include 'ADCL.inc'
    integer, intent(in) :: cnt, dims, rank, size, topo 
    integer sdim, rdim, i 
    integer rcnts(size), displ(size)
    integer ierror
    integer svec, rvec
    integer svmap, rvmap
    integer request
    double precision, allocatable, dimension(:) :: sdata, rdata


    sdim = dims
    rdim = dims*size

    allocate(sdata(sdim), rdata(rdim))

    call set_data_1D ( sdata, -5, sdim)
    call set_data_1D ( rdata, -10, rdim)

    do i = 1, size
       rcnts(i) = dims
       displ(i) = dims * (i-1) 
    end do 

    call adcl_vmap_all_allocate( svmap, ierror ) 
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_all_allocate not successful"   
    call adcl_vmap_list_allocate( size, rcnts, displ, rvmap, ierror ) 
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_list_allocate not successful"   

    call adcl_vector_register_generic ( 1,  sdim, 0, svmap, MPI_DOUBLE_PRECISION, sdata, svec, ierror )
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_vector_register for sdim not successful"   
    call adcl_vector_register_generic ( 1,  rdim, 0, rvmap, MPI_DOUBLE_PRECISION, rdata, rvec, ierror )
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_vector_register for rdim not successful"   

    call adcl_request_create_generic ( svec, rvec, topo, ADCL_FNCTSET_ALLGATHERV, request, ierror )
    if ( ADCL_SUCCESS .ne. ierror) print *, "request_create not successful"   

    do i = 1, cnt
       call set_data_1D ( sdata, rank, sdim)
       call set_data_1D ( rdata, -1, rdim)

!#ifdef VERBOSE
!       call dump_vector_1D ( sdata, rank, sdim)
!       call dump_vector_1D ( rdata, rank, rdim)
!#endif

       call adcl_request_start( request, ierror )
       if ( ADCL_SUCCESS .ne. ierror) then
           print *, "request_start not successful"   
           stop
       endif 
       call check_data_1D ( rdata, rcnts, displ, rank, size, ierror )
       if ( ADCL_SUCCESS .ne. ierror) print *, "check_data_1D not successful"   
    end do


    call adcl_vector_deregister( svec, ierror )
    call adcl_vector_deregister( rvec, ierror )

    call MPI_Barrier ( MPI_COMM_WORLD, ierror )

    if ( ADCL_SUCCESS .ne. ierror) print *, "ADCL ierror nr.", ierror

    if ( ADCL_REQUEST_NULL .ne. request) call adcl_request_free ( request )
    if ( ADCL_VMAP_NULL    .ne. svmap)   call adcl_vmap_free (svmap)
    if ( ADCL_VMAP_NULL    .ne. rvmap)   call adcl_vmap_free (rvmap)

    deallocate (sdata, rdata)

    return
end subroutine allgatherv_test1

!*******************************************************************************
subroutine allgatherv_test2(cnt, dims, rank, size, topo)
!*******************************************************************************

    implicit none
    include 'ADCL.inc'
    integer, intent(in) :: cnt, dims, rank, size, topo 
    double precision, allocatable, dimension(:) :: sdata, rdata
    integer sdim, rdim, i 
    integer rcnts(size), displ(size)
    integer err, errc, offset 
    integer svec, rvec
    integer svmap, rvmap
    integer request
    integer ierror

    sdim = dims+rank

    rdim = dims*size + size*(size-1)/2

    allocate(sdata(sdim), rdata(rdim))

    call set_data_1D ( sdata, -5, sdim)
    call set_data_1D ( rdata, -10, rdim)

    offset = 0
    do i = 1, size
       rcnts(i) = dims+i-1
       displ(i) = offset
       offset   = offset + rcnts(i)
    end do 

    call adcl_vmap_all_allocate( svmap, ierror ) 
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_all_allocate not successful"   
    call adcl_vmap_list_allocate( size, rcnts, displ, rvmap, ierror ) 
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_list_allocate not successful"   

    call adcl_vector_register_generic ( 1,  sdim, 0, svmap, MPI_DOUBLE_PRECISION, sdata, svec, ierror )
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_vector_register for sdim not successful"   
    call adcl_vector_register_generic ( 1,  rdim, 0, rvmap, MPI_DOUBLE_PRECISION, rdata, rvec, ierror )
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_vector_register for rdim not successful"   

    call adcl_request_create_generic ( svec, rvec, topo, ADCL_FNCTSET_ALLGATHERV, request, ierror )
    if ( ADCL_SUCCESS .ne. ierror) print *, "request_create not successful"   

    do i = 1, cnt
       call set_data_1D ( sdata, rank, sdim)
       call set_data_1D ( rdata, -1, rdim)

!#ifdef VERBOSE
!       call dump_vector_1D ( sdata, rank, sdim)
!       call dump_vector_1D ( rdata, rank, rdim)
!#endif

       call adcl_request_start( request, ierror )
       if ( ADCL_SUCCESS .ne. ierror) then
           print *, "request_start not successful"   
           stop
       endif 
       call check_data_1D ( rdata, rcnts, displ, rank, size, ierror )
       if ( ADCL_SUCCESS .ne. ierror) print *, "check_data_1D not successful"   
    end do


    call adcl_vector_deregister( svec, ierror )
    call adcl_vector_deregister( rvec, ierror )
    call MPI_Barrier ( MPI_COMM_WORLD, ierror)

    if ( ADCL_SUCCESS .ne. ierror) print *, "ADCL ierror nr.", ierror

    deallocate(sdata, rdata)

    if ( ADCL_REQUEST_NULL .ne. request) call adcl_request_free ( request, ierror )
    if ( ADCL_VMAP_NULL    .ne. svmap)   call adcl_vmap_free ( svmap, ierror )
    if ( ADCL_VMAP_NULL    .ne. rvmap)   call adcl_vmap_free ( rvmap, ierror )
    return

end subroutine allgatherv_test2

!*******************************************************************************
subroutine allgatherv_test3(cnt, dims, rank, size, topo)
!*******************************************************************************

    implicit none
    include 'ADCL.inc'
    integer, intent(in) :: cnt, dims, rank, size, topo 
    double precision, allocatable, dimension(:) :: data
    integer sdim, rdim, i 
    integer rcnts(size), displ(size)
    integer err, errc, offset 
    integer svec, rvec
    integer svmap, rvmap
    integer request
    integer ierror

    sdim = dims+rank

    rdim = dims*size + size*(size-1)/2

    allocate(data(rdim))

    offset = 0
    do i = 1, size
       rcnts(i) = dims+i-1
       displ(i) = offset
       offset   = offset + rcnts(i)
    end do 

    call adcl_vmap_inplace_allocate( svmap, ierror ) 
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_inplace_allocate not successful"   
    call adcl_vmap_list_allocate( size, rcnts, displ, rvmap, ierror ) 
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_list_allocate not successful"   

    call adcl_vector_register_generic ( 0, 0, 0, svmap, MPI_DATATYPE_NULL, MPI_IN_PLACE, svec, ierror )
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_vector_register for sdim not successful"   
    call adcl_vector_register_generic ( 1,  rdim, 0, rvmap, MPI_DOUBLE_PRECISION, data, rvec, ierror )
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_vector_register for rdim not successful"   

    call adcl_request_create_generic ( svec, rvec, topo, ADCL_FNCTSET_ALLGATHERV, request, ierror )
    if ( ADCL_SUCCESS .ne. ierror) print *, "request_create not successful"   

    do i = 1, cnt
       call set_data_1D ( data, -1, rdim)
       call set_data_1D ( data(displ(rank+1)+1), rank, sdim)

!#ifdef VERBOSE
!       call dump_vector_1D ( data, rank, sdim)
!#endif

       call adcl_request_start( request, ierror )
       if ( ADCL_SUCCESS .ne. ierror) then
           print *, "request_start not successful"   
           stop
       endif 
       call check_data_1D ( data, rcnts, displ, rank, size, ierror )
       if ( ADCL_SUCCESS .ne. ierror) print *, "check_data_1D not successful"   
    end do


    call adcl_vector_deregister( svec, ierror )
    call adcl_vector_deregister( rvec, ierror )
    call MPI_Barrier ( MPI_COMM_WORLD, ierror)

    if ( ADCL_SUCCESS .ne. ierror) print *, "ADCL ierror nr.", ierror

    deallocate(data)

    if ( ADCL_REQUEST_NULL .ne. request) call adcl_request_free ( request, ierror )
    if ( ADCL_VMAP_NULL    .ne. svmap)   call adcl_vmap_free ( svmap, ierror )
    if ( ADCL_VMAP_NULL    .ne. rvmap)   call adcl_vmap_free ( rvmap, ierror )
    return

end subroutine allgatherv_test3

