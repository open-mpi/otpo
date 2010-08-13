!
! Copyright (c) 2009           HLRS. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!

module auxdata3df
   use adcl
contains

!***************************************************************************
subroutine set_data_3D_cont ( data, rank, dims, hwidth, cart_comm)
! numbers the data array continously over all processes 

    implicit none
    include 'ADCL.inc'

    integer, intent(in) :: rank, hwidth, cart_comm
    integer, dimension(3), intent(in) :: dims
    double precision, intent(inout) :: data(dims(1),dims(2),dims(3))

    integer :: i, j, k, ierr
    integer, dimension(3) :: coords, cart_dims, dims_wo_halos
    logical, dimension(3) :: period

    call MPI_Cart_get(cart_comm, 3, cart_dims, period, coords, ierr)
    ! cube: cart_dims with index coords
    ! inside cube: dims with index (i,j,k)

    do i=1, dims(1)
        do j=1, dims(2)
            do k=1, hwidth
                data(i,j,k) = -1
            end do
            do k=dims(3)-hwidth+1, dims(3)
                data(i,j,k) = -1
           end do
       end do
   end do

    do i=1, dims(1)
        do k=1, dims(3)
            do j=1, hwidth
                data(i,j,k) = -1
           end do
            do j=dims(2)-hwidth+1, dims(2)
                data(i,j,k) = -1
           end do
       end do
   end do

    do j=1, dims(2)
        do k=1, dims(3)
            do i=1, hwidth
                data(i,j,k) = -1
           end do
            do i=dims(1)-hwidth+1, dims(1)
                data(i,j,k) = -1
           end do
       end do
   end do

    !be aware of the change in i and j in the equation compare to the c code. (i-1) and (j-1)
    ! without hwidth: 
    !    in x direction offset dims(1)*coords(1) + i
    !    in y direction offset dims(1)*cart_dims(1) * ( dims(2)*coords(2) + j-1 )
    !    in z direction offset dims(1)*cart_dims(1) * dims(2)*cart_dims(2) * ( dims(3)*coords(3) + z-1 )
    dims_wo_halos = dims - 2*hwidth 

    do  i=hwidth+1, dims(1)-hwidth
        do j=hwidth+1, dims(2)-hwidth
            do k=hwidth+1, dims(3)-hwidth
                !print *, "rank =", rank, ", coords = ( ", coords, " )"
                data(i,j,k) = i-hwidth + dims_wo_halos(1)*coords(1) +                                                     & 
                              dims_wo_halos(1)*cart_dims(1) * ( ( dims_wo_halos(2)*coords(2) + j-hwidth-1 ) +             & 
                                             dims_wo_halos(2)*cart_dims(2) * ( dims_wo_halos(3)*coords(3) + k-hwidth-1 ) )
           end do  
       end do
    end do

    return
end subroutine set_data_3D_cont

!****************************************************************************
function check_data_3D_cont ( data, rank, dims, hwidth, neighbors, cart_comm ) result(isok)

   implicit none
   include 'ADCL.inc'

   integer, intent(in) :: rank, hwidth, cart_comm
   integer, dimension(3), intent(in) :: dims
   integer, dimension(6), intent(in) :: neighbors
   double precision, intent(in) :: data(dims(1),dims(2), dims(3))
   logical :: isok

   integer :: i, j, k, ierr, lres=1, gres, prod, ierror
   integer, dimension(3) :: coords, n_coords, c_coords, cart_size
   double precision should_be

   integer :: x_direction, y_direction, z_direction 

   ! check for each of the 27 possible locations
   do z_direction = -1, 1
        do y_direction = -1, 1
            do x_direction = -1, 1
                prod = x_direction * y_direction * z_direction  
#ifdef INCCORNER
                if ( prod .ne. 0 ) then
                    ! corner
                    lres = check_region_3D (x_direction, y_direction, z_direction, data, rank, cart_comm, & 
                           dims, hwidth, neighbors)
                endif 
#endif
                if ( prod == 0) then
                    ! edge, face or inside
                    lres = check_region_3D (x_direction, y_direction, z_direction, data, rank, cart_comm, &
                           dims, hwidth, neighbors )
                end if
                call MPI_Allreduce ( lres, gres, 1, MPI_INTEGER, MPI_MIN, cart_comm, ierror )
                if ( gres .ne. 1 ) then 
                   isok = .false.
                   return 
                endif 
            end do
        end do
    end do

    !if ( gres .eq. 1 ) then
    !    if ( rank == 0 )  then
    !        write(*,'(1x,a,i0,a)') "3-D C testsuite: hwidth = ", hwidth, ", nc = 0 passed"
    !    end if
    !else 
    !    if ( rank == 0 ) then
    !        write(*,'(1x,a,i0,a)') "3-D C testsuite: hwidth = ", hwidth, ", nc = 0 failed"
    !    end if
    !    call dump_vector_3D_mpi_dp ( data, dims, cart_comm )
    !endif

   isok = .true.
   return

end function check_data_3D_cont

!****************************************************************************

function check_region_3D (x_direction, y_direction, z_direction, data, rank, cart_comm, dims, hwidth, neighbors) result(lres)
! checks all data in one direction i.e. on one face, edge (and corner if defined) 
    integer, intent(in) :: x_direction, y_direction, z_direction, rank, cart_comm
    integer, intent(in) :: dims(3)                          ! size of one cube
    integer, intent(in) :: hwidth, neighbors(6)
    double precision, intent(in) :: data (dims(1), dims(2), dims(3) )
    integer :: lres

    integer :: i, j, k, ierr
    double precision :: should_be
    integer :: cart_dims(3), dims_wo_halos(3)
    integer :: coords(3), n_coords(3), c_coords(3) ! coords, coords of neighbor and corrected coords of MPI process
    logical, dimension(3) :: period ! not really used
    integer :: compensate(3)  ! what do I have to add / substract to my coordinate on the 
                              ! neighboring process to compare the values in the halo cell and 
                              ! in the domain of the neighboring process
                              ! be aware of shift (hwidth+1,hwidth+1,hwidth+1) is no. 1 !  
    integer :: loopstart(3), loopend(3)  ! defines part of data to check
    logical :: neighbor_cond(3)          ! is there a neighbor in x,y,z-direction?

    lres = 1

    call MPI_Cart_get (cart_comm, 3, cart_dims, period, coords, ierr)
    neighbor_cond = .false.

    select case (x_direction)
       case (0)
            loopstart(1)     = hwidth+1
            loopend(1)       = dims(1)-hwidth
            compensate(1)    = -hwidth
            c_coords(1)      = coords(1)
            neighbor_cond(1) = .true.
       case (-1)
            loopstart(1) = 1
            loopend(1) = hwidth
            compensate(1) = dims(1) - 3*hwidth ! 2*hwidth for positioning and hwidth for shift of numbering
            if (neighbors(1) .ne. MPI_PROC_NULL ) then
                call MPI_Cart_coords (cart_comm, neighbors(1), 3, n_coords, ierror )
                c_coords(1) = n_coords(1)
                neighbor_cond(1) = .true.
            endif
       case (1)
            loopstart(1)     = dims(1) - hwidth + 1
            loopend(1)       = dims(1) 
            compensate(1)    = - dims(1) + hwidth 
            if (neighbors(2) .ne. MPI_PROC_NULL ) then 
                call MPI_Cart_coords (cart_comm, neighbors(2), 3, n_coords, ierror )
                c_coords(1) = n_coords(1)
                neighbor_cond(1) = .true.
            endif
    end select

    select case (y_direction)
        case (0)
            loopstart(2) = hwidth+1
            loopend(2) = dims(2) - hwidth
            compensate(2) = -hwidth 
            c_coords(2) = coords(2)
            neighbor_cond(2) = .true.
        case (-1)
            loopstart(2) = 1
            loopend(2)   = hwidth
            compensate(2) = dims(2) - 3*hwidth 
            if (neighbors(3) .ne. MPI_PROC_NULL ) then
                call MPI_Cart_coords (cart_comm, neighbors(3), 3, n_coords, ierror )
                c_coords(2) = n_coords(2)
                neighbor_cond(2) = .true.
            endif
        case (1)
            loopstart(2)  = dims(2) - hwidth + 1 
            loopend(2)    = dims(2)
            compensate(2) = - dims(2) + hwidth 
            if (neighbors(4) .ne. MPI_PROC_NULL ) then
                call MPI_Cart_coords (cart_comm, neighbors(4), 3, n_coords, ierror )
                c_coords(2) = n_coords(2)
                neighbor_cond(2) = .true.
            endif
    end select

    select case (z_direction)
        case (0)
            loopstart(3)  = hwidth + 1
            loopend(3)    = dims(3) - hwidth
            compensate(3) = -hwidth 
            c_coords(3)   = coords(3)
            neighbor_cond(3) = .true.
        case (-1)
            loopstart(3)  = 1
            loopend(3)    = hwidth
            compensate(3) = dims(3) - 3*hwidth  
            if (neighbors(5) .ne. MPI_PROC_NULL ) then
                call MPI_Cart_coords (cart_comm, neighbors(5), 3, n_coords, ierror )
                c_coords(3) = n_coords(3)
                neighbor_cond(3) = .true.
            endif
        case (1)
            loopstart(3)  = dims(3) - hwidth + 1 
            loopend(3)    = dims(3) 
            compensate(3) = - dims(3) + hwidth
            if (neighbors(6) .ne. MPI_PROC_NULL ) then
                call MPI_Cart_coords (cart_comm, neighbors(6), 3, n_coords, ierror )
                c_coords(3) = n_coords(3)
                neighbor_cond(3) = .true.
            endif
    end select


   dims_wo_halos = dims - 2*hwidth 

    do k = loopstart(3), loopend(3)
       do  j= loopstart(2), loopend(2)
           do  i = loopstart(1), loopend(1)
               if (neighbor_cond(1) .and. neighbor_cond(2) .and. neighbor_cond(3)) then 
                  should_be = i + compensate(1) + dims_wo_halos(1)*c_coords(1) +               & 
                       dims_wo_halos(1)*cart_dims(1) *  & 
                                ( ( dims_wo_halos(2)*c_coords(2) + j-1 + compensate(2) ) +                      & 
                                    dims_wo_halos(2)*cart_dims(2) * ( dims_wo_halos(3)*c_coords(3) + k-1 + compensate(3) ) )
               else
                   should_be = -1
               end if 
               if ( data(i,j,k) .ne. should_be ) then
                   lres = 0
                   write(*,'(i4,a,3I4,a,f12.4,a,f12.4,a,3i3)') rank, ": data(",i,j,k,") = ", data(i,j,k), & 
                      ", should_be, ", should_be, ", direction =", x_direction, y_direction, z_direction
               end if
            end do
        end do
    end do
    return

end function check_region_3D


!****************************************************************************
      subroutine DUMP_VECTOR ( arr, rank, dims )

        implicit none
        include 'ADCL.inc'
        character(100) ::  fmt

        integer rank, dims(3)
        DATATYPE arr(dims(1), dims(2), dims(3))
        integer i, j, k

        write (fmt,'(a,i0,a)') "(i3,a1,2i5,a1,", dims(1), "f12.5)"
        do k = 1, dims(3)
           do j = 1, dims(2)
              write (*,fmt) rank, ":", k, j, ":", (arr(i,j,k), i=1,dims(1))
           end do
        end do

        return
      end subroutine DUMP_VECTOR


!***************************************************************************
      subroutine DUMP_VECTOR_MPI ( arr, dims, comm )

        implicit none
        include 'ADCL.inc'

        integer dims(3), comm
        DATATYPE arr(dims(1), dims(2), dims(3))
        integer i, j, k, iproc, rank, size, ierror
        character(100) ::  fmt 

        call MPI_Comm_rank ( comm, rank, ierror )
        call MPI_Comm_size ( comm, size, ierror )

        write (fmt,'(a,i0,a)') "(i3,a1,2i5,a1,", dims(1), "f12.5)"

        do iproc = 0, size-1
           if ( iproc .eq. rank) then
              do k = 1, dims(3)
                 do j = 1, dims(2)
                    write (6,fmt) rank, ":", k, j, ":", (arr(i,j,k), i=1,dims(1))
                 end do
              end do
           end if 
           call MPI_Barrier ( comm, ierror )
        end do
        return
      end subroutine DUMP_VECTOR_MPI

end module auxdata3df

