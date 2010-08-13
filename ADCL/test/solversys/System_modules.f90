!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
        MODULE globale_daten

!...n1g, n2g, n3g are the number of total gridpoints
!   in x,y,z direction. These are the variables used 
!   throughout the code. 
!   on the other hand problemsx, problemsy and problemsz
!   store overall all 

          integer :: n1g, n2g, n3g 
          integer, allocatable, dimension (:)::problemsx, problemsy, problemsz
          integer, allocatable, dimension (:)::solvarr
          integer :: maxnsolver, maxnproblem

!...dmxo, dmxu geben jeweils die untere und die obere Grenze
!   der lokalen Datenbereiche an

          integer :: dmiu, dmio, dmju, dmjo, dmku, dmko

!...Anzahl der Zellen in jeder Richtung pro Prozessor

          integer :: n1, n2, n3

!...Anzahl der Prozessoren in jede Richtung

          integer :: n1p, n2p, n3p

!...Anzahl der Elemente pro Block

          integer, parameter :: nc = 1

!...Topology communicator
          integer :: cart_comm
          integer :: adcl_topo
          integer :: adcl_vec_dq, adcl_req_dq
          integer :: adcl_vec_loes, adcl_req_loes
          integer :: adcl_vmap

!...Logische Variablen, die die Lage eines Blockes beschreiben

          logical :: rand_ab, rand_zu, rand_sing, rand_festk
          logical :: rand_symo, rand_symu

!..Variablen ,die die Nummer des Nachbars in jede Richtung 
!  beinhalten

          integer :: tid_io, tid_iu, tid_jo, tid_ju, tid_ko, tid_ku

!...Groesse des Datentypes REAL

          integer, parameter :: SIZE_OF_REALx = 8

!...Abstand zwischen zwei Punkten bei der Diskretisierung
          double precision :: deltax, deltaquadx, deltay, deltaquady, &
               deltaz, deltaquadz


        END MODULE globale_daten


        MODULE matrix

!...Daten und Struktur der Heptadiagonalmatrix
          double precision, allocatable, dimension ( :, :, :, :, : ) ::  &
            rm000, rmb00, rmf00, rm0b0, rm0f0, rm00b, rm00f

        END MODULE matrix


        MODULE rechte_seite

!...Defintion der rechten Seite
          double precision, allocatable, dimension ( :, :, :, : ) :: rhs

        END MODULE rechte_seite


        MODULE ergebnis

!...Definition des Ergebnisvektors
          double precision, allocatable, dimension ( :, :, :, : ) :: dq

        END MODULE ergebnis



        MODULE numerik_daten

!...Deklaration der Numerikparameter
!   Sollwerte

        double precision :: tol, subtol
        integer :: nit

!...Istwerte
        integer :: r_nit, verbose
        double precision :: r_tol

        character*80 :: solver_text, precon_text, set_text

        END MODULE numerik_daten



        MODULE trans_mat

!...Dieses Module enthaelt diejenigen Teile der Matrix, die fuer
!   die Matrix-Transponier-Vektor-Multiplikation ausgetauscht werden
!   muessen

        double precision, allocatable, dimension ( :, :, :, :, :) ::  &
             rmf00temp, rmb00temp, rm0f0temp, rm0b0temp, rm00ftemp,   &
             rm00btemp

        double precision, allocatable, dimension (:) ::                 &
             sendbuf1, sendbuf2, sendbuf3, sendbuf4, sendbuf5, sendbuf6,&
             recvbuf1, recvbuf2, recvbuf3, recvbuf4, recvbuf5, recvbuf6 
             
        END MODULE trans_mat

        MODULE timing

!..Variablen, die fuer die Zeitmessungen benoetigt werden

          double precision :: solv_ende, solv_anfang, solvtime
          double precision :: comm_ende, comm_anfang, commtime

       END MODULE timing


        MODULE loesung

          double precision, allocatable, dimension ( :, :, :, : ) :: &
               loes
          logical :: loesung_bekannt

        END MODULE loesung


        MODULE precon_matrix

          double precision, allocatable, dimension ( : , : , : , :, :) :: &
               d1, d2, d3

        END MODULE precon_matrix

        MODULE constants

          integer, parameter :: solv_tfqmr          = 1
          integer, parameter :: solv_qmr            = 4

        end MODULE constants





