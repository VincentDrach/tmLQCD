/* $Id$ */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "global.h"
#include "su3.h"
#include "init_gauge_field.h"

su3 * gauge_field = NULL;
su3 * gauge_field_back = NULL;

int init_gauge_field(const int V, const int back) {

  g_gauge_field = calloc(V, sizeof(su3*));
  if(errno == ENOMEM) {
    return(1);
  }
  gauge_field = calloc(4*V+1, sizeof(su3));
  if(errno == ENOMEM) {
    return(1);
  }
#if (defined SSE || defined SSE2 || defined SSE3)
  g_gauge_field[0] = (su3*)(((unsigned int)(gauge_field)+ALIGN_BASE)&~ALIGN_BASE);
#else
  g_gauge_field[0] = gauge_field;
#endif

  if(back == 1) {
    g_gauge_field_back = calloc(V, sizeof(su3*));
    if(errno == ENOMEM) {
      return(2);
    }
    gauge_field_back = calloc(4*V+1, sizeof(su3));
    if(errno == ENOMEM) {
      return(2);
    }
#if (defined SSE || defined SSE2 || defined SSE3)
    g_gauge_field_back[0] = (su3*)(((unsigned int)(gauge_field)+ALIGN_BASE)&~ALIGN_BASE);
#else
    g_gauge_field_back[0] = gauge_field;
#endif
  }
  return(0);
}

void free_gauge_field() {
  free(gauge_field);
  free(g_gauge_field);
  free(gauge_field_back);
  free(g_gauge_field_back);
}