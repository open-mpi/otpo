!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
        subroutine System_Solver_qmr ( n, npattern, ierror )


!...Diese Routine stellt den eigentlichen Gleichungsloeser
!   nach dem QMR-Algorithmus von Martin Buecker und Manfred 
!   Sauren dar

        USE globale_daten
        USE matrix
        USE rechte_seite
        USE ergebnis
        USE numerik_daten
        USE trans_mat

        implicit none

        include 'mpif.h'

!...Hilfsvariablen fuer das QMR-Verfahren

!...Indexzaehler
        integer :: n, npattern

!...Lanczos-Vektoren

        double precision, dimension ( dmiu: dmio, dmju:dmjo, dmku:dmko, nc) :: &
           v_t, qmr_q 

!...p und w_t sind als Lanczos-Vektor groesser ( wie dq) wegen der 
!   Matrix-Multiplikation

        double precision, dimension(0:n1+1, 0:n2+1, 0:n3+1, nc ) :: qmr_p, w_t

!...Residuum Vektoren:
        double precision, dimension ( dmiu: dmio, dmju:dmjo, dmku:dmko, nc)  :: &
        residuum, s, d

!...Parameter des Lanczos-Prozesses:
        double precision :: gamma, gamma_old
        double precision :: tau, tau_old
        double precision :: epsilon
        double precision :: qmr_rho, qmr_rho_old 
        double precision :: nue  
        double precision :: ksi, ksi_old 

!...Parameter des Least-squares-problems
        double precision :: lambda, kappa, zeta

!...Lokale Variablen und Hilfsgroessen
        integer :: ierror, info, rank
        integer :: i, j, k, l

        double precision :: skal_1, skal_2, skal_3, nenner, tau_konj, qmr_tol
        double precision, dimension  (dmiu:dmio, dmju:dmjo, dmku:dmko, nc) :: &
            atw_zwischen, ap_zwischen 
        double precision, dimension ( 5 ) :: all_vekt, all_erg

!================================================================

!...Initialisierung der Parameter
!================================================================

        call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror )

        do  k = 0, n3+1
           do  j = 0, n2+1
              do  i = 0, n1+1
                 do  l = 1, nc
                    qmr_p(i, j, k, l) = 0.0
                 end do
              end do
           end do	
        end do

        do  k = dmku,dmko
           do  j = dmju, dmjo
              do  i =dmiu, dmio
                 do  l = 1, nc
                    qmr_q ( i, j, k, l )    = 0.0
                    d ( i, j, k, l)     = 0.0
                    s ( i, j, k, l)     = 0.0
                 end do
              end do
           end do	
        end do

        lambda     = 1.0
        kappa      = -1.0
        nue        = 0.0
        
!...Datenaustausch fuer dq! Notwendig wegen der Matrixmultipliaktion

        call System_Matmul ( dq, residuum, npattern, ierror )

        do  l = 1, nc
           do  k = dmku, dmko
              do  j = dmju, dmjo
                 do  i = dmiu, dmio
                    residuum(i, j, k, l) = rhs(i, j, k, l) -  &
                                residuum ( i, j, k, l)
                    w_t ( i, j, k, l)    = residuum( i, j, k, l)
                    v_t ( i, j, k, l)    = residuum(i, j, k, l)
                 end do
              end do
           end do	
         end do

!- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
!...Algorithmus zum Berechnen des Betrags eines Vektors

        skal_1 = 0.0

        do  l = 1, nc
           do  k = dmku, dmko
              do  j = dmju, dmjo
                 do  i = dmiu, dmio
                    skal_1 = skal_1 + v_t ( i, j, k, l) * &
                        v_t ( i, j, k, l)
                 end do
               end do
           end do	
        end do


!...Fuer den Betrag braucht man noch die Summe ueber alle Teilergebnisse
!   ( MPI_ALLREDUCE) und dann die Wurzel aus dem Gesamtergebnis. Dies wird weiter 
!   unten ausgefuehrt, da wir diese und die naechsten 2 Allreduce-Operationen 
!   dadurch zu einem einzelnen zusammenfassen koennen. Dazu dient auch das Speichern
!   des Zwischenergebnisses in einem Zwischenspeichervektor.

        all_vekt(1) = skal_1

!- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
!...Algorithmus einer Vektor-Multiplikation

        skal_3 = 0.0
        do  l = 1, nc
           do  k = dmku, dmko
              do  j = dmju, dmjo
                 do  i = dmiu, dmio
                    skal_3 = skal_3 + w_t ( i, j, k, l) * &
                            v_t ( i, j, k, l)
                 end do
              end do
           end do	
        end do

!...Der Allreduce wird nach unten verlegt, da ich dadurch aus 3 Operationen 
!   eine einzelne machen kann. Dazu speichern des Zwischenergebnisses im 
!   Zwischenspeichervektor.
       
        all_vekt(2) = skal_3

!- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
!...Austausch von w_t mit den Nachbarprozessen wegen der Matrixmultiplikation

        call System_Transmatmul ( w_t, atw_zwischen, npattern, ierror )

!- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
!...Algorithmus einer Vektor-Multiplikation

        skal_3 = 0.0
        do  l = 1, nc
           do  k = dmku, dmko
              do  j = dmju, dmjo
                 do  i = dmiu, dmio
                    skal_3 = skal_3 + atw_zwischen ( i, j, k, l) * &
                            v_t ( i, j, k, l)
                 end do
              end do
           end do	
        end do

!...Der Allreduce wird nach unten verlegt, da ich dadurch aus 3 Operationen 
!   eine einzelne machen kann. Dazu speichern des Zwischenergebnisses im 
!   Zwischenspeichervektor.

        all_vekt(3) = skal_3

!- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
!...Neuer MPI_Allreduce, der alle drei Operationen auf einmal durchfuehrt

         call MPI_ALLREDUCE ( all_vekt, all_erg, 3, &
                           MPI_DOUBLE_PRECISION, MPI_SUM, MPI_COMM_WORLD, ierror  )

!...Aus dem Ergebnisvektor all_erg werden nun die Teilergebnisse 
!   herausgefiltert

         gamma_old = sqrt ( all_erg(1) )
         qmr_rho_old  = all_erg(2)
         epsilon  = all_erg(3) 

         ksi_old = gamma_old
         gamma   = gamma_old
         
         qmr_rho = qmr_rho_old
         
         tau_old = epsilon / qmr_rho_old
         tau     = tau_old

!==============================================================
!...Hauptiterationsschleife
         do 1000 n = 1, nit
!==============================================================

            skal_1 = 1 / gamma_old
            skal_2 = 1 /  ksi_old
            skal_3 = gamma_old * nue / ksi_old
            
            do  l = 1, nc
               do  k = dmku, dmko
                  do  j = dmju, dmjo
                     do  i = dmiu, dmio
                        qmr_p( i, j, k, l) = skal_1 * v_t ( i, j, k, l) -  &
                                       nue * qmr_p (i, j, k, l)
                     end do
                  end do
               end do	
            end do

            do  l = 1, nc
               do  k = dmku, dmko
                  do  j = dmju, dmjo
                     do  i = dmiu, dmio
                        qmr_q( i, j, k, l) = skal_2 * atw_zwischen(i, j, k, l) &
                            - skal_3 * qmr_q (i, j, k, l)         
                     end do
                  end do
               end do	
            end do
   
!...Datenaustausch von p mit den benachbarten Prozessen
!   Wegen Matrixmultiplikation notwendig

            call System_Matmul ( qmr_p, ap_zwischen, npattern, ierror )

            skal_1 = tau_old / gamma_old
            skal_2 = tau_old / ksi_old

            do  l = 1, nc
               do  k =  dmku, dmko
                  do  j = dmju, dmjo
                     do  i = dmiu, dmio
                        v_t (i, j, k, l) = ap_zwischen ( i, j, k, l) -  &
                            skal_1 * v_t (i, j, k, l)
                        w_t (i, j, k, l) = qmr_q (i, j, k, l) - skal_2 *  &
                            w_t (i, j, k, l)
                     end do
                  end do
               end do	
            end do

           
!============================================================
!     
!...Abbruchbedingung 
!   Im Augenblick berechnen wir den Betrag des Residuumvektors.
!
!- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
!...Algorithmus zum Berechnen des Betrags eines Vektors

            skal_1 = 0.0
            
            do k = dmku, dmko
               do  j = dmju, dmjo
                  do  i = dmiu, dmio
                     do l = 1, nc
                        skal_1 = skal_1 + residuum (i, j, k, l) *  &
                            residuum (i, j, k, l)
                     end do
                  end do
               end do	
            end do

!...Der Allreduce wird nach unten verlegt, da ich dadurch aus 5 Operationen 
!   eine einzelne machen kann. Dazu speichern des Zwischenergebnisses im 
!   Zwischenspeichervektor.
     
            all_vekt(1) = skal_1

!- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
!...Update einiger Parameter, die eigentlich zum skalieren
!   der Lanczos - Vektoren dienen. Deshalb brauchen wir die 
!   Betraege der Lanczos-Vektoren v_t und w_t

!- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
!...Algorithmus zum Berechnen des Betrags eines Vektors

            skal_2 = 0.0
            do  l = 1, nc
               do  k = dmku, dmko
                  do  j = dmju, dmjo
                     do  i = dmiu, dmio           
                        skal_2 = skal_2 + v_t (i, j, k, l) *  &
                            v_t (i, j, k, l)
                     end do
                  end do
               end do
            end do

!...Der Allreduce wird nach unten verlegt, da ich dadurch aus 5 Operationen 
!   eine einzelne machen kann. Dazu speichern des Zwischenergebnisses im 
!   Zwischenspeichervektor.

            all_vekt(2) = skal_2

!- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
!...Algorithmus zum Berechnen des Betrags eines Vektors

            skal_3 = 0.0
            do  l = 1, nc
               do  k = dmku, dmko
                  do  j = dmju, dmjo
                     do  i = dmiu, dmio
                        skal_3 = skal_3 + w_t (i, j, k, l) *  &
                            w_t (i, j, k, l)
                     end do
                  end do
               end do
            end do

!...Der Allreduce wird nach unten verlegt, da ich dadurch aus 5 Operationen 
!   eine einzelne machen kann. Dazu speichern des Zwischenergebnisses im 
!   Zwischenspeichervektor.

            all_vekt(3) = skal_3

           
!- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
!...Algorithmus einer Vektor-Multiplikation
!     qmr_rho = w_t * v_t

            skal_3 = 0.0
            do  l = 1, nc
               do  k = dmku, dmko
                  do  j = dmju, dmjo
                     do  i = dmiu, dmio
                        skal_3 = skal_3 + w_t ( i, j, k, l) * &
                            v_t ( i, j, k, l)
                     end do
                  end do
               end do
            end do

!...Der Allreduce wird nach unten verlegt, da ich dadurch aus 5 Operationen 
!   eine einzelne machen kann. Dazu speichern des Zwischenergebnisses im 
!   Zwischenspeichervektor.

            all_vekt(4) = skal_3
      
!- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

!...Datenaustausch von w_t mit den benachbarten Prozessen
!   Wegen Matrixmultiplikation notwendig

            call System_Transmatmul ( w_t, atw_zwischen, npattern, ierror )
            
!- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
!...Algorithmus einer Vektor-Multiplikation
!   epsilon = atw_zwischen * v_t

            skal_3 = 0.0
            
            do  l = 1, nc
               do  k = dmku, dmko
                  do  j = dmju, dmjo
                     do  i = dmiu, dmio
                        skal_3 = skal_3 + atw_zwischen ( i, j, k, l) * &
                            v_t ( i, j, k, l)
                     end do
                  end do
               end do
            end do

!...Der Allreduce wird nach unten verlegt, da ich dadurch aus 5 Operationen 
!   eine einzelne machen kann. Dazu speichern des Zwischenergebnisses im 
!   Zwischenspeichervektor.

            all_vekt(5) = skal_3
         
!- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
!...Neuer Allreduce fuer alle fuenf Operationen auf einmal

            call MPI_Allreduce ( all_vekt, all_erg, 5, &
                MPI_DOUBLE_PRECISION, MPI_SUM, MPI_COMM_WORLD, ierror)

!...Herausfiltern  der Teilergebnisse aus dem Zwischenergebnisvektor

            qmr_tol   = sqrt ( all_erg(1) )

!...Abbruchbedingung
            if ( qmr_tol.lt.tol ) goto  1001

!...Retten von gamma, da in spaeteren Berechnungen auch noch der Wert
!   gamma_(n-1) gebraucht wird.

            gamma     = sqrt ( all_erg(2) )
            ksi       = sqrt ( all_erg(3) )
            qmr_rho   = all_erg(4)
            epsilon   = all_erg(5)
!===============================================================
!...Retten von nue, s.o

            nue = (gamma_old * ksi_old * qmr_rho) / &
                ( gamma * tau_old * qmr_rho_old )

            if ( verbose .eq. 1 ) then 
               if ( rank.eq. 0 ) then
                  write(*,*) 'qmr_tol      = ', qmr_tol
               end if
            end if

            tau = epsilon / qmr_rho - gamma * nue

!...Berechnungen fuer die Loesung des Least-Squares-Problems

            nenner = (lambda * tau_old*tau_old + gamma*gamma)

!...Abfangen von nenner = 0

            if ( nenner .eq. 0 ) then
               call MPI_Abort ( MPI_COMM_WORLD, 1, info )
            end if

            zeta = (tau_old*tau_old * (1 - lambda) ) / nenner
            
!...Berechnen von kappa
           
           if (tau_old .ne.0.0) then
              tau_konj  = 1.0 / tau_old
           else
              tau_konj = 0.0
           endif

           kappa = ( -gamma_old * tau_konj * kappa) / nenner
           
!...Berechnen  von lambda

           lambda = (lambda * tau_old * tau_old ) / nenner
        
           do  l = 1, nc
              do  k = dmku, dmko
                 do  j = dmju, dmjo
                    do  i = dmiu, dmio
                       d(i, j, k, l) = zeta * d (i, j, k, l)  &
                           + kappa * qmr_p (i, j, k, l)
                       s(i, j, k, l) = zeta * s (i, j, k, l) +  &
                           kappa * ap_zwischen (i, j, k, l)
                    end do
                 end do
              end do
           end do

!...Berechnen der neuen Naeherungsloesung und des neuen
!   Residuumvektors

           do  l = 1, nc
              do  k = dmku, dmko
                 do  j = dmju, dmjo
                    do  i = dmiu, dmio
                       dq (i, j, k, l)       = dq (i, j, k, l) +  &
                           d (i, j, k, l)
                       residuum (i, j, k, l) = residuum (i, j, k, l) &
                           - s(i, j, k, l)
                    end do
                 end do
              end do
          end do

          ksi_old = ksi
          qmr_rho_old = qmr_rho
          gamma_old = gamma
          tau_old = tau

!==============================================================
!...Ende der Hauptiterationsschleife

 1000   continue
!==============================================================


!...Marke fuer den Ausstieg aus der Schleife

 1001   continue

        
        return
        end






