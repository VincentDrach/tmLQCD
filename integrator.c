/***********************************************************************
 * Copyright (C) 2002,2003,2004,2005,2006,2007,2008 Carsten Urbach
 *               2012 Carsten Urbach
 *
 * This file is part of tmLQCD.
 *
 * tmLQCD is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * tmLQCD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with tmLQCD.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************/

#ifdef HAVE_CONFIG_H
# include<config.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "su3.h"
#include "su3adj.h"
#include "expo.h"
#include "ranlxd.h"
#include "sse.h"
#include "global.h"
#include "linalg_eo.h"
#include "start.h"
#include "linsolve.h"
#include "tm_operators.h"
#include "monomial.h"
#include "update_momenta.h"
#include "update_gauge.h"
#include "hamiltonian_field.h"
#include "integrator.h"

integrator Integrator;

/* second order minimal norm integration scheme */
void integrate_2mn(const double tau, const int S, const int halfstep);
/* second order minimal norm integration scheme in velocity version */
void integrate_2mnp(const double tau, const int S, const int halfstep);
/* Leap Frog integration scheme */
void integrate_leap_frog(const double tau, const int S, const int halfstep);

/* function to initialise the integrator, to be called once at the beginning */

int init_integrator() {
  int i, ts;
  Integrator.hf.gaugefield = (su3 **) NULL;
  Integrator.hf.momenta = (su3adj **) NULL;
  Integrator.hf.derivative = (su3adj **) NULL;
  for(i = 0; i < 10; i++) {
    Integrator.no_mnls_per_ts[i] = 0;
  }
  if(Integrator.type[Integrator.no_timescales-1] == MN2p) {
    for(i = 0; i < Integrator.no_timescales; i++) {
      Integrator.type[i] = MN2p;
      Integrator.integrate[i] = &integrate_2mnp;
    }
  }
  else {
    for(i = 0; i < Integrator.no_timescales; i++) {
      if(Integrator.type[i] == MN2 || Integrator.type[i] == MN2p) {
	Integrator.integrate[i] = &integrate_2mn;
      }
      else if(Integrator.type[i] == LEAPFROG) {
	Integrator.integrate[i] = &integrate_leap_frog;
      }
    }
  }

  for(i = 0; i < no_monomials; i++) {
    ts = monomial_list[i].timescale;
    if(ts < Integrator.no_timescales && ts > -1 && monomial_list[i].type != CLOVERTRLOG) {
      Integrator.mnls_per_ts[ ts ][ Integrator.no_mnls_per_ts[ts] ] = monomial_list[i].id;
      Integrator.no_mnls_per_ts[ ts ]++;
    }
    else {
      if(g_proc_id == 0) {
	fprintf(stderr, "Warning: monomial %d is not on a valid timescale and will not be integrated\n", i);
      }
    }
  }
  for(i = 0; i < Integrator.no_timescales; i++) {
    if(Integrator.no_mnls_per_ts[ i ] < 1) {
      fprintf(stderr, "Error, no monomial on timescale %d!\nAborting...\n", i);
      exit(-1);
    }
  }
  return(0);
}

/* function to set the gauge and momenta fields for the integration */

void integrator_set_fields(hamiltonian_field_t * hf) {
  Integrator.hf.gaugefield = hf->gaugefield;
  Integrator.hf.momenta = hf->momenta;
  Integrator.hf.derivative = hf->derivative;
  Integrator.hf.update_gauge_copy = hf->update_gauge_copy;
  Integrator.hf.update_gauge_energy = hf->update_gauge_energy;
  Integrator.hf.update_rectangle_energy = hf->update_rectangle_energy;
  return;
}

/* and unsets again (to NULL pointer ) */

void integrator_unset_fields() {
  Integrator.hf.gaugefield = (su3 **) NULL;
  Integrator.hf.momenta = (su3adj **) NULL;
  Integrator.hf.derivative = (su3adj **) NULL;
  return;
}

/* the following are only needed locally */

void integrate_2mn(const double tau, const int S, const int halfstep) {
  int i,j=0;
  integrator * itgr = &Integrator;
  double eps,
    oneminus2lambda = (1.-2.*itgr->lambda[S]);

  if(S == itgr->no_timescales-1) {

    eps = tau/((double)itgr->n_int[S]);
    for(i = S; i > 0; i--) {
      update_momenta(itgr->mnls_per_ts[i], itgr->lambda[i]*eps, itgr->no_mnls_per_ts[i], &itgr->hf);
      if(itgr->type[i-1] == LEAPFROG) eps /= ((double)itgr->n_int[i-1]);
      else eps /= ((double)itgr->n_int[i-1])*2;
    }
    update_momenta(itgr->mnls_per_ts[0], itgr->lambda[0]*eps, itgr->no_mnls_per_ts[0], &itgr->hf);
  }
  
  eps = tau/((double)itgr->n_int[S]);
  if(S == 0) {

    for(j = 1; j < itgr->n_int[0]; j++) {
      update_gauge(0.5*eps, &itgr->hf);
      update_momenta(itgr->mnls_per_ts[0], oneminus2lambda*eps, itgr->no_mnls_per_ts[0], &itgr->hf);
      update_gauge(0.5*eps, &itgr->hf);
      update_momenta(itgr->mnls_per_ts[0], 2.*itgr->lambda[0]*eps, itgr->no_mnls_per_ts[0], &itgr->hf);
    }
    update_gauge(0.5*eps, &itgr->hf);
    update_momenta(itgr->mnls_per_ts[0], oneminus2lambda*eps, itgr->no_mnls_per_ts[0], &itgr->hf);
    update_gauge(0.5*eps, &itgr->hf);
    if(halfstep != 1) {
      update_momenta(itgr->mnls_per_ts[0], 2*itgr->lambda[0]*eps, itgr->no_mnls_per_ts[0], &itgr->hf);
    }
  }
  else {
    for(i = 1; i < itgr->n_int[S]; i++){
      itgr->integrate[S-1](eps/2., S-1, 0);
      update_momenta(itgr->mnls_per_ts[S], oneminus2lambda*eps, itgr->no_mnls_per_ts[S], &itgr->hf);
      itgr->integrate[S-1](eps/2., S-1, 0);
      update_momenta(itgr->mnls_per_ts[S], 2*itgr->lambda[S]*eps, itgr->no_mnls_per_ts[S], &itgr->hf);
    }
    itgr->integrate[S-1](eps/2., S-1, 0);
    update_momenta(itgr->mnls_per_ts[S], oneminus2lambda*eps, itgr->no_mnls_per_ts[S], &itgr->hf);
    if(S == itgr->no_timescales-1) {
      itgr->integrate[S-1](eps/2., S-1, 1);
    }
    else itgr->integrate[S-1](eps/2., S-1, halfstep);
    if(halfstep != 1 && S != itgr->no_timescales-1) {
      update_momenta(itgr->mnls_per_ts[S], 2*itgr->lambda[S]*eps, itgr->no_mnls_per_ts[S], &itgr->hf);
    }
  }

  if(S == itgr->no_timescales-1) {
    for(i = S; i > 0; i--) {
      update_momenta(itgr->mnls_per_ts[i], itgr->lambda[i]*eps, itgr->no_mnls_per_ts[i], &itgr->hf);
      eps /= ((double)itgr->n_int[i-1])*2;
    }
    update_momenta(itgr->mnls_per_ts[0], itgr->lambda[0]*eps, itgr->no_mnls_per_ts[0], &itgr->hf);
  }
}

void integrate_2mnp(const double tau, const int S, const int halfstep) {
  int i;
  integrator * itgr = &Integrator;
  double eps = tau/((double)itgr->n_int[S]);
  double oneminus2lambda = (1.-2.*itgr->lambda[S]);
  
  if(S == 0) {
    update_gauge(itgr->lambda[0]*eps, &itgr->hf);
    for(i = 1; i < itgr->n_int[0]; i++) {
      update_momenta(itgr->mnls_per_ts[0], 0.5*eps, itgr->no_mnls_per_ts[0], &itgr->hf);
      update_gauge(oneminus2lambda*eps, &itgr->hf);
      update_momenta(itgr->mnls_per_ts[0], 0.5*eps, itgr->no_mnls_per_ts[0], &itgr->hf);
      update_gauge(2*itgr->lambda[0]*eps, &itgr->hf);
    }
    update_momenta(itgr->mnls_per_ts[0], 0.5*eps, itgr->no_mnls_per_ts[0], &itgr->hf);
    update_gauge(oneminus2lambda*eps, &itgr->hf);
    update_momenta(itgr->mnls_per_ts[0], 0.5*eps, itgr->no_mnls_per_ts[0], &itgr->hf);
    update_gauge(itgr->lambda[0]*eps, &itgr->hf);
  }
  else {
    for(i = 0; i < itgr->n_int[S]; i++) {
      integrate_2mnp(itgr->lambda[S]*eps, S-1, halfstep);
      update_momenta(itgr->mnls_per_ts[S], 0.5*eps, itgr->no_mnls_per_ts[S], &itgr->hf);

      integrate_2mnp(oneminus2lambda*eps, S-1, halfstep);
      update_momenta(itgr->mnls_per_ts[S], 0.5*eps, itgr->no_mnls_per_ts[S], &itgr->hf);

      integrate_2mnp(itgr->lambda[S]*eps, S-1, halfstep);
    }
  }
}


void integrate_leap_frog(const double tau, const int S, const int halfstep) {
  int i;
  integrator * itgr = &Integrator;
  double eps, eps0;

  if(S == itgr->no_timescales-1) {
    /* initialize the counter for the inverter */
    eps = tau/((double)itgr->n_int[S]);

    for(i = S; i > 0; i--) {
      update_momenta(itgr->mnls_per_ts[S], 0.5*eps, itgr->no_mnls_per_ts[S], &itgr->hf);
      if(itgr->type[i-1] == LEAPFROG) eps /= ((double)itgr->n_int[i-1]);
      else eps /= ((double)itgr->n_int[i-1])*2;
    }
    update_momenta(itgr->mnls_per_ts[0], 0.5*eps, itgr->no_mnls_per_ts[0], &itgr->hf);
  }

  eps = tau/((double)itgr->n_int[S]);
  if(S == 0) {
    eps0 = tau/((double)itgr->n_int[0]);
    for(i = 1; i < itgr->n_int[0]; i++) {
      update_gauge(eps0, &itgr->hf);
      update_momenta(itgr->mnls_per_ts[0], eps0, itgr->no_mnls_per_ts[0], &itgr->hf);
    }
    update_gauge(eps0, &itgr->hf);
    if(halfstep != 1) {
      update_momenta(itgr->mnls_per_ts[0], eps0, itgr->no_mnls_per_ts[0], &itgr->hf);
    }
  }
  else {
    for(i = 1; i < itgr->n_int[S]; i++){
      itgr->integrate[S-1](eps, S-1, 0);
      update_momenta(itgr->mnls_per_ts[S], eps, itgr->no_mnls_per_ts[S], &itgr->hf);
    }
    if(S == itgr->no_timescales-1) {
      itgr->integrate[S-1](eps, S-1, 1);
    }
    else itgr->integrate[S-1](eps, S-1, halfstep);
    if(halfstep != 1 && S != itgr->no_timescales-1) {
      update_momenta(itgr->mnls_per_ts[S], eps, itgr->no_mnls_per_ts[S], &itgr->hf);
    }
  }

  if(S == itgr->no_timescales-1) {
    for(i = S; i > 0; i--) {
      update_momenta(itgr->mnls_per_ts[S], 0.5*eps, itgr->no_mnls_per_ts[S], &itgr->hf);
      if(itgr->type[i-1] == LEAPFROG) eps /= ((double)itgr->n_int[i-1]);
      else eps /= ((double)itgr->n_int[i-1])*2;
    }
    update_momenta(itgr->mnls_per_ts[0], 0.5*eps, itgr->no_mnls_per_ts[0], &itgr->hf);
  }
}
