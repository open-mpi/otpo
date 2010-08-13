!
! Copyright (c) 2007-2009      University of Houston. All rights reserved.
! Copyright (c) 2008-2009      HLRS. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!

!*******************************************************************************
subroutine check_data_1D ( data, rcounts, rdispl, rank, size, ierror )
!*******************************************************************************
    implicit none
    include "mpif.h"
    integer, intent(in) :: rank, size
    integer, dimension(size) :: rcounts, rdispl
    double precision, dimension(rcounts(size)), intent(in) :: data
    integer, intent(out) :: ierror
    integer proc, j
    integer :: gerr = 0

    ierror = 0

    do proc = 1, size
       do j = 1, rcounts(proc)
           if (data( rdispl(proc)+j ) .ne. proc-1 ) then
               print *, "Wrong data: proc ", proc, ", pos ", rdispl(proc)+j,  &
                  ", value ", data( rdispl(proc)+j ), ", expected value ", proc
               ierror = ierror + 1
           end if
       end do
    end do

    call MPI_Allreduce ( ierror, gerr, 1, MPI_INTEGER, MPI_SUM, MPI_COMM_WORLD, ierror )
    if ( gerr .eq. 0 ) then
       !if ( rank .eq. 0 ) print *, "1-D testsuite passed"
    else
       if ( rank .eq. 0 ) print *, "1-D testsuite failed"
       ierror = 1
    end if

    return
end subroutine check_data_1D

!*******************************************************************************
subroutine set_data_1D ( data, value, dim)
!*******************************************************************************
    implicit none
    integer, intent(in) :: value, dim
    double precision, dimension(dim), intent(out) :: data
    integer i

    do i = 1, dim
       data(i) = value
    end do

    return
end subroutine set_data_1D

!*******************************************************************************
subroutine dump_vector_1D ( data, rank, dim)
!*******************************************************************************
    implicit none
    integer, intent(in) :: rank, dim
    double precision, dimension(dim), intent(in) :: data

    print *, rank, ": ", data

    return
end subroutine dump_vector_1D
