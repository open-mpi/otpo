!
! Copyright (c) 2007-2009      University of Houston. All rights reserved.
! Copyright (c) 2009           HLRS. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!

! simple routines
! 
! SET_DATA: initializes the domain with rank and the halos with -1
! DUMP_VECTOR: prints 2d array
! DUMP_VECTOR_MPI: prints 2d array ordered by rank
! CHECK_DATA: checks data set by SET_DATA

! more sophisticated routines:
! * work for 2D and 2D + nc > 0
!   if 2d array without nc, reshape array with 3rd dimension 1 before calling those functions
! * number data array continously over all processes
!
! set_data_2D_plus: numbers the domain continously over all processes and sets halos to -1
! check_data_2D_plus_cont: for checking 2d arrays with / without nc 
! function check_region_2D_plus_cont: internally used by check_data_2D_plus_cont
!    checks all data in one direction i.e. on one edge (and corner if defined) 

!****************************************************************************
!****************************************************************************
!****************************************************************************
module auxdata2df
   use adcl
contains

      subroutine SET_DATA ( arr, rank, dims, hwidth )

        implicit none

        integer rank, dims(2), hwidth
        DATATYPE arr(dims(1),dims(2))

        integer i, j

        do i=1,hwidth
           do j = 1, dims(2)
              arr(i,j) = -1
           end do
        end do

        do i = dims(1)-hwidth+1, dims(1)
           do j = 1, dims(2)
              arr(i, j) = -1
           end do
        end do

        do i=1,dims(1)
           do j = 1, hwidth
              arr(i,j) = -1
           end do
        end do

        do i = 1, dims(1)
           do j = dims(2)-hwidth+1, dims(2)
              arr(i, j) = -1
           end do
        end do


        do i = hwidth+1, dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              arr(i,j) = rank
           end do
        end do

        return
      end subroutine SET_DATA 

!****************************************************************************
!****************************************************************************
!****************************************************************************
      subroutine DUMP_VECTOR ( arr, rank, dims )

        implicit none
        include 'ADCL.inc'

        integer rank, dims(2)
        DATATYPE arr(dims(1), dims(2))
        integer i, j

        do j = 1, dims(1)
           write (*,*) rank, (arr(j,i), i=1,dims(2))
        end do
        return
      end subroutine DUMP_VECTOR


!****************************************************************************
!****************************************************************************
!****************************************************************************
      subroutine DUMP_VECTOR_MPI ( arr, dims, comm )

        implicit none
        include 'ADCL.inc'

        integer dims(2), comm
        DATATYPE arr(dims(1), dims(2))
        integer i, j, iproc, rank, size, ierror
        character(100) ::  fmt 

        write (fmt,'(a,i0,a)') "(i3,a1,i5,a1,", dims(1), "f12.5)"

        call MPI_Comm_rank ( comm, rank, ierror )
        call MPI_Comm_size ( comm, size, ierror )

        do iproc = 0, size-1
           if ( iproc .eq. rank) then
              do j = 1, dims(2)
                 write (*,fmt) rank, ":", j, ":", (arr(i,j), i=1,dims(1))
              end do
           end if 
           call MPI_Barrier ( comm, ierror )
        end do
        return
      end subroutine DUMP_VECTOR_MPI


!****************************************************************************
!****************************************************************************
!****************************************************************************

      subroutine CHECK_DATA ( arr, rank, dims, hwidth, neighbors ) 

        implicit none
        include 'ADCL.inc'

        integer rank, dims(2), hwidth, neighbors(4)
        DATATYPE arr(dims(1),dims(2))
        integer lres, gres, i, j, ierr
        DATATYPE should_be
        
        lres = 1
      
        if ( neighbors(1) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(1)
        endif
        do i=1,hwidth
           do j = hwidth+1, dims(2)-hwidth
              if ( arr(i,j) .ne. should_be ) then
                 lres = 0 
              endif
           end do
        end do

        if ( neighbors(2) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(2)
        endif
        do i = dims(1)-hwidth+1, dims(1)
           do j = hwidth+1, dims(2)-hwidth
              if ( arr(i, j) .ne. should_be ) then 
                 lres = 0
              endif
           end do
        end do


        if ( neighbors(3) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(3)
        endif
        do i=hwidth+1,dims(1)-hwidth
           do j = 1, hwidth
              if ( arr(i,j) .ne. should_be ) then
                 lres = 0
              endif
           end do
        end do

        if ( neighbors(4) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(4)
        endif
        do i = hwidth+1, dims(1)-hwidth
           do j = dims(2)-hwidth+1, dims(2)
              if ( arr(i, j) .ne. should_be ) then 
                 lres = 0
              endif
           end do
        end do


        do i = hwidth+1, dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              if ( arr(i,j) .ne. rank ) then 
                 lres = 0
              endif
           end do
        end do



        call MPI_Allreduce ( lres, gres, 1, MPI_INTEGER, MPI_MIN, MPI_COMM_WORLD, ierr)
        if ( gres .eq. 0 ) then
           call DUMP_VECTOR ( arr, rank, dims )
           if ( rank .eq. 0 ) then
              write (*,*) '2-D Fortran testsuite hwidth =', hwidth, &
                   'nc = ', 0, ' failed'  
           end if
        else
           if ( rank .eq. 0 ) then
              write (*,*) '2-D Fortran testsuite hwidth =', hwidth, &
                   'nc = ', 0, ' passed'  
           end if
        end if


        return
      end subroutine CHECK_DATA


!!****************************************************************************
! obsolete: merged to set_data_2d_plus_cont, can be also used for 2d arrays by reshaping the array with 3rd dimension 1
!
!subroutine set_data_2D_cont ( data, rank, dims, hwidth, cart_comm )
!!****************************************************************************
!   implicit none
!   include 'ADCL.inc'
!
!   integer, intent(in) :: rank, hwidth, cart_comm
!   integer, dimension(2), intent(in) :: dims
!   double precision, intent(inout) :: data(dims(1),dims(2))
!   
!   integer :: i, j, ierr
!   integer, dimension(2) :: coords, cart_dims, period, dims_wo_halos
!
!   call MPI_Cart_get(cart_comm, 2, cart_dims, period, coords, ierr)
!   
!   ! lower halo cells and corners
!   do j = 1, dims(2)
!      do i=1,hwidth
!         data(i, j) = -1
!       end do
!   end do
!   
!   ! upper halo cells and corners 
!   do j = 1, dims(2)
!      do i = dims(1)-hwidth+1, dims(1)
!         data(i, j) = -1
!      end do
!   end do
!   
!   ! left halo cells
!   do j = 1, hwidth 
!      do i=1,dims(1)
!         data(i,j) = -1
!      end do
!   end do
!   
!   ! right halo cells
!   do j = dims(2)-hwidth+1, dims(2)
!      do i = 1, dims(1)
!         data(i, j) = -1
!      end do
!   end do
!   
!   dims_wo_halos = dims - 2*hwidth 
! 
!   !beaware of the change in i and j in the equation compare to the c code. (i-1) and (j-1)
!   do j = hwidth+1, dims(2)-hwidth
!      do i = hwidth+1, dims(1)-hwidth
!         data(i,j) = i-hwidth + dims_wo_halos(1)*coords(1) +   & 
!                     dims_wo_halos(1)*cart_dims(1) * ( dims_wo_halos(2)*coords(2) + j-hwidth-1 ) 
!      end do
!   end do
!
!   return
!end subroutine set_data_2D_cont

!***************************************************************************
subroutine set_data_2D_plus_cont ( data, rank, dims, hwidth, cart_comm)
! numbers the data array continously over all processes with / without nc 
! if 2d array without nc, reshape array with 3rd dimension 1 before calling this function
!
! dims      - dimension of array <data>, 3rd component is nc or 1
! hwidth    - #halo cells
! neighbors - list of neighboring processes 

   implicit none
   include 'ADCL.inc'

   integer, intent(in) :: rank, hwidth, cart_comm
   integer, dimension(3), intent(in) :: dims
   double precision, intent(inout) :: data(dims(1),dims(2),dims(3))

   integer :: i, j, k, ierr
   integer, dimension(2) :: coords, cart_dims, period, dims_wo_halos

   call MPI_Cart_get(cart_comm, 2, cart_dims, period, coords, ierr)
   ! cube: cart_dims with index coords
   ! inside cube: dims with index (i,j,k)

   call MPI_Cart_get(cart_comm, 2, cart_dims, period, coords, ierr)
   
   ! lower halo cells and corners
   do k=1, dims(3) 
      do j = 1, dims(2)
         do i=1,hwidth
            data(i,j,k) = -1
          end do
      end do
   end do
 
   ! upper halo cells and corners 
   do k=1, dims(3) 
      do j = 1, dims(2)
         do i = dims(1)-hwidth+1, dims(1)
            data(i,j,k) = -1
         end do
      end do
   end do
   
   ! left halo cells
   do k=1, dims(3) 
      do j = 1, hwidth 
         do i=1,dims(1)
            data(i,j,k) = -1
         end do
      end do
   end do
   
   ! right halo cells
   do k=1, dims(3) 
      do j = dims(2)-hwidth+1, dims(2)
         do i = 1, dims(1)
            data(i,j,k) = -1
         end do
      end do
   end do

   !be aware of the change in i and j in the equation compare to the c code. (i-1) and (j-1)
   ! without hwidth: 
   !    in x direction offset dims(1)*coords(1) + i
   !    in y direction offset dims(1)*cart_dims(1) * ( dims(2)*coords(2) + j-1 )
   !    in z direction offset dims(1)*cart_dims(1) * dims(2)*cart_dims(2) * ( dims(3)*coords(3) + z-1 )
   dims_wo_halos(1:2) = dims(1:2) - 2*hwidth 

   do k=1,dims(3)
      do j=hwidth+1, dims(2)-hwidth
         do i=hwidth+1, dims(1)-hwidth
            !print *, "rank =", rank, ", coords = ( ", coords, " )"
            data(i,j,k) = i-hwidth + dims_wo_halos(1)*coords(1) +                                                     & 
                          dims_wo_halos(1)*cart_dims(1) * ( ( dims_wo_halos(2)*coords(2) + j-hwidth-1 ) +             & 
                                         dims_wo_halos(2)*cart_dims(2) * ( k-1 ) )
          end do  
      end do
   end do

   return
end subroutine set_data_2D_plus_cont


!****************************************************************************
function check_data_2D_plus_cont ( data, rank, dims, hwidth, neighbors, cart_comm ) result (isok)
! for checking 2d arrays with / without nc 
! if 2d array without nc, reshape array with 3rd dimension 1 before calling this function
!
! dims      - dimension of array <data>, 3rd component is nc or 1
! hwidth    - #halo cells
! neighbors - list of neighboring processes 
   implicit none
   include 'ADCL.inc'

   integer, intent(in) :: rank, hwidth, cart_comm
   integer, dimension(3), intent(in) :: dims
   integer, dimension(8), intent(in) :: neighbors
   double precision, intent(in) :: data(dims(1),dims(2),dims(3))
   logical :: isok

   integer :: i, j, k, ierr, lres=1, gres, nerror, prod, ierror
   integer, dimension(2) :: coords, n_coords, c_coords, cart_size, period
   double precision should_be

   integer :: x_direction, y_direction

   ! check for each of the 9 possible locations
   do y_direction = -1, 1
      do x_direction = -1, 1
         prod = x_direction * y_direction
#ifdef INCCORNER
           if ( prod .ne. 0 ) then
               ! corner
               lres = check_region_2D_plus_cont (x_direction, y_direction, data, rank, cart_comm, & 
                      dims, hwidth, neighbors)
           endif 
#endif
           if ( prod == 0) then
               ! edge, face or inside
               lres = check_region_2D_plus_cont (x_direction, y_direction, data, rank, cart_comm, &
                      dims, hwidth, neighbors )
           end if
           call MPI_Allreduce ( lres, gres, 1, MPI_INTEGER, MPI_MIN, MPI_COMM_WORLD, ierror )
           if ( gres .ne. 1 ) then 
              isok = .false.
              return 
           endif 
       end do
   end do

   isok = .true.
   return 

   !if ( gres .eq. 1 ) then
   !    if ( rank == 0 )  then
   !        write(*,'(1x,a,i0,a)') "2D C testsuite: hwidth = ", hwidth, ", nc = 0 passed"
   !    end if
   !else 
   !    if ( rank == 0 ) then
   !        write(*,'(1x,a,i0,a)') "2D C testsuite: hwidth = ", hwidth, ", nc = 0 failed"
   !    end if
   !    call dump_vector_2D_mpi_dp ( data, dims, cart_comm )
   !endif

   return

end function check_data_2D_plus_cont

!!****************************************************************************
! obsolete: merged to check_region_2d_plus_cont, can be also used for 2d arrays by reshaping the array with 3rd dimension 1
! 
!function check_region_2D_cont (x_direction, y_direction, data, rank, cart_comm, dims, hwidth, neighbors ) result(lres)
!! checks all data in one direction i.e. on one edge (and corner if defined) 
!    integer, intent(in) :: x_direction, y_direction, rank, cart_comm
!    integer, intent(in) :: dims(2)                          ! size of one cube
!    integer, intent(in) :: hwidth, neighbors(8)
!    !double precision, intent(in) :: data (dims(1),dims(2))
!    double precision :: data (dims(1),dims(2))
!    integer :: lres
!
!    integer :: i, j, k, ierr
!    double precision :: should_be
!    integer :: cart_dims(2), dims_wo_halos(2), neighbor_ind
!    integer :: coords(2), n_coords(2), c_coords(2) ! coords, coords of neighbor and corrected coords of MPI process
!    integer :: period(2)      ! not really used
!    integer :: compensate(2)  ! what do I have to add / substract to my coordinate on the 
!                              ! neighboring process to compare the values in the halo cell and 
!                              ! in the domain of the neighboring process
!                              ! be aware of shift (hwidth+1,hwidth+1,hwidth+1) is no. 1 !  
!    integer :: loopstart(2), loopend(2)  ! defines part of data to check
!    logical :: neighbor_cond  ! is there a neighbor in this direction? 
!
!    lres = 1
!
!    call MPI_Cart_get (cart_comm, 2, cart_dims, period, coords, ierr)
!    neighbor_cond = .false.
!
!    select case (x_direction)
!       case (0)
!            loopstart(1)     = hwidth+1
!            loopend(1)       = dims(1)-hwidth
!            compensate(1)    = -hwidth
!            c_coords(1)      = coords(1)
!       case (-1)
!            loopstart(1) = 1
!            loopend(1) = hwidth
!            compensate(1) = dims(1) - 3*hwidth ! 2*hwidth for positioning and hwidth for shift of numbering
!       case (1)
!            loopstart(1)     = dims(1) - hwidth + 1
!            loopend(1)       = dims(1) 
!            compensate(1)    = - dims(1) + hwidth 
!    end select
!
!    select case (y_direction)
!        case (0)
!            loopstart(2) = hwidth+1
!            loopend(2) = dims(2) - hwidth
!            compensate(2) = -hwidth 
!            c_coords(2) = coords(2)
!        case (-1)
!            loopstart(2) = 1
!            loopend(2)   = hwidth
!            compensate(2) = dims(2) - 3*hwidth 
!        case (1)
!            loopstart(2)  = dims(2) - hwidth + 1 
!            loopend(2)    = dims(2)
!            compensate(2) = - dims(2) + hwidth 
!    end select
!
!    if     ( x_direction .eq. -1 .and. y_direction .eq. -1 ) then; neighbor_ind = 5
!    elseif ( x_direction .eq.  0 .and. y_direction .eq. -1 ) then; neighbor_ind = 3
!    elseif ( x_direction .eq.  1 .and. y_direction .eq. -1 ) then; neighbor_ind = 7
!    elseif ( x_direction .eq. -1 .and. y_direction .eq.  0 ) then; neighbor_ind = 1 
!    elseif ( x_direction .eq.  0 .and. y_direction .eq.  0 ) then; neighbor_ind = 0
!    elseif ( x_direction .eq.  1 .and. y_direction .eq.  0 ) then; neighbor_ind = 2
!    elseif ( x_direction .eq. -1 .and. y_direction .eq.  1 ) then; neighbor_ind = 8
!    elseif ( x_direction .eq.  0 .and. y_direction .eq.  1 ) then; neighbor_ind = 4
!    elseif ( x_direction .eq.  1 .and. y_direction .eq.  1 ) then; neighbor_ind = 6
!    endif
!
!    if ( neighbor_ind .eq. 0 ) then 
!       c_coords(1) = coords(1)
!       c_coords(2) = coords(2)
!       neighbor_cond = .true.
!    else
!       if (neighbors(neighbor_ind) .ne. MPI_PROC_NULL ) then
!           call MPI_Cart_coords (cart_comm, neighbors(neighbor_ind), 2, n_coords)
!           c_coords(1) = n_coords(1)
!           c_coords(2) = n_coords(2)
!           neighbor_cond = .true.
!       endif
!    endif
!    dims_wo_halos = dims - 2*hwidth 
!
!    do  j= loopstart(2), loopend(2)
!        do  i = loopstart(1), loopend(1)
!            if ( neighbor_cond ) then 
!                should_be = i + compensate(1) + dims_wo_halos(1)*c_coords(1) +               & 
!                        dims_wo_halos(1)*cart_dims(1) *  ( dims_wo_halos(2)*c_coords(2) + j-1 + compensate(2) )
!            else
!                should_be = -1
!            endif
!
!            if ( data(i,j) .ne. should_be ) then
!                lres = 0
!                write(*,'(i4,a,2I4,a,f12.4,a,f12.4,a,2i3)') rank, ": data(",i,j,") = ", data(i,j), & 
!                   ", should_be, ", should_be, ", direction =", x_direction, y_direction
!            end if
!            if (data(i,j) .eq. -5) print*, "double check"
!            data(i,j) = -5
!
!         end do
!    end do
!    return
!
!end function check_region_2D_cont


function check_region_2D_plus_cont (x_direction, y_direction, data, rank, cart_comm, dims, hwidth, neighbors) result(lres)
! checks all data in one direction i.e. on one edge (and corner if defined) 
    integer, intent(in) :: x_direction, y_direction, rank, cart_comm
    integer, intent(in) :: dims(3)                          ! size of one cube
    integer, intent(in) :: hwidth, neighbors(8)
    !double precision, intent(in) :: data (dims(1),dims(2),dims(3))
    double precision :: data (dims(1),dims(2),dims(3))
    integer :: lres

    integer :: i, j, k, ierr
    double precision :: should_be
    integer :: cart_dims(2), dims_wo_halos(2), neighbor_ind
    integer :: coords(2), n_coords(2), c_coords(2) ! coords, coords of neighbor and corrected coords of MPI process
    integer :: period(2)      ! not really used
    integer :: compensate(2)  ! what do I have to add / substract to my coordinate on the 
                              ! neighboring process to compare the values in the halo cell and 
                              ! in the domain of the neighboring process
                              ! be aware of shift (hwidth+1,hwidth+1,hwidth+1) is no. 1 !  
    integer :: loopstart(2), loopend(2)  ! defines part of data to check
    logical :: neighbor_cond  ! is there a neighbor in this direction? 

    lres = 1

    call MPI_Cart_get (cart_comm, 2, cart_dims, period, coords, ierr)
    neighbor_cond = .false.

    select case (x_direction)
       case (0)
            loopstart(1)     = hwidth+1
            loopend(1)       = dims(1)-hwidth
            compensate(1)    = -hwidth
            c_coords(1)      = coords(1)
       case (-1)
            loopstart(1) = 1
            loopend(1) = hwidth
            compensate(1) = dims(1) - 3*hwidth ! 2*hwidth for positioning and hwidth for shift of numbering
       case (1)
            loopstart(1)     = dims(1) - hwidth + 1
            loopend(1)       = dims(1) 
            compensate(1)    = - dims(1) + hwidth 
    end select

    select case (y_direction)
        case (0)
            loopstart(2) = hwidth+1
            loopend(2) = dims(2) - hwidth
            compensate(2) = -hwidth 
            c_coords(2) = coords(2)
        case (-1)
            loopstart(2) = 1
            loopend(2)   = hwidth
            compensate(2) = dims(2) - 3*hwidth 
        case (1)
            loopstart(2)  = dims(2) - hwidth + 1 
            loopend(2)    = dims(2)
            compensate(2) = - dims(2) + hwidth 
    end select

    if     ( x_direction .eq. -1 .and. y_direction .eq. -1 ) then; neighbor_ind = 5
    elseif ( x_direction .eq.  0 .and. y_direction .eq. -1 ) then; neighbor_ind = 3
    elseif ( x_direction .eq.  1 .and. y_direction .eq. -1 ) then; neighbor_ind = 7
    elseif ( x_direction .eq. -1 .and. y_direction .eq.  0 ) then; neighbor_ind = 1 
    elseif ( x_direction .eq.  0 .and. y_direction .eq.  0 ) then; neighbor_ind = 0
    elseif ( x_direction .eq.  1 .and. y_direction .eq.  0 ) then; neighbor_ind = 2
    elseif ( x_direction .eq. -1 .and. y_direction .eq.  1 ) then; neighbor_ind = 8
    elseif ( x_direction .eq.  0 .and. y_direction .eq.  1 ) then; neighbor_ind = 4
    elseif ( x_direction .eq.  1 .and. y_direction .eq.  1 ) then; neighbor_ind = 6
    endif

    if ( neighbor_ind .eq. 0 ) then 
       c_coords(1) = coords(1)
       c_coords(2) = coords(2)
       neighbor_cond = .true.
    else
       if (neighbors(neighbor_ind) .ne. MPI_PROC_NULL ) then
           call MPI_Cart_coords (cart_comm, neighbors(neighbor_ind), 2, n_coords)
           c_coords(1) = n_coords(1)
           c_coords(2) = n_coords(2)
           neighbor_cond = .true.
       endif
    endif
    dims_wo_halos = dims(1:2) - 2*hwidth 

    do k = 1, dims(3)
       do j= loopstart(2), loopend(2)
          do i = loopstart(1), loopend(1)
             if ( neighbor_cond ) then 
                 should_be = i + compensate(1) + dims_wo_halos(1)*c_coords(1) +            &
                         dims_wo_halos(1)*cart_dims(1) *                                   & 
                                ( ( dims_wo_halos(2)*c_coords(2) + j-1 + compensate(2) ) + &
                                    dims_wo_halos(2)*cart_dims(2) * (k-1) )      
             else
                 should_be = -1
             endif

             if ( data(i,j,k) .ne. should_be ) then
                 lres = 0
                 write(*,'(i4,a,3I4,a,f12.4,a,f12.4,a,2i3)') rank, ": data(",i,j,k,") = ", data(i,j,k), & 
                    ", should_be, ", should_be, ", direction =", x_direction, y_direction
             end if
          end do
       end do
    end do

    return
end function check_region_2D_plus_cont


end module auxdata2df
