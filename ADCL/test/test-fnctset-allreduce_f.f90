!
!* Copyright (c) 2006-2007      University of Houston. All rights reserved.
!* $COPYRIGHT$
!*
!* Additional copyrights may follow
!*
!* $HEADER$
!*

!******************************************************************************
!******************************************************************************
program test_fnctset_allreduce
!******************************************************************************
!******************************************************************************

    implicit none
    include 'ADCL.inc'

    integer :: cnt, dims, err
    integer :: rank, size
    integer cdims, periods, ierror

    integer topo
    integer cart_comm

    cdims = 0
    periods = 0

    call MPI_Init ( ierror )
    call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror )
    call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )

    call adcl_init ( ierror )

    call MPI_Dims_create ( size, 1, cdims, ierror )
    call MPI_Cart_create ( MPI_COMM_WORLD, 1, cdims, periods, 0, cart_comm, ierror )

    call adcl_topology_create ( cart_comm, topo, ierror )

    cnt = 10 !200
    dims = 3
    
    !MPI_DOUBLE, MPI_SUM, Vector_register
    !call allreduce_test1(cnt, dims, rank, size, topo)

    !MPI_INT, MPI_MIN, Vector_register
    !call allreduce_test2(cnt, dims, rank, size, topo)

    !MPI_DOUBLE, MPI_SUM, Vector_register, MPI_IN_PLACE
    call allreduce_test3(cnt, dims, rank, size, topo)

    if ( ADCL_TOPOLOGY_NULL .ne. topo)  call adcl_topology_free ( topo, ierror )
    call MPI_Comm_free ( cart_comm, ierror )
    call adcl_finalize ( ierror )
    call MPI_Finalize ( ierror )

end program test_fnctset_allreduce


!******************************************************************************
!******************************************************************************
subroutine allreduce_test1(cnt, dim, rank, size, topo)
!******************************************************************************
!******************************************************************************

    implicit none
    include 'ADCL.inc'

    double precision, allocatable, dimension (:) :: sdata, rdata
    integer :: i, ierror, dim, cnt, size, topo, rank
    integer :: svec, rvec
    integer :: svmap, rvmap
    integer request
    
    allocate(sdata(dim), rdata(dim))

    call adcl_vmap_allreduce_allocate( MPI_SUM, svmap, ierror ) 
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_allreduce_allocate not successful"   
    call adcl_vmap_allreduce_allocate( MPI_SUM, rvmap, ierror ) 
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_allreduce_allocate not successful"   

    call adcl_vector_register_generic ( 1,  dim, 0, svmap, MPI_DOUBLE_PRECISION, sdata, svec, ierror )
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_vector_register for sdim not successful"   
    call adcl_vector_register_generic ( 1,  dim, 0, rvmap, MPI_DOUBLE_PRECISION, rdata, rvec, ierror )
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_vector_register for rdim not successful"   

    call adcl_request_create_generic ( svec, rvec, topo, ADCL_FNCTSET_ALLREDUCE, request, ierror )
    if ( ADCL_SUCCESS .ne. ierror) print *, "request_create not successful"   


    do i = 1, cnt
       call set_data_double ( sdata, rank, dim);
       call set_data_double ( rdata, -1,   dim);

!#ifdef VERBOSE
!*      dump_vector_double ( sdata, rank, dim);
!*      dump_vector_double ( rdata, rank, dim);
!#endif

       call adcl_request_start( request, ierror );
       if ( ADCL_SUCCESS .ne. ierror) then
           print *, "request_start not successful"   
           stop
       endif 

    call check_data_double_sum ( rdata, rank, dim, size, ierror )
       if ( ADCL_SUCCESS .ne. ierror) print *, "check_data_double_sum not successful"   
    end do

    call adcl_vector_deregister( svec, ierror )
    call adcl_vector_deregister( rvec, ierror )

    call MPI_Barrier ( MPI_COMM_WORLD, ierror );


    if ( ADCL_SUCCESS .ne. ierror) then 
      print *, "ADCL ierror nr.", ierror
    end if
    deallocate (sdata, rdata)



    if ( ADCL_REQUEST_NULL .ne. request) call adcl_request_free ( request, ierror )
    if ( ADCL_VMAP_NULL    .ne. svmap)   call adcl_vmap_free ( svmap, ierror )
    if ( ADCL_VMAP_NULL    .ne. rvmap)   call adcl_vmap_free ( rvmap, ierror )

    return


end subroutine allreduce_test1


!******************************************************************************
!******************************************************************************
subroutine allreduce_test2( cnt, dim, rank, size, topo )
!******************************************************************************
!******************************************************************************

    implicit none
    include 'ADCL.inc'

    integer, allocatable, dimension(:) :: sdata, rdata
    integer :: i, ierror, dim, rank, size, topo, cnt
    integer :: svec, rvec
    integer :: svmap, rvmap
    integer :: request
    
    allocate(sdata(dim), rdata(dim))


    call adcl_vmap_allreduce_allocate( MPI_MIN, svmap, ierror ) 
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_allreduce_allocate not successful"   
    call adcl_vmap_allreduce_allocate( MPI_MIN, rvmap, ierror ) 
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_allreduce_allocate not successful"   

    call adcl_vector_register_generic ( 1,  dim, 0, svmap, MPI_INTEGER, sdata, svec, ierror )
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_vector_register for sdim not successful"   
    call adcl_vector_register_generic ( 1,  dim, 0, rvmap, MPI_INTEGER, rdata, rvec, ierror )
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_vector_register for rdim not successful"   

    call adcl_request_create_generic ( svec, rvec, topo, ADCL_FNCTSET_ALLREDUCE, request, ierror )
    if ( ADCL_SUCCESS .ne. ierror) print *, "request_create not successful" 


    do i = 1, cnt
       call set_data_int ( sdata, size-rank, dim)
       call set_data_int ( rdata, -1,   dim)

!#ifdef VERBOSE
!*      dump_vector_int ( sdata, rank, dim);
!*      dump_vector_int ( rdata, rank, dim);
!#endif

       call adcl_request_start( request, ierror )
       if ( ADCL_SUCCESS .ne. ierror) then
           print *, "request_start not successful"   
           stop
       endif 
       call check_data_int_min ( rdata, rank, dim, size, ierror )
       if ( ADCL_SUCCESS .ne. ierror) print *, "check_data_int_min not successful"   
   end do


    call adcl_vector_deregister( svec, ierror )
    call adcl_vector_deregister( rvec, ierror )
    call MPI_Barrier ( MPI_COMM_WORLD, ierror )

    if ( ADCL_SUCCESS .ne. ierror ) then 
      print *, "ADCL ierror nr.", ierror
    end if

    deallocate(sdata, rdata)

    if ( ADCL_REQUEST_NULL .ne. request) call adcl_request_free ( request, ierror )
    if ( ADCL_VMAP_NULL    .ne. svmap)   call adcl_vmap_free ( svmap, ierror )
    if ( ADCL_VMAP_NULL    .ne. rvmap)   call adcl_vmap_free ( rvmap, ierror)
    return

end subroutine allreduce_test2


!******************************************************************************
!******************************************************************************
subroutine allreduce_test3(cnt, dim, rank, size, topo)
!******************************************************************************
!******************************************************************************

    implicit none
    include 'ADCL.inc'

    double precision, allocatable, dimension (:) :: data
    integer :: i, ierror, dim, cnt, size, topo, rank
    integer :: svec, rvec
    integer :: svmap, rvmap
    integer request
    
    allocate(data(dim))

    call adcl_vmap_inplace_allocate( svmap, ierror ) 
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_inplace_allocate not successful"   
    call adcl_vmap_allreduce_allocate( MPI_SUM, rvmap, ierror ) 
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_allreduce_allocate not successful"   

    call adcl_vector_register_generic ( 0, 0, 0, svmap, MPI_DATATYPE_NULL, MPI_IN_PLACE, svec, ierror )
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_vector_register for sdim not successful"   
    call adcl_vector_register_generic ( 1,  dim, 0, rvmap, MPI_DOUBLE_PRECISION, data, rvec, ierror )
    if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_vector_register for rdim not successful"   

    call adcl_request_create_generic ( svec, rvec, topo, ADCL_FNCTSET_ALLREDUCE, request, ierror )
    if ( ADCL_SUCCESS .ne. ierror) print *, "request_create not successful"   


    do i = 1, cnt
       call set_data_double ( data, rank, dim);

!#ifdef VERBOSE
!*      dump_vector_double ( data, rank, dim);
!#endif

       call adcl_request_start( request, ierror );
       if ( ADCL_SUCCESS .ne. ierror) then
           print *, "request_start not successful"   
           stop
       endif 

    call check_data_double_sum ( data, rank, dim, size, ierror )
       if ( ADCL_SUCCESS .ne. ierror) print *, "check_data_double_sum not successful"   
    end do

    call adcl_vector_deregister( svec, ierror )
    call adcl_vector_deregister( rvec, ierror )

    call MPI_Barrier ( MPI_COMM_WORLD, ierror );


    if ( ADCL_SUCCESS .ne. ierror) then 
      print *, "ADCL ierror nr.", ierror
    end if
    deallocate (data)



    if ( ADCL_REQUEST_NULL .ne. request) call adcl_request_free ( request, ierror )
    if ( ADCL_VMAP_NULL    .ne. svmap)   call adcl_vmap_free ( svmap, ierror )
    if ( ADCL_VMAP_NULL    .ne. rvmap)   call adcl_vmap_free ( rvmap, ierror )

    return


end subroutine allreduce_test3


!******************************************************************************
!******************************************************************************
subroutine check_data_double_sum ( data, rank, dim, size, ierror) 
!******************************************************************************
!******************************************************************************

    implicit none
    include "mpif.h"

    integer i
    integer, intent(in) :: rank, dim, size
    integer, intent(out) :: ierror
    double precision, dimension(dim), intent(in) :: data
    integer :: gerr = 0

   

    do i = 1, dim
      if ( data(i) .ne. (size * (size-1))/2) then
	print *, "Wrong data: proc ", rank, ", pos ", i, ", value ", data(i)
	ierror = ierror + 1
      end if
    end do
    
    call MPI_Allreduce (ierror, gerr, 1, MPI_INTEGER, MPI_SUM, MPI_COMM_WORLD, ierror )
    if ( gerr .eq. 0 ) then 
       !if ( rank .eq. 0 ) print *, "1-D testsuite passed"
    else 
       if ( rank .eq. 0 ) print *, "1-D testsuite failed"
       ierror = 1
    end if 

    return


end subroutine check_data_double_sum 


!******************************************************************************
!******************************************************************************
subroutine check_data_int_min ( data, rank, dim, size, ierror) 
!******************************************************************************
!******************************************************************************
    implicit none
    include "mpif.h"

    integer i
    integer, intent(in) :: rank, dim, size
    integer, intent(out) :: ierror
    integer, dimension(dim), intent(in) :: data
    integer :: gerr = 0

    do i = 1, dim
      if ( data(i) .ne. 1) then
	print *, "Wrong data: proc ", rank, ", pos ", i, ", value ", data(i)
	ierror = ierror + 1
      end if
    end do
    
    call MPI_Allreduce (ierror, gerr, 1, MPI_INTEGER, MPI_SUM, MPI_COMM_WORLD, ierror )
    if ( gerr .eq. 0 ) then 
       !if ( rank .eq. 0 ) print *, "1-D testsuite passed"
    else 
       if ( rank .eq. 0 ) print *, "1-D testsuite failed"
       ierror = 1
    end if 
    return


end subroutine check_data_int_min



!******************************************************************************
!******************************************************************************
subroutine set_data_double ( data, rank, dim )
!******************************************************************************
!******************************************************************************

implicit none
    integer, intent(in) :: rank, dim
    double precision, dimension(dim), intent(out) :: data
    integer i

    do i = 1, dim
       data(i) = rank 
    end do

    return

end subroutine set_data_double


!******************************************************************************
!******************************************************************************
subroutine dump_vector_double ( data, rank, dim)
!******************************************************************************
!******************************************************************************

    implicit none
    integer, intent(in) :: rank, dim
    double precision, dimension(dim), intent(in) :: data
    integer i

    print *, rank, ": ", data

    return

end subroutine dump_vector_double


!******************************************************************************
!******************************************************************************
subroutine set_data_int ( data, rank, dim) 
!******************************************************************************
!******************************************************************************

implicit none
    integer, intent(in) :: rank, dim
    integer, dimension(dim), intent(out) :: data
    integer i

    do i = 1, dim
       data(i) = rank 
    end do

    return

end subroutine set_data_int


!******************************************************************************
!******************************************************************************
subroutine dump_vector_int ( data, rank, dim)
!******************************************************************************
!******************************************************************************

    implicit none
    integer, intent(in) :: rank, dim
    integer, dimension(dim), intent(in) :: data
    integer i

    print *, rank, ": ", data

    return

end subroutine dump_vector_int

