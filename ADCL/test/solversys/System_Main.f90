!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
        program System

!...This is a simple benchmarking framework for parallel solvers.
!   Originally developed for 

      implicit none
      include 'ADCL.inc'

      integer :: ierror
      integer :: nproblem,  nsolver
      integer :: maxproblem,  maxsolver
      integer :: px, py, pz
      integer :: size, rank
      integer ::  solv

!...Initialize parallel environment

      call MPI_Init ( ierror )
      call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )
      call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror )

!...Initialize the ADCL environment
      call ADCL_Init ( ierror )

!...Read configuration file
      call System_Read_config (maxproblem, maxsolver, ierror)
      if ( ierror.ne. 0 ) then
         write (*,*)rank, 'Error in System_read_config'
      end if
      
!...Initialize benchmarking independent things
      call System_Init ( ierror )
      if ( ierror.ne. 0 ) then
         write (*,*)rank, 'Error in System_Init'
      end if

!...Loop over problem sizes
      do nproblem=1, maxproblem 

!.......Initialize problem size dependent things (e.g. matrix)
         call System_Get_problemsize (nproblem, px, py, pz, ierror)
         if ( ierror.ne. 0 ) then
            write (*,*)rank, 'Error in System_Get_problemsize'
         end if
         
         call System_Init_matrix ( px, py, pz, ierror )
         if ( ierror.ne. 0 ) then
            write (*,*)rank, 'Error in System_Init_matrix'
         end if
         
!.......Loop over solvers
         do nsolver=1, maxsolver
            call System_Get_solver(nsolver, solv, ierror )

!...........Reset solution vectors
            call System_Reset_dq ( ierror )

!...........Preconditioning
            call System_Precon ( ierror )

!...........Call the solvers
            call System_Solver ( solv, ierror )

!...........Display results
            call System_Display_Result ( ierror )

         end do

!.......free matrix and vector etc..
       call System_Free_matrix ( ierror )
      end do


!...Close parallel environment
      call ADCL_Finalize ( ierror )
      call MPI_Finalize ( ierror )

      end




