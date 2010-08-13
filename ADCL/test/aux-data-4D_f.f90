module auxdata4df
   use adcl
contains

!****************************************************************************
      subroutine DUMP_VECTOR ( arr, rank, dims )

        implicit none
        include 'ADCL.inc'
        character(100) ::  fmt

        integer rank, dims(4)
        DATATYPE arr(dims(1), dims(2), dims(3), dims(4))
        integer :: i, j, k, l

        write (fmt,'(a,i0,a)') "(i3,a1,2i5,", dims(1), "f12.5)"
        do l = 1, dims(4)
           do k = 1, dims(3)
             do j = 1, dims(2)
                write (*,fmt) rank, ":", l, k, j, ":", (arr(i,j,k,l), i=1,dims(1))
             end do
          end do
        end do
        return
      end subroutine DUMP_VECTOR


!******************************************************m*********************
      subroutine DUMP_VECTOR_MPI ( arr, dims, comm )

        implicit none
        include 'ADCL.inc'

        integer dims(4), comm
        DATATYPE arr(dims(1), dims(2), dims(3), dims(4))
        integer i, j, k, l, iproc, rank, size, ierror
        character(100) ::  fmt

        call MPI_Comm_rank ( comm, rank, ierror )
        call MPI_Comm_size ( comm, size, ierror )

        write (fmt,'(a,i0,a)') "(i3,a1,2i5,a1,", dims(1), "f12.5)"

        print *, fmt
        do iproc = 0, size-1
           if ( iproc .eq. rank) then
              do l = 1, dims(4) 
                 do k = 1, dims(3)
                    do j = 1, dims(2)
                       write (*,'(i3,a1,2i5,a1,6f12.5)') rank, ":", l, k, j, ":", (arr(i,j,k,l), i=1,dims(1))
                    end do
                 end do
              end do
           end if 
           call MPI_Barrier ( comm, ierror )
        end do
        return
      end subroutine DUMP_VECTOR_MPI

end module auxdata4df

