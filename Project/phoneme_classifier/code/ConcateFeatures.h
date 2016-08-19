/*
 *  ConcateFeatures.h
 *  pa_primal
 *
 *  Created by Joseph Keshet on 15/9/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <infra.h>
#include "Dataset.h"

class ConcateFeatures 
  {
  public:
    ConcateFeatures(int _d, int _num_concate_frames);
    int  features_dim() { return (d*num_concate_frames); }
    void concate_inplace(InstanceType &X);
    
  private: 
    int num_concate_frames;
    int d;
		int num_context_frames;
  };