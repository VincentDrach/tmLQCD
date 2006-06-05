/* $Id$ */

#ifdef HAVE_CONFIG_H
# include<config.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "global.h"
#include "linsolve.h"
#include "linalg_eo.h"
#include "start.h"
#include "tm_operators.h"
#include "Ptilde_nd.h"
#include "Nondegenerate_Matrix.h"

#define PI 3.141592653589793

/*
extern double cheb_evmin, cheb_evmax;
*/

double func_tilde(double u, double exponent){

  double ff=0.0;
  double d=0,ddd=0, sv, z, z2;
  int j;
  double res=0.0;

  z = (2.0*u - cheb_evmin - cheb_evmax)/(double)(cheb_evmax - cheb_evmin);
  z2 = 2.0*z;

  for(j=dop_n_cheby-1; j>=1; j--){
    sv = d;
    d = z2*d - ddd + dop_cheby_coef[j];
    ddd = sv;
    }

  res = z*d - ddd + 0.5*dop_cheby_coef[0];

  ff = (double)(res * sqrt(u));

  return(pow(ff,exponent));
}

void Ptilde_cheb_coefs(double aa, double bb, double dd[], int n, double exponent){
  int k,j;
  double fac,bpa,bma,*f;
  double inv_n;

  printf("\n hello in  PTILDE-chebyshev_polynomial\n");
  inv_n=1./(double)n;
  printf("n= %d inv_n=%e \n",n,inv_n);
  f=calloc(n,sizeof(double));/*vector(0,n-1);*/
  printf("allocation !!!\n");
  fflush(stdout);
  bma=0.5*(bb-aa);
  bpa=0.5*(bb+aa);
  for (k=0;k<n;k++) {
    double y=cos(PI*(k+0.5)*inv_n);
    f[k]=func_tilde(y*bma+bpa,exponent);
  }
  fac=2.0*inv_n;
  for (j=0;j<n;j++) {
    double sum=0.0;
      for (k=0;k<n;k++)
	sum += f[k]*cos(PI*j*(k+0.5)*inv_n);      
    dd[j]=fac*sum;
  }
  free(f);
}
#undef PI


/****************************************************************************  
 *
 * computation of the second poplynomial, Ptilde, on a vector
 *   by using the chebyshev approximation for the function ()^1/4
 * subtraction of low-lying eigenvalues is not yet implemented for this
 *
 **************************************************************************/

void Poly_tilde_ND(spinor *R_s, spinor *R_c, double *dd, int n, 
                   spinor *S_s, spinor *S_c){

  int j;
  double fact1, fact2, temp, temp1, temp2, temp3, temp4;

  spinor *svs_=NULL, *svs=NULL, *ds_=NULL, *ds=NULL, *dds_=NULL, *dds=NULL, 
         *auxs_=NULL, *auxs=NULL, *aux2s_=NULL, *aux2s=NULL, *aux3s_=NULL, 
         *aux3s=NULL;
  spinor *svc_=NULL, *svc=NULL, *dc_=NULL, *dc=NULL, *ddc_=NULL, 
         *ddc=NULL, *auxc_=NULL, *auxc=NULL, *aux2c_=NULL, *aux2c=NULL, 
         *aux3c_=NULL, *aux3c=NULL;


#if ( defined SSE || defined SSE2 )
   svs_  = calloc(VOLUMEPLUSRAND+1, sizeof(spinor));
   svs   = (spinor *)(((unsigned int)(svs_)+ALIGN_BASE)&~ALIGN_BASE);
   ds_   = calloc(VOLUMEPLUSRAND+1, sizeof(spinor));
   ds    = (spinor *)(((unsigned int)(ds_)+ALIGN_BASE)&~ALIGN_BASE);
   dds_  = calloc(VOLUMEPLUSRAND+1, sizeof(spinor));
   dds   = (spinor *)(((unsigned int)(dds_)+ALIGN_BASE)&~ALIGN_BASE);
   auxs_ = calloc(VOLUMEPLUSRAND+1, sizeof(spinor));
   auxs  = (spinor *)(((unsigned int)(auxs_)+ALIGN_BASE)&~ALIGN_BASE);
   aux2s_= calloc(VOLUMEPLUSRAND+1, sizeof(spinor));
   aux2s = (spinor *)(((unsigned int)(aux2s_)+ALIGN_BASE)&~ALIGN_BASE);
   aux3s_= calloc(VOLUMEPLUSRAND+1, sizeof(spinor));
   aux3s = (spinor *)(((unsigned int)(aux3s_)+ALIGN_BASE)&~ALIGN_BASE);
   svc_  = calloc(VOLUMEPLUSRAND+1, sizeof(spinor));
   svc   = (spinor *)(((unsigned int)(svc_)+ALIGN_BASE)&~ALIGN_BASE);
   dc_   = calloc(VOLUMEPLUSRAND+1, sizeof(spinor));
   dc    = (spinor *)(((unsigned int)(dc_)+ALIGN_BASE)&~ALIGN_BASE);
   ddc_  = calloc(VOLUMEPLUSRAND+1, sizeof(spinor));
   ddc   = (spinor *)(((unsigned int)(ddc_)+ALIGN_BASE)&~ALIGN_BASE);
   auxc_ = calloc(VOLUMEPLUSRAND+1, sizeof(spinor));
   auxc  = (spinor *)(((unsigned int)(auxc_)+ALIGN_BASE)&~ALIGN_BASE);
   aux2c_= calloc(VOLUMEPLUSRAND+1, sizeof(spinor));
   aux2c = (spinor *)(((unsigned int)(aux2c_)+ALIGN_BASE)&~ALIGN_BASE);
   aux3c_= calloc(VOLUMEPLUSRAND+1, sizeof(spinor));
   aux3c = (spinor *)(((unsigned int)(aux3c_)+ALIGN_BASE)&~ALIGN_BASE);
#else
   svs_=calloc(VOLUMEPLUSRAND, sizeof(spinor));
   svs = svs_;
   ds_=calloc(VOLUMEPLUSRAND, sizeof(spinor));
   ds = ds_;
   dds_=calloc(VOLUMEPLUSRAND, sizeof(spinor));
   dds = dds_;
   auxs_=calloc(VOLUMEPLUSRAND, sizeof(spinor));
   auxs = auxs_;
   aux2s_=calloc(VOLUMEPLUSRAND, sizeof(spinor));
   aux2s = aux2s_;
   aux3s_=calloc(VOLUMEPLUSRAND, sizeof(spinor));
   aux3s = aux3s_;
   svc_=calloc(VOLUMEPLUSRAND, sizeof(spinor));
   svc = svc_;
   dc_=calloc(VOLUMEPLUSRAND, sizeof(spinor));
   dc = dc_;
   ddc_=calloc(VOLUMEPLUSRAND, sizeof(spinor));
   ddc = ddc_;
   auxc_=calloc(VOLUMEPLUSRAND, sizeof(spinor));
   auxc = auxc_;
   aux2c_=calloc(VOLUMEPLUSRAND, sizeof(spinor));
   aux2c = aux2c_;
   aux3c_=calloc(VOLUMEPLUSRAND, sizeof(spinor));
   aux3c = aux3c_;
#endif
 

   fact1=4/(cheb_evmax-cheb_evmin);
   fact2=-2*(cheb_evmax+cheb_evmin)/(cheb_evmax-cheb_evmin);

   zero_spinor_field(&ds[0],VOLUME/2);
   zero_spinor_field(&dds[0],VOLUME/2); 
   zero_spinor_field(&dc[0],VOLUME/2);
   zero_spinor_field(&ddc[0],VOLUME/2); 
   
   /*   sub_low_ev(&aux3[0], &S[0]);  */
   assign(&aux3s[0], &S_s[0],VOLUME/2);  
   assign(&aux3c[0], &S_c[0],VOLUME/2);  
   
   /*  Use the Clenshaw's recursion for the Chebysheff polynomial */
   for (j=n-1; j>=1; j--) {
     assign(&svs[0],&ds[0],VOLUME/2);
     assign(&svc[0],&dc[0],VOLUME/2); 
       
     /*     
     if ( (j%10) == 0 ) {
  	 sub_low_ev(&aux[0], &d[0]);
     }
     else { */
     assign(&auxs[0], &ds[0], VOLUME/2);
     assign(&auxc[0], &dc[0], VOLUME/2);
     /*   } */  


     Q_Qdagger_ND(&R_s[0], &R_c[0], &auxs[0], &auxc[0]);

     temp1=-1.0;
     temp2=dd[j];
     assign_mul_add_mul_add_mul_add_mul_r(&ds[0] , &R_s[0], &dds[0], &aux3s[0], fact2, fact1, temp1, temp2,VOLUME/2);
     assign_mul_add_mul_add_mul_add_mul_r(&dc[0] , &R_c[0], &ddc[0], &aux3c[0], fact2, fact1, temp1, temp2,VOLUME/2);
     assign(&dds[0], &svs[0],VOLUME/2);
     assign(&ddc[0], &svc[0],VOLUME/2);
   }
     
   /*     sub_low_ev(&R[0],&d[0]);  */ 
   assign(&R_s[0], &ds[0],VOLUME/2);  
   assign(&R_c[0], &dc[0],VOLUME/2);  


   Q_Qdagger_ND(&auxs[0], &auxc[0], &R_s[0], &R_c[0]);

   temp1=-1.0;
   temp2=dd[0]/2;
   temp3=fact1/2;
   temp4=fact2/2;
   assign_mul_add_mul_add_mul_add_mul_r(&auxs[0], &ds[0], &dds[0], &aux3s[0], temp3, temp4, temp1, temp2,VOLUME/2);
   assign_mul_add_mul_add_mul_add_mul_r(&auxc[0], &dc[0], &ddc[0], &aux3c[0], temp3, temp4, temp1, temp2,VOLUME/2);
   assign(&R_s[0], &auxs[0],VOLUME/2);
   assign(&R_c[0], &auxc[0],VOLUME/2);
     
   /*     addproj_q_invsqrt(&R[0], &S[0]); */
    
   /*
#ifndef _SOLVER_OUTPUT
     if(g_proc_id == g_stdio_proc){
       printf("Order of Chebysheff approximation = %d\n",j); 
       fflush( stdout);};
#endif
   */
     
    
   free(svs_);  
   free(ds_);   
   free(dds_);  
   free(auxs_); 
   free(aux2s_);
   free(aux3s_);
   free(svc_);  
   free(dc_);   
   free(ddc_);  
   free(auxc_); 
   free(aux2c_);
   free(aux3c_);
   
}
  


/**************************************************************************
 *
 * The externally accessible function is
 *
 *   void degree_of_Ptilde(void)
 *     Computation of (QdaggerQ)^1/4
 *     by using the chebyshev approximation for the function ()^1/4  
 *
 * Author: Thomas Chiarappa <Thomas.Chiarappa@mib.infn.it> Mai 2006 
 *
*****************************************************************************/


double stopeps_ptilde=1.0e-10;


void degree_of_Ptilde(){
  int i;
  double temp, temp2;
  static int ini=0;

  spinor *ss=NULL, *ss_=NULL, *sc=NULL, *sc_=NULL;
  spinor *auxs=NULL, *auxs_=NULL, *auxc=NULL, *auxc_=NULL;
  spinor *aux2s=NULL, *aux2s_=NULL, *aux2c=NULL, *aux2c_=NULL;

  if(ini==0){
    ptilde_cheby_coef = calloc(N_CHEBYMAX,sizeof(double));
    ini=1;
  }   

#if ( defined SSE || defined SSE2 || defined SSE3)
   ss_   = calloc(VOLUMEPLUSRAND/2+1, sizeof(spinor));
   auxs_ = calloc(VOLUMEPLUSRAND/2+1, sizeof(spinor));
   aux2s_= calloc(VOLUMEPLUSRAND/2+1, sizeof(spinor));
   sc_   = calloc(VOLUMEPLUSRAND/2+1, sizeof(spinor));
   auxc_ = calloc(VOLUMEPLUSRAND/2+1, sizeof(spinor));
   aux2c_= calloc(VOLUMEPLUSRAND/2+1, sizeof(spinor));

   ss    = (spinor *)(((unsigned int)(ss_)+ALIGN_BASE)&~ALIGN_BASE);
   auxs  = (spinor *)(((unsigned int)(auxs_)+ALIGN_BASE)&~ALIGN_BASE);
   aux2s = (spinor *)(((unsigned int)(aux2s_)+ALIGN_BASE)&~ALIGN_BASE);
   sc    = (spinor *)(((unsigned int)(sc_)+ALIGN_BASE)&~ALIGN_BASE);
   auxc  = (spinor *)(((unsigned int)(auxc_)+ALIGN_BASE)&~ALIGN_BASE);
   aux2c = (spinor *)(((unsigned int)(aux2c_)+ALIGN_BASE)&~ALIGN_BASE);

#else
   ss   =calloc(VOLUMEPLUSRAND/2, sizeof(spinor));
   auxs =calloc(VOLUMEPLUSRAND/2, sizeof(spinor));
   aux2s=calloc(VOLUMEPLUSRAND/2, sizeof(spinor));
   sc   =calloc(VOLUMEPLUSRAND/2, sizeof(spinor));
   auxc =calloc(VOLUMEPLUSRAND/2, sizeof(spinor));
   aux2c=calloc(VOLUMEPLUSRAND/2, sizeof(spinor));
#endif


   /* evmin and evmax have been defined in chebyshev_pol_ND  */

   Ptilde_cheb_coefs(cheb_evmin, cheb_evmax, ptilde_cheby_coef, N_CHEBYMAX, -1.0);
   
   temp=1.0;
   temp2=1.0;
   random_spinor_field(ss,VOLUME/2);
   random_spinor_field(sc,VOLUME/2);


   if(g_proc_id == g_stdio_proc){
     printf("\n determine the degree of the polynomial:\n");
     fflush(stdout);
   }

   /* OLD WAY NOT USED HERE
  ptilde_n_cheby=(int)5./sqrt(cheb_evmin);
   */

   ptilde_n_cheby=dop_n_cheby;
  for(i = 0;i < 100 ; i++){
    
    /* if (ptilde_n_cheby >= N_CHEBYMAX) { */
    if (ptilde_n_cheby > N_CHEBYMAX) {
      if(g_proc_id == g_stdio_proc){
	printf("Error: n_cheby=%d > N_CHEBYMAX=%d\n",ptilde_n_cheby,N_CHEBYMAX);
	printf("Increase n_chebymax\n");
      }
      errorhandler(35,"degree_of_polynomial");
    }


    /* either directly Ptilde X - X */
    Poly_tilde_ND(&auxs[0], &auxc[0], ptilde_cheby_coef, ptilde_n_cheby, &ss[0], &sc[0]);


    diff(&aux2s[0],&auxs[0],&ss[0],VOLUME/2);
    temp=square_norm(&aux2s[0],VOLUME/2)/square_norm(&ss[0],VOLUME/2)/4.0;

    diff(&aux2c[0],&auxc[0],&sc[0],VOLUME/2);
    temp2=square_norm(&aux2c[0],VOLUME/2)/square_norm(&sc[0],VOLUME/2)/4.0;

    if(g_epsbar == 0){
      temp2 = 0.0;
    }
    if(g_proc_id == g_stdio_proc) {
      printf("At n=%d  differences:  UP=%e  DN=%e stop=%e \n",ptilde_n_cheby,temp, temp2, stopeps_ptilde);
  
    }  
     if(temp < stopeps_ptilde && temp2 < stopeps_ptilde){ /* break;*/
      printf("differences:  UP=%e  DN=%e \n",temp, temp2);
      printf(" Latest (TILDE) polynomial degree = %d \n \n", ptilde_n_cheby);  
    break;
    }
      

    /* OLD INCREMENTATION
    ptilde_n_cheby*=1.05;
    BUT WE NEED AN EVEN DEGREE !!!! */
    ptilde_n_cheby+=2;
  }


#if ( defined SSE || defined SSE2 || defined SSE3)
   free(ss_);   
   free(auxs_); 
   free(aux2s_);
   free(sc_);   
   free(auxc_); 
   free(aux2c_);
#else
   free(ss);   
   free(auxs); 
   free(aux2s);
   free(sc);   
   free(auxc); 
   free(aux2c);
#endif

}
