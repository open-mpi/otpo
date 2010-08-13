!
! Copyright (c) 2006-2007      University of Houston. All rights reserved.
! Copyright (c) 2009           HLRS. All rights reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!
!#ifdef ADCL_DUMMY_MPI
!       include  "ADCL_dummy_mpi.h"
!#else
       include "mpif.h"
!#endif
       
       integer ADCL_SUCCESS, ADCL_NO_MEMORY, ADCL_ERROR_INTERNAL
       integer ADCL_USER_ERROR, ADCL_UNDEFINED, ADCL_NOT_FOUND

       parameter ( ADCL_SUCCESS        = 0 )
       parameter ( ADCL_NO_MEMORY      = -1)
       parameter ( ADCL_ERROR_INTERNAL = -2)
       parameter ( ADCL_USER_ERROR     = -3)
       parameter ( ADCL_UNDEFINED      = -4)
       parameter ( ADCL_NOT_FOUND      = -5)

       integer ADCL_INVALID_ARG, ADCL_INVALID_NDIMS
       integer ADCL_INVALID_DIMS, ADCL_INVALID_HWIDTH
       integer ADCL_INVALID_DAT, ADCL_INVALID_DATA
       integer ADCL_INVALID_COMTYPE, ADCL_INVALID_COMM
       integer ADCL_INVALID_REQUEST, ADCL_INVALID_NC
       integer ADCL_INVALID_TYPE, ADCL_INVALID_TOPOLOGY
       integer ADCL_INVALID_ATTRIBUTE, ADCL_INVALID_ATTRSET
       integer ADCL_INVALID_FUNCTION, ADCL_INVALID_WORK_FUNCTION_PTR
       integer ADCL_INVALID_FNCTSET, ADCL_INVALID_VECTOR
       integer ADCL_INVALID_VECTSET, ADCL_INVALID_DIRECTION
       integer ADCL_INVALID_VMAP

       parameter ( ADCL_INVALID_ARG     = 10)
       parameter ( ADCL_INVALID_NDIMS   = 11)
       parameter ( ADCL_INVALID_DIMS    = 12)
       parameter ( ADCL_INVALID_HWIDTH  = 13)
       parameter ( ADCL_INVALID_DAT     = 14)
       parameter ( ADCL_INVALID_DATA    = 15)
       parameter ( ADCL_INVALID_COMTYPE = 16)
       parameter ( ADCL_INVALID_COMM    = 17)
       parameter ( ADCL_INVALID_REQUEST = 18)
       parameter ( ADCL_INVALID_NC      = 19)
       parameter ( ADCL_INVALID_TYPE    = 20)
       parameter ( ADCL_INVALID_TOPOLOGY = 21)
       parameter ( ADCL_INVALID_ATTRIBUTE = 22)
       parameter ( ADCL_INVALID_ATTRSET   = 23)
       parameter ( ADCL_INVALID_FUNCTION  = 24)
       parameter ( ADCL_INVALID_WORK_FUNCTION_PTR = 25)
       parameter ( ADCL_INVALID_FNCTSET    = 26)
       parameter ( ADCL_INVALID_VECTOR     = 27)
       parameter ( ADCL_INVALID_VECTSET    = 28)
       parameter ( ADCL_INVALID_DIRECTION  = 29)
       parameter ( ADCL_INVALID_VMAP       = 30)
       
       integer ADCL_VECTOR_NULL, ADCL_VECTSET_NULL
       integer ADCL_REQUEST_NULL, ADCL_TOPOLOGY_NULL
       integer ADCL_ATTRIBUTE_NULL, ADCL_ATTRSET_NULL
       integer ADCL_FUNCTION_NULL, ADCL_FNCTSET_NULL, ADCL_NULL_FNCT_PTR 
       integer ADCL_VMAP_NULL
 
       parameter ( ADCL_VECTOR_NULL    = -1)
       parameter ( ADCL_VECTSET_NULL   = -2)
       parameter ( ADCL_REQUEST_NULL   = -3)
       parameter ( ADCL_TOPOLOGY_NULL  = -4)
       parameter ( ADCL_ATTRIBUTE_NULL = -5)
       parameter ( ADCL_ATTRSET_NULL   = -6)
       parameter ( ADCL_FUNCTION_NULL  = -7)
       parameter ( ADCL_FNCTSET_NULL   = -8)
       parameter ( ADCL_NULL_FNCT_PTR  = -9)
       parameter ( ADCL_VMAP_NULL      = -10)

       integer ADCL_MAX_ATTRLEN, ADCL_MAX_NAMELEN
       parameter ( ADCL_MAX_ATTRLEN = 32 ) 
       parameter ( ADCL_MAX_NAMELEN = 32 )

       integer ADCL_DIRECTION_BOTH, ADCL_DIRECTION_LEFT_TO_RIGHT
       integer ADCL_DIRECTION_RIGHT_TO_LEFT
       parameter ( ADCL_DIRECTION_BOTH = 1 )
       parameter ( ADCL_DIRECTION_LEFT_TO_RIGHT = 2 )
       parameter ( ADCL_DIRECTION_RIGHT_TO_LEFT = 3 )

       integer ADCL_VECTOR_HALO, ADCL_VECTOR_ALL
       integer ADCL_VECTOR_UP_TRIANG, ADCL_VECTOR_LO_TRIANG
       integer ADCL_VECTOR_LIST, ADCL_VECTOR_ALLREDUCE
       integer ADCL_VECTOR_INPLACE, ADCL_VECTOR_REDUCE
       parameter ( ADCL_VECTOR_HALO = 1 ) 
       parameter ( ADCL_VECTOR_ALL = 2 )
       parameter ( ADCL_VECTOR_UP_TRIANG = 3 )
       parameter ( ADCL_VECTOR_LO_TRIANG = 4 )
       parameter ( ADCL_VECTOR_LIST      = 5 )
       parameter ( ADCL_VECTOR_ALLREDUCE = 6 )
       parameter ( ADCL_VECTOR_INPLACE   = 7 )
       parameter ( ADCL_VECTOR_REDUCE = 9 )

       integer ADCL_FNCTSET_NEIGHBORHOOD 
       !integer ADCL_FNCTSET_SHIFT_LTOR
       !integer ADCL_FNCTSET_SHIFT_RTOL
       integer ADCL_FNCTSET_ALLGATHERV, ADCL_FNCTSET_ALLREDUCE, ADCL_FNCTSET_REDUCE
       integer ADCL_FNCTSET_ALLTOALL
       integer ADCL_FNCTSET_ALLTOALLV
       parameter ( ADCL_FNCTSET_NEIGHBORHOOD = 0 )
       parameter ( ADCL_FNCTSET_ALLGATHERV   = 1 )
       parameter ( ADCL_FNCTSET_ALLREDUCE    = 2 )
       parameter ( ADCL_FNCTSET_ALLTOALLV    = 3 )
       parameter ( ADCL_FNCTSET_ALLTOALL     = 4 )
       parameter ( ADCL_FNCTSET_REDUCE     = 5 )

       ! this is not such a good idea: 
       ! parameter ( ADCL_FNCTSET_SHIFT_RTOL   = 0 )
       ! parameter ( ADCL_FNCTSET_SHIFT_LTOR   = 0 )
       
