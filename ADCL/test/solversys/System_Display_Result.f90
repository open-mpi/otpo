!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
         subroutine System_Display_Result ( ierror )

!...Darstellung des Loesungsergebnisses, unabhaengig von dem
!   verwendeten Gleichungloesern

         USE globale_daten
         USE matrix
         USE rechte_seite
         USE ergebnis
         USE numerik_daten
         USE timing
         USE constants
         USE loesung

         implicit none

         include 'mpif.h'
         
         integer :: i, j, k, ierror
         integer :: rank, size 

         double precision    :: res_max, zwischenspeicher, betrag
         double precision    :: l0, l2, res
         double precision, dimension (dmiu:dmio, dmju:dmjo, dmku:dmko, nc ) :: &
             tmp_vect, tmp_vect_2
         

         call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror )
         call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )
         l0  = 0.0
         l2  = 0.0
         res = 0.0

!...Berechnen des Residuums, sowie Speichern des groessten 
!   Elements

         call System_Matmul ( adcl_req_dq, dq, tmp_vect, ierror )

         res_max = 0
         do k = dmku, dmko
            do j = dmju, dmjo
               do i = dmiu, dmio
                  tmp_vect_2 (i, j, k, 1 ) = rhs(i, j, k, 1 ) - &
                                        tmp_vect(i, j, k, 1)
                  betrag = ABS ( tmp_vect_2(i, j, k, 1 ) )
                  if ( betrag.gt.res_max) then
                     res_max = betrag
                  end if
                  l2 = l2 + tmp_vect_2 (i, j, k, 1 ) * &
                       tmp_vect_2 (i, j, k, 1 ) 
               end do
            end do
         end do

         call MPI_Reduce ( l2, zwischenspeicher, 1, MPI_DOUBLE_PRECISION,  &
                  MPI_SUM, 0, MPI_COMM_WORLD, ierror )

         l2 = SQRT ( zwischenspeicher )

         
         call MPI_Reduce ( res_max, l0, 1, MPI_DOUBLE_PRECISION,  &
                  MPI_MAX, 0, MPI_COMM_WORLD, ierror )

         if ( loesung_bekannt ) then

            do k = dmku, dmko
               do j = dmju, dmjo
                  do i = dmiu, dmio
                     res = res + ( dq(i, j, k, 1) - loes(i, j, k, 1) )&
                          * ( dq(i, j, k, 1) - loes(i, j, k, 1))
                  end do
               end do
            end do

            call MPI_Allreduce ( res, zwischenspeicher, 1, MPI_DOUBLE_PRECISION, &
                 MPI_SUM, MPI_COMM_WORLD, ierror )

            res = SQRT ( zwischenspeicher)

         end if



!...Knoten Null macht nun die Bildschirmausgabe
         if ( rank.eq.0 ) then

            solvtime  = solv_ende - solv_anfang

            write (*,*)
            write (*,*) '=================================================='
            write (*,99) set_text
            write (*,*) 'Problem size :', n1g, ' x ', n2g, ' x ', n3g
            write (*,99) solver_text
            write (*,99) precon_text            
            write (*,*) '=================================================='
            write (*,*)
            write (*,*) 'Number of iterations          : ', r_nit
            write (*,*) 'Required residuum             : ', tol
            write (*,*) 'L2 - Residuum                 : ', l2
            write (*,*) 'Largest single  proc. residuum: ', l0
            if ( loesung_bekannt ) then
            write (*,*) 'Deviation from correct result : ', res
            end if
            write (*,*) 
            write (*,*) '=================================================='
            write (*,*) 'Timing'
            write (*,*) 'Time for solving the equations    :',  &
                        solvtime 
               write (*,*) '=================================================='
            end if


99       format (a)

         end


