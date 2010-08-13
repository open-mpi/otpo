#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int attr_test_gen_init ( int argc, char ** argv, char **outfilename, 
			             int *numattrs, int **nattrvals );
int attr_test_gen_fileheader ( FILE *outf, int numattrs, int numfuncs, int maxnattrvals );
int attr_test_gen_filetrail  ( FILE *outf, int numattrs, int numfuncs );
int attr_test_gen_attributes ( FILE *outf, int numattrs, int *nattrvals, int **attrs );
int attr_test_gen_adcl_funcs ( FILE *outf, int numfuncs, int numattrs, 
			                   int *nattrvals, int **attrs );
int attr_test_gen_funcs      ( FILE *outf, int numfuncs );
int attr_test_gen_func_prototypes ( FILE *outf, int numfuncs );

#define NUMMEAS 10

int main (int argc, char **argv )
{
    int numfuncs=1, maxnattrvals=0, numattrs, *nattrvals;
    int i, j, count;
    char *outfilename;
    int **attrs;
    FILE *outf;


    /* Aquire the required memory and read input files */
    attr_test_gen_init ( argc, argv, &outfilename, &numattrs, &nattrvals );

    printf("No. of attributes = %d outfilename = %s\n", numattrs, outfilename );
    attrs = (int **) malloc ( sizeof(int*) * numattrs ) ;
    for ( count=0, i=0; i< numattrs; i++ ) {
        attrs[i] = (int *) malloc ( sizeof(int) * nattrvals[i] );
        printf(" %d: no. of attr values %d ", i, nattrvals[i]);
        numfuncs *= nattrvals[i];
	if ( nattrvals[i] > maxnattrvals ) {
	    maxnattrvals = nattrvals[i];
	}
	for ( j=0; j<nattrvals[i]; j++ ) {
	    attrs[i][j] = count++;
	    printf (" %d ", attrs[i][j]);
	}
	printf ("\n");
    }
    printf(" maxnattrvals=%d numfuncs=%d\n", maxnattrvals, numfuncs );

    /* open file */
    outf = fopen(outfilename, "a");
    if ( NULL == outf ) {
        printf("attr_test_generator could not open %s for writing\n", outfilename );
        exit (-1);
    }

    /* write generic header information */
    attr_test_gen_fileheader ( outf, numattrs, numfuncs, maxnattrvals );

    /* generate the attribute objects */
    attr_test_gen_attributes ( outf, numattrs, nattrvals, attrs );

    /* generate the ADCL functions and the functionset */
    attr_test_gen_adcl_funcs ( outf, numfuncs, numattrs, nattrvals, attrs );

    fprintf(outf, "    for ( i=0; i<%d; i++ ) { \n", numfuncs*NUMMEAS);
    fprintf(outf, "        ADCL_Request_start ( req );\n");
    fprintf(outf, "    }\n");

    /* Free the ADCL objects and close the parallel environment*/
    attr_test_gen_filetrail ( outf, numattrs, numfuncs );

    /* generate the functions stubs */
    attr_test_gen_funcs ( outf, numfuncs );

    fclose (outf );
    for ( i=0; i< numattrs; i++ ) {
	free ( attrs[i] );
    }
    free ( attrs );
    free ( outfilename );
    free ( nattrvals );
    return 0;
}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int attr_test_gen_funcs ( FILE *outf, int numfuncs )
{
    int i;
    char string1[]="%d: In ";
    char string2[]="size=%d";
    
    for ( i=0; i<numfuncs; i++ ) {
        fprintf(outf, "\nvoid test_func_%d ( ADCL_Request req )\n", i);
        fprintf(outf, "{\n");
        fprintf(outf, "    MPI_Comm comm;\n");
        fprintf(outf, "    int rank, size;\n\n");
        fprintf(outf, "    ADCL_Request_get_comm ( req, &comm, &rank, &size );\n");
        fprintf(outf, "#ifdef VERBOSE\n");
        fprintf(outf, "    printf(\" %s test_func_%d %s\\%s\", rank, size);\n",
                string1, i, string2, "n");
        fprintf(outf, "#endif\n");
        fprintf(outf, "    usleep ( TIME%d );\n",i);
        fprintf(outf, "    return;\n");
        fprintf(outf, "}\n");
    }

    return 0;
}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static int get_next_attrval_combination ( int numattrs, int *nattrvals, 
					  int ** attrs, int *attrvals )

{
    int i, limit;

    for ( i=0; i< numattrs; i++  ){
        limit = nattrvals[i]-1;
        if ( attrvals[i] < ( attrs[i][limit] ) ) {
            attrvals[i]++;
            return 0;
        }
        else if ( attrvals[i] == attrs[i][limit] ) {
            attrvals[i] = attrs[i][0];
        }
    }

    return 1;
}

int attr_test_gen_adcl_funcs ( FILE *outf, int numfuncs, int numattrs, 
			       int *nattrvals, int **attrs )
{
    int *attrvals;
    int i, count=0, done=0;
    
    attrvals = (int *) malloc ( numattrs * sizeof(int));
    for ( i=0; i< numattrs; i++ ) {
	    attrvals[i] = attrs[i][0];
    }

    while ( !done ) {
        fprintf (outf, "    /* Generating ADCL function %d */\n", count );
        for ( i=0; i<numattrs; i++ ) {
            fprintf(outf, "    funcvals[%d] = %d;\n", i, attrvals[i]);
        }
        fprintf(outf, "    ADCL_Function_create ( (ADCL_work_fnct_ptr *)test_func_%d, "
                " attrset, funcvals, \"test_func_%d\", &(funcs[%d]));\n\n", 
                count, count, count );
        count++;
        done = get_next_attrval_combination ( numattrs, nattrvals, attrs, attrvals );
    }

    fprintf(outf, "    /* Generate the ADCL function set */\n");
    fprintf(outf, "    ADCL_Fnctset_create ( %d, funcs, \"trivial functions\", "
            "&fnctset);\n\n",  numfuncs );

    fprintf(outf, "    /* Generate the ADCL request */\n");
    fprintf(outf, "    ADCL_Request_create ( ADCL_VECTOR_NULL, topo, fnctset, &req );\n\n");

    free ( attrvals );
    return 0;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int attr_test_gen_attributes ( FILE *outf, int numattrs, int *nattrvals, int **attrs ) 
{
    int i,j;

    for (i=0; i<numattrs; i++ ) {
        fprintf(outf, "    /* Generate attribute %d */\n", i );
        for ( j=0; j<nattrvals[i]; j++ ) {
            fprintf(outf, "    attrvals[%d]=%d;\n", j, attrs[i][j] );
        }
        fprintf(outf, "    ADCL_Attribute_create ( %d, attrvals, NULL, NULL, &(attrs[%d]));\n\n", 
                nattrvals[i], i );
    }
    
    fprintf(outf, "    /* Create the attribute set */\n");
    fprintf(outf, "    ADCL_Attrset_create ( %d, attrs, &attrset );\n\n", numattrs );

    return 0;
}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int attr_test_gen_fileheader ( FILE *outf, int numattrs, int numfuncs, int maxnumattrvals )
{
    fprintf(outf, "#include <stdio.h> \n");
    fprintf(outf, "#include <unistd.h> \n");
    fprintf(outf, "#include <mpi.h> \n");
    fprintf(outf, "#include \"ADCL.h\" \n\n");

    attr_test_gen_func_prototypes ( outf, numfuncs );

    fprintf(outf, "int main ( int argc, char ** argv )\n");
    fprintf(outf, "{\n");
    fprintf(outf, "    ADCL_Topology topo;\n");
    fprintf(outf, "    ADCL_Request req; \n");
    fprintf(outf, "    ADCL_Attrset attrset;\n");
    fprintf(outf, "    ADCL_Fnctset fnctset;\n");
    fprintf(outf, "    ADCL_Attribute attrs[%d];\n", numattrs );
    fprintf(outf, "    ADCL_Function funcs[%d];\n", numfuncs );
    fprintf(outf, "    int rank, size, i;\n");
    fprintf(outf, "    int numfuncs=%d, numattrs=%d;\n", numfuncs, numattrs);
    fprintf(outf, "    int attrvals[%d];\n", maxnumattrvals);
    fprintf(outf, "    int funcvals[%d];\n", numattrs);
    fprintf(outf, "    \n\n");

    fprintf(outf, "    MPI_Init ( &argc, &argv );\n");
    fprintf(outf, "    MPI_Comm_rank ( MPI_COMM_WORLD,  &rank );\n");
    fprintf(outf, "    MPI_Comm_size ( MPI_COMM_WORLD,  &size );\n");
    fprintf(outf, "    ADCL_Init ( );\n");

    fprintf(outf, "    ADCL_Topology_create_generic ( 0, NULL, NULL, NULL, "
	    "ADCL_DIRECTION_BOTH,  MPI_COMM_WORLD, &topo ); \n\n");

    return 0;
}

int attr_test_gen_func_prototypes ( FILE *outf, int numfuncs )
{
    int i, len;

    for ( i=0;i<numfuncs; i++ ) {
        fprintf(outf, "void test_func_%d ( ADCL_Request req );\n", i );
        fprintf(outf, "#define TIME%d %d\n", i, (1000*i)%100000);
    } 
    fprintf(outf, "\n");

    return 0;
}
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int attr_test_gen_filetrail ( FILE *outf, int numattrs, int numfuncs )
{

    fprintf(outf, "    ADCL_Request_free ( &req );\n");
    fprintf(outf, "    ADCL_Fnctset_free ( &fnctset );\n");
    fprintf(outf, "    for (i=0; i<numfuncs; i++ ) { \n");
    fprintf(outf, "        ADCL_Function_free ( &funcs[i]);\n");
    fprintf(outf, "    }\n");
    fprintf(outf, "    ADCL_Attrset_free ( &attrset );\n");
    fprintf(outf, "    for (i=0; i<numattrs; i++ ) { \n");
    fprintf(outf, "        ADCL_Attribute_free ( &attrs[i]);\n");
    fprintf(outf, "    }\n");
    fprintf(outf, "    ADCL_Topology_free ( &topo );\n");
    fprintf(outf, "    ADCL_Finalize();\n");
    fprintf(outf, "    MPI_Finalize();\n");
    fprintf(outf, "    return 0;\n");
    fprintf(outf, "}\n");

    return 0;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
int attr_test_gen_init ( int argc, char ** argv, char **outfilename, 
			 int *numattrs, int **nattrvals )
{
    int *tmpattrvals;
    int i;

    if (argc < 3 )
    {
        printf(" Usage : attr_test_generator <ofilename> <numattrs> <no_attr_vals_1,"
               " ...no_attr_vals_n> \n\n");
        printf("   attr_test_generator generates a test file for ADCL  \n");
        printf("   given a certain number of attributes and the according \n");
        printf("   no of attribute values per attribute \n");
        printf(" Options: \n");
        printf("   <ofilename>     : name of the test-file \n");
        printf("   <numattrs>      : number of attributes \n");
        printf("   <no_attr_valsx> : no. of attribute values for attr x  \n");
        
        exit ( 1 ) ;
    }
    
    *outfilename = strdup (argv[1]);
    *numattrs = atoi(argv[2]);
    if ( argc < (2+ (*numattrs) ) ){
        printf(" Insufficient number of maximum attribute values for %d attributes argc=%d\n",
               *numattrs, argc ) ;
        exit ( 1 );
    }
    
    tmpattrvals = ( int* ) malloc ( (*numattrs) * sizeof (int ));
    for ( i=0; i< (*numattrs) ; i++ ) {
        tmpattrvals[i] = atoi ( argv[3+i]);
    }
    *nattrvals = tmpattrvals;
    
    return 0;
}
