/*
 * feat_1s_c_d_cd.c -- Cepstral feature stream; Sphinx3 version: single vector of
 * 		12 cep, 12 dcep, 3 pow, 12 ddcep.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 10-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string.h>
#include <assert.h>

#include "feat_1s_c_d_cd.h"
#include <../libutil/libutil.h>
#include <../libio/include/libio.h>


#define N_FEAT			1

#define FEAT_DCEP_WIN		2


static int32 feat_size[1];
static int32 basefeatlen;

int32 feat_1s_c_d_cd_cepsize ( int32 veclen )
{
    basefeatlen = veclen;
    feat_size[0] = veclen+veclen+veclen;
    return (veclen);
}


int32 feat_1s_c_d_cd_featsize (int32 **size)
{
    *size = feat_size;
    return N_FEAT;
}


/*
 * Feature vectors computed from input mfc vectors using this window (+/- window).
 */
int32 feat_1s_c_d_cd_window_size ( void )
{
    return (FEAT_DCEP_WIN+1);
}


void feat_1s_c_d_cd_cep2feat (float32 **mfc, float32 **feat)
{
    float32 *f;
    float32 *w, *_w;
    float32 *w1, *w_1, *_w1, *_w_1;
    float32 d1, d2;
    int32 i, j;
    
    /* CEP */
    memcpy (feat[0], mfc[0], basefeatlen * sizeof(float32));
    
    /*
     * DCEP: mfc[w] - mfc[-w], where w = FEAT_DCEP_WIN;
     */
    f = feat[0] + basefeatlen;
    w  = mfc[ FEAT_DCEP_WIN];
    _w = mfc[-FEAT_DCEP_WIN];

    for (i = 0; i < basefeatlen; i++)
	f[i] = w[i] - _w[i];
    
    /* 
     * D2CEP: (mfc[w+1] - mfc[-w+1]) - (mfc[w-1] - mfc[-w-1]), 
     * where w = FEAT_DCEP_WIN 
     */
    f += basefeatlen;

    w1   = mfc[ FEAT_DCEP_WIN+1];
    _w1  = mfc[-FEAT_DCEP_WIN+1];
    w_1  = mfc[ FEAT_DCEP_WIN-1];
    _w_1 = mfc[-FEAT_DCEP_WIN-1];

    for (i = 0; i < 2; i++) { /* compute regular ddcep for c0,c1 only */
	d1 =  w1[i] -  _w1[i];
	d2 = w_1[i] - _w_1[i];

	f[i] = d1 - d2;
    }
    for (i = 2; i < basefeatlen; i++) {
	f[i] = -mfc[0][i]+mfc[0][i-1];
    }
}
