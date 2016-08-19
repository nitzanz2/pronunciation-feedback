/*
 * feat_s2_4x.h -- Cepstral features computation.
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
 * 09-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#ifndef _LIBFEAT_FEAT_S2_4X_H_
#define _LIBFEAT_FEAT_S2_4X_H_


#include <../libutil/prim_type.h>


/*
 * Return input speech cepstrum vector size.
 */
int32 feat_s2_4x_cepsize ( int32 veclen );


/*
 * Return feature vector size for each feature stream.
 * Return value: #feature streams.
 */
int32 feat_s2_4x_featsize (int32 **size); /* Out: Upon return, *size is a ptr to the array,
					     allocated by this function, of feature stream
					     sizes.  It must NOT be modified or freed. */

/*
 * Each feature frame is computed from a window of 2W+1 input mfc vectors:
 *   mfc[-W]..mfc[W] centered around mfc[0].
 * Return value: W.
 */
int32 feat_s2_4x_window_size ( void );


/*
 * Compute feature vectors from a window of mfc frames.
 */
void feat_s2_4x_cep2feat (float32 **mfc,  /* In: Array of 2W+1 mfc vectors, CENTERED on
					     FEATURE WINDOW; ie, feature vectors computed
					     from mfc vectors mfc[-W]..mfc[W]. */
			  float32 **feat);/* Out: feat[f] = output vector for feature
					     stream f.  Caller must allocate this space */

#endif
