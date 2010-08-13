!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
      program test_fnctset_single

        implicit none
        include 'ADCL.inc'

        integer attr1vals(2), attr2vals(3),attr3vals(2)
        integer attrs(3), attrset
        integer fnctset
        integer topo
        integer request
        integer NIT, i, rank, size, ierror
        external init_test_func, wait_test_func

        NIT = 500

        attr1vals(1) = 10
        attr1vals(2) = 11
        attr2vals(1) = 20
        attr2vals(2) = 21
        attr2vals(3) = 22
        attr3vals(1) = 30
        attr3vals(2) = 31

        call MPI_Init ( ierror )
        call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror )
        call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )

        call ADCL_Init ( ierror )

        call ADCL_Attribute_create ( 2, attr1vals, attrs(1), ierror )
        call ADCL_Attribute_create ( 3, attr2vals, attrs(2), ierror )
        call ADCL_Attribute_create ( 2, attr3vals, attrs(3), ierror )

        call ADCL_Attrset_create ( 3, attrs, attrset, ierror );

        call ADCL_fnctset_create_single ( init_test_func, wait_test_func,& 
                 attrset,"test function", 0, 0, fnctset, ierror)

        call ADCL_Topology_create_generic ( 0, 0, 0, 0, & 
                 ADCL_DIRECTION_BOTH, MPI_COMM_WORLD, topo, ierror )

        call ADCL_Request_create( ADCL_VECTOR_NULL, topo, fnctset, &
                 request, ierror )

        do i=0, NIT 
           call ADCL_Request_start( request, ierror )
        end do

        call ADCL_Request_free ( request, ierror )
        call ADCL_Topology_free ( topo, ierror )
        call ADCL_Fnctset_free ( fnctset, ierror )
        call ADCL_Attrset_free ( attrset, ierror )

        do i=0, 3 
           call ADCL_Attribute_free ( attrs(i), ierror )
        end do

        call ADCL_Finalize ( ierror )
        call MPI_Finalize ( ierror )

      end program test_fnctset_single

      subroutine init_test_func ( request ) 

        implicit none
        include 'ADCL.inc'

        integer request
        integer comm, rank, size, ierror

        call ADCL_Request_get_comm ( request, comm, rank, size, ierror )

      end subroutine init_test_func

      subroutine wait_test_func ( request ) 

        implicit none
        include 'ADCL.inc'

        integer request
        integer comm, rank, size, ierror

        call ADCL_Request_get_comm ( request, comm, rank, size, ierror )

      end subroutine wait_test_func
