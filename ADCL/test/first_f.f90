!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
        program first

        implicit none
        include 'ADCL.inc'

        integer i, rank, size, ierror, NIT, allocstat
        integer dims(4), cdims(3), periods(3)        
        integer vmap
        integer vec
        integer topo
        integer request
        integer cart_comm
        double precision, allocatable, dimension (:,:,:,:) :: data
        
        NIT = 200

        dims(1) = 18
        dims(2) = 34
        dims(3) = 34
        dims(4) = 1
        cdims(1) = 0
        cdims(2) = 0
        cdims(3) = 0
        periods(1) = 0
        periods(2) = 0
        periods(3) = 0
        
        call MPI_Init ( ierror )
        call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror)
        call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )

        allocate ( data (0:dims(1)-1, 0:dims(2)-1, 0:dims(3)-1, 1), &
             stat = allocstat ) 
        if ( allocstat.gt.0 ) then 
           write (*,*) rank, ' : Error allocating memory'
           call MPI_Abort ( MPI_COMM_WORLD, 1, ierror )
        end if

        call ADCL_Init ( ierror )
        call ADCL_Vmap_halo_allocate( 1, vmap, ierror ) 
        if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_halo_allocate not successful"   
        call ADCL_Vector_register_generic ( 3, dims, 1, vmap, MPI_DOUBLE_PRECISION,&
                                    data, vec, ierror)

        call MPI_Dims_create ( size, 3, cdims, ierror)
        call MPI_Cart_create ( MPI_COMM_WORLD, 3, cdims, periods, 0,  &
                               cart_comm, ierror )
        call ADCL_Topology_create ( cart_comm, topo, ierror )

        call ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &
            request, ierror )

        do i=0, NIT 
           call ADCL_Request_start( request, ierror )
        end do

        call ADCL_Request_free ( request, ierror )
        call ADCL_Topology_free ( topo, ierror )
        call ADCL_Vector_deregister ( vec, ierror )
        call ADCL_Vmap_free ( vmap )
        call MPI_Comm_free ( cart_comm, ierror )
    
        deallocate ( data, stat=allocstat)

        call ADCL_Finalize ( ierror )
        call MPI_Finalize ( ierror )
      end program first
