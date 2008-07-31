!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
         subroutine System_Matmul_blocking ( vekt, erg_vekt, ierror )

!....Diese Routine fuehrt eine Matrix-Vektor Multiplikation aus.

         USE globale_daten
         USE matrix

         implicit none

         double precision, dimension(0:n1+1, 0:n2+1, 0:n3+1, nc ):: vekt
         double precision, dimension(dmiu:dmio, dmju:dmjo, dmku:dmko, nc)::erg_vekt

!...Hilfsgroessen und Laufvariablen

         integer :: i, j, k
         integer :: oben, ierror

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
               do i = 1, dmio
                  erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                      rmb00(i,j,k,1,1) * vekt(i-1,j,k,1) 
               end do
            end do
         end do

!        write (*,*) mytid, ' :::: After rmb00 multiplication'

!...Multiplikation mit d3

         if ( rand_ab ) then
            oben = dmio -1
         else
            oben = dmio
         end if
         
         do k = dmku, dmko
            do j = dmju, dmjo
               do i = dmiu, oben
                  erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                      rmf00(i,j,k,1,1)*vekt(i+1,j,k,1)
               end do
            end do
         end do
         
!         write (*,*) mytid, ' :::: After rmf00 multiplication'
!...Multiplikation mit d4

         do k = dmku, dmko
            do j = 1, dmjo
               do i = dmiu, dmio
                  erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                      rm0b0(i,j,k,1,1)*vekt(i,j-1,k,1) 
               end do
            end do
         end do

!         write (*,*) mytid, ' :::: After rm0b0 multiplication'
!...Multiplikation mit d5

         if ( rand_zu ) then
            oben = dmjo -1
         else
            oben = dmjo
         end if
         
         do k = dmku, dmko
            do j = dmju, oben
               do i = dmiu, dmio
                  erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) +  &
                      rm0f0(i,j,k,1,1)*vekt(i,j+1,k,1) 
               end do
            end do
         end do
         
!         write (*,*) mytid, ' :::: After rm0f0 multiplication'

!...Multiplikation mit d6

         do k = 1, dmko
            do j = dmju, dmjo
               do i = dmiu, dmio
                  erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                      rm00b(i,j,k,1,1)*vekt(i,j,k-1,1) 
               end do
            end do
         end do
       
!...Multiplikation mit d7
       
         if ( rand_symo ) then
            oben = dmko -1
         else
            oben = dmko
         end if
         
         do k = dmku, oben
            do j = dmju, dmjo
               do i = dmiu, dmio
                  erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) +  &
                      rm00f(i,j,k,1,1)*vekt(i,j,k+1,1) 
               end do
            end do
         end do
       

         ierror = 0

         return
         end














