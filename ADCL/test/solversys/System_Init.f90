!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
        subroutine System_Init ( ierror )

        USE globale_daten
        USE timing

        implicit none
        include 'mpif.h'

!...Set logical variables indicating, whether I am
!   at the boundaries of the domain or not

        integer :: ierror, i, j, rank, size, reorder
        integer, dimension(3) :: dims, periods

        call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror )
        call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )
        
        dims(1)    = 0; dims(2)    = 0; dims(3)    = 0
        periods(1) = 0; periods(2) = 0; periods(3) = 0
        reorder    = 0

        rand_ab    =  .false.
        rand_zu    =  .false.
        rand_sing  =  .false.
        rand_festk =  .false.
        rand_symo  =  .false.
        rand_symu  =  .false.
        
        call MPI_Dims_create ( size, 3, dims, ierror);
        call MPI_Cart_create ( MPI_COMM_WORLD, 3, dims, periods,  &
             reorder, cart_comm, ierror )
        call ADCL_Topology_create ( cart_comm, adcl_topo, ierror)
        n1p = dims(1)
        n2p = dims(2)
        n3p = dims(3)

        if ( size .ne.(n1p * n2p * n3p) ) then
           write (*,*) 'UUPS wrong process distribution'
           ierror = 1 
        end if

        call MPI_Cart_Shift ( cart_comm, 0, 1, tid_iu, tid_io, ierror )
        if ( tid_iu .eq. MPI_PROC_NULL ) then
           rand_sing = .true.
        endif
        if ( tid_io .eq. MPI_PROC_NULL ) then
           rand_ab = .true.
        endif

        call MPI_Cart_Shift ( cart_comm, 1, 1, tid_ju, tid_jo, ierror )
        if ( tid_ju .eq. MPI_PROC_NULL ) then
           rand_festk = .true.
        endif
        if ( tid_jo .eq. MPI_PROC_NULL ) then
           rand_zu = .true.
        endif

        call MPI_Cart_Shift ( cart_comm, 2, 1, tid_ku, tid_ko, ierror )
        if ( tid_ku .eq. MPI_PROC_NULL ) then
           rand_symu = .true.
        endif
        if ( tid_ko .eq. MPI_PROC_NULL ) then
           rand_symo = .true.
        endif

        
        
      end subroutine System_Init

!*******************************************************************
!*******************************************************************
!*******************************************************************
!*******************************************************************
!*******************************************************************
        subroutine System_Init_matrix ( px, py, pz, ierror )

!...Allocate and initialize matrix and vectors

        USE globale_daten
        USE matrix
        USE rechte_seite
        USE ergebnis
        USE timing
        USE loesung

        implicit none
        
        include 'ADCL.inc'
        
        integer :: px, py, pz, ierror 
        integer :: status, rank
        integer, dimension(3) :: dims
        
        n1g = px
        n2g = py
        n3g = pz

        call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror )
        
        n1 = n1g/n1p 
        n2 = n2g/n2p
        n3 = n3g/n3p

        dmiu = 1 
        dmju = 1 
        dmku = 1

        dmio = n1g/n1p 
        dmjo = n2g/n2p 
        dmko = n3g/n3p


!...Allocate all dynamic arrays

      allocate (rm000 ( dmiu:dmio, dmju:dmjo, dmku:dmko, 1, 1 ), &
           rmb00 ( dmiu:dmio, dmju:dmjo, dmku:dmko, 1, 1 ), &
           rmf00 ( dmiu:dmio, dmju:dmjo, dmku:dmko, 1, 1 ), &
           rm0b0 ( dmiu:dmio, dmju:dmjo, dmku:dmko, 1, 1 ), &
           rm0f0 ( dmiu:dmio, dmju:dmjo, dmku:dmko, 1, 1 ), &
           rm00b ( dmiu:dmio, dmju:dmjo, dmku:dmko, 1, 1 ), &
           rm00f ( dmiu:dmio, dmju:dmjo, dmku:dmko, 1, 1 ), & 
           stat=status )
      if (status.gt.0 ) then 
         write (*,*) rank, ' : Error allocating memory'
      end if

      allocate (rhs ( dmiu:dmio, dmju:dmjo, dmku:dmko, 1 ), &
           dq ( 0:n1+1, 0:n2+1, 0:n3+1, 1), & 
           loes( 0:n1+1, 0:n2+1, 0:n3+1, 1), stat=status )
      if (status.gt.0 ) then 
         write (*,*) rank, ' : Error allocating memory'
      end if



!...Generate an ADCL-Vector object out of dq and loes
      dims(1) = n1+2
      dims(2) = n2+2
      dims(3) = n3+2
      call ADCL_Vector_register ( 3, dims, nc, ADCL_VECTOR_HALO, 1, MPI_DOUBLE_PRECISION, &
           dq, adcl_vec_dq, ierror )
      call ADCL_Vector_register ( 3, dims, nc, ADCL_VECTOR_HALO, 1, MPI_DOUBLE_PRECISION, &
           loes, adcl_vec_loes, ierror )

!...Generate now the ADCL-Request object for dq
      call ADCL_Request_create (adcl_vec_dq, adcl_topo, & 
           ADCL_FNCTSET_NEIGHBORHOOD, adcl_req_dq, ierror)
      call ADCL_Request_create (adcl_vec_loes,adcl_topo,&
           ADCL_FNCTSET_NEIGHBORHOOD, adcl_req_loes,ierror)



!...Initialise timing variables
      solv_ende= 0.0 ; solv_anfang= 0.0 ; solvtime  = 0.0
      comm_ende= 0.0 ; comm_anfang= 0.0 ; commtime  = 0.0  
            
!...reset Matrix and rhs to zero
      call System_Reset ( ierror )
      if ( ierror .ne. 0 ) then
         write (*,*) rank, ' : Error in System_Reset '
      end if
      
!...this variable indicates, whether we know the correct
!   answer of the problem. Is reset to .true. in 
!   in System_Set
      loesung_bekannt = .false.

!...Belegt die Matrix und die rechte Seite mit Werten
      call System_Set ( ierror )
      if ( ierror.ne.0 ) then
         write (*,*) rank, ' : Error in System_Set'
      end if
      
      ierror = 0
      
      end subroutine System_Init_matrix

!*******************************************************************
!*******************************************************************
!*******************************************************************
!*******************************************************************
!*******************************************************************
        subroutine System_Free_matrix ( ierror )

        USE globale_daten
        USE matrix
        USE rechte_seite
        USE ergebnis
        USE loesung

        implicit none
        
        include 'ADCL.inc'
        
        integer :: ierror 
        integer :: status, rank
        
        call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror ) 

        deallocate ( rm000, rmb00, rmf00, rm0b0, rm0f0, rm00b, rm00f, &
             stat=status )
        if ( status .gt. 0 ) then
           write (*,*) rank, ' : Error deallocating memory'
        end if

!... Release the ADCL-dq objects
        call ADCL_Request_free ( adcl_req_dq, ierror )
        call ADCL_Request_free ( adcl_req_loes, ierror )
        call ADCL_Vector_deregister ( adcl_vec_dq, ierror )
        call ADCL_Vector_deregister ( adcl_vec_loes, ierror )

        deallocate ( rhs, dq, loes, stat=status )
        if ( status .gt. 0 ) then
           write (*,*) rank, ' : Error deallocating memory'
        end if

        end subroutine System_Free_matrix
!*******************************************************************
!*******************************************************************
!*******************************************************************
!*******************************************************************
!*******************************************************************
        subroutine System_Get_problemsize ( n, px, py, pz, ierror )

        USE globale_daten

        implicit none

        integer :: n, px, py, pz, ierror

        px = problemsx(n)
        py = problemsy(n)
        pz = problemsz(n)

        ierror = 0

        end subroutine System_Get_problemsize
!*******************************************************************
!*******************************************************************
!*******************************************************************
!*******************************************************************
!*******************************************************************
        subroutine System_Get_solver ( n, solv, ierror )

        USE globale_daten

        implicit none

        integer :: n, solv, ierror

        solv = solvarr(n)

        ierror = 0

        end subroutine System_Get_solver
!*******************************************************************
!*******************************************************************
!*******************************************************************
!*******************************************************************
!*******************************************************************

        subroutine System_Reset ( ierror )

!...Zu Null setzen von Matrix, rechte_seite und ergebnis-Vektor

        USE globale_daten
        USE matrix
        USE rechte_seite
	USE loesung

        implicit none

        integer :: ierror
        integer :: i, j, k, l, n 

!...Reseten der Matrix

        do l = 1, nc
           do n = 1, nc
              do k = dmku, dmko
                 do j = dmju, dmjo
                    do i = dmiu, dmio
                       rm000(i, j, k, l, n ) = 0.0
                       rmb00(i, j, k, l, n ) = 0.0
                       rmf00(i, j, k, l, n ) = 0.0
                       rm0b0(i, j, k, l, n ) = 0.0
                       rm0f0(i, j, k, l, n ) = 0.0
                       rm00b(i, j, k, l, n ) = 0.0
                       rm00f(i, j, k, l, n ) = 0.0
                    end do
                 end do
              end do
           end do
        end do

!...Reseten der rechten_seite

        do l = 1, nc
           do k = dmku, dmko
              do j = dmju, dmjo
                 do i = dmiu, dmio
                    rhs(i, j, k, l ) = 1.0
                 end do
              end do
           end do
        end do

!...Reseten des Loesungsvektors
!   Wird spaeter auch als Anfangsschaetzung benuetzt
        
        call System_Reset_dq(ierror)
        
!...Reseten der korrekten Loesung

        do l = 1, nc
           do k = 0, n3+1
              do j = 0, n2+1
                 do i = 0, n1+1
                    loes ( i, j, k, l ) = 0.0
                 end do
              end do
           end do
        end do

        ierror = 0 

        return
        end

!*******************************************************************
!*******************************************************************
!*******************************************************************
!*******************************************************************
!*******************************************************************
        subroutine System_Reset_dq (ierror)

        USE globale_daten
        USE ergebnis

        implicit none
        integer :: ierror
        integer :: i, j, k, l

          do l = 1, nc
             do k = 0, n3+1
                do j = 0, n2+1
                   do i = 0, n1+1
                      dq ( i, j, k, l ) = 0.0
                 end do
              end do
           end do
        end do

        ierror = 0
        end subroutine System_Reset_dq
!*******************************************************************
!*******************************************************************
!*******************************************************************











