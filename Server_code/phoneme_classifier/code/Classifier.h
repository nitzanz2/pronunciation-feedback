#ifndef _CLASSIFIER_H
#define _CLASSIFIER_H

/************************************************************************
 Project:  Segmentation and Alignment
 Module:   Classifier
 Purpose:  Alignment discriminative algorithm
 Date:     10 Apr., 2004
 Programmer: Shai Shalev-Shwartz

 *************************** INCLUDE FILES ******************************/
#include <infra.h>
#include <string>
#include <map>
#include "Dataset.h"
#include <active_set/active_set.h>
#include <active_set/active_set.imp>
#include <kernels/kernels.h>
#include <kernels/kernels.imp>

class Classifier
{
public:
  Classifier(double _BB, double _CC, int _num_phonemes, int _features_dim);
  ~Classifier();
  double update(infra::vector_view x, int y, int y_hat);
	double update_crf( infra::vector_view x, int y, int y_hat);
  infra::vector_view predict(infra::vector_view x, int &y_hat);
  void load(std::string &filename);
  void save(std::string &filename);
  void averaging();
  
protected:
  double BB;
  double CC;
public:
  infra::matrix W;
  infra::matrix W_averaging;
  infra::vector ranks;
protected:
  bool averaging_enabled;
  int num_updates;
};

#endif // _CLASSIFIER_H
