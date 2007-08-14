!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
        subroutine System_Solver ( nsolv, ierror )

        USE globale_daten
        USE numerik_daten
        USE timing
        USE constants

        implicit none

        include 'mpif.h'
        integer :: rank
        integer :: ierror, nsolv, npattern
        integer :: nreal

        call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror )

        select case (nsolv) 
           case (solv_tfqmr)
              solver_text = ' Solver : TFQMR'
              call MPI_Barrier ( MPI_COMM_WORLD, ierror )
              solv_anfang = MPI_Wtime ()
              call System_Solver_tfqmr ( nreal, ierror)
              if ( ierror .ne. 0 ) then
                 write (*,*) rank, ': Error in System_Subsolver ', &
                      nsolv
              end if
              solv_ende = MPI_Wtime () 


           case (solv_qmr)
              solver_text = ' Solver : QMR'
              call System_Matchange_init ( ierror )
              call System_Matchange ( ierror )
              if ( ierror.ne.0) then
                 write (*,*) rank, ': Error in System_Matchange'
              end if
              call MPI_Barrier ( MPI_COMM_WORLD, ierror )
              solv_anfang = MPI_Wtime ()
              call System_Solver_qmr ( nreal, ierror)
              if ( ierror .ne. 0 ) then
                 write (*,*) rank, ': Error in System_Subsolver ', &
                      nsolv
              end if
              solv_ende = MPI_Wtime () 

           case default
              write (*,*) rank, ': Unknown solver: ', nsolv

           end select

!...Correct the number of iterations
        nit = nreal - 1 
        
        end subroutine System_Solver

