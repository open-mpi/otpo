!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
        subroutine System_Solver_tfqmr ( nreal, ierror )

!...Implementierung eines QMR-Algorithmuses, basierend auf dem original
!   TFQMR-Paper von Freund (A Transpose-Free Quasi-Minimal Residual Algorithm for 
!   Non-Hermitian Linear Systems, SIAM Journal on Scientific Computing, vol. 14, 
!   issue 2, 1993). 
!   Grundgedanken: Aufbau einer Basis mit Hilfe des CGS-Algorihtmuses und
!   Bestimmung der naechsten Iterierten mit Hilfe der QMR-Property.
!   Vorteil gegenueber dem normalen QMR-Verfahren: keine Matrix_transponiert
!   -Vektor Multiplikation.


        USE globale_daten
        USE matrix
        USE rechte_seite
        USE ergebnis
        USE numerik_daten
        USE timing

        implicit none

        include 'ADCL.inc'
        
!...Variablen der Uebergabeklammer

        integer :: nreal, ierror

!...Variablen des Gleichungsloesers
        double precision tfqmr_limit
        parameter ( tfqmr_limit = 1.0e-09 )
        
        double precision :: tau, theta, theta_old, eta, eta_old  &
            ,tfqmr_rho, tfqmr_rho_old, tfqmr_sigma, alpha, tfqmr_c, beta
        double precision, dimension ( 0:n1+1, 0:n2+1, 0:n3+1, nc ) :: &
            tfqmr_y, tfqmr_y_old, tfqmr_y_old_1 
        double precision, dimension ( dmiu:dmio, dmju:dmjo, dmku:dmko, nc ) ::  &
            tfqmr_r, tfqmr_v, tfqmr_r_tilde, tfqmr_d, tfqmr_w


!...lokale Hilfsvariablen

        integer :: i, j, k, l, m
        integer :: rank, size
        double precision :: skalar_1, skalar_2, skalar_3 
        double precision, dimension ( 2 ) :: all_vekt, all_erg
        double precision, dimension ( dmiu:dmio, dmju:dmjo, dmku:dmko, nc ) ::  &
            res, tmp_vect_2
        double precision, dimension ( 0:n1+1, 0:n2+1, 0:n3+1, nc ) :: tmp_vect

!... Define the ADCL-objects
        integer, dimension(3) :: dims
        integer :: adcl_vec_tfqmr_y, adcl_vec_tfqmr_y_old, adcl_vec_tfqmr_y_old_1
        integer :: adcl_tfqmr_y, adcl_tfqmr_y_old, adcl_tfqmr_y_old_1

!        character *80 name;
!        double precision tbegin, tend
!        double precision, allocatable, dimension (:) :: allred, iter
!        integer acount, itercount, status

!       allocate (allred ( 6*nit ), iter ( nit ), stat=status )
!        if (status.gt.0 ) then 
!           write (*,*) rank, ' : Error allocating memory'
!        end if
!        
!        acount = 1
!        itercount = 1
        tfqmr_v = 0.0
        tfqmr_y = 0.0
        tfqmr_y_old = 0.0
        tfqmr_y_old_1 = 0.0

        dims(1) = n1+2
        dims(2) = n2+2
        dims(3) = n3+2
        call ADCL_Vector_register ( 3, dims, nc, ADCL_VECTOR_HALO, 1, MPI_DOUBLE_PRECISION, &
             tfqmr_y, adcl_vec_tfqmr_y, ierror )
        call ADCL_Request_create ( adcl_vec_tfqmr_y, adcl_topo, &
             ADCL_FNCTSET_NEIGHBORHOOD, adcl_tfqmr_y, ierror )

        call ADCL_Vector_register ( 3, dims, nc, ADCL_VECTOR_HALO, 1, MPI_DOUBLE_PRECISION, &
             tfqmr_y_old, adcl_vec_tfqmr_y_old, ierror )
        call ADCL_Request_create ( adcl_vec_tfqmr_y_old, adcl_topo, &
             ADCL_FNCTSET_NEIGHBORHOOD, adcl_tfqmr_y_old, ierror )

        call ADCL_Vector_register ( 3, dims, nc, ADCL_VECTOR_HALO, 1, MPI_DOUBLE_PRECISION, &
             tfqmr_y_old_1, adcl_vec_tfqmr_y_old_1, ierror )
        call ADCL_Request_create ( adcl_vec_tfqmr_y_old_1, adcl_topo, &
             ADCL_FNCTSET_NEIGHBORHOOD, adcl_tfqmr_y_old_1, ierror )

!...Initialisierung der Variablen
        
        call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror )
        call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )

        ! tmp_vect_2 = A * x_0
        call System_Matmul ( adcl_req_dq, dq, tmp_vect_2, ierror )
 
        ! r_0 = b - tmp_vect_2
        do l = 1, nc
           do k = dmku, dmko
              do j = dmju, dmjo
                 do i = dmiu, dmio
                    tfqmr_r (i, j, k, l ) = rhs ( i, j, k, l ) - &
                                      tmp_vect_2 ( i, j, k, l )
                 end do
              end do
           end do
        end do

        do l = 1, nc
           do k = dmku, dmko
              do j = dmju, dmjo
                 do i = dmiu, dmio
                    ! w == r_i^{CGS}
                    tfqmr_w ( i, j, k, l )       = tfqmr_r ( i, j, k, l)
                    tfqmr_y ( i, j, k, l )       = tfqmr_r ( i, j, k, l)
                    tfqmr_y_old ( i, j, k, l )   = tfqmr_r ( i, j, k, l)
                    tfqmr_y_old_1 (i, j, k, l )  = tfqmr_r ( i, j, k, l)
                    tfqmr_d ( i, j, k, l )       = 0.0
                    tfqmr_r_tilde ( i, j, k, l ) = 1.0
                 end do
              end do
           end do
        end do

        call System_Matmul ( adcl_tfqmr_y, tfqmr_y, tfqmr_v, ierror )
        
        ! || r || to calculate omega_1 and subsequently tau_0 
        skalar_1 = 0.0
        do l = 1, nc
           do k = dmku, dmko
              do j = dmju, dmjo
                 do i = dmiu, dmio
                   skalar_1 = skalar_1 + tfqmr_r ( i, j, k, l ) * &
                        tfqmr_r ( i, j, k, l ) 
                 end do
              end do
           end do
        end do
        
!...Der Allreduce wird mit einem weiteren zusammengelegt, und deshalb
!   weiter unten durchgefuehrt.
        all_vekt ( 1 ) = skalar_1

        theta     = 0.0
        theta_old = 0.0
        eta       = 0.0
        eta_old   = 0.0

!...Skalarprodukt: tfqmr_r_tilde^T * tfqmr_r
        skalar_1 = 0.0
        do l = 1, nc
           do k = dmku, dmko
              do j = dmju, dmjo
                 do i = dmiu, dmio
                   skalar_1 = skalar_1 + tfqmr_r_tilde ( i, j, k, l ) * &
                        tfqmr_r ( i, j, k, l ) 
                 end do
              end do
           end do
        end do

        all_vekt( 2 ) = skalar_1

        ! communicate || r || (all_vekt(1), for tau=sqrt(omega_1)) and 
        !     r_tilde^T * r_0 (all_vect(2), for rho_0) 
        comm_anfang = MPI_WTIME()
        call MPI_Allreduce ( all_vekt, all_erg, 2, MPI_DOUBLE_PRECISION, MPI_SUM,  &
             MPI_COMM_WORLD, ierror )
        comm_ende = MPI_WTIME()
        commtime = commtime + (comm_ende - comm_anfang) 

        ! tau_0 = omega_1, omega_1 = sqrt( || r_1 || )
        tau       = SQRT ( all_erg(1))

        ! rho_0 = r_tilde_0^H * r_0
        tfqmr_rho = all_erg(2)
        tfqmr_rho_old = tfqmr_rho

!        write ( name, 99) rank, '_app.out'
!99      format ( I2,a8)
!        open  ( unit=10, file=name)

!========================================================================
!...Beginn der Hauptiterationsschleife
        do 1000 nreal = 1, nit
!========================================================================
!           tbegin=MPI_WTIME()

           ! sigma_{n-1} = r_tilde_0^T * v_{n-1}
           skalar_1 = 0.0
           do l = 1, nc
              do k = dmku, dmko
                 do j = dmju, dmjo
                    do i = dmiu, dmio
                       skalar_1 = skalar_1 + tfqmr_r_tilde( i, j, k, l)* &
                           tfqmr_v ( i, j, k, l ) 
                    end do
                 end do
              end do
           end do
           
           comm_anfang = MPI_WTIME()
           call MPI_Allreduce ( skalar_1, tfqmr_sigma, 1, MPI_DOUBLE_PRECISION,  &
               MPI_SUM, MPI_COMM_WORLD, ierror )
           comm_ende = MPI_WTIME()
!           allred(acount) = comm_ende - comm_anfang
!           acount = acount + 1

!           if ( tfqmr_sigma.lt.tfqmr_limit ) then
!              write (*,*) rank, ' : Breakdown in TFQMR '
!              ierror = -1
!              call MPI_Abort ( MPI_COMM_WORLD, ierror, info )
!           end if
           
           ! alpha_{n-1} = rho_{n-1} / sigma_{n-1}
           alpha = tfqmr_rho_old / tfqmr_sigma
          
           ! q_n = u_{n-1} - alpha_{n-1} * v_{n-1} 
           do l = 1, nc
              do k = dmku, dmko
                 do j = dmju, dmjo
                    do i = dmiu, dmio
                       tfqmr_y_old(i, j, k, l)=tfqmr_y_old_1(i, j, k, l) &
                           - alpha * tfqmr_v (i, j, k, l )
                    end do
                 end do
              end do
           end do
           
!************************************************************************
!...Innere Schleife des CGS-Algorithmuses
           do 500 m = (2*nreal-1), (2*nreal)
!************************************************************************
              ! compute omega
              if ( m.eq.(2*nreal-1) ) then
                 ! tmp_vect_2 = A * u_{n-1} 
                 call System_Matmul (adcl_tfqmr_y_old_1,tfqmr_y_old_1,tmp_vect_2,ierror)
              else
                 ! tmp_vect_2 = A *  q_n
                 call System_Matmul(adcl_tfqmr_y_old,tfqmr_y_old,tmp_vect_2,ierror)
              end if

              ! compute residual
              ! ??? in theory: r_n = r_{n-1} - alpha_{n-1} A (u_{n-1} + q_n)  
              !     outside of inner loop
              ! here:  
              ! r_{n-1/2} = r_{n-1} - alpha * A * u_{n-1},  if m .eq. 2*nreal-1
              ! r_n = r_{n-1} - alpha * A * q_n    ,  if m .eq. 2*nreal 
              do l = 1, nc
                 do k = dmku, dmko
                    do j = dmju, dmjo
                       do i = dmiu, dmio
                          tfqmr_w ( i, j, k, l ) = tfqmr_w (i, j, k, l) &
                              - alpha * tmp_vect_2 ( i, j, k, l)
                       end do
                    end do
                 end do
              end do

              ! ??? in theory: 
              ! omega_{m+1} = || r_n ||                           if m .eq. 2*nreal-1
              ! omega_{m+1} = sqrt( || r_{n-1} || * || r_n || ),  if m .eq. 2*nreal 
              skalar_1 = 0.0
              do l = 1, nc
                 do k = dmku, dmko
                    do j = dmju, dmjo
                       do i = dmiu, dmio
                          skalar_1 = skalar_1 + tfqmr_w(i, j, k, l ) * &
                              tfqmr_w ( i, j, k, l)
                       end do
                    end do
                 end do
              end do

              comm_anfang = MPI_WTIME()
              call MPI_Allreduce ( skalar_1, skalar_2, 1, MPI_DOUBLE_PRECISION,  &
                   MPI_SUM, MPI_COMM_WORLD, ierror )
              comm_ende = MPI_WTIME()
!              allred(acount) = comm_ende - comm_anfang
!              acount = acount + 1


              skalar_3 = SQRT ( skalar_2 )
              
              ! theta = omega_{m+1} / tau_{m-1}
              theta = skalar_3/ tau
              ! c_m = 1 / sqrt( 1 + theta_m^2)
              tfqmr_c = 1 / SQRT ( 1+ theta * theta )
              ! tau_m = tau_{m-1} * theta_m * c_m
              tau = tau * theta * tfqmr_c
              ! eta_m = c_m^2 * alpha_{n-1}
              eta = tfqmr_c * tfqmr_c * alpha
              
              !  tmp_vect2 = (theta_{m-1}^2 * eta_{m-1} / alpha_{n-1} ) * d_{m-1}
              !  thetha_old == thetha_{m-1}, eta_old == eta_{m-1}, 
              !  alpha = alpha_{n-1} 
              do l = 1, nc
                 do k = dmku, dmko
                    do j = dmju, dmjo
                       do i = dmiu, dmio
                          tmp_vect_2 ( i, j, k, l ) = &
                              (theta_old * theta_old * eta_old /  &
                              alpha ) * tfqmr_d ( i, j, k, l )
                       end do
                    end do
                 end do
              end do


              if ( m.eq.(2*nreal-1)) then
                 ! d_m = u_m + tmp_vect_2, tmp_vect2 = (theta_{m-1}^2 * eta_{m-1} / alpha_{n-1} ) * d_{m-1}
                 do l = 1, nc
                    do k = dmku, dmko
                       do j = dmju, dmjo
                          do i = dmiu, dmio
                             tfqmr_d(i, j, k, l) = tfqmr_y_old_1(i,j,k,l) &
                                 + tmp_vect_2 ( i, j, k, l )
                          end do
                       end do
                    end do
                 end do
              else
                 ! d_m = q_n + tmp_vect_2
                 do l = 1, nc
                    do k = dmku, dmko
                       do j = dmju, dmjo
                          do i = dmiu, dmio
                             tfqmr_d(i, j, k, l) = tfqmr_y_old(i,j,k,l)  &
                                 + tmp_vect_2 ( i, j, k, l )
                          end do
                       end do
                    end do
                 end do
              end if

!...Neuen Loesungsvektor Berechnen
              ! x_{m-1} = x_{m} + \eta * d_m
              do l = 1, nc
                 do k = dmku, dmko
                    do j = dmju, dmjo
                       do i = dmiu, dmio
                          dq ( i, j, k, l ) = dq (i, j, k, l) + eta *  &
                              tfqmr_d ( i, j, k, l )
                       end do
                    end do
                 end do
              end do
              
!... HIER MUSS DIE UEBERPRUEFUNG DER ABBRUCHBEDINGUNG NOCH HINEIN!!!
!    NACHTIGALL SCHLAEGT VOR:
!     || tfqmr_r || <= SQRT ( m +  1 ) * tau
!    MOEGLICH WAERE ABER AUCH
!     res = A*rhs -dq
!     || res || <= qmr_tol

!              skalar_1 = m + 1.0
!              skalar_3 = SQRT ( skalar_1) * tau
!              write (*,*) rank, ' : res = ', skalar_3
!              if ( skalar_3.lt.tol ) then
!                 goto 1001
!              end if
              
              

!...Zur KOntrolle berechne ich ersteinmal das Residuum
              ! tmp_vect_2 = A * x, dq == x 
              call System_Matmul ( adcl_req_dq, dq, tmp_vect_2, ierror )

              do l = 1, nc
                 do k = dmku, dmko
                    do j = dmju, dmjo
                       do i = dmiu, dmio
                          res ( i, j, k, l) = rhs (i, j, k, l ) - &
                              tmp_vect_2 ( i, j, k, l )
                       end do
                    end do
                 end do
              end do

              skalar_1 = 0.0
              do l = 1, nc
                 do k = dmku, dmko
                    do j = dmju, dmjo
                       do i = dmiu, dmio
                          skalar_1 = skalar_1 + res (i, j, k, l ) * &
                              res ( i, j, k, l )
                       end do
                    end do
                 end do
              end do

              comm_anfang = MPI_WTIME()
              call MPI_Allreduce ( skalar_1, skalar_2, 1, MPI_DOUBLE_PRECISION,  &
                  MPI_SUM, MPI_COMM_WORLD, ierror )
              comm_ende = MPI_WTIME()
!              allred(acount) = comm_ende - comm_anfang
!              acount = acount + 1

              skalar_3 = SQRT ( skalar_2 )

              if ( skalar_3.lt.tol ) then
                 goto 1001
              end if

!...Kopieren einiger Parameter, von denen schon in der inneren
!   Schleife die alten Werte gebraucht werden.

              eta_old       = eta
              theta_old     = theta

!***********************************************************************
!...Ende der inneren Schleife
 500       continue
!***********************************************************************
           if ( verbose .eq. 1 ) then
              if ( rank.eq.0 ) then
                 write (*,*) nreal, skalar_3
              end if
           end if

           ! rho_n = r_tilde_0^H * r_n^{CGS}
           skalar_1 = 0.0
           do l = 1, nc
              do k = dmku, dmko
                 do j = dmju, dmjo
                    do i = dmiu, dmio
                       skalar_1 = skalar_1 + tfqmr_r_tilde (i, j, k, l) &
                           * tfqmr_w ( i, j, k, l )
                    end do
                 end do
              end do
           end do
           
           comm_anfang = MPI_WTIME()
           call MPI_Allreduce ( skalar_1, tfqmr_rho, 1, MPI_DOUBLE_PRECISION,  &
               MPI_SUM, MPI_COMM_WORLD, ierror )
           comm_ende = MPI_WTIME()
!           allred(acount) = comm_ende - comm_anfang
!           acount = acount + 1

           ! beta_n = rho_n / rho_{n-1}
           beta = tfqmr_rho / tfqmr_rho_old

           ! u_n = r_n^{CGS} + beta * q_n
           do l = 1, nc
              do k = dmku, dmko
                 do j = dmju, dmjo
                    do i = dmiu, dmio
                       tfqmr_y ( i, j, k, l ) = tfqmr_w ( i, j, k, l) &
                           + beta * tfqmr_y_old ( i, j, k, l )
                    end do
                 end do
              end do
           end do
           
           ! tmp_vect_2 = A * q_n 
           call System_Matmul ( adcl_tfqmr_y_old, tfqmr_y_old, tmp_vect_2, ierror )

           ! p_{n-1} = q_n + beta * A * p_{n-1} 
           !         = q_n + beta * v_{n-1} 
           do l = 1, nc
              do k = dmku, dmko
                 do j = dmju, dmjo
                    do i = dmiu, dmio
                       tmp_vect(i, j, k, l)=tmp_vect_2(i,j,k,l) &
                           + beta * tfqmr_v(i, j, k, l)
                    end do
                 end do
              end do
           end do
          
           ! tmp_vect_2 = A * u_n 
           call System_Matmul ( adcl_tfqmr_y, tfqmr_y, tmp_vect_2, ierror )

           ! v_n = A * u_n + beta * (  A * q_n + beta * A * p_{n-1} )
           do l = 1, nc
              do k = dmku, dmko
                 do j = dmju, dmjo
                    do i = dmiu, dmio
                       tfqmr_v(i, j, k, l) = tmp_vect_2(i, j, k, l) &
                           + beta * tmp_vect(i, j, k, l)
                    end do
                 end do
              end do
           end do

           
!...Sichern der Variablen, von denen spaeter auch der alte Wert noch
!   benoetigt wird
           tfqmr_rho_old = tfqmr_rho

           do l = 1, nc
              do k = dmku, dmko
                 do j = dmju, dmjo
                    do i = dmiu, dmio
                       tfqmr_y_old_1(i, j, k, l) = tfqmr_y(i,j,k,l)
                    end do
                 end do
              end do
           end do

!           tend = MPI_WTIME()
!           iter(itercount) = tend-tbegin
!           itercount = itercount + 1
!============================================================================
!...Ende der Hauptiterationsschleife
 1000   continue
!============================================================================

!...Marke zum Ausstieg bei einem Breakdown
 1001   continue

!           do i = 1, nit
!              do j = 1, 6
!                 write (10,98) allred(6*i+j)
!              end do
!              write (10,*) i, iter(i)
!           end do
!98         format (5x, f12.5)

!...Free the ADCL objects
        call ADCL_Request_free ( adcl_tfqmr_y, ierror )
        call ADCL_Request_free ( adcl_tfqmr_y_old, ierror )
        call ADCL_Request_free ( adcl_tfqmr_y_old_1, ierror )

        call ADCL_Vector_deregister ( adcl_vec_tfqmr_y, ierror )
        call ADCL_Vector_deregister ( adcl_vec_tfqmr_y_old, ierror )
        call ADCL_Vector_deregister ( adcl_vec_tfqmr_y_old_1, ierror )

!        close (10)
!
!        deallocate ( iter, allred, stat=status )

        r_nit = nreal

        end


