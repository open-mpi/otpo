!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
        subroutine System_Precon ( ierror )

!...Vorkonditionierung der Matrix durch ein einfaches Jakobi-
!   Verfahren. Dabei wird die Hauptdiagonale als einfachste 
!   Naeherung der Matrix genommen und invertiert.

        USE globale_daten
        USE matrix
        USE rechte_seite
        USE ergebnis
        USE numerik_daten

        implicit none
        
        integer :: ierror, i, j, k 
        double precision, dimension ( dmiu:dmio, dmju:dmjo, dmku:dmko, nc ) :: precon_m

        precon_text = ' Preconditioning : Jakobi'
        
!...Austellen und gleichzeitiges Invertieren der Preconditionierungsmatrix

        do k = dmku, dmko
           do j = dmju, dmjo
              do i = dmiu, dmio
                 if ( rm000(i, j, k, 1, 1 ).ne.0.0 ) then
                    precon_m ( i, j, k, 1 ) = 1.0 /  &
                        rm000(i, j, k, 1, 1 )
                 else
                    precon_m ( i, j, k, 1 ) = 0.0
                 end if
              end do
           end do
        end do

!...Multiplizieren der rechten Seite und der Matrixdiagonalen
!   mit der Preconditionierungsmatrix

        call System_Precon_Matset ( precon_m, ierror)

        return
        end
        
!*******************************************************************
!*******************************************************************
!*******************************************************************
!*******************************************************************
!*******************************************************************

        subroutine System_Precon_Matset ( precon_m , ierror )

!...Nach dem berechnen der Vorkonditionierungsmatrix, muss die Matrix
!   mit der inversen multipliziert werden. Aufgrund der einfachen 
!   Struktur der Matrix und der Vorkonditionierungsmatrix 
!   wird das Diagonale fuer Diagonale erledigt

        USE globale_daten
        USE matrix
        USE rechte_seite
        
        implicit none
       
        double precision, dimension ( dmiu:dmio, dmju:dmjo, dmku:dmko, nc ) :: & 
             precon_m


        integer :: i, j, k, ierror

        do k = dmku, dmko
           do j = dmju, dmjo
              do i = dmiu, dmio
                 rhs(i, j, k, 1) = rhs(i, j, k, 1) * &
                     precon_m ( i, j, k, 1 )
              end do
           end do
        end do

        do k = dmku, dmko
           do j = dmju, dmjo
              do i = dmiu, dmio
                 rm000(i, j, k, 1, 1) = rm000(i, j, k, 1, 1) * &
                     precon_m ( i, j, k, 1 )
                 rmb00(i, j, k, 1, 1) = rmb00(i, j, k, 1, 1) * &
                     precon_m ( i, j, k, 1 )
                 rmf00(i, j, k, 1, 1) = rmf00(i, j, k, 1, 1) * &
                     precon_m ( i, j, k, 1 )
                 rm0b0(i, j, k, 1, 1) = rm0b0(i, j, k, 1, 1) * &
                     precon_m ( i, j, k, 1 )
                 rm0f0(i, j, k, 1, 1) = rm0f0(i, j, k, 1, 1) * &
                     precon_m ( i, j, k, 1 )
                 rm00b(i, j, k, 1, 1) = rm00b(i, j, k, 1, 1) * &
                     precon_m ( i, j, k, 1 )
                 rm00f(i, j, k, 1, 1) = rm00f(i, j, k, 1, 1) * &
                     precon_m ( i, j, k, 1 )
              end do
           end do
        end do
!	write (*,*)
!	write (*,*) '================='
!        write (*,*) 'System_Precon()'
!	write (*,*) rm000(1,1,1,1,1), 'rm000(1,1,1,1,1)'
!	write (*,*) rmb00(1,1,1,1,1), 'rmb00(1,1,1,1,1)'
!	write (*,*) rmf00(1,1,1,1,1), 'rmf00(1,1,1,1,1)'
!	write (*,*) rm0b0(1,1,1,1,1), 'rm0b0(1,1,1,1,1)'
!	write (*,*) rm0f0(1,1,1,1,1), 'rm0f0(1,1,1,1,1)'
!	write (*,*) rm00b(1,1,1,1,1), 'rm00b(1,1,1,1,1)'
!	write (*,*) rm00f(1,1,1,1,1), 'rm00f(1,1,1,1,1)'
!	write (*,*) precon_m(1,1,1,1), 'precon_m(1,1,1,1,1)'
!	write (*,*) rhs(3,3,3,1), 'rhs(3,3,3,3,1)'
!	write (*,*)
!	write (*,*) '================='

        ierror = 0

        return
        end


