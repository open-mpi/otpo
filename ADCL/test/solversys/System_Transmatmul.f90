!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
        subroutine System_Transmatmul ( adcl_req, vect, res_vect, ierror )

!...Algorithmus einer Matrix-Transponiert Vektor Multiplikation

        USE globale_daten
        USE matrix
        USE constants
        USE timing
        USE trans_mat

        implicit none
        include 'ADCL.inc'

        integer :: adcl_req, ierror
        double precision vect(*) 
        double precision res_vect (*)

        call ADCL_Request_start (adcl_req, ierror)
        call System_Transmatmul_blocking(vect, res_vect, ierror)
           
        return
        end subroutine System_Transmatmul



