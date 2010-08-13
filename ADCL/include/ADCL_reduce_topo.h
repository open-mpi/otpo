#ifndef __ADCL_REDUCE_TOPO_H__
#define __ADCL_REDUCE_TOPO_H__

#define MAXTREEFANOUT 32


    typedef struct ADCL_tree_t {
        int32_t tree_root;
        int32_t tree_fanout;
        int32_t tree_bmtree;
        int32_t tree_prev;
        int32_t tree_next[MAXTREEFANOUT];
        int32_t tree_nextsize;
    } ADCL_tree_t;


    ADCL_tree_t* ADCL_build_tree( int fanout, ADCL_request_t *req, int root );
    ADCL_tree_t* ADCL_build_in_order_bintree( ADCL_request_t *req );
    ADCL_tree_t* ADCL_build_in_order_bmtree( ADCL_request_t *req, int root );
    ADCL_tree_t* ADCL_build_chain( ADCL_request_t *req, int root,int fanout );
#endif
