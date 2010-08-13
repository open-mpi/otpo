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

        integer rank, size, ierror 
        integer dims(2), cdims, periods, nc, hwidth
        integer vmap
        integer vec
        integer topo
        integer request
        integer cart_comm
        double precision :: data(10), data1(12), data2(10,1), data3(12,1), data4(12,2)
        
        cdims   = 0
        periods = 0
        
        call MPI_Init ( ierror )
        call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror)
        call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )

        call ADCL_Init ( ierror )

        call MPI_Dims_create ( size, 1, cdims, ierror)
        call MPI_Cart_create ( MPI_COMM_WORLD, 1, cdims, periods, 0,  &
                               cart_comm, ierror )
        call ADCL_Topology_create ( cart_comm, topo, ierror )

!.......Test 1: hwidth=1, nc=0 
        dims   = 10
        nc     = 0
        hwidth = 1
        call adcl_vmap_halo_allocate( hwidth, vmap, ierror ) 
        if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_halo_allocate not successful"   
        call adcl_vector_register_generic ( 1,  dims, nc, vmap, MPI_DOUBLE_PRECISION, & 
                                    data, vec, ierror )
        call ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &
             request, ierror )

        call set_data_1D ( data, rank, dims, hwidth ) 
        call ADCL_Request_start( request, ierror )
        call check_data_1D ( data, rank, size, dims, hwidth )

        call ADCL_Request_free ( request, ierror )
        call ADCL_Vector_deregister ( vec, ierror )
        call ADCL_Vmap_free ( vmap, ierror )

!.......Test 2: hwidth=2, nc=0 
        dims = 12
        hwidth = 2
        nc   = 0
        call adcl_vmap_halo_allocate( hwidth, vmap, ierror ) 
        if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_halo_allocate not successful"   
        call adcl_vector_register_generic ( 1,  dims, nc, vmap, MPI_DOUBLE_PRECISION, & 
                                    data1, vec, ierror )
        call ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &
             request, ierror )

        call set_data_1D ( data1, rank, dims, hwidth ) 
        call ADCL_Request_start( request, ierror )
        call check_data_1D ( data1, rank, size, dims, hwidth )

        call ADCL_Request_free ( request, ierror )
        call ADCL_Vector_deregister ( vec, ierror )
        call ADCL_Vmap_free ( vmap, ierror )

!.......Test 3: hwidth=1, nc=1 
        dims   = 10
        hwidth = 1
        nc     = 1
        call adcl_vmap_halo_allocate( hwidth, vmap, ierror ) 
        if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_halo_allocate not successful"   
        call adcl_vector_register_generic ( 1,  dims, nc, vmap, MPI_DOUBLE_PRECISION, & 
                                    data2, vec, ierror )
        call ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &
             request, ierror )

        call set_data_2D ( data2, rank, dims, nc, hwidth, nc ) 
        call ADCL_Request_start( request, ierror )
        call check_data_2D ( data2, rank, size, dims, nc, hwidth, nc )

        call ADCL_Request_free ( request, ierror )
        call ADCL_Vector_deregister ( vec, ierror )
        call ADCL_Vmap_free ( vmap, ierror )

!.......Test 4: hwidth=2, nc=1 
        dims   = 12
        hwidth = 2
        nc     = 1
        call adcl_vmap_halo_allocate( hwidth, vmap, ierror ) 
        if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_halo_allocate not successful"   
        call adcl_vector_register_generic ( 1,  dims, nc, vmap, MPI_DOUBLE_PRECISION, & 
                                    data3, vec, ierror)
        call ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &
             request, ierror )

        call set_data_2D ( data3, rank, dims, nc, hwidth, nc ) 
        call ADCL_Request_start( request, ierror )
        call check_data_2D ( data3, rank, size, dims, nc, hwidth, nc )

        call ADCL_Request_free ( request, ierror )
        call ADCL_Vector_deregister ( vec, ierror )
        call ADCL_Vmap_free ( vmap, ierror )

!.......Test 5: hwidth=2, nc=2 
        dims   = 12
        hwidth = 2
        nc     = 2
        call adcl_vmap_halo_allocate( hwidth, vmap, ierror ) 
        if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_halo_allocate not successful"   
        call adcl_vector_register_generic ( 1,  dims, nc, vmap, MPI_DOUBLE_PRECISION, & 
                                    data4, vec, ierror)
        call ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &
             request, ierror )

        call set_data_2D ( data4, rank, dims, nc, hwidth, nc ) 
        call ADCL_Request_start( request, ierror )
        call check_data_2D ( data4, rank, size, dims, nc, hwidth, nc )

        call ADCL_Request_free ( request, ierror )
        call ADCL_Vector_deregister ( vec, ierror )
        call ADCL_Vmap_free ( vmap, ierror )

!.......done
        call ADCL_Topology_free ( topo, ierror )
        call MPI_Comm_free ( cart_comm, ierror )
    
        call ADCL_Finalize ( ierror )
        call MPI_Finalize ( ierror )
      end program first

!****************************************************************************
!****************************************************************************
!****************************************************************************

      subroutine set_data_1D ( data, rank, dim, hwidth )

        implicit none
        include 'ADCL.inc'

        double precision data(*)
        integer rank, dim, hwidth
        integer i

        do i=1,hwidth
           data(i) = -1
        end do

        do i = hwidth+1, dim-hwidth
           data(i) = rank
        end do
        
        do i = dim-hwidth+1, dim
           data(i) = -1
        end do

        return
      end subroutine set_data_1D

!****************************************************************************
!****************************************************************************
!****************************************************************************

      subroutine set_data_2D ( data, rank, dim1, dim2, hwidth, nc )

        implicit none
        include 'ADCL.inc'

        integer rank, dim1, dim2, hwidth, nc
        double precision data(dim1,dim2)
        integer i, j

        do i=1,hwidth
           do j = 1, nc
              data(i,j) = -1
           end do
        end do

        do i = hwidth+1, dim1-hwidth
           do j = 1, nc
              data(i,j) = rank
           end do
        end do
        
        do i = dim1-hwidth+1, dim1
           do j = 1, nc
              data(i, j) = -1
           end do
        end do

        return
      end subroutine set_data_2D

!****************************************************************************
!****************************************************************************
!****************************************************************************
      subroutine check_data_1D ( data, rank, size, dim, hwidth ) 

        implicit none
        include 'ADCL.inc'

        double precision data(*)
        integer rank, size, dim, hwidth
        integer lres, gres, i, ierr
        
        lres = 1

        if ( rank .eq. 0 ) then
           do i=1,hwidth
              if ( data(i) .ne. -1 ) then
                 lres = 0
              end if
           end do
        else 
           do i=1,hwidth
              if ( data(i) .ne. rank-1 ) then
                 lres = 0
              end if
           end do
        end if

        do i = hwidth+1, dim-hwidth
           if ( data(i) .ne. rank ) then
              lres = 0
           end if
        end do
        
        if ( rank .eq. size-1 ) then
           do i = dim-hwidth+1, dim
              if ( data(i) .ne. -1 ) then
                 lres = 1
              end if
           end do
        else
           do i = dim-hwidth+1, dim
              if ( data(i) .ne. rank+1 ) then
                 lres = 1
              end if
           end do
        end if

        call MPI_Allreduce ( lres, gres, 1, MPI_INTEGER, MPI_MIN, MPI_COMM_WORLD, ierr)
        if ( gres .eq. 0 ) then
           call dump_vector_1D ( data, rank, dim )
           if ( rank .eq. 0 ) then
              write (*,*) '1-D Fortran testsuite hwidth =', hwidth, &
                   'nc =',0, 'failed'  
           end if
        else
           if ( rank .eq. 0 ) then
              write (*,*) '1-D Fortran testsuite hwidth =', hwidth, &
                   'nc =',0,'passed'  
           end if
        end if

        return
      end subroutine check_data_1D

!****************************************************************************
!****************************************************************************
!****************************************************************************
      subroutine check_data_2D ( data, rank, size, dim1, dim2, hwidth, nc ) 

        implicit none
        include 'ADCL.inc'

        integer rank, size, dim1, dim2, hwidth, nc
        double precision data(dim1,dim2)
        integer lres, gres, i, j, ierr
        
        lres = 1
        

        if ( rank .eq. 0 ) then
           do i=1,hwidth
              do j = 1, nc
                 if ( data(i,j) .ne. -1 ) then
                    lres = 0
                 end if
              end do
           end do
        else
           do i=1,hwidth
              do j = 1, nc
                 if ( data(i,j) .ne. rank-1 ) then
                    lres = 0
                 end if
              end do
           end do
        end if

        do i = hwidth+1, dim1-hwidth
           do j = 1, nc
              if ( data(i,j) .ne. rank ) then
                 lres = 0
              end if
           end do
        end do
        
        if ( rank .eq. size-1 ) then
           do i = dim1-hwidth+1, dim1
              do j = 1, nc
                 if ( data(i, j) .ne. -1 ) then
                    lres = 0
                 end if
              end do
           end do
        else
           do i = dim1-hwidth+1, dim1
              do j = 1, nc
                 if ( data(i, j) .ne. rank+1 ) then
                    lres = 0
                 end if
              end do
           end do
        end if

        call MPI_Allreduce ( lres, gres, 1, MPI_INTEGER, MPI_MIN, MPI_COMM_WORLD, ierr)
        if ( gres .eq. 0 ) then
           call dump_vector_2D ( data, rank, dim1, dim2 )
           if ( rank .eq. 0 ) then
              write (*,*) '1-D Fortran testsuite hwidth =', hwidth, &
                   'nc = ', nc, 'failed'  
           end if
        else
           if ( rank .eq. 0 ) then
              write (*,*) '1-D Fortran testsuite hwidth =', hwidth, &
                   'nc =', nc, 'passed'  
           end if
        end if


        return
      end subroutine check_data_2D


!****************************************************************************
!****************************************************************************
!****************************************************************************
      subroutine dump_vector_1D ( data, rank, dim )

        implicit none
        include 'ADCL.inc'

        double precision data(*)
        integer rank, dim
        integer i

        write (*,*) rank, (data(i), i=1,dim)
        return
      end subroutine dump_vector_1D

!****************************************************************************
!****************************************************************************
!****************************************************************************
      subroutine dump_vector_2D ( data, rank, dim1, dim2 )

        implicit none
        include 'ADCL.inc'

        integer rank, dim1, dim2
        double precision data(dim1, dim2)
        integer i, j

        do j = 1, dim1
           write (*,*) rank, (data(j,i), i=1,dim2)
        end do
        return
      end subroutine dump_vector_2D
