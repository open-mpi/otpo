#ifndef __MINMAX_H__
#define __MINMAX_H__

#define MAXLINE 128

struct lininf {
    int    req;
    int    method;
    double min;
    double max;
    int    minloc;
    int    maxloc;
};

struct emethod{
    int     em_count;
    int     em_rescount;
    double  *em_time;
    int     *em_poison;
    int     em_num_outliers;
    double  em_avg;
    double  em_median;
    double  em_1stquartile;
    double  em_3rdquartile;
    double  em_iqr;
    double  em_llimit;
    double  em_ulimit;
    double  em_sum_filtered;
    int     em_cnt_filtered;
    double  em_avg_filtered;
    double  em_perc_filtered;
};

#define TLINE_INIT(_t) {_t.req=-1; _t.min=9999999999.99; _t.max=0.0; \
       _t.minloc=-1; _t.maxloc=-1;}
#define TLINE_MIN(_t, _time, _i){ \
           if ( _time < _t.min ) { \
	       _t.min    = _time;  \
	       _t.minloc = _i;}}
#define TLINE_MAX(_t, _time, _i) { \
	    if ( _time > _t.max ) { \
		_t.max = _time;     \
		_t.maxloc =_i;}}


int ml(const int ndata,  double* const data, 
       double* nu, double *mu, double *sigma, double *val);


#endif
