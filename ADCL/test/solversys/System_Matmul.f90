!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
           subroutine System_Matmul ( adcl_req, vect, res_vect, ierror )
       
           USE constants
           USE timing
           implicit none
           
           include 'ADCL.inc'

           integer :: adcl_req, ierror
           double precision vect(*), res_vect(*)

!... Single-block version
           call ADCL_Request_start ( adcl_req, ierror )
           call System_Matmul_blocking (vect, res_vect, ierror )

!... Dual block version -have to test first!
!           call ADCL_Request_init ( adcl_req, ierror )
!           call System_Matmul_init (vect, res_vect, ierror )
!           call ADCL_Request_wait ( adcl_req, ierror )
!           call System_Matmul_end ( vect, res_rect, ierror )
 
!... Using both methods - have to test first!
!           call ADCL_Request_start_overlap ( adcl_req,  &
!                System_Matmul_init, System_Matmul_end, 
!                System_Matmul_blocking, vect, res_vect, ierror )


           return
           end subroutine System_Matmul














