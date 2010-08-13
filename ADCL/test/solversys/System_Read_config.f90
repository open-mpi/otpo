!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
        subroutine System_Read_config ( npr, nsol, ierror )

!...This routine reads the configarution file

        USE globale_daten
        USE numerik_daten

        implicit none

        include 'mpif.h'

        integer :: npr, nsol, ierror, punkt, status
        integer :: anfang, ende, line, i
        integer, dimension (7) :: iarraz
        integer :: rank, size

        double precision, dimension ( 2 ) :: rarraz
        logical :: ex, belegt
        logical, save :: isallocated
        character *250 :: s 
        character*7 :: s_chen


        call MPI_Comm_rank ( MPI_COMM_WORLD, rank, ierror )
        call MPI_Comm_size ( MPI_COMM_WORLD, size, ierror )
        line = 0
        isallocated = .false.

!...Preset some numerical parameters

        nit    = 300
        tol    = 1.0e-06
        subtol = 1.0e-06
        verbose = 1

        if ( rank.eq.0 ) then

!...Nachschauen, ob das File ueberhaupt existiert        
           inquire ( file = 'System.config', exist = ex )

!...Falls es existiert, lesen des Files
           if ( ex ) then
           
              open ( unit=10, file = 'System.config' )

 100          continue
              
              anfang = 0
              ende   = 0
              belegt = .false.
              
              read ( 10, 99, err=130, end=120 ) s
              line = line + 1

!...Zeilen, die mit einem # beginnen, werden als Kommentare
!   ueberlesen

              if ( s(1:1).eq.'#') then
                 goto 100
              end if

!...Suchen nach den Schluesselwoertern: Jedes Schluesselwort endet mit einem 
!   Doppelpunkt!
           
              punkt = index ( s, ':' ) 
           
!...Suche nach dem Beginn des numerischen Wertes

              i = punkt 
 110          continue
              i = i + 1
              if ( s(i:i).ne.' '.and.( s(i-1:i-1).eq.' '.or. &
                  s(i-1:i-1).eq.':')) then 
                 anfang = i
              end if
              
              if ( s(i:i).eq.' '.and.s(i-1:i-1).ne.' '.and.&
                   s(i-1:i-1).ne.':' ) then
                 ende = i
                 belegt = .true.
              end if
              
              if ( .not.belegt ) goto 110
              
!...Lesen des Schluesselwortes aus dem String
              
              read ( s(1:punkt), 991 ) s_chen
              
              select case ( s_chen )
                 case ( 'nit:   ' )
                    read ( s(anfang:ende), 992 ) nit
                 case ( 'tol:   ' )
                    read ( s(anfang:ende), 993 ) tol
                 case ( 'subtol:' )
                    read ( s(anfang:ende), 993 ) subtol
                 case ( 'verbos:' )
                    read ( s(anfang:ende), 992 ) verbose
                 case ('nprobs:') 
                    read (s(anfang:ende),992) maxnproblem
                    allocate (problemsx(maxnproblem), &
                         problemsy(maxnproblem), problemsz(maxnproblem), &
                    stat=status)
                    if ( status.gt.0 ) then
                       write (*,*) rank, ' : Error allocating memory '
                    end if
                    read ( 10, 99, err=130, end=120 ) s
                    read ( s, *)(problemsx(i), i= 1, maxnproblem)
                    read ( 10, 99, err=130, end=120 ) s
                    read ( s, *)(problemsy(i), i= 1, maxnproblem)
                    read ( 10, 99, err=130, end=120 ) s
                    read ( s, *)(problemsz(i), i= 1, maxnproblem)
                    line = line + 3
                 case ('nsolv: ')
                    read (s(anfang:ende), 992) maxnsolver
                    allocate (solvarr(maxnsolver),  stat=status)
                    if ( status.gt.0 ) then
                       write (*,*) rank, ' : Error allocating memory '
                    end if
                    read ( 10, 99, err=130, end=120 ) s
                    read (s, *)(solvarr(i), i= 1, maxnsolver)
                    line = line + 1
                 case default
                    write (*,*) 'Unknown keqword in System.config, line :',  &
                        line

              end select
                 
              goto 100
                 
 130          continue
	      write (*,*) 'Error reading the configuration file'

 120          continue
!...Close configuration file
              close ( 10 )

           else
              write (*,*) 'No File System.config in this directory'
           end if

        end if


!...Distribute the collected data
        if ( rank .eq. 0 ) then 
           iarraz (1) = nit
           iarraz (2) = maxnproblem
           iarraz (3) = maxnsolver
           iarraz (4) = verbose  
           rarraz (1) = tol
           rarraz (2) = subtol
        endif

        call MPI_Bcast ( iarraz, 4, MPI_INTEGER, 0, MPI_COMM_WORLD,  &
             ierror )
        
        call MPI_Bcast ( rarraz, 2, MPI_DOUBLE_PRECISION, 0, MPI_COMM_WORLD,  &
             ierror )

        if ( rank .ne. 0 ) then
           nit         = iarraz (1)
           maxnproblem = iarraz (2)
           maxnsolver  = iarraz (3)
           verbose     = iarraz (4)
           
           tol    = rarraz (1)
           subtol = rarraz (2)
           
           if ( .NOT.isallocated) then
              allocate (solvarr(maxnsolver), problemsx(maxnproblem),  &
                   problemsy(maxnproblem), problemsz(maxnproblem),    &
                   stat=status)
              if ( status.gt.0 ) then
                 write (*,*) rank, ' : Error allocating memory '
              else
                 isallocated = .true. 
              end if
           end if
        end if
           
        call MPI_Bcast (problemsx, maxnproblem, MPI_INTEGER, 0, MPI_COMM_WORLD, ierror)
        call MPI_Bcast (problemsy, maxnproblem, MPI_INTEGER, 0, MPI_COMM_WORLD, ierror)
        call MPI_Bcast (problemsz, maxnproblem, MPI_INTEGER, 0, MPI_COMM_WORLD, ierror)
        call MPI_Bcast (solvarr, maxnsolver, MPI_INTEGER, 0, MPI_COMM_WORLD, ierror)


        
        
99      format ( a )
991     format ( a7)
992     format ( i5 )
993     format ( e7.5 )
      
        npr  = maxnproblem
        nsol = maxnsolver
        ierror = 0
        
        return
        end subroutine System_Read_config
        
        
        
        
