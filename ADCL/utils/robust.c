/* applying a t-model to performance data to obtain a robust
   estimate of the mean via ML of the loglikelihood function */

/* PREPROCESSOR FLAGS:
   NR - Algorithms from Numerical Recipes
     NELDER - Nelder-Mead algorithm, not working
     POWELL - Powell's method
     QUASI_NEWTON - quasi-Newton (Newton-Raphson) method
   GSL - algrithms from GSL library
     (select Conjugate gradient, BFGS or steepest decent in function 
     gsl_minimize_loglikelihood) 
   MAIN - compile with main() and test data
   SUMMARY - short output of results
   DEBUG - turns on various output
*/

// Nealder and Mead: does not work
//gcc -DNR -DMAIN -DNELDER robust.c dfpmin.c lnsrch.c nrutil.c amoeba.c amotry.c -I/usr/include/gsl -lm -L/usr/local/lib -lgsl -lgslcblas -g  -o test.x

// quasi-Newton:
//gcc -DNR -DMAIN -DQUASI_NEWTON robust.c dfpmin.c lnsrch.c nrutil.c -I/usr/include/gsl -lm -L/usr/local/lib -lgsl -lgslcblas -g  -o test.x

// Powell:
// gcc -DNR -DMAIN -DPOWELL robust.c  nrutil.c  powell.c linmin.c f1dim.c mnbrak.c brent.c -I/usr/include/gsl -lm -L/usr/local/lib -lgsl -lgslcblas -g  -o test.x

// GSL:
// gcc -DGSL -DMAIN robust.c dfpmin.c lnsrch.c nrutil.c -I/usr/include/gsl -lm -L/usr/local/lib -lgsl -lgslcblas -g  -o test.x
#if defined(GSL) || defined(NR)

#ifdef NR
#include <stdlib.h>
#include <stdio.h>
#define NRANSI
#include "nr.h"
#include "nrutil.h"
#endif

#ifdef GSL
#include <gsl_math.h>
#include <gsl_sort.h>
#include <gsl_sf_gamma.h>
#include <gsl_sf_psi.h>
#include <gsl_sf_log.h>
#include <gsl/gsl_sf_pow_int.h>
#include <gsl_statistics_double.h>
#include <gsl/gsl_sf_pow_int.h>
#include <gsl_multimin.h>
#endif 

#ifdef NR
void get_starting_value(float* x0);
void output_results(float* x0, int iter, float* val);
#endif 
#ifdef GSL
void get_starting_value(gsl_vector* x0, void* params);
void output_results(gsl_vector* x0, int iter, double* val, void* params);

void loglikelihood_fdf (const gsl_vector *x, void *params, double *f,
   gsl_vector *df);
int  gsl_minimize_loglikelihood(const double step_size, const double tol,
   const double eps_abs, const int maxiter, gsl_vector* x0, double* val, int* iter,
   void* params);
int  gsl_minimize_loglikelihood_old(const double step_size, const double tol,
   const double eps_abs, const int maxiter, gsl_vector* x0, double* val, int* iter,
   void* params);
#endif

#ifdef NR
void nelder_mead(float* x0, float (*func)(float []), float* ftol,
   float* val);
#endif


static int nfunc;  // number of function calls
static int ndfunc; // number of calls to compute derivative
double nu=3;
int with_nu;
double nustart=4.0;
int ndim;

#ifdef MAIN
/* speed of light data set */
double c_data[66] = { 
      28.0, 26.0, 33.0, 24.0, 34.0, -44.0, 27.0, 16.0, 40.0, -2.0,
      29.0, 22.0, 24.0, 21.0, 25.0, 30.0, 23.0, 29.0, 31.0, 19.0,
      24.0, 20.0, 36.0, 32.0, 36.0, 28.0, 25.0, 21.0, 28.0, 29.0,
      37.0, 25.0, 28.0, 26.0, 30.0, 32.0, 36.0, 26.0, 30.0, 22.0,
      36.0, 23.0, 27.0, 27.0, 28.0, 27.0, 31.0, 27.0, 26.0, 33.0,
      26.0, 32.0, 32.0, 24.0, 39.0, 28.0, 24.0, 25.0, 32.0, 25.0,
      29.0, 27.0, 28.0, 29.0, 16.0, 23.0};
#endif

int     ndata;    // #data points
double *data;

/*****************************************************************************/
#ifdef NR
float loglikelihood_f(float x[])
#endif
#ifdef GSL
double loglikelihood_f(const gsl_vector *x, void *params)
#endif
/*****************************************************************************/
{
   double mu, // estimate of location  
     sigma2;  // estimate of scale
   double delta, g, psi;

#ifdef GSL
   int ndata;
   double nu, *data;
   double * tmp = (double *) params;
#endif
   double y;  // temporary variable
   int i;

   nfunc++;

#ifdef NR
   mu      = x[1];
   sigma2  = x[2];
   if (with_nu) nu = x[3];
#else
   /* extract arguments and parameters */
   mu     = gsl_vector_get(x, 0);
   sigma2 = gsl_vector_get(x, 1);
   nu     = tmp[0];
   if (nu == 0){
      with_nu = 1;
      nu = gsl_vector_get(x,2);
   }
   ndata   = (int) tmp[1];
   data    = &(tmp[2]);
   //printf("%d %f %f\n", n, nu, data[0]);
#endif

   y = - 0.5 * gsl_sf_log(sigma2) * ndata;
   for (i=0; i<ndata; i++){
     delta = (data[i] - mu)*( data[i] - mu) / (nu * sigma2);
     y = y - 0.5 * (nu + 1.) * gsl_sf_log( 1+delta );
   }

   if (with_nu){ /* compute whole log likelyhood function */
      g =  gsl_sf_lngamma(0.5*(nu+1.)) - gsl_sf_lngamma(0.5*nu)
           - 0.5 * gsl_sf_log(nu);
      y = y + ndata * g;
   }

#ifdef DEBUG
   if (with_nu)
      printf("mu=%.12f, sigma=%.12f, nu=%.12f, y=%.12f\n", mu, sqrt(sigma2), nu, y);
   else
      printf("mu=%.12f, sigma=%.12f, y=%.12f\n", mu, sqrt(sigma2), y);
#endif

   return -y; /* since we're minimizing, not maximizing */

}


/*****************************************************************************/
#ifdef NR
void loglikelihood_df(float x[],float df[])
#endif
#ifdef GSL
void loglikelihood_df(const gsl_vector *x, void *params, gsl_vector *df)
#endif
/*****************************************************************************/
/* this function should store the n-dimensional gradient 
g_i = d f(x,params) / d x_i in the vector g for argument x and parameters 
params, returning an appropriate error code if the function cannot be computed. */
{
   double mu, sigma2;

   int i, k;
   double dldtheta, dldphi, dldnu, y, r, t, d, g, s1, s2;
#ifdef GSL
   int ndata, with_nu = 0;
   double nu, *data;
   double * tmp = (double *) params;
#endif


   ndfunc++;

#ifdef NR
   /* extract arguments and parameters */
   mu     = x[1];
   sigma2 = x[2];
   if (with_nu)    nu = x[3];
#endif
#ifdef GSL
   /* extract arguments and parameters */
   mu       = gsl_vector_get(x, 0);
   sigma2   = gsl_vector_get(x, 1);
   nu       = tmp[0];
   if (nu == 0){
      with_nu = 1;
      nu = gsl_vector_get(x,2);
   }
   ndata    = (int) tmp[1];
   data     = &(tmp[2]);
   //printf("%d %f %f\n", ndata, nu, data[0]);
   //printf("df mu=%lf sigma=%lf nu=%lf \n", mu, sqrt(sigma2), nu);
#endif

   dldtheta = 0.;
   dldphi   = 0.;
   if (with_nu) {
      dldnu = 0.;
      s1 = 0.;
      s2 = 0.;
   }

   /* calculate terms depending on data[i] */
   for (i=0; i<ndata; i++){
      y = data[i];
      d = y - mu;
      r = d*d / (sigma2*nu);

      dldtheta = dldtheta + d / ( 1 + r );
      dldphi   = dldphi + r / (1+r); // + 0.5/sigma2;
      if (with_nu){
         s1 = s1 +  r / (1+r);  //t * d;
         s2 = s2 +  gsl_sf_log(1+r) ;
      }
   }

   /* add constant terms */
   dldtheta = (nu+1)/(sigma2*nu) * dldtheta;
   dldphi   = - 0.5*ndata/sigma2 + 0.5*(nu+1)/sigma2 * dldphi;
   if (with_nu) {
      /* digamma(z) = psi(z) = d/dz( ln gamma(z) ) */
      g =  0.5 * ndata * ( gsl_sf_psi((nu+1)/2) - gsl_sf_psi(0.5*nu) - 1./nu );
      dldnu    = g + 0.5*(nu+1)/nu * s1 - 0.5 * s2;
   }

   /* change signs since we are minimizing, not maximizing */
#ifdef NR
   df[1] = - dldtheta;
   df[2] = - dldphi;
   if (with_nu)  df[3] = - dldnu;
#endif
#ifdef GSL
   gsl_vector_set(df, 0, - dldtheta);
   gsl_vector_set(df, 1, - dldphi);
   if (with_nu) {
      gsl_vector_set(df, 2, - dldnu);
   }
#endif

#ifdef DEBUG
#ifdef NR
   if (with_nu) printf("  df %lf %lf %lf\n", df[1], df[2], df[3]);
   else printf("  df %lf %lf \n", df[1], df[2]);
#endif
#ifdef GSL
   if (with_nu) printf("  df %lf %lf %lf\n", gsl_vector_get(df, 0), gsl_vector_get(df, 1),
       gsl_vector_get(df, 2));
   else printf("  df %lf %lf \n", gsl_vector_get(df, 0), gsl_vector_get(df, 1));
#endif
#endif
}


/*****************************************************************************/
#ifdef NR
int ml(const int ndata_loc,  double* const data_loc, double* nu, double* mu,
       double* sigma, float* val)
#endif
#ifdef GSL
int ml(const int ndata,      double* const data,     double* nu, double* mu,
       double* sigma, double* val)
#endif
/*****************************************************************************/
/* performs a ML method for a t model
   ndata  (in)      size of data
   data   (in)      data / measurements
   nu     (in/out)  if nu is 0, optimize also for nu and return 
                    optimized value
                    if nu is != 0, accept nu as parameter
   mu     (out)     optimized value for mu
   sigma  (out)     optimized value for sigma
   val    (out)     if nu is variable, value of log likelihood function 
                    if nu is fixed, value of simplified log likelihood
                    function*/
{
#ifdef NR
   float *x0;            // starting point / maximum
#endif
#ifdef GSL
   gsl_vector *x0;       // starting point / maximum
   double *params;

   double step_size, tol, eps_abs;
#endif
   int maxiter = 1000;
   int iter = 0, i, j;

   /* init starting point */
   if (*nu == 0)
      ndim = 3;
   else
      ndim = 2;

#ifdef NR
   ndata = ndata_loc;
   data  = data_loc;

   x0  = vector(1,ndim);
#endif

#ifdef GSL
   /* init parameters */
   params = malloc((ndata+2)*sizeof(double));
   params[0] = *nu;
   params[1] = (double) ndata;
   for (i=0; i<ndata; i++){
      params[2+i] = data[i];
   }

   x0  = gsl_vector_alloc(ndim);
#endif

#ifdef NR
   get_starting_value(x0);
#endif
#ifdef GSL
   get_starting_value(x0, params);
#endif

   nfunc = ndfunc = 0;


#ifdef NR

#ifdef NELDER
   float ftol = 1.0e-6;
   nelder_mead(x0, loglikelihood_f, &(ftol), val);
#endif

#ifdef POWELL
   float ftol = 1.0e-6;
   float **p;

   p=matrix(1,ndim,1,ndim);
   for (i=1;i<=ndim;i++)
      for (j=1;j<=ndim;j++)
         p[i][j]=(i == j ? 1. : 0.0);
   if (with_nu) 
      p[3][3] = 0.01;

   iter = maxiter;
   powell(x0,p,ndim,ftol,&iter,val,loglikelihood_f);

   free_matrix(p,1,ndim,1,ndim);
#endif

#ifdef QUASI_NEWTON
   float gtol = 1.0e-8;
   dfpmin(x0,ndim,gtol,&iter,val,loglikelihood_f,loglikelihood_df);
#endif

// endif NR
#endif

#ifdef GSL
   step_size = 0.1;   // size of first trial step
   tol       = 0.1;   // accuracy of line minimization, 0.1 will do
   eps_abs   = 1e-8;

   gsl_minimize_loglikelihood(step_size, tol, eps_abs, maxiter, x0, val, 
      &(iter), params);
#endif

#ifdef SUMMARY
#ifdef NR
   output_results(x0, iter, val);
#endif
#ifdef GSL
   output_results(x0, iter, val, params);
#endif
#endif

#ifdef NR
   *mu     = (double) x0[1];
   *sigma = sqrt((double) x0[2]);
   if (with_nu) 
      *nu = (double) x0[3];

   free_vector(x0,1,ndim);
#endif
#ifdef GSL
   *mu    = gsl_vector_get(x0, 0);
   *sigma = sqrt(gsl_vector_get(x0, 1));
   if (with_nu) 
      *nu = gsl_vector_get(x0, 2);

   gsl_vector_free(x0);
   free (params);
#endif

   return 0;

}

#ifdef MAIN
/*****************************************************************************/
int main()
/*****************************************************************************/
{
   int i, varnu=0;
#ifdef NR
   float val;
#endif
#ifdef GSL
   double val; 
#endif
   double mu, sigma;

   const int ndata_loc = 66;
   double *data_loc = &(c_data[0]);

   //nu = 0.0; with_nu = 1;
   nu =  4.0; with_nu = 0;

   ml(ndata_loc, data_loc, &(nu), &(mu), &(sigma), &(val));

#ifdef SUMMARY
   printf("\n\nReturn values:\n");
   printf("nu=%.5lf mu=%.5lf sigma=%.5lf val=%.5lf\n", nu, mu, sigma, val);
#endif

   return 0;

}
#endif


/*****************************************************************************/
#ifdef NR
void get_starting_value(float* x0)
#endif 
#ifdef GSL
void get_starting_value(gsl_vector* x0, void* params)
#endif
/*****************************************************************************/
/* computes an estimate of location and scale                                */
/*****************************************************************************/
{
   double mean, variance;
   double median, *madv, mad;
   int i;

#ifdef GSL
   double* tmp = (double*) params;
   int ndata   = (int) tmp[1];
   double* data    = &(tmp[2]);
#endif

   /* Possiblity I: Use mean and variance as starting value
                    this is probably going to crash when outliers are present */   
   mean = gsl_stats_mean(&(data[0]), (size_t) 1,(size_t) ndata);
   variance = gsl_stats_variance_with_fixed_mean(&(data[0]),
      (size_t) 1, (size_t) ndata, mean);
#ifdef NR
   x0[1] = mean;
   x0[2] = variance;
#endif 
#ifdef GSL
   gsl_vector_set(x0, 0, mean);
   gsl_vector_set(x0, 1, variance);
#endif

   /* Possiblity II: Use median and 1.483*MAD as starting value
                     Absolute Deviatons MAD(x) = med_i {|x_i - med_j (x_j)|} */
   gsl_sort(data, (size_t) 1, (size_t) ndata);
   median = gsl_stats_median_from_sorted_data(data, (size_t) 1, (size_t) ndata);
   madv = (double*) malloc(ndata * sizeof(double));
   for (i=0; i<ndata; i++){
       madv[i] = abs(data[i] - median);
   }
   gsl_sort(madv, (size_t) 1, (size_t) ndata);
   mad = 1.483 * gsl_stats_median_from_sorted_data(madv, (size_t) 1, (size_t) ndata);
   //printf("median: %20.5f, mad     : %20.5f\n", median, mad);
   //mad = 2 * median; //5.25 * gsl_stats_median_from_sorted_data(madv, (size_t) 1, (size_t) ndata);
   free(madv);

#ifdef NR
   x0[1] = median;
   x0[2] = 0.7*mad*mad;
   if (with_nu)
      x0[3] = nustart;
#ifdef SUMMARY
   printf("\nStarting vector        : ");
   for (i=1;i<=ndim;i++) printf("%12.6f",x0[i]);
#endif
#endif 

#ifdef GSL
   gsl_vector_set(x0, 0, median);
   gsl_vector_set(x0, 1, 0.7*mad*mad);
   if (with_nu)
      gsl_vector_set(x0, 2, nustart);
#ifdef SUMMARY
   printf("\nStarting vector        : ");
   for (i=0;i<ndim;i++) printf("%12.6f", gsl_vector_get(x0, i));
#endif
#endif 

}

/*****************************************************************************/
#ifdef NR
void output_results(float* x0, int iter, float* val)
#endif
#ifdef GSL
void output_results(gsl_vector* x0, int iter, double* val, void* params)
#endif
/*****************************************************************************/
/* output of minimum  and statistics                                         */
/*****************************************************************************/
{
   int i;
#ifdef NR
   float      *xdf;        // gradient at maximum
   xdf = vector(1,ndim);
#endif

#ifdef GSL
   int with_nu = 0;
   gsl_vector *xdf;      // gradient at maximum
   double* tmp = (double*) params;
   double nu;

   xdf = gsl_vector_alloc(ndim);

   nu = tmp[0];
   if (nu == 0.0) with_nu = 1;
#endif

   printf("\n\nIterations             : %3d\n",iter);
   printf("Func. evals            : %3d\n",nfunc);
#ifdef QUASI_NEWTON
   printf("Deriv. evals           : %3d\n",ndfunc);
#endif

   printf("\nMaximum found at       : ");
#ifdef NR
   x0[2] = sqrt(x0[2]);
   for (i=1;i<=ndim;i++) printf("%12.6f",x0[i]);
   x0[2] = x0[2] * x0[2];
#endif
#ifdef GSL
   gsl_vector_set(x0, 1, sqrt(gsl_vector_get(x0, 1)));
   for (i=0;i<ndim;i++) printf("%12.6f", gsl_vector_get(x0, i));
   gsl_vector_set(x0, 1, gsl_vector_get(x0, 1) * gsl_vector_get(x0, 1));
#endif

   printf("\nMaximum function value : %12.6f\n",*val);

   printf("Derivative at maximum  : ");
#ifdef NR
   loglikelihood_df(x0, xdf);
   for (i=1;i<=ndim;i++) printf("%12.6f",xdf[i]);
#endif
#ifdef GSL
   loglikelihood_df(x0, params, xdf);
   for (i=0;i<ndim;i++) printf("%12.6f",gsl_vector_get(xdf, i));
#endif


#ifdef MAIN
   if (with_nu){ // for variable nu
       printf("\n\nTrue maximum is at (mu_hat, sigma_hat, vu) = (27.40, 3.81, 2.13)\n");
#ifdef NR
       x0[1] =  27.40;
       x0[2] =  3.81*3.81;
       x0[3] =  2.13;
       printf("Test: function value at maximum = %12.6f\n",  loglikelihood_f(x0));
       printf("Derivative at maximum           : ");
       loglikelihood_df(x0, xdf);
       for (i=1;i<=ndim;i++) printf("%12.6f",xdf[i]);
#endif
#ifdef GSL
       gsl_vector_set(x0, 0, 27.40);
       gsl_vector_set(x0, 1, 3.81*3.81);
       gsl_vector_set(x0, 2, 2.13);

       printf("Test: function value at maximum = %12.6f\n",  loglikelihood_f(x0, params));
       loglikelihood_df(x0, params, xdf);
       printf("Derivative at maximum           : ");
       for (i=0;i<ndim;i++) printf("%12.6f",gsl_vector_get(xdf, i));
#endif
   } else {         // for fixed nu=4:
       printf("\n\nTrue maximum is at (mu_hat, sigma_hat) = (27.49, 4.51)\n");
#ifdef NR
       x0[1] =  27.49;
       x0[2] =  4.51 * 4.51;
       printf("Test: function value at maximum : %12.6f\n",  loglikelihood_f(x0));
       printf("Derivative at true maximum      : ");
       loglikelihood_df(x0, xdf);
       for (i=1;i<=ndim;i++) printf("%12.6f",xdf[i]);
#endif
#ifdef GSL
       gsl_vector_set(x0, 0, 27.40);
       gsl_vector_set(x0, 1, 4.51 * 4.51);
       printf("Test: function value at maximum : %12.6f\n",  loglikelihood_f(x0, params));
       loglikelihood_df(x0, params, xdf);
       printf("Derivative at true maximum      : ");
       for (i=0;i<ndim;i++) printf("%12.6f",gsl_vector_get(xdf, i));
#endif
   }
#endif

#ifdef NR
   free_vector(xdf,1,ndim);
#endif
#ifdef GSL
   gsl_vector_free(xdf);
#endif

}


#ifdef NR
#ifdef NELDER
/*****************************************************************************/
void nelder_mead(float* x0, float(*func)(float []), float* ftol,
   float* val)
/*****************************************************************************/
/* computes minimum with method of Nelder and Mead from numerical recipes    */
/*
/* parameters:
/* x0    on input: starting value; on output: minimum
/* func  function
/* ftol  tolerance
/* val   function value at minimum */
/*****************************************************************************/
{
   int i, j;
   float *x, *y, **p;
   double delta_mu, delta_sigma2;

   if (ndim != 2) {
      printf("ndim != 2 not implemented\n");
      exit(0);
   }

   x=vector(1,ndim);
   y=vector(1,ndim+1);
   p=matrix(1,ndim+1,1,ndim);

   delta_mu = 0.5e0;
   delta_sigma2 = 1e0;

   for (i=1;i<=ndim+1;i++) {
      x[1] = p[i][1]=(i == (j+1) ? x0[1] + delta_mu     : x0[1] - delta_mu    );
      x[2] = p[i][2]=(i == (j+1) ? x0[2] + delta_sigma2 : x0[2] - delta_sigma2);
      y[i]=loglikelihood_f(x);
   }

   amoeba(p,y,ndim,*ftol,func,&nfunc);
   y[2] = sqrt(y[2]);

   printf("Vertices of final 3-d simplex and\n");
   printf("function values at the vertices:\n\n");
   printf("%3s %10s %12s %14s\n\n",
      "i","x[i]","y[i]", "function");
   for (i=1;i<=ndim+1;i++) {
      printf("%3d ",i);
      for (j=1;j<=ndim;j++) printf("%12.6f ",p[i][j]);
         printf("%12.6f\n",y[i]);
   }

   for (j=1;j<=ndim;j++) {
      // sum up vertices of final simplex 
      x0[j] = p[1][j];
      for (i=2;i<=ndim+1;i++) 
         x0[j] = x0[j] + p[i][j];
      // ... and average ...
      x0[j] = x0[j]/(ndim+1);
   }

   // calculate average function value at maximum
   *val = y[1];
   for (i=2;i<=ndim+1;i++) 
      *val = *val + y[i];
   *val = *val / (ndim+1);

   free_vector(x,1,ndim);
   free_matrix(p,1,ndim+1,1,ndim);
   free_vector(y,1,ndim+1);
}
//endif NELDER
#endif

//endif NR
#endif


#ifdef GSL
/* Compute both f and df together. */
void loglikelihood_fdf (const gsl_vector *x, void *params, double *f,
   gsl_vector *df) 
{
     *f = loglikelihood_f(x, params); 
     loglikelihood_df(x, params, df);

     //printf("fdf =%lf %lf %lf %lf \n", gsl_vector_get(x,0), gsl_vector_get(x,1),
     //   gsl_vector_get(df,0), gsl_vector_get(df,1));
}

int  gsl_minimize_loglikelihood(const double step_size, const double tol,
   const double eps_abs, const int maxiter, gsl_vector* x0, double* val, int* iter,
   void* params){
   /* 
   s       (in)      GSL multimin fminimizer object
   eps_abs (in)
   maxiter (in)      maximum number of iterations
   iter    (in/out)  number of iterations             */

   const gsl_multimin_fdfminimizer_type *T;
   gsl_multimin_fdfminimizer *s;
   gsl_multimin_function_fdf my_func;
   double* tmp = (double*) params;
   double nu = tmp[0];
   double sigma2;
   int my_iter = 0;
   int status;

   /* init function object */
   if (nu != 0.0) my_func.n = 2;
   else           my_func.n = 3;
   my_func.f   = &loglikelihood_f;
   my_func.df  = &loglikelihood_df;
   my_func.fdf = &loglikelihood_fdf;
   my_func.params = params;

   /* set up solver */
   T = gsl_multimin_fdfminimizer_conjugate_fr;
   // T = gsl_multimin_fdfminimizer_vector_bfgs;
   // T = gsl_multimin_fdfminimizer_steepest_descent;
   if (nu != 0.0)  s = gsl_multimin_fdfminimizer_alloc(T,2);
   else            s = gsl_multimin_fdfminimizer_alloc(T,3);

   /* call minimizer */
   //s->f = my_func->f;
   //gsl_multimin_fminimizer_set(s, &my_func, x0, step_size)
   gsl_multimin_fdfminimizer_set(s, &my_func, x0, step_size, tol);

   /* let gsl minimizer iterate */
   do
   {
      status = gsl_multimin_fdfminimizer_iterate(s);
      if (status) break;

      status = gsl_multimin_test_gradient(s->gradient, eps_abs);
      /* if (status == GSL_SUCCESS){
         printf("Maximum found after %d iterations at:\n", *iter);
      }
      printf ("%5d %.5f %.5f %10.5f\n", *iter,
        gsl_vector_get (s->x, 0), sqrt(gsl_vector_get (s->x, 1)), s->f); */
      (*iter)++;
   } while (status == GSL_CONTINUE && *iter < maxiter);

   /* just precaution */
   sigma2 =  gsl_vector_get(x0, 1);
   if (isnan(sigma2)){
     printf("!!! sigma2 is nan\n");
     printf("%5d %15.5f %15.5f", iter, 
     gsl_vector_get(s->x, 0),
     sqrt(gsl_vector_get(s->x, 1)));
     if (nu == 0.0) printf("%15.5f", gsl_vector_get(s->x, 2));
     printf("%15.5f\n", -gsl_multimin_fdfminimizer_minimum(s));

     exit;
   }

   /* set return values */
   gsl_vector_set(x0, 0, gsl_vector_get(s->x, 0));
   gsl_vector_set(x0, 1, gsl_vector_get(s->x, 1));
   if (nu == 0.0)  /* return optimized value */ 
       gsl_vector_set(x0, 2, gsl_vector_get(s->x, 2));
   *val   = -gsl_multimin_fdfminimizer_minimum(s);

   /* clean up */
   //gsl_multimin_fminimizer_free(s);
   gsl_multimin_fdfminimizer_free(s);

   return 0;
}

// obsolete
int  gsl_minimize_loglikelihood_old(const double step_size, const double tol,
   const double eps_abs, const int maxiter, gsl_vector* x0, double* val, int* iter,
   void* params){
   /* 
   s       (in)      GSL multimin fminimizer object
   eps_abs (in)
   maxiter (in)      maximum number of iterations
   iter    (in/out)  number of iterations             */

   const gsl_multimin_fdfminimizer_type *T;
   gsl_multimin_fdfminimizer *s;
   gsl_multimin_function_fdf my_func;
   double* tmp = (double*) params;
   double nu = tmp[0];
   double sigma2;
   int my_iter = 0;
   int status;

   int converged = 0; 
   double start=0.4;
   double median = gsl_vector_get(x0, 0);
   double mad    = gsl_vector_get(x0, 1);
   mad = sqrt(mad/0.7);

   /* init function object */
   if (nu != 0.0) my_func.n = 2;
   else           my_func.n = 3;
   my_func.f   = &loglikelihood_f;
   my_func.df  = &loglikelihood_df;
   my_func.fdf = &loglikelihood_fdf;
   my_func.params = params;

   /* set up solver */
   T = gsl_multimin_fdfminimizer_conjugate_fr;
   // T = gsl_multimin_fdfminimizer_vector_bfgs;
   // T = gsl_multimin_fdfminimizer_steepest_descent;
   if (nu != 0.0)  s = gsl_multimin_fdfminimizer_alloc(T,2);
   else            s = gsl_multimin_fdfminimizer_alloc(T,3);

   /* call minimizer */
   //s->f = my_func->f;
   //gsl_multimin_fminimizer_set(s, &my_func, x0, step_size)

   gsl_multimin_fdfminimizer_set(s, &my_func, x0, step_size, tol);

   do {//while (!converged && my_iter < 10){
      converged = 1;

      /* let gsl minimizer iterate */
      do
      {
         status = gsl_multimin_fdfminimizer_iterate(s);
         if (status) break;

         status = gsl_multimin_test_gradient(s->gradient, eps_abs);
         /* if (status == GSL_SUCCESS){
            printf("Maximum found after %d iterations at:\n", *iter);
         }
         printf ("%5d %.5f %.5f %10.5f\n", *iter,
            gsl_vector_get (s->x, 0), sqrt(gsl_vector_get (s->x, 1)), s->f); */
         (*iter)++;
      } while (status == GSL_CONTINUE && *iter < maxiter);

      sigma2 =  gsl_vector_get(x0, 1);
      if (!isnan(sigma2)) break;

      gsl_vector_set(x0, 0, median);
      gsl_vector_set(x0, 1, start*mad*mad);
      gsl_multimin_fdfminimizer_set (s, &my_func, x0, step_size, tol);
      start = start + 0.5;
      my_iter = my_iter + 1; 
      if (my_iter > 1) printf("-- it=%d, sigma=%lf\n", my_iter, sqrt(sigma2));
   } while (my_iter < 10);
  if (*iter > maxiter){
     printf("!!! sigma2 is nan\n");
     printf("%5d %15.5f %15.5f", iter, 
     gsl_vector_get(s->x, 0),
     sqrt(gsl_vector_get(s->x, 1)));
     if (nu == 0.0) printf("%15.5f", gsl_vector_get(s->x, 2));
     printf("%15.5f\n", -gsl_multimin_fdfminimizer_minimum(s));
   }

   gsl_vector_set(x0, 0, gsl_vector_get(s->x, 0));
   gsl_vector_set(x0, 1, gsl_vector_get(s->x, 1));
   if (nu == 0.0)  /* return optimized value */ 
       gsl_vector_set(x0, 2, gsl_vector_get(s->x, 2));
   *val   = -gsl_multimin_fdfminimizer_minimum(s);

   /* clean up */
   //gsl_multimin_fminimizer_free(s);
   gsl_multimin_fdfminimizer_free(s);

   return 0;
}
//endif GSL
#endif


#endif 
// GSL || NR
