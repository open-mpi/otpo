!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
        program two

        implicit none
        include 'ADCL.inc'

        integer rank, size, ierror 
        integer nc, hwidth
        integer vec, topo, request
        integer cart_comm
        integer, dimension(2) :: dims, cdims, periods
        integer, dimension(4) :: neighbors
        double precision :: data(10,6), data1(12,8), data2(10,6,1), data3(12,8,1), data4(12,8,2)
        
        cdims(1)   = 0
        cdims(2)   = 0
        periods(1) = 0
        periods(2) = 0
        
        call MPI_Init ( ierror )
        call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror)
        call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )

        call ADCL_Init ( ierror )

        call MPI_Dims_create ( size, 2, cdims, ierror)
        call MPI_Cart_create ( MPI_COMM_WORLD, 2, cdims, periods, 0,  &
                               cart_comm, ierror )
        call ADCL_Topology_create ( cart_comm, topo, ierror )
        call MPI_Cart_shift ( cart_comm, 0, 1, neighbors(1), neighbors(2), ierror )
        call MPI_Cart_shift ( cart_comm, 1, 1, neighbors(3), neighbors(4), ierror )
        

!.......Test 1: hwidth=1, nc=0 
        dims(1) = 10
        dims(2) = 6
        nc     = 0
        hwidth = 1
        call ADCL_Vector_register ( 2, dims, nc, ADCL_VECTOR_HALO, hwidth, MPI_DOUBLE_PRECISION,&
                                    data, vec, ierror)
        call ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &
             request, ierror )

        call set_data_2D ( data, rank, dims, hwidth ) 
        call ADCL_Request_start( request, ierror )
        call check_data_2D ( data, rank, dims, hwidth, neighbors )

        call ADCL_Request_free ( request, ierror )
        call ADCL_Vector_deregister ( vec, ierror )

!!.......Test 2: hwidth=2, nc=0 
        dims(1) = 12
        dims(2) = 8
        hwidth = 2
        nc   = 0
        call ADCL_Vector_register ( 2, dims, nc, ADCL_VECTOR_HALO, hwidth, MPI_DOUBLE_PRECISION,&
                                    data1, vec, ierror)
        call ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &
             request, ierror )

        call set_data_2D ( data1, rank, dims, hwidth ) 
        call ADCL_Request_start( request, ierror )
        call check_data_2D ( data1, rank, dims, hwidth, neighbors )

        call ADCL_Request_free ( request, ierror )
        call ADCL_Vector_deregister ( vec, ierror )

!!.......Test 3: hwidth=1, nc=1 
        dims(1) = 10
        dims(2) = 6
        hwidth = 1
        nc     = 1
        call ADCL_Vector_register ( 2, dims, nc, ADCL_VECTOR_HALO, hwidth, MPI_DOUBLE_PRECISION,&
                                    data2, vec, ierror)
        call ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &
             request, ierror )

        call set_data_3D ( data2, rank, dims, hwidth, nc ) 
        call ADCL_Request_start( request, ierror )
        call check_data_3D ( data2, rank, dims, hwidth, neighbors, nc )

        call ADCL_Request_free ( request, ierror )
        call ADCL_Vector_deregister ( vec, ierror )

!!.......Test 4: hwidth=2, nc=1 
        dims(1) = 12
        dims(2) = 8
        hwidth = 2
        nc     = 1
        call ADCL_Vector_register ( 2, dims, nc, ADCL_VECTOR_HALO, hwidth, MPI_DOUBLE_PRECISION,&
                                    data3, vec, ierror)
        call ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &
             request, ierror )

        call set_data_3D ( data3, rank, dims, hwidth, nc ) 
        call ADCL_Request_start( request, ierror )
        call check_data_3D ( data3, rank, dims, hwidth, neighbors, nc )

        call ADCL_Request_free ( request, ierror )
        call ADCL_Vector_deregister ( vec, ierror )

!!.......Test 5: hwidth=2, nc=2 
        dims(1) = 12
        dims(2) = 8
        hwidth = 2
        nc     = 2
        call ADCL_Vector_register ( 2, dims, nc, ADCL_VECTOR_HALO, hwidth, MPI_DOUBLE_PRECISION,&
                                    data4, vec, ierror)
        call ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &
             request, ierror )

        call set_data_3D ( data4, rank, dims, hwidth, nc ) 
        call ADCL_Request_start( request, ierror )
        call check_data_3D ( data4, rank, dims, hwidth, neighbors, nc )

        call ADCL_Request_free ( request, ierror )
        call ADCL_Vector_deregister ( vec, ierror )

!!.......done
        call ADCL_Topology_free ( topo, ierror )
        call MPI_Comm_free ( cart_comm, ierror )

        call ADCL_Finalize ( ierror )
        call MPI_Finalize ( ierror )
      end program two

!****************************************************************************
!****************************************************************************
!****************************************************************************

      subroutine set_data_3D ( data, rank, dims, hwidth, nc )

        implicit none
        include 'ADCL.inc'

        integer rank, dims(2), hwidth, nc
        double precision data(dims(1),dims(2), nc)

        integer i, j, k

        do i=1,hwidth
           do j = 1, dims(2)
              do k = 1,nc
                 data(i,j,k) = -1
              end do
           end do
        end do

        do i = dims(1)-hwidth+1, dims(1)
           do j = 1, dims(2)
              do k = 1, nc
                 data(i, j, k) = -1
              end do
           end do
        end do

        do i=1,dims(1)
           do j = 1, hwidth
              do k = 1, nc 
                 data(i,j, k) = -1
              end do
           end do
        end do

        do i = 1, dims(1)
           do j = dims(2)-hwidth+1, dims(2)
              do k = 1, nc
                 data(i, j, k) = -1
              end do
           end do
        end do


        do i = hwidth+1, dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k = 1, nc 
                 data(i,j, k) = rank
              end do
           end do
        end do

        return
      end subroutine set_data_3D



!****************************************************************************
!****************************************************************************
!****************************************************************************
      subroutine dump_vector_3D ( data, rank, dims, nc )

        implicit none
        include 'ADCL.inc'

        integer rank, dims(2), nc
        double precision data(dims(1), dims(2), nc)
        integer i, j

        if ( nc .le. 1 ) then
           do j = 1, dims(1)
              write (*,*) rank, (data(j,i,1), i=1,dims(2))
           end do
        else if ( nc .eq. 2 ) then 
           do j = 1, dims(1)
              write (*,*) rank, (data(j,i,1), data(j,i,2),  i=1,dims(2))
           end do
        endif

        return
      end subroutine dump_vector_3D


!****************************************************************************
!****************************************************************************
!****************************************************************************

      subroutine check_data_3D ( data, rank, dims, hwidth, neighbors, nc ) 

        implicit none
        include 'ADCL.inc'

        integer rank, dims(2), hwidth, nc, neighbors(4)
        double precision data(dims(1),dims(2), nc)
        integer lres, gres, i, j, k, ierr
        double precision should_be
        
        lres = 1
      
        if ( neighbors(1) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(1)
        endif
        do i=1,hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k=1,nc 
                 if ( data(i,j,k) .ne. should_be ) then
                    lres = 0 
                    write (*,99) rank, ' : ', i,j,k, 'is ', &
                         data(i,j,k), ' should be ', should_be
                 endif
              end do
           end do
        end do

        if ( neighbors(2) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(2)
        endif
        do i = dims(1)-hwidth+1, dims(1)
           do j = hwidth+1, dims(2)-hwidth
              do k = 1, nc
                 if ( data(i, j, k) .ne. should_be ) then 
                    lres = 0
                    write (*,99) rank, ' : ', i,j,k, 'is ', &
                         data(i,j,k), ' should be ', should_be
                 endif
              end do
           end do
        end do


        if ( neighbors(3) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(3)
        endif
        do i=hwidth+1,dims(1)-hwidth
           do j = 1, hwidth
              do k = 1, nc
                 if ( data(i,j,k) .ne. should_be ) then
                    lres = 0
                    write (*,99) rank, ' : ', i,j,k, 'is ', &
                         data(i,j,k), ' should be ', should_be
                 endif
              end do
           end do
        end do

        if ( neighbors(4) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(4)
        endif
        do i = hwidth+1, dims(1)-hwidth
           do j = dims(2)-hwidth+1, dims(2)
              do k = 1, nc 
                 if ( data(i, j, k) .ne. should_be ) then 
                    lres = 0
                    write (*,99) rank, ' : ', i,j,k, 'is ', &
                         data(i,j,k), ' should be ', should_be
                 endif
              end do
           end do
        end do


        do i = hwidth+1, dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k = 1, nc
                 if ( data(i,j, k) .ne. rank ) then 
                    lres = 0
                    write (*,99) rank, ' : ', i,j,k, 'is ', &
                         data(i,j,k), ' should be ', should_be
                 endif
              end do
           end do
        end do

99      format (i1,a3,3i3,a4,f12.5,a11,f12.5)


        call MPI_Allreduce ( lres, gres, 1, MPI_INTEGER, MPI_MIN, MPI_COMM_WORLD, ierr)
        if ( gres .eq. 0 ) then
!           call dump_vector_2D ( data, rank, dims )
           if ( rank .eq. 0 ) then
              write (*,*) '2-D Fortran testsuite hwidth =', hwidth, &
                   'nc = ', nc, ' failed'  
           end if
        else
           if ( rank .eq. 0 ) then
              write (*,*) '2-D Fortran testsuite hwidth =', hwidth, &
                   'nc = ', nc, ' passed'  
           end if
        end if


        return
      end subroutine check_data_3D

!****************************************************************************
!****************************************************************************
!****************************************************************************

      subroutine set_data_2D ( data, rank, dims, hwidth )

        implicit none
        include 'ADCL.inc'

        integer rank, dims(2), hwidth
        double precision data(dims(1),dims(2))

        integer i, j

        do i=1,hwidth
           do j = 1, dims(2)
              data(i,j) = -1
           end do
        end do

        do i = dims(1)-hwidth+1, dims(1)
           do j = 1, dims(2)
              data(i, j) = -1
           end do
        end do

        do i=1,dims(1)
           do j = 1, hwidth
              data(i,j) = -1
           end do
        end do

        do i = 1, dims(1)
           do j = dims(2)-hwidth+1, dims(2)
              data(i, j) = -1
           end do
        end do


        do i = hwidth+1, dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              data(i,j) = rank
           end do
        end do

        return
      end subroutine set_data_2D

!****************************************************************************
!****************************************************************************
!****************************************************************************
      subroutine dump_vector_2D ( data, rank, dims )

        implicit none
        include 'ADCL.inc'

        integer rank, dims(2)
        double precision data(dims(1), dims(2))
        integer i, j

        do j = 1, dims(1)
           write (*,*) rank, (data(j,i), i=1,dims(2))
        end do
        return
      end subroutine dump_vector_2D


!****************************************************************************
!****************************************************************************
!****************************************************************************

      subroutine check_data_2D ( data, rank, dims, hwidth, neighbors ) 

        implicit none
        include 'ADCL.inc'

        integer rank, dims(2), hwidth, neighbors(4)
        double precision data(dims(1),dims(2))
        integer lres, gres, i, j, ierr
        double precision should_be
        
        lres = 1
      
        if ( neighbors(1) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(1)
        endif
        do i=1,hwidth
           do j = hwidth+1, dims(2)-hwidth
              if ( data(i,j) .ne. should_be ) then
                 lres = 0 
              endif
           end do
        end do

        if ( neighbors(2) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(2)
        endif
        do i = dims(1)-hwidth+1, dims(1)
           do j = hwidth+1, dims(2)-hwidth
              if ( data(i, j) .ne. should_be ) then 
                 lres = 0
              endif
           end do
        end do


        if ( neighbors(3) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(3)
        endif
        do i=hwidth+1,dims(1)-hwidth
           do j = 1, hwidth
              if ( data(i,j) .ne. should_be ) then
                 lres = 0
              endif
           end do
        end do

        if ( neighbors(4) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(4)
        endif
        do i = hwidth+1, dims(1)-hwidth
           do j = dims(2)-hwidth+1, dims(2)
              if ( data(i, j) .ne. should_be ) then 
                 lres = 0
              endif
           end do
        end do


        do i = hwidth+1, dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              if ( data(i,j) .ne. rank ) then 
                 lres = 0
              endif
           end do
        end do



        call MPI_Allreduce ( lres, gres, 1, MPI_INTEGER, MPI_MIN, MPI_COMM_WORLD, ierr)
        if ( gres .eq. 0 ) then
           call dump_vector_2D ( data, rank, dims )
           if ( rank .eq. 0 ) then
              write (*,*) '2-D Fortran testsuite hwidth =', hwidth, &
                   'nc = ', 0, ' failed'  
           end if
        else
           if ( rank .eq. 0 ) then
              write (*,*) '2-D Fortran testsuite hwidth =', hwidth, &
                   'nc = ', 0, ' passed'  
           end if
        end if


        return
      end subroutine check_data_2D
