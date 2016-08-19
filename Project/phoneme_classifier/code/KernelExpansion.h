/*
 *  KernelExpansion.h
 *  pa_primal
 *
 *  Created by Joseph Keshet on 28/8/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <string>
#include <infra.h>
#include "Dataset.h"

class KernelExpansion 
{
public:
  KernelExpansion(std::string _kernel_name, int _mfcc_dim, double _sigma = 1.0);
  int features_dim();
  infra::vector_view expand(infra::vector_base x);
  void expand_inplace(InstanceType &X);

private: 
  std::string kernel_name;
  int d;
  double sigma;
};