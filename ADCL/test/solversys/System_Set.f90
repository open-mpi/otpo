!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
        subroutine System_Set ( ierror )

!...Diskretisieren der Gleichung, belegen der Matrix und der
!   rechten Seite. Dieses File diskretisiert eine Laplacegleichung,
!   wobei ein Einheitsquader in n1g,n2g,n3g Punkte
!   unterteilt wird. Diskretisiert wird mit einer Zentralen-Differenz.

        USE globale_daten
        USE matrix
        USE rechte_seite
        USE numerik_daten

        implicit none

        integer :: ierror
        integer :: i, j, k 

        double precision :: hauptdiag, xdiag_1, xdiag_2, ydiag, zdiag

        set_text = ' Set: Beispiel 2 '
!...Berechnen des Abstandes zwischen zwei Punkten bei der 
!   Diskretisierung

        deltax = 1/(n1g - 1.0 )
        deltay = 1/(n2g - 1.0 )
        deltaz = 1/(n3g - 1.0 )
        
        deltaquadx = deltax * deltax
        deltaquady = deltay * deltay
        deltaquadz = deltaz * deltaz

        hauptdiag = (1.0/deltaquadx + 1.0/deltaquady + 1.0/deltaquadz)*2.0
        xdiag_1   = -(1.0/deltaquadx + 1000.0/(2.0*deltax) ) 
        xdiag_2   = -(1.0/deltaquadx - 1000.0/(2.0*deltax) )
        ydiag = -1.0/deltaquady
        zdiag = -1.0/deltaquadz

!...Kurze Darstellung der Gleichung:
!   - d^2 u / dx^2 - d^2 u / dy^2 - d^u / dz^2 + 1000 * du/dx = rhs
!
!   Aus der Zentralen Differenz folgt, dass auf der Hauptdiagonalen 
!   immer 6/deltaquad steht, und auf den Nebendiagonalen -1/deltaquad,

        do k = dmku, dmko
           do j = dmju, dmjo
              do i = dmiu, dmio
                 rm000(i, j, k, 1, 1 ) = hauptdiag
                 rmb00(i, j, k, 1, 1 ) = xdiag_1
                 rmf00(i, j, k, 1, 1 ) = xdiag_2
                 rm0b0(i, j, k, 1, 1 ) = ydiag
                 rm0f0(i, j, k, 1, 1 ) = ydiag
                 rm00b(i, j, k, 1, 1 ) = zdiag
                 rm00f(i, j, k, 1, 1 ) = zdiag
              end do
           end do
        end do


!...Korriegieren der Raender

        if ( rand_sing ) then
           do k = dmku, dmko
              do j = dmju, dmjo
                 rm000 ( dmiu, j, k, 1, 1 ) = 1.0
                 rmb00 ( dmiu, j, k, 1, 1 ) = 0.0
                 rmf00 ( dmiu, j, k, 1, 1 ) = 0.0
                 rm0b0 ( dmiu, j, k, 1, 1 ) = 0.0
                 rm0f0 ( dmiu, j, k, 1, 1 ) = 0.0
                 rm00b ( dmiu, j, k, 1, 1 ) = 0.0
                 rm00f ( dmiu, j, k, 1, 1 ) = 0.0
              end do
           end do
        end if

        if ( rand_ab ) then
           do k = dmku, dmko
              do j = dmju, dmjo
                 rm000 ( dmio, j, k, 1, 1 ) = 1.0
                 rmb00 ( dmio, j, k, 1, 1 ) = 0.0
                 rmf00 ( dmio, j, k, 1, 1 ) = 0.0
                 rm0b0 ( dmio, j, k, 1, 1 ) = 0.0
                 rm0f0 ( dmio, j, k, 1, 1 ) = 0.0
                 rm00b ( dmio, j, k, 1, 1 ) = 0.0
                 rm00f ( dmio, j, k, 1, 1 ) = 0.0
              end do
           end do
        end if
           
        if ( rand_festk ) then
           do k = dmku, dmko
              do i = dmiu, dmio
                 rm000 ( i, dmju, k, 1, 1 ) = 1.0
                 rmb00 ( i, dmju, k, 1, 1 ) = 0.0
                 rmf00 ( i, dmju, k, 1, 1 ) = 0.0
                 rm0b0 ( i, dmju, k, 1, 1 ) = 0.0
                 rm0f0 ( i, dmju, k, 1, 1 ) = 0.0
                 rm00b ( i, dmju, k, 1, 1 ) = 0.0
                 rm00f ( i, dmju, k, 1, 1 ) = 0.0
              end do
           end do
        end if
           
        if ( rand_zu ) then
           do k = dmku, dmko
              do i = dmiu, dmio
                 rm000 ( i, dmjo, k, 1, 1 ) = 1.0
                 rmb00 ( i, dmjo, k, 1, 1 ) = 0.0
                 rmf00 ( i, dmjo, k, 1, 1 ) = 0.0
                 rm0b0 ( i, dmjo, k, 1, 1 ) = 0.0
                 rm0f0 ( i, dmjo, k, 1, 1 ) = 0.0
                 rm00b ( i, dmjo, k, 1, 1 ) = 0.0
                 rm00f ( i, dmjo, k, 1, 1 ) = 0.0
              end do
           end do
        end if

        if ( rand_symu ) then
           do j = dmju, dmjo
              do i = dmiu, dmio
                 rm000 ( i, j, dmku, 1, 1 ) = 1.0
                 rmb00 ( i, j, dmku, 1, 1 ) = 0.0
                 rmf00 ( i, j, dmku, 1, 1 ) = 0.0
                 rm0b0 ( i, j, dmku, 1, 1 ) = 0.0
                 rm0f0 ( i, j, dmku, 1, 1 ) = 0.0
                 rm00b ( i, j, dmku, 1, 1 ) = 0.0
                 rm00f ( i, j, dmku, 1, 1 ) = 0.0
              end do
           end do
        end if
           
        if ( rand_symo ) then
           do j = dmju, dmjo
              do i = dmiu, dmio
                 rm000 ( i, j, dmko, 1, 1 ) = 1.0
                 rmb00 ( i, j, dmko, 1, 1 ) = 0.0
                 rmf00 ( i, j, dmko, 1, 1 ) = 0.0
                 rm0b0 ( i, j, dmko, 1, 1 ) = 0.0
                 rm0f0 ( i, j, dmko, 1, 1 ) = 0.0
                 rm00b ( i, j, dmko, 1, 1 ) = 0.0
                 rm00f ( i, j, dmko, 1, 1 ) = 0.0
              end do
           end do
        end if
           
!...Setzen der rechten Seite

        call System_Set_rhs ( ierror )

        ierror = 0

        return
      end subroutine System_Set

!***********************************************************************
!***********************************************************************
!***********************************************************************
!***********************************************************************
!***********************************************************************

        subroutine System_Set_rhs ( ierror )

!...Setzen der rechten Seite so, dass man die zum Schluss die
!   erwuenschte Loesung kennt.

          USE globale_daten
          USE matrix
          USE rechte_seite
          USE loesung
          USE constants

          implicit none
          include 'mpif.h'

          integer :: ierror, dummy, i, j, k
          integer :: size, rank
          integer, dimension(3) :: coords
          double precision, parameter :: my_pi = 3.1415926535897932384
          double precision :: x, y, z, xoffset, yoffset, zoffset
          double precision :: skalar_1, skalar_2, skalar_3

          call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror )
          call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )

!...Die korrekte Loesung ist in diesem Fall bekannt:

          loesung_bekannt = .true.

!...Bestimmen der eigenen Position im globalen Feld:
          call MPI_Cart_coords ( cart_comm, rank, 3, coords, ierror )

          xoffset = coords(1) * n1
          yoffset = coords(2) * n2
          zoffset = coords(3) * n3


!...Die Loesung soll spaeter folgender Funktion entsprechen
!   u(x, y, z ) = exp (xyz) * sin (pi*x) * sin(pi*y) * sin(pi*z)


          do k = dmku, dmko
             z = ( zoffset + k - 1) * deltaz
             if ( z.gt.1.0) then
                z = 1.0
             end if
             do j = dmju, dmjo
                y = ( yoffset + j - 1) * deltay 
                if ( y.gt.1.0) then
                   y = 1.0
                end if
                do i = dmiu, dmio
                   x = ( xoffset + i - 1) * deltax
                   if ( x.gt.1.0) then
                      x = 1.0
                   end if
                   loes (i, j, k, 1 ) = dexp (x*y*z) * &
                        dsin (my_pi*x) * dsin(my_pi*y) * &
                        dsin (my_pi*z)
                end do
             end do
          end do
18        format (3i3, 2x, f12.5)

          call System_Matmul ( adcl_req_loes, loes, rhs, ierror )

!...Korriegieren der Raender

        if ( rand_sing ) then
           do k = dmku, dmko
              do j = dmju, dmjo
                 rhs ( dmiu, j, k, 1 ) = 0.0
                 loes ( dmiu, j, k, 1 ) = 0.0
              end do
           end do
        end if

        if ( rand_ab ) then
           do k = dmku, dmko
              do j = dmju, dmjo
                 rhs ( dmio, j, k, 1 ) = 0.0
                 loes ( dmio, j, k, 1 ) = 0.0
              end do
           end do
        end if
           
        if ( rand_festk ) then
           do k = dmku, dmko
              do i = dmiu, dmio
                 rhs ( i, dmju, k, 1 ) = 0.0
                 loes ( i, dmju, k, 1 ) = 0.0
              end do
           end do
        end if
           
        if ( rand_zu ) then
           do k = dmku, dmko
              do i = dmiu, dmio
                 rhs ( i, dmjo, k, 1 ) = 0.0
                 loes ( i, dmjo, k, 1 ) = 0.0
              end do
           end do
        end if

        if ( rand_symu ) then
           do j = dmju, dmjo
              do i = dmiu, dmio
                 rhs  ( i, j, dmku, 1 ) = 0.0
                 loes ( i, j, dmku, 1 ) = 0.0
              end do
           end do
        end if
           
        if ( rand_symo ) then
           do j = dmju, dmjo
              do i = dmiu, dmio
                 rhs  ( i, j, dmko, 1 ) = 0.0
                 loes ( i, j, dmko, 1 ) = 0.0
              end do
           end do
        end if

        ierror = 0

        return
        end subroutine System_Set_rhs

        
