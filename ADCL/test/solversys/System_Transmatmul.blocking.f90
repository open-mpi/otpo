!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
        subroutine System_Transmatmul_blocking ( vekt, erg_vekt, ierror )

!...Algorithmus einer Matrix-Transponiert Vektor Multiplikation

        USE globale_daten
        USE matrix
        USE trans_mat

        implicit none

        integer :: ierror

        double precision, dimension (0:n1+1, 0:n2+1, 0:n3+1, nc) :: vekt 
        double precision, dimension (dmiu: dmio, dmju:dmjo, dmku:dmko, nc ) :: &
         erg_vekt

!...Hilfsgroessen und Laufvariablen

        integer :: i, j, k

!...Multiplikation mit d1

        do k = dmku, dmko
           do j = dmju, dmjo
              do i = dmiu, dmio
                 erg_vekt(i,j,k,1) = rm000(i,j,k,1,1)*vekt(i,j,k,1)  
              end do
           end do
        end do

!...Multiplikation mit d2

        do k = dmku, dmko
           do j = dmju, dmjo
              do i = dmiu, dmio
                 if ( i.ne.dmio ) then
                    erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                          rmb00(i+1,j,k,1,1)*vekt(i+1,j,k,1) 
                 else
                    if ( .not.rand_ab ) then
                       erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                          rmb00temp(1,j,k,1,1)*vekt(i+1,j,k,1) 
                      end if
                 end if
              end do
           end do
        end do


!...Multiplikation mit d3

        do k = dmku, dmko
           do j = dmju, dmjo
              do i = dmiu, dmio
                 if ( i.ne.dmiu ) then
                    erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                          rmf00(i-1,j,k,1,1)*vekt(i-1,j,k,1) 
                 else
                    if ( .not. rand_sing) then
                       erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                             rmf00temp(1,j,k,1,1)*vekt(i-1,j,k,1) 
                    end if
                 end if
              end do
           end do
        end do

!...Multiplikation mit d4

        do k = dmku, dmko
           do j = dmju, dmjo
              do i = dmiu, dmio
                 if ( j.ne.dmjo) then
                    erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                           rm0b0(i,j+1,k,1,1)*vekt(i,j+1,k,1) 
                 else
                    if ( .not.rand_zu ) then
                       erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                              rm0b0temp(i,1,k,1,1)*vekt(i,j+1,k,1) 
                    end if
                 end if
              end do
           end do
        end do

!...Multiplikation mit d5

        do k = dmku, dmko
           do j = dmju, dmjo
              do i = dmiu, dmio
                 if ( j.ne.dmju ) then
                      erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                          rm0f0(i,j-1,k,1,1)*vekt(i,j-1,k,1) 
                 else
                    if ( .not. rand_festk ) then
                       erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                             rm0f0temp(i,1,k,1,1)*vekt(i,j-1,k,1) 
                    end if
                 end if
              end do
           end do
        end do

!...Multiplikation mit d6

        do k = dmku, dmko
           do j = dmju, dmjo
              do i = dmiu, dmio
                 if ( k .ne. dmko ) then
                    erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                          rm00b(i,j,k+1,1,1)*vekt(i,j,k+1,1) 
                  else
                     if ( .not. rand_symo ) then
                        erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                             rm00btemp(i,j,1,1,1)*vekt(i,j,k+1,1) 
                     end if
                 end if
              end do
           end do
        end do

!...Multiplikation mit d7

        do k = dmku, dmko
           do j = dmju, dmjo
              do i = dmiu, dmio
                 if ( k .ne. dmku ) then
                    erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                          rm00f(i,j,k-1,1,1)*vekt(i,j,k-1,1) 
                 else
                    if ( .not.rand_symu ) then
                       erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                             rm00ftemp(i,j,1,1,1)*vekt(i,j,k-1,1) 
                    end if
                 end if
              end do
           end do
        end do


        ierror = 0
        
        return
        end



