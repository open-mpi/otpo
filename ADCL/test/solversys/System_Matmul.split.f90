!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
         subroutine System_Matmul_init ( vekt, erg_vekt, ierror )

!....Diese Routine fuehrt eine Matrix-Vektor Multiplikation aus.
!
!    Diese Version ist in vier Teile aufgeteilt: 
!
!    1. Initiieren der Kommunikation
!    2. Berechnung der internen Werte 
!    3. Beenden der Kommunikation
!    4. Berechnen der Randwerte
!
!    Dies stellt die Stufe zwei dar....

         USE globale_daten
         USE matrix

         implicit none

         double precision, dimension(0:n1+1, 0:n2+1, 0:n3+1, nc ):: vekt
         double precision, dimension(dmiu:dmio, dmju:dmjo, dmku:dmko, nc)::erg_vekt

!...Hilfsgroessen und Laufvariablen

         integer :: i, j, k
         integer :: ierror

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
               do i = 2, dmio
                  erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                      rmb00(i,j,k,1,1)*vekt(i-1,j,k,1) 
               end do
            end do
         end do

!...Multiplikation mit d3
         
         do k = dmku, dmko
            do j = dmju, dmjo
               do i = dmiu, dmio-1
                  erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                      rmf00(i,j,k,1,1)*vekt(i+1,j,k,1)
               end do
            end do
         end do
         
!...Multiplikation mit d4

         do k = dmku, dmko
            do j = 2, dmjo
               do i = dmiu, dmio
                  erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                      rm0b0(i,j,k,1,1)*vekt(i,j-1,k,1) 
               end do
            end do
         end do

!...Multiplikation mit d5

         do k = dmku, dmko
            do j = dmju, dmjo-1
               do i = dmiu, dmio
                  erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) +  &
                      rm0f0(i,j,k,1,1)*vekt(i,j+1,k,1) 
               end do
            end do
         end do
         
!...Multiplikation mit d6

         do k = 2, dmko
            do j = dmju, dmjo
               do i = dmiu, dmio
                  erg_vekt(i,j,k,1) = erg_vekt(i,j,k,1) + &
                      rm00b(i,j,k,1,1)*vekt(i,j,k-1,1) 
               end do
            end do
         end do
       
!...Multiplikation mit d7
       
         do k = dmku, dmko-1
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

!*****************************************************************
!*****************************************************************
!*****************************************************************
!*****************************************************************
!*****************************************************************

         subroutine System_Matmul_end ( vekt, erg_vekt, ierror )

!....Diese Routine fuehrt eine Matrix-Vektor Multiplikation aus.
!
!    Diese Version ist in vier Teile aufgeteilt: 
!
!    1. Initiieren der Kommunikation
!    2. Berechnung der internen Werte 
!    3. Beenden der Kommunikation
!    4. Berechnen der Randwerte
!
!    Dies stellt die Stufe vier dar....

         USE globale_daten
         USE matrix

         implicit none

         double precision, dimension(0:n1+1, 0:n2+1, 0:n3+1, nc ):: vekt
         double precision, dimension(dmiu:dmio, dmju:dmjo, dmku:dmko, nc)::erg_vekt

!...Hilfsgroessen und Laufvariablen

         integer :: i, j, k
         integer :: oben, ierror

!...Multiplikation mit d2

         do k = dmku, dmko
            do j = dmju, dmjo
               erg_vekt(1,j,k,1) = erg_vekt(1,j,k,1) + &
                      rmb00(1,j,k,1,1)*vekt(0,j,k,1) 
            end do
         end do

!        write (*,*) mytid, ' :::: After rmb00 multiplication'

!...Multiplikation mit d3

         if ( rand_ab ) then
            oben = dmio -1
         else
            oben = dmio
         
         do k = dmku, dmko
            do j = dmju, dmjo
               erg_vekt(oben,j,k,1) = erg_vekt(oben,j,k,1) + &
                      rmf00(oben,j,k,1,1)*vekt(oben+1,j,k,1)
            end do
         end do
         end if
         
!         write (*,*) mytid, ' :::: After rmf00 multiplication'
!...Multiplikation mit d4

         do k = dmku, dmko
            do i = dmiu, dmio
               erg_vekt(i,1,k,1) = erg_vekt(i,1,k,1) + &
                      rm0b0(i,1,k,1,1)*vekt(i,0,k,1) 
            end do
         end do

!         write (*,*) mytid, ' :::: After rm0b0 multiplication'
!...Multiplikation mit d5

         if ( rand_zu ) then
            oben = dmjo -1
         else
            oben = dmjo
         
         do k = dmku, dmko
            do i = dmiu, dmio
               erg_vekt(i,oben,k,1) = erg_vekt(i,oben,k,1) +  &
                    rm0f0(i,oben,k,1,1)*vekt(i,oben+1,k,1) 
            end do
         end do
         end if
         
!         write (*,*) mytid, ' :::: After rm0f0 multiplication'

!...Multiplikation mit d6

         do j = dmju, dmjo
            do i = dmiu, dmio
               erg_vekt(i,j,1,1) = erg_vekt(i,j,1,1) + &
                    rm00b(i,j,1,1,1)*vekt(i,j,0,1) 
            end do
         end do
       
!...Multiplikation mit d7
       
         if ( rand_symo ) then
            oben = dmko -1
         else
            oben = dmko
         
         do j = dmju, dmjo
            do i = dmiu, dmio
               erg_vekt(i,j,oben,1) = erg_vekt(i,j,oben,1) +  &
                      rm00f(i,j,oben,1,1)*vekt(i,j,oben+1,1) 
            end do
         end do
         end if
       

         ierror = 0

         return
         end

	subroutine set_vekt ( vekt, id )

        USE globale_daten

	implicit none
	include 'mpif.h'

	double precision, dimension(0:n1+1, 0:n2+1, 0:n3+1, nc ):: vekt 
	integer :: i, j, k, id, count  

        count = id
	 do k = 0, dmko + 1
            do j = 0, dmjo + 1
               do i = 0, dmio + 1
                vekt(i,j,k,1)  = count
                count = count + 1
               end do
            end do
         end do

       end subroutine set_vekt


	subroutine dump_vekt ( vekt, id )

        USE globale_daten

	implicit none
	include 'mpif.h'

	double precision, dimension(0:n1+1, 0:n2+1, 0:n3+1, nc ):: vekt 
	integer :: i, j, k, rank, ierror, id  
        character*80 name

        call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror )

        write(name,99) rank,'_', id, '.out'
99      format (I1,a1,i1,a4)
        open ( unit=10, file =name )
        write (10,*) "New entry"
        write (10,*) "============================"
	 do k = 0, dmko + 1
            do j = 0, dmjo + 1
               do i = 0, dmio + 1
                write (10,*) rank, ':', i, j, k, vekt(i,j,k,1)  
               end do
            end do
         end do

         close (10)
    	end subroutine dump_vekt

       subroutine dump_vekt_short ( vekt, id )

        USE globale_daten

        implicit none
        include 'mpif.h'

        double precision, dimension(dmiu:dmio, dmju:dmjo, dmku:dmko, nc ):: vekt
        integer :: i, j, k, rank, ierror, id
        character*80 name

        call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror )

        write(name,99) rank,'_', id, '.out'
99      format (I1,a1,i1,a4)
        open ( unit=10, file =name )
        write (10,*) "New entry"
        write (10,*) "============================"
         do k = dmku, dmko
            do j = dmju, dmjo
               do i = dmiu, dmio
                write (10,*) rank, ':', i, j, k, vekt(i,j,k,1)
               end do
            end do
         end do

         close (10)
        end subroutine dump_vekt_short


       subroutine dump_matrix ( id )

        USE globale_daten
        USE matrix
        implicit none
        include 'mpif.h'

!...Hilfsgroessen und Laufvariablen

        integer :: i, j, k, rank, ierror, id
        character*80 name

        call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror )

        write(name,99) rank,'_', id, '.out'
99      format (I1,a1,i1,a4)
        open ( unit=10, file =name )

        write (10,*) "rm000"
        write (10,*) "============================"
        do k = 1, dmko
           do j = 1, dmjo
              do i = 1, dmio
                 write (10,*) rank, ':', i, j, k, rm000(i,j,k,1,1)
               end do
            end do
         end do

!...Multiplikation mit d2

        write (10,*) "rmb00"
        write (10,*) "============================"
        do k = 1, dmko
           do j = 1, dmjo
              do i = 1, dmio
                 write (10,*) rank, ':', i, j, k, rmb00(i,j,k,1,1)
               end do
            end do
         end do

        write (10,*) "rm0b0"
        write (10,*) "============================"
        do k = 1, dmko
           do j = 1, dmjo
              do i = 1, dmio
                 write (10,*) rank, ':', i, j, k, rm0b0(i,j,k,1,1)
               end do
            end do
         end do

       write (10,*) "rm00b"
        write (10,*) "============================"
        do k = 1, dmko
           do j = 1, dmjo
              do i = 1, dmio
                 write (10,*) rank, ':', i, j, k, rm00b(i,j,k,1,1)
               end do
            end do
         end do

        write (10,*) "rmf00"
        write (10,*) "============================"
        do k = 1, dmko
           do j = 1, dmjo
              do i = 1, dmio
                 write (10,*) rank, ':', i, j, k, rmf00(i,j,k,1,1)
               end do
            end do
         end do

        write (10,*) "rm0f0"
        write (10,*) "============================"
        do k = 1, dmko
           do j = 1, dmjo
              do i = 1, dmio
                 write (10,*) rank, ':', i, j, k, rm0f0(i,j,k,1,1)
               end do
            end do
         end do

        write (10,*) "rm00f"
        write (10,*) "============================"
        do k = 1, dmko
           do j = 1, dmjo
              do i = 1, dmio
                 write (10,*) rank, ':', i, j, k, rm00f(i,j,k,1,1)
               end do
            end do
         end do

         ierror = 0

         return
         end
	     
