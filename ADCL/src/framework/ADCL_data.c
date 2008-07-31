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
static int get_int_data_from_xml (char *str, int *res);
static int get_str_data_from_xml (char *str, char **dest);

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
    /* Attribute information */
    data->d_asmaxnum = e->em_orgfnctset->fs_attrset->as_maxnum;
    data->d_attrvals = (int *)malloc( data->d_asmaxnum * sizeof(int) );
    /* Initialization */
    for (i=0; i<data->d_asmaxnum ; i++){
        data->d_attrvals[i] = -1;
    }
    if ( e->em_perfhypothesis ) {
        for (i=0; i<data->d_asmaxnum ; i++){
            if( e->em_hypo.h_attr_confidence[i] > 1 ) {
                data->d_attrvals[i] = e->em_fnctset.fs_fptrs[0]->f_attrvals[i];
            }
        }
    }
    /* Function set and winner function */
    data->d_fsname = strdup ( e->em_orgfnctset->fs_name );
    data->d_wfname = strdup ( e->em_wfunction->f_name );

    return ADCL_SUCCESS;
}

void ADCL_data_free ( void )
{
    int i, last;
    ADCL_data_t *data;
#ifdef ADCL_KNOWLEDGE_TOFILE
    ADCL_data_dump_to_file ( );
#endif
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
        fp = fopen ("ADCL.xml", "w");
        last = ADCL_array_get_last ( ADCL_data_array );
        fprintf ( fp, "<?xml version=\"1.0\" ?>\n<?xml-stylesheet"
                  " type=\"text/xsl\" href=\"ADCL.xsl\"?>\n<ADCL>\n" );
        fprintf ( fp, "  <NUM>%d</NUM>\n", last+1 );

        for ( i=0; i<=last; i++ ) {
            fprintf ( fp, "  <RECORD>\n" );
            data = ( ADCL_data_t * ) ADCL_array_get_ptr_by_pos( ADCL_data_array, i );
            /* Topology information */
            fprintf ( fp, "    <TOPO>\n" );
            tndims = data->d_tndims;
            fprintf ( fp, "      <NDIM>%d</NDIM>\n", tndims );
            fprintf ( fp, "      <PERIOD>\n");
            for ( j=0; j<tndims; j++) {
                fprintf ( fp, "        <DIM>%d</DIM>\n", data->d_tperiods[j] );
            }
            fprintf ( fp, "      </PERIOD>\n");
            fprintf ( fp, "    </TOPO>\n" );
            /* Vector information */
            fprintf ( fp, "    <VECT>\n" );
            vndims = data->d_vndims;
            fprintf ( fp, "      <NDIM>%d</NDIM>\n", vndims );
            fprintf ( fp, "      <DIMS>\n");            
            for ( j=0; j<vndims; j++) {
                fprintf ( fp, "        <DIM>%d</DIM>\n", data->d_vdims[j] );
            }
            fprintf ( fp, "      </DIMS>\n");
            fprintf ( fp, "      <NC>%d</NC>\n", data->d_nc );
            fprintf ( fp, "      <HWIDTH>%d</HWIDTH>\n", data->d_hwidth );
            fprintf ( fp, "      <COMTYPE>%d</COMTYPE>\n", data->d_comtype );
            fprintf ( fp, "    </VECT>\n" );
            /* Attribute information */
            fprintf ( fp, "    <ATTR>\n" );
            fprintf ( fp, "      <NUM>%d</NUM>\n", data->d_asmaxnum );
            fprintf ( fp, "      <ATTRVALS>\n" );
            for ( j=0; j<data->d_asmaxnum; j++) {
                fprintf ( fp, "        <VAL>%d</VAL>\n", data->d_attrvals[j] );
            }
            fprintf ( fp, "      </ATTRVALS>\n" );
            fprintf ( fp, "    </ATTR>\n" );
            /* Function set and winner function */
            fprintf ( fp, "    <FUNC>\n" );
            fprintf ( fp, "      <FNCTSET>%s</FNCTSET>\n", data->d_fsname );
            fprintf ( fp, "      <WINNER>%s</WINNER>\n", data->d_wfname );
            fprintf ( fp, "    </FUNC>\n" );
            fprintf ( fp, "  </RECORD>\n" );
        }
        fprintf ( fp, "</ADCL>" );
        fclose ( fp );
    }

    return;
}

void ADCL_data_read_from_file ( void )
{
    int i, j, ndata;
    int nchar = 80;
    char *line;
    ADCL_data_t *data;
    FILE *fp;

    fp = fopen ("ADCL.xml", "r");
    if ( NULL == fp ) {
        return;
    }
    line = (char *)malloc( nchar * sizeof(char) );
    /* Read the XML file line by line */
    fgets( line, nchar, fp ); /* XML header lines */
    fgets( line, nchar, fp );
    fgets( line, nchar, fp ); /* ADCL Tag */
    fgets( line, nchar, fp );
    get_int_data_from_xml ( line, &ndata );
    for ( i=0; i<ndata; i++ ) {
        data = (ADCL_data_t *) calloc (1, sizeof(ADCL_data_t));
        if ( NULL == data ) {
            return;
        }

        /* Internal info for object management */
        data->d_id = ADCL_local_id_counter++;
        ADCL_array_get_next_free_pos ( ADCL_data_array, &data->d_findex );
        ADCL_array_set_element ( ADCL_data_array, data->d_findex, data->d_id, data );
        data->d_refcnt = 1;
	fgets( line, nchar, fp ); /* RECORD Tag */

        /* Topology information */
        fgets( line, nchar, fp ); /* TOPO Tag */
        fgets( line, nchar, fp ); /* NDIM Tag */
        get_int_data_from_xml ( line, &data->d_tndims );
        fgets( line, nchar, fp ); /* PERIOD Tag */
        data->d_tperiods = (int *)malloc( data->d_tndims*sizeof(int) );
        for ( j=0; j<data->d_tndims; j++ ) {
            fgets( line, nchar, fp ); /* DIM Tag */
            get_int_data_from_xml ( line, &(data->d_tperiods[j]) );
        }
        fgets( line, nchar, fp ); /* Close PERIOD Tag */
        fgets( line, nchar, fp ); /* Close TOPO Tag */

        /* Vector information */
        fgets( line, nchar, fp ); /* VECT Tag */
        fgets( line, nchar, fp ); /* NDIM Tag */
        get_int_data_from_xml ( line, &data->d_vndims );
        fgets( line, nchar, fp ); /* DIMS Tag */
        data->d_vdims = (int *)malloc( data->d_vndims*sizeof(int) );
        for ( j=0; j<data->d_vndims; j++ ) {
            fgets( line, nchar, fp ); /* DIM Tag */
	    get_int_data_from_xml ( line, &(data->d_vdims[j]) );
	}
        fgets( line, nchar, fp ); /* Close DIMS Tag */
        fgets( line, nchar, fp ); /* NC Tag */
        get_int_data_from_xml ( line, &data->d_nc );
        fgets( line, nchar, fp ); /* HWIDTH Tag */
        get_int_data_from_xml ( line, &data->d_hwidth );
        fgets( line, nchar, fp ); /* COMTYPE Tag */
        get_int_data_from_xml ( line, &data->d_comtype );
        fgets( line, nchar, fp ); /* Close VECT Tag */

        /* Attribute information */
        fgets( line, nchar, fp ); /* ATTR Tag */
        fgets( line, nchar, fp ); /* NUM Tag */
        get_int_data_from_xml ( line, &data->d_asmaxnum );
        fgets( line, nchar, fp ); /* ATTRVALS Tag */
        data->d_attrvals = (int *)malloc( data->d_asmaxnum*sizeof(int) );
        for ( j=0; j<data->d_asmaxnum; j++ ) {
            fgets( line, nchar, fp ); /* VAL Tag */
            get_int_data_from_xml ( line, &(data->d_attrvals[j]) );
        }
        fgets( line, nchar, fp ); /* Close ATTRVALS Tag */
        fgets( line, nchar, fp ); /* Close ATTR Tag */

        /* Function set and winner function */
        fgets( line, nchar, fp ); /* FUNC Tag */
        fgets( line, nchar, fp ); /* FNCTSET Tag */
        get_str_data_from_xml ( line, &data->d_fsname );
        fgets( line, nchar, fp ); /* WINNER  Tag */
        get_str_data_from_xml ( line, &data->d_wfname );
        fgets( line, nchar, fp ); /* Close FUNC Tag */
        fgets( line, nchar, fp ); /* Close RECORD Tag */
    }
    fclose ( fp );
    free ( line );
    return;
}

static int get_int_data_from_xml (char *str, int *res)
{
    char *n, *p;
    char *ext;
    int num;
    int ret = ADCL_ERROR_INTERNAL;
    n = strstr (str,">");
    p = strstr (str,"/");
    num = p-n-2;
    if ( NULL != n && NULL != p && num>0 ) {
        ext = (char *)calloc( num, sizeof(char) );
        strncpy ( ext, n+1, num );
        *res = atoi( ext );
        free( ext );
        ret = ADCL_SUCCESS;
    }
    return ret;
}

static int get_str_data_from_xml (char *str, char **dest)
{
    char *n, *p;
    int num;
    int ret = ADCL_ERROR_INTERNAL;
    n = strstr (str,">");
    p = strstr (str,"/");
    if ( NULL != n && NULL != p ) {
        num = p-n-2;
        (*dest) = (char *)calloc( num, sizeof(char) );
        strncpy ((*dest), n+1, num);
        ret = ADCL_SUCCESS;
    }
    return ret;
}
