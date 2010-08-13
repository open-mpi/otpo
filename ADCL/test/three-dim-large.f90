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
        integer vmap, vec, topo, request
        integer cart_comm
        integer, dimension(3) :: dims, cdims, periods
        integer, dimension(6) :: neighbors
        double precision :: data2(34,34,66,1)
        
        cdims(1)   = 0
        cdims(2)   = 0
        cdims(3)   = 0
        periods(1) = 0
        periods(2) = 0
        periods(3) = 0
        
        call MPI_Init ( ierror )
        call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror)
        call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )

        call ADCL_Init ( ierror )

        call MPI_Dims_create ( size, 3, cdims, ierror)
        call MPI_Cart_create ( MPI_COMM_WORLD, 3, cdims, periods, 0,  &
                               cart_comm, ierror )
        call ADCL_Topology_create ( cart_comm, topo, ierror )
        call MPI_Cart_shift ( cart_comm, 0, 1, neighbors(1), neighbors(2), ierror )
        call MPI_Cart_shift ( cart_comm, 1, 1, neighbors(3), neighbors(4), ierror )
        call MPI_Cart_shift ( cart_comm, 2, 1, neighbors(5), neighbors(6), ierror )
        

!!......hwidth=1, nc=1 
        dims(1) = 34
        dims(2) = 34
        dims(3) = 66
        hwidth = 1
        nc     = 1
        call adcl_vmap_halo_allocate( hwidth, vmap, ierror ) 
        if ( ADCL_SUCCESS .ne. ierror) print *, "vmap_halo_allocate not successful"   
        call adcl_vector_register_generic ( 3,  dims, nc, vmap, MPI_DOUBLE_PRECISION, & 
                                    data2, vec, ierror)
        call ADCL_Request_create ( vec, topo, ADCL_FNCTSET_NEIGHBORHOOD, &
             request, ierror )

        call set_data_4D ( data2, rank, dims, hwidth, nc ) 

        call ADCL_Request_start( request, ierror )
        call check_data_4D ( data2, rank, dims, hwidth, neighbors, nc )

        call ADCL_Request_free ( request, ierror )
        call ADCL_Vector_deregister ( vec, ierror )
        call ADCL_Vmap_free ( vmap, ierror )

!!.......done
        call ADCL_Topology_free ( topo, ierror )
        call MPI_Comm_free ( cart_comm, ierror )

        call ADCL_Finalize ( ierror )
        call MPI_Finalize ( ierror )
      end program two

!****************************************************************************
!****************************************************************************
!****************************************************************************

      subroutine set_data_4D ( data, rank, dims, hwidth, nc )

        implicit none
        include 'ADCL.inc'

        integer rank, dims(3), hwidth, nc
        double precision data(dims(1),dims(2), dims(3),nc)

        integer i, j, k, l

        do i=1,hwidth
           do j = 1, dims(2)
              do k = 1, dims(3)
                 do l = 1, nc
                    data(i,j,k,l) = -1
                 end do
              end do
           end do
        end do

        do i = dims(1)-hwidth+1, dims(1)
           do j = 1, dims(2)
              do k = 1, dims(3)
                 do l = 1, nc
                    data(i, j, k, l) = -1
                 end do
              end do
           end do
        end do

        do i=1,dims(1)
           do j = 1, hwidth
              do k = 1, dims(3)
                 do l = 1, nc
                    data(i, j, k, l) = -1
                 end do
              end do
           end do
        end do


        do i = 1, dims(1)
           do j = dims(2)-hwidth+1, dims(2)
              do k = 1, dims(3)
                 do l = 1, nc
                    data(i, j, k, l) = -1
                 end do
              end do
           end do
        end do


        do i = 1, dims(1)
           do j = 1, dims(2)
              do k = 1, hwidth
                 do l = 1, nc
                    data(i, j, k, l) = -1
                 end do
              end do
           end do
        end do

        do i = 1,dims(1)
           do j = 1, dims(2)
              do k = dims(3)-hwidth+1, dims(3)
                 do l = 1, nc
                    data(i, j, k, l) = -1
                 end do
              end do
           end do
        end do


        do i = hwidth+1, dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k = hwidth+1, dims(3)-hwidth
                 do l = 1, nc
                    data(i,j,k,l) = rank
                 end do
              end do
           end do
        end do


        return
      end subroutine set_data_4D



!****************************************************************************
!****************************************************************************
!****************************************************************************
      subroutine dump_vector_4D ( data, rank, dims, nc )

        implicit none
        include 'ADCL.inc'

        integer rank, dims(3), nc
        double precision data(dims(1), dims(2), dims(3), nc)
        integer i, j, k


        if ( nc .le. 1 ) then
           do i = 1, dims(1)
              do j = 1, dims(2)
                 write (*,*) 'Rank: ',rank, 'dim(1)=',dims(1), 'dim(2)=',dims(2),&
                      (data(i,j,k,1), k=1,dims(3))
              end do
           end do
        else if ( nc .eq. 2 ) then 
           do i = 1, dims(1)
              do j = 1, dims(2)
                 write (*,*) 'Rank: ',rank, 'dim(1)=',dims(1), 'dim(2)=',dims(2),&
                      (data(i,j,k,1), data(i,j,k,2), k=1,dims(3))
              end do
           end do
        end if


        return
      end subroutine dump_vector_4D


!****************************************************************************
!****************************************************************************
!****************************************************************************

      subroutine check_data_4D ( data, rank, dims, hwidth, neighbors, nc ) 

        implicit none
        include 'ADCL.inc'

        integer rank,  dims(3), hwidth, nc, neighbors(6)
        double precision data(dims(1),dims(2),dims(3), nc)
        integer lres, gres, i, j, k, l, ierr
        double precision should_be
        
        lres = 1

        if ( neighbors(1) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(1)
        endif
        do i=1,hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k = hwidth+1, dims(3)-hwidth
                 do l = 1, nc 
                    if ( data(i,j,k, l) .ne. should_be ) then
                       write (*,*) rank, ': element ',i,j,k,l, ' is ', &
                            data(i,j,k,l), ' should be ', should_be                  
                       lres = 0 
                    endif
                 end do
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
              do k = hwidth+1, dims(3)-hwidth
                 do l = 1, nc 
                    if ( data(i, j, k, l) .ne. should_be ) then 
                       write (*,*) rank, ': element ',i,j,k,l, ' is ',  &
                            data(i,j,k,l), ' should be ', should_be                  
                       lres = 0
                    end if
                 end do
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
              do k = hwidth+1, dims(3)-hwidth
                 do l = 1, nc 
                    if ( data(i,j, k, l) .ne. should_be ) then
                       write (*,*) rank, ': element ',i,j,k,l, ' is ',  &
                            data(i,j,k,l), ' should be ', should_be                  
                       lres = 0                    
                    endif
                 end do
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
              do k = hwidth+1, dims(3)-hwidth
                 do l = 1, nc 
                    if ( data(i, j, k, l) .ne. should_be ) then 
                       write (*,*) rank, ': element ',i,j,k,l, ' is ',  &
                            data(i,j,k,l), ' should be ', should_be                  
                       lres = 0
                    endif
                 end do
              end do
           end do
        end do

        if ( neighbors(5) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(5)
        endif
        do i=hwidth+1,dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k = 1, hwidth
                 do l = 1, nc 
                    if ( data(i,j, k, l) .ne. should_be ) then
                        write (*,*) rank, ': element ',i,j,k,l, ' is ', &
                             data(i,j,k,l), ' should be ', should_be                  
                       lres = 0                    
                    endif
                 end do
              end do
           end do
        end do

        if ( neighbors(6) .eq. MPI_PROC_NULL ) then 
           should_be = -1
        else
           should_be = neighbors(6)
        endif
        do i = hwidth+1, dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k = dims(3)-hwidth+1, dims(3)
                 do l = 1, nc 
                    if ( data(i, j, k, l) .ne. should_be ) then 
                       write (*,*) rank, ': element ',i,j,k,l, ' is ', &
                            data(i,j,k,l), ' should be ', should_be                  
                       lres = 0
                    endif
                 end do
              end do
           end do
        end do


        do i = hwidth+1, dims(1)-hwidth
           do j = hwidth+1, dims(2)-hwidth
              do k = hwidth+1, dims(3)-hwidth
                 do l = 1, nc 
                    if ( data(i,j,k,l) .ne. rank ) then 
                       write (*,*) rank, ': element ',i,j,k,l, ' is ', &
                            data(i,j,k,l), ' should be ', rank                  
                       lres = 0
                    endif
                 end do
              end do
           end do
        end do


        call MPI_Allreduce ( lres, gres, 1, MPI_INTEGER, MPI_MIN, MPI_COMM_WORLD, ierr)
        if ( gres .eq. 0 ) then
!           call dump_vector_4D ( data, rank, dims )
           if ( rank .eq. 0 ) then
              write (*,*) '3-D Fortran testsuite hwidth =', hwidth, &
                   'nc = ', nc, ' failed'  
           end if
        else
           if ( rank .eq. 0 ) then
              write (*,*) '3-D Fortran testsuite hwidth =', hwidth, &
                   'nc = ', nc, ' passed'  
           end if
        end if


        return
      end subroutine check_data_4D

