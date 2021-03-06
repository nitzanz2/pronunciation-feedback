/*
 * mllr.c -- Application of MLLR regression matrices to codebook means
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
 * 02-Dec-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added reading of MLLR classes in transformation file.  Currently must
 * 		be 1.
 * 
 * 26-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started (copied from Vipul Parikh's implementation).
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <libutil/libutil.h>
#include <s3.h>

#include "mllr.h"


int32 mllr_read_regmat (const char *regmatfile,
			float32 ****A,
			float32 ***B,
			int32 *streamlen,
			int32 n_stream)
{
    int32 i, j, k, n;
    FILE  *fp;
    float32 ***lA, **lB;

    if ((fp = fopen(regmatfile, "r")) == NULL) {
	E_ERROR ("fopen(%s,r) failed\n", regmatfile);
	return -1;
    } else
	E_INFO ("Reading MLLR transformation file %s\n", regmatfile);
    
    lA = (float32 ***) ckd_calloc (n_stream, sizeof (float32 **));
    lB = (float32 **) ckd_calloc (n_stream, sizeof (float32 *));
    for (i = 0; i < n_stream; ++i) {
	lA[i] = (float32 **) ckd_calloc_2d (streamlen[i], streamlen[i], sizeof(float32));
	lB[i] = (float32 *) ckd_calloc (streamlen[i], sizeof(float32));
    }

    /* Read #MLLR-classes; must be 1 for now (rkm@cs.cmu.edu, 12-Dec-1996) */
    if ((fscanf (fp, "%d", &n) != 1) || (n != 1))
	goto readerror;
    
    if ((fscanf (fp, "%d", &n) != 1) || (n != n_stream))
	goto readerror;
    
    for (i = 0; i < n_stream; ++i) {
	if ((fscanf(fp, "%d", &n) != 1) || (streamlen[i] != n))
	    goto readerror;
	
	for (j = 0; j < streamlen[i]; j++) {
	    for (k = 0; k < streamlen[i]; ++k) {
		if (fscanf(fp, "%f ", &lA[i][j][k]) != 1)
		    goto readerror;
	    }
	}
	for (j = 0; j < streamlen[i]; j++) {
	    if (fscanf(fp, "%f ", &lB[i][j]) != 1)
		goto readerror;
	}
    }

    *A = lA;
    *B = lB;

    fclose(fp);

    return 0;

readerror:
    E_ERROR("Error reading MLLR file %s\n", regmatfile);
    for (i = 0; i < n_stream; i++) {
	ckd_free_2d ((void **)lA[i]);
	ckd_free (lB[i]);
    }
    ckd_free (lA);
    ckd_free (lB);
    fclose (fp);
    
    *A = NULL;
    *B = NULL;
    
    return -1;
}


int32 mllr_free_regmat (float32 ***A,
			float32 **B,
			int32 *streamlen,
			int32 n_stream)
{
    int32 i;
    
    for (i = 0; i < n_stream; i++) {
	ckd_free_2d ((void **) A[i]);
	ckd_free (B[i]);
    }

    ckd_free (A);
    ckd_free (B);

    return 0;
}


int32 mllr_norm_mgau (float32 ***mean,
		      int32 n_density,
		      float32 ***A,
		      float32 **B,
		      int32 *streamlen,
		      int32 n_stream)
{
    int32 s, d, l, m;
    float64 *temp;
    
    /* Transform codebook for each stream s */
    for (s = 0; s < n_stream; s++) {
	temp = (float64 *) ckd_calloc (streamlen[s], sizeof(float64));

	/* Transform each density d in selected codebook */
	for (d = 0; d < n_density; d++) {
	    for (l = 0; l < streamlen[s]; l++) {
		temp[l] = 0.0;
		for (m = 0; m < streamlen[s]; m++) {
		    temp[l] += A[s][l][m] * mean[s][d][m];
		}
		temp[l] += B[s][l];
	    }

	    for (l = 0; l < streamlen[s]; l++) {
		mean[s][d][l] = (float32) temp[l];
	    }
	}

	ckd_free (temp);
    }

    return 0;
}
