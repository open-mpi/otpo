/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL_internal.h"

static int ADCL_local_id_counter = 0;
ADCL_array_t *ADCL_data_array = NULL;

static void ADCL_data_dump_to_file ( void );

int ADCL_data_create ( ADCL_emethod_t *e ) 
{

    ADCL_data_t *data;
    ADCL_topology_t *topo = e->em_topo;
    ADCL_vector_t   *vec  = e->em_vec;
    int i, topo_status;
    int *dims, *coords;

    /* For now we support only cartesian topology */
    MPI_Topo_test ( topo->t_comm, &topo_status );
    if ( ( MPI_CART != topo_status ) || ( ADCL_VECTOR_NULL == vec ) ) {
        return ADCL_INVALID_ARG;
    }
    data = (ADCL_data_t *) calloc (1, sizeof(ADCL_data_t));
    if ( NULL == data ) {
        return ADCL_NO_MEMORY;
    }
    /* Internal info for object management */
    data->d_id = ADCL_local_id_counter++;
    ADCL_array_get_next_free_pos ( ADCL_data_array, &data->d_findex );
    ADCL_array_set_element ( ADCL_data_array, data->d_findex, data->d_id, data );
    data->d_refcnt = 1;
    /* Topology information */
    data->d_tndims = topo->t_ndims;
    data->d_tperiods = (int *)malloc( data->d_tndims*sizeof(int) );
    dims = (int *)malloc( data->d_tndims*sizeof(int) );
    coords = (int *)malloc( data->d_tndims*sizeof(int) );
    MPI_Cart_get ( topo->t_comm, topo->t_ndims, dims, data->d_tperiods, coords );
    free( dims );
    free( coords );
    /* Vector information */
    data->d_vndims = vec->v_ndims;
    data->d_vdims = (int *)malloc(data->d_vndims*sizeof(int) );
    for ( i=0; i<data->d_vndims ;i++ ) {
        data->d_vdims[i] = vec->v_dims[i];
    }
    data->d_nc = vec->v_nc;
    data->d_hwidth = vec->v_hwidth;
    data->d_comtype = vec->v_comtype;
    /* Function set and winner function */
    data->d_fsname = strdup ( e->em_orgfnctset->fs_name );
    data->d_wfname = strdup ( e->em_wfunction->f_name );

    return ADCL_SUCCESS;
}

void ADCL_data_free ( void )
{
    int i, last;
    ADCL_data_t *data;

    ADCL_data_dump_to_file ( );

    last = ADCL_array_get_last ( ADCL_data_array );
    /* Free all the data objects */
    for ( i=0; i<= last; i++ ) {
        data = ( ADCL_data_t * ) ADCL_array_get_ptr_by_pos( ADCL_data_array, i );
        if ( NULL != data  ) {
            if ( NULL != data->d_tperiods ) {
                free ( data->d_tperiods );
            }
            if ( NULL != data->d_vdims ) {
                free ( data->d_vdims );
            }
            if ( NULL != data->d_fsname ) {
                free ( data->d_fsname );
            }
            if ( NULL != data->d_wfname ) {
                free ( data->d_wfname );
            }
            ADCL_array_remove_element ( ADCL_data_array, data->d_findex );
            free ( data );
        }
    }
    return;
}

int ADCL_data_find ( ADCL_emethod_t *e, ADCL_data_t **found_data )
{
    ADCL_data_t *data;
    ADCL_topology_t *topo = e->em_topo;
    ADCL_vector_t   *vec  = e->em_vec;
    int ret = ADCL_UNEQUAL;
    int i, j, last, explored_data, found, result;
    int *dims, *periods, *coords;

    if ( ADCL_VECTOR_NULL == vec ) {
        return ret;
    }
    last = ADCL_array_get_last ( ADCL_data_array );
    explored_data = e->em_explored_data;
    if ( last > explored_data ) {
        for ( i=(explored_data+1); i<= last; i++ ) {
            data = ( ADCL_data_t * ) ADCL_array_get_ptr_by_pos( ADCL_data_array, i );
            if ( ( topo->t_ndims    == data->d_tndims  ) &&
                 ( vec->v_ndims     == data->d_vndims  ) &&
                 ( vec->v_nc        == data->d_nc      ) &&
                 ( vec->v_comtype   == data->d_comtype ) &&
                 ( vec->v_hwidth    == data->d_hwidth  ) &&
                 ( 0 == strncmp (data->d_fsname,
                                 e->em_orgfnctset->fs_name,
                                 strlen(e->em_orgfnctset->fs_name))) ) {
                found = i;
                periods = (int *)malloc( topo->t_ndims*sizeof(int) );
                dims = (int *)malloc( topo->t_ndims*sizeof(int) );
                coords = (int *)malloc( topo->t_ndims*sizeof(int) );

                MPI_Cart_get ( topo->t_comm, topo->t_ndims, dims, periods, coords );
                for ( j=0; j<topo->t_ndims; j++ ) {
                    if ( periods[j] != data->d_tperiods[j] ) {
                        found = -1;
                        break;
                    }
                }

                free( periods );
                free( dims );
                free( coords );

                for ( j=0 ; j<vec->v_ndims; j++ ){
                    if ( vec->v_dims[j] != data->d_vdims[j] ) {
                        found = -1;
                        break;
                    }
                }
            }
            else {
                continue;
            }
            if ( found == -1 ) {
                continue;
            }
	    else {
                *found_data = data;
                ret = ADCL_IDENT;
                ADCL_printf("#%d An identical problem is found, winner is %s \n",
                            topo->t_rank, data->d_wfname);
            }
        }
    }
    return ret;
}

void ADCL_data_dump_to_file ( void )
{
    int i, j, rank, last, tndims, vndims ;
    ADCL_data_t *data;
    FILE *fp;

    rank = 0;
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    if ( 0 == rank ) {
        fp = fopen ("ADCL.dat", "w");
        last = ADCL_array_get_last ( ADCL_data_array );
        fprintf ( fp, "%d\n", last+1 );

        for ( i=0; i<=last; i++ ) {
            data = ( ADCL_data_t * ) ADCL_array_get_ptr_by_pos( ADCL_data_array, i );
            tndims = data->d_tndims;
            fprintf ( fp, "%d\n", tndims );
            for ( j=0; j<tndims; j++) {
                fprintf ( fp, "%d\n", data->d_tperiods[j] );
            }
            vndims = data->d_vndims;
            fprintf ( fp, "%d\n", vndims );
            for ( j=0; j<vndims; j++) {
                fprintf ( fp, "%d\n", data->d_vdims[j] );
            }
            fprintf ( fp, "%d\n", data->d_nc );
            fprintf ( fp, "%d\n", data->d_hwidth );
            fprintf ( fp, "%d\n", data->d_comtype );
            fprintf ( fp, "%s\n", data->d_fsname );
            fprintf ( fp, "%s\n", data->d_wfname );
        }
        fclose ( fp );
    }
    return;
}

void ADCL_data_read_from_file ( void )
{
    int i, j, ndata, len;
    int nchar = 80;
    char *name;
    ADCL_data_t *data;
    FILE *fp;

    fp = fopen ("ADCL.dat", "r");
    if ( NULL == fp ) {
        return;
    }
    fscanf ( fp, "%d", &ndata );
    for ( i=0; i<ndata; i++ ) {
        data = (ADCL_data_t *) calloc (1, sizeof(ADCL_data_t));
        if ( NULL == data ) {
            return ADCL_NO_MEMORY;
        }
        /* Internal info for object management */
        data->d_id = ADCL_local_id_counter++;
        ADCL_array_get_next_free_pos ( ADCL_data_array, &data->d_findex );
        ADCL_array_set_element ( ADCL_data_array, data->d_findex, data->d_id, data );
        data->d_refcnt = 1;
        /* Topology information */
        fscanf ( fp, "%d", &data->d_tndims );
        data->d_tperiods = (int *)malloc( data->d_tndims*sizeof(int) );
        for ( j=0; j<data->d_tndims; j++ ) {
            fscanf ( fp, "%d", &data->d_tperiods[j] );
        }
        /* Vector information */
        fscanf ( fp, "%d", &data->d_vndims );
        data->d_vdims = (int *)malloc(data->d_vndims*sizeof(int) );
        for ( j=0; j<data->d_vndims; j++ ) {
            fscanf ( fp, "%d", &data->d_vdims[j] );
        }
        fscanf ( fp, "%d", &data->d_nc );
        fscanf ( fp, "%d", &data->d_hwidth );
        fscanf ( fp, "%d", &data->d_comtype );
        /* Function set and winner function */
        name = (char *)malloc( nchar * sizeof(char) );
        getline( &name, &nchar, fp );
        getline( &name, &nchar, fp );
        len = strlen(name) - 1;
        data->d_fsname = (char *)calloc ( len , sizeof(char) );
        strncpy( data->d_fsname, name, len );
        getline( &name, &nchar, fp );
        len = strlen(name) - 1;
        data->d_wfname = (char *)calloc ( len , sizeof(char) );
        strncpy( data->d_wfname, name, len );
    }
    fclose ( fp );
    return;
}
