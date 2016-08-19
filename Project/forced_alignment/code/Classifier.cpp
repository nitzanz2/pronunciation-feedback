/************************************************************************
 Copyright (c) 2006 Joseph Keshet
 
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are 
 met: Redistributions of source code must retain the above copyright 
 notice, this list of conditions and the following disclaimer.
 Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the 
 documentation and/or other materials provided with the distribution.
 The name of the author may not be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR 
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, 
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
 THE POSSIBILITY OF SUCH DAMAGE.
 ************************************************************************/

/************************************************************************
 Project:  Phonetic Segmentation
 Module:   Classifier
 Purpose:  Segmentation classifier
 Date:     10 Apr., 2004
 Programmer: Joseph Keshet
 
 **************************** INCLUDE FILES *****************************/
#include "Classifier.h"
#include "array3dim.h"
#include <iostream>

#define GAMMA_EPSILON 1
#define MISPAR_KATAN_MEOD (-1000000)
#define NORM_TYPE1 // normalize each phoneme in phi_0 by its num of frames
//#define NORM_TYPE2
#define NORM_SCORES_0_1
#define _min(a,b) (((a)<(b))?(a):(b))

int Classifier::phi_size = 7; 
std::string Classifier::loss_type;

/************************************************************************
 Function:     Classifier::Classifier
 
 Description:  Constructor
 Inputs:       none.
 Output:       none.
 Comments:     none.
 ***********************************************************************/
Classifier::Classifier(unsigned int _frame_rate, double _min_phoneme_length, 
                       double _max_phoneme_length, double _PA1_C,
                       double _beta1, double _beta2, double _beta3, 
                       double _min_sqrt_gamma, std::string _loss_type) : 
frame_rate(_frame_rate),
w(phi_size), 
w_old(phi_size),
PA1_C(_PA1_C),
beta1(_beta1), beta2(_beta2), beta3(_beta3), min_sqrt_gamma(_min_sqrt_gamma)
{
  
  min_num_frames = int(_min_phoneme_length/double(frame_rate));
  max_num_frames = int(_max_phoneme_length/double(frame_rate));
  
  w.zeros();
  w_old.zeros();
  
	if (_loss_type != "tau_insensitive_loss" && _loss_type != "alignment_loss") {
		std::cout << "Error: undefined loss type" << std::endl;
    exit(-1);
  }
	std::cout << "Info: training using " << _loss_type << "." << std::endl;
  loss_type = _loss_type;
} 



/************************************************************************
 Function:     Classifier::~Classifier
 
 Description:  Destructor
 Inputs:       none.
 Output:       none.
 Comments:     none.
 ***********************************************************************/
Classifier::~Classifier()
{
}


/************************************************************************
 Function:     Classifier::load_phoneme_stats
 
 Description:  Load phoneme statistics
 Inputs:       none.
 Output:       none.
 Comments:     none.
 ***********************************************************************/
void Classifier::load_phoneme_stats(std::string &filename)
{
  std::ifstream ifs(filename.c_str());
  if (!ifs.good()) {
    std::cerr << "Error: Unable to load phoneme stats from " << filename << std::endl;
    exit(-1);
  }
  
  try {
	infra::matrix tmp1(ifs);
  phoneme_length_mean.resize(tmp1.width());
  phoneme_length_mean = tmp1.row(0);
  phoneme_length_std.resize(tmp1.width());
  phoneme_length_std = tmp1.row(1);
  ifs.close();
	}
	catch (...) {
		std::cerr << "Exception occurred. Cannot read " << filename << " as an infra matrix file." << std::endl;
	}
  if (phoneme_length_mean.size() != PhonemeSequence::num_phonemes || 
      phoneme_length_std.size() != PhonemeSequence::num_phonemes) {
    std::cerr << "Error: number of phonemes loaded from phoneme stats file "
    << filename << " is incorrect:" << std::endl;
    std::cerr << "phoneme_length_mean=" << phoneme_length_mean << std::endl;
    std::cerr << "phoneme_length_std=" << phoneme_length_std << std::endl;
  }
}


/************************************************************************
 Function:     Classifier::update
 
 Description:  Train classifier with one example 
 Inputs:       infra::vector& x - example instance 
 int y - label 
 Output:       double - squared loss
 Comments:     none.
 ***********************************************************************/
double Classifier::update(SpeechUtterance& x, StartTimeSequence &y,
                          StartTimeSequence &y_hat) 
{
  double loss = 0.0;
  
  loss = sqrt( gamma(y,y_hat) );
  
  std::cout << "sqrt(gamma) = " << loss << std::endl;
  
  if (loss < min_sqrt_gamma) {
    w_changed = false;
    return 0.0;
  }
  
  infra::vector delta_phi(phi_size);
  infra::vector_view phi_x_y = phi(x,y);
  infra::vector_view phi_x_y_hat = phi(x,y_hat);
  
  delta_phi = phi_x_y - phi_x_y_hat;
  delta_phi /= y.size();
  loss -= w*delta_phi;
  
  std::cout << "phi(x,y)= " << phi_x_y << std::endl;
  std::cout << "phi(x,y_hat)= " << phi_x_y_hat << std::endl;
  std::cout << "delta_phi = " << delta_phi << std::endl;
  std::cout << "delta_phi.norm2() = "  << delta_phi.norm2() << std::endl;
  std::cout << "loss(PA) = " << loss << std::endl;
  
  if (loss < 0.0) {
    std::cerr << "Error: loss is less than 0" << std::endl;
	}
  else {
		// keep old w
		w_old = w;
    // update
    double tau = loss / delta_phi.norm2();
    if (tau > PA1_C) tau = PA1_C; // PA-I
    std::cout << "tau(PA) = " << tau << std::endl;
		delta_phi *= tau;
		w += delta_phi;
  }
  
	std::cout << "w = " << w << std::endl;  
	w_changed = true;
  
  return loss;
}

/************************************************************************
 Function:     Classifier::update_direct_loss
 
 Description:  Train classifier with one example 
 Inputs:       infra::vector& x - example instance 
 int y - label 
 Output:       double - squared loss
 Comments:     none.
 ***********************************************************************/
double Classifier::update_direct_loss(SpeechUtterance& x, StartTimeSequence &y_hat_eps, 
                                      StartTimeSequence &y_hat, StartTimeSequence &y, 
                                      double epsilon) 
{  
  infra::vector_view phi_x_y_hat_eps = phi(x,y_hat_eps);
  infra::vector_view phi_x_y_hat = phi(x,y_hat);
  
  infra::vector delta_phi(phi_size);
  delta_phi = phi_x_y_hat_eps - phi_x_y_hat;
  delta_phi /= y_hat_eps.size();
  
  std::cout << "phi(x,y_hat_eps)= " << phi_x_y_hat_eps << std::endl;
  std::cout << "phi(x,y_hat)= " << phi_x_y_hat << std::endl;
  std::cout << "delta_phi = " << delta_phi << std::endl;
  std::cout << "delta_phi.norm2() = "  << delta_phi.norm2() << std::endl;
  
	// compute the PA update
  double loss_pa = sqrt( gamma(y_hat_eps,y_hat) );
	loss_pa -= w*delta_phi;
	double tau_pa = 0.0;
	if (loss_pa > 0.0) {
		tau_pa = loss_pa / delta_phi.norm2();
    std::cout << "tau(PA) = " << tau_pa << std::endl;
	}
	
  // keep the loss and w before update
  infra::vector w_before_update(w);
  
  // start up values for update
  StartTimeSequence y_hat_eps_tmp(y_hat_eps);
  StartTimeSequence y_hat_tmp(y_hat);
  double tau = tau_pa;
  double loss_before_update = gamma(y_hat_eps,y_hat);
  double loss_after_update;
  
  if (loss_before_update == 0) {
    w = w_before_update;  
    w_changed = false;
    return 0;
  }
  
  // update
  do {
    w = w_before_update;
    infra::vector delta_phi_temp(delta_phi);
    delta_phi_temp *= tau;
    w += delta_phi_temp;
    // check loss after update
    predict(x, y_hat_tmp);
    predict_epsilon(x, y_hat_eps_tmp, y, epsilon);
    loss_after_update = gamma(y_hat_eps_tmp,y_hat_tmp);
    std::cout << "tau(DA)=" << tau 
    << " loss_before=" << loss_before_update
    << " loss_after=" << loss_after_update << std::endl;
    tau /= 2;
  } while (loss_after_update >= loss_before_update && tau > 0.05) ;
  
  
  if (loss_after_update >= loss_before_update) {
    w = w_before_update;  
    w_changed = false;
  }
  else {
    // keep old w
    w_old = w_before_update;
    std::cout << "w = " << w << std::endl;  
    w_changed = true;
  }
  
  return loss_after_update;
}



/************************************************************************
 Function:     phi
 
 Description:  calculate phi of x for update
 Inputs:       SpeechUtterance &x
 StartTimeSequence &y
 Output:       infra::vector_view
 Comments:     none.
 ***********************************************************************/
infra::vector_view Classifier::phi(SpeechUtterance& x, StartTimeSequence& y) 
{
  infra::vector v(phi_size);
  v.zeros();
  for (int i=0; i < int(y.size()); i++) {
    int phoneme_end_at;
    if (i == int(y.size())-1) 
      phoneme_end_at = x.scores.height()-1;
    else
      phoneme_end_at = y[i+1]-1;
    v.subvector(0,phi_size-1) += phi_1(x, i, phoneme_end_at, phoneme_end_at-y[i]+1);
    if (i > 0) 
      v(phi_size-1) += phi_2(x, i, phoneme_end_at, phoneme_end_at-y[i]+1, y[i]-y[i-1]);
	}
  return v;
}

/************************************************************************
 Function:     phi_1
 
 Description:  calculate static part of phi for inference
 Inputs:       SpeechUtterance &x - raw features
 int i - phoneme index
 int t - phoneme end time
 int l - phoneme length
 Output:       infra::vector_view
 Comments:     none.
 ***********************************************************************/
infra::vector_view Classifier::phi_1(SpeechUtterance& x, 
                                     int i, // phoneme index
                                     int t, // phoneme end time
                                     int l) // phoneme length
{
  infra::vector v(phi_size-1);
  v.zeros();
  
#ifdef NORM_SCORES_0_1
  for (int tau = t-l+1; tau <= t; tau++) 
    v(0) +=  x.scores(tau,x.phonemes[i]);
  // for NORM_SCORES_0_1 the normalization is done at load time in Dataset.cpp
#else
  for (int tau = t-l+1; tau <= t; tau++) 
    v(0) +=  x.scores(tau,x.phonemes[i]) - x.scores.row(tau).max();
#endif
  
#ifdef NORM_TYPE1 
  v(0) /= l;
#endif
  v(1) = beta1*x.distances(t-l+1,0); 
  v(2) = beta1*x.distances(t-l+1,1);
  v(3) = beta1*x.distances(t-l+1,2);
  v(4) = beta1*x.distances(t-l+1,3);
  v(5) = beta2*gaussian(l, phoneme_length_mean[x.phonemes[i]], 
                        phoneme_length_std[x.phonemes[i]]);
  
  return (v/double(x.phonemes.size()));
}

/************************************************************************
 Function:     phi_2
 
 Description:  calculate dynamic part of phi for inference
 Inputs:       SpeechUtterance &x - raw features
 int i - phoneme index
 int t - phoneme end time
 int l1 - phoneme length
 int l2 - previous phoneme length
 Output:       infra::vector_view
 Comments:     none.
 ***********************************************************************/
double Classifier::phi_2(SpeechUtterance& x, 
                         int i, // phoneme index
                         int t, // phoneme end time
                         int l1, // phoneme length
                         int l2) // previous phoneme len
{
  double v = 0;
  v = (double(l1)/phoneme_length_mean[x.phonemes[i]] -
       double(l2)/phoneme_length_mean[x.phonemes[i-1]]);
  v *= v;
  v *= beta3;
  
  return (v/double(x.phonemes.size()));
}


/************************************************************************
 Function:     Classifier::predict
 
 Description:  Predict label of instance x
 Inputs:       SpeechUtterance &x 
 StartTimeSequence &y_hat
 Output:       void
 Comments:     none.
 ***********************************************************************/
double Classifier::predict(SpeechUtterance& x, StartTimeSequence &y_hat)
{
  // predict label - the argmax operator
  int P = x.phonemes.size();
  int T = x.scores.height();
  int L = max_num_frames+1;
  threeDimArray<int> prev_l(P,T,L); // the value of l2 for back-tracking
  threeDimArray<int> prev_t(P,T,L); // the value of t2 for back-tracking
  threeDimArray<double> D0(P,T,L); // phoneme i finished at time t and started at t-l+1
  double D1, D2, D2_max; // helper variables
  
  // Initialization
  for (int i = 0; i < P; i++) 
    for (int t = 0; t < T; t++) 
      for (int l1 = 0; l1 < L; l1++)
        D0(i,t,l1) = MISPAR_KATAN_MEOD;
  
  // Here calculate the calculation for the culculata
  for (int t = min_num_frames; t < _min(T,max_num_frames); t++) {
    D1 = w.subvector(0,phi_size-1) * phi_1(x,0,t,t+1);
    D0(0,t,t+1) = D1;
  }
  
  // Recursion
  for (int i = 1; i < P; i++) {
    //		for (int t = i*min_num_frames; t < _min(i*max_num_frames,T); t++) { 
    for (int t = min_num_frames; t < T; t++) { 
      int stop_l1_at = (t < _min(T,max_num_frames) ) ? t : _min(T,max_num_frames);
      for (int l1 = min_num_frames; l1 <= stop_l1_at; l1++) {
        D1 = w.subvector(0,phi_size-1) * phi_1(x,i,t,l1);
        D2_max = MISPAR_KATAN_MEOD;
        for (int l2 = min_num_frames; l2 <= _min(T,max_num_frames); l2++) {
          D2 = D0(i-1,t-l1,l2) + w(phi_size-1) * phi_2(x,i,t,l1,l2);
          if (D2 >= D2_max) {
            D2_max = D2;
            prev_l(i,t,l1) = l2;
            prev_t(i,t,l1) = t-l1;
          }
        }
        D0(i,t,l1) = D1 + D2_max;
      }
    }
  }
  
  // Termination
	std::vector<int> pred_l(P);
  std::vector<int>  pred_t(P);
  D2_max = MISPAR_KATAN_MEOD;
  for (int l = min_num_frames; l <= _min(T,max_num_frames); l++) {
    if (D0(P-1,T-1,l) > D2_max) {
      D2_max = D0(P-1,T-1,l);
      pred_l[P-1] = l;
      pred_t[P-1] = T-1;
    }
  }
  y_hat[P-1] = T-1-pred_l[P-1]+1;
  
  // Back-tracking
  for (short p = P-2; p >= 0; p--) {
    pred_l[p] = prev_l(p+1,pred_t[p+1],pred_l[p+1]); 
    pred_t[p] = prev_t(p+1,pred_t[p+1],pred_l[p+1]); 
    y_hat[p] = pred_t[p]-pred_l[p]+1;
  }
  y_hat[0] = 0;
  
  std::cout << "D2_max=" << D2_max << " w*phi(x,y_hat)=" 
  << w*phi(x,y_hat) << std::endl;
  
  return (D2_max/double(T));
}

double Classifier::confidence_general(SpeechUtterance& x, StartTimeSequence& y_hat)
{
        double confidence = w*phi(x,y_hat);
        return confidence;
}


infra::matrix Classifier::confidence_per_phoneme(SpeechUtterance& x, StartTimeSequence& y)
{
  infra::vector v(y.size());
  v.zeros();
  infra::matrix confidence_matrix(int(y.size()),x.scores.width());
  confidence_matrix.zeros();
  for (int i=0; i < int(y.size()); i++) {
    int phoneme_end_at;
    if (i == int(y.size())-1)
      phoneme_end_at = x.scores.height()-1;
    else
      phoneme_end_at = y[i+1]-1;
    for (int phoneme_index=0; phoneme_index < int(x.scores.width()); phoneme_index++) {
      confidence_matrix(i,phoneme_index) = w.subvector(0,phi_size-1) * phi_1_per_phoneme(x, phoneme_index, phoneme_end_at, phoneme_end_at-y[i]+1);
      if (i > 0)
        confidence_matrix(i,phoneme_index) +=  w(phi_size-1) * phi_2_per_phoneme(x, phoneme_index,x.phonemes[i-1], phoneme_end_at, phoneme_end_at-y[i]+1, y[i]-y[i-1]);
    }
    //v(i) = w.subvector(0,phi_size-1) * phi_1(x, i, phoneme_end_at, phoneme_end_at-y[i]+1);
    //if (i > 0)
    //  v(i) +=  w(phi_size-1) * phi_2(x, i, phoneme_end_at, phoneme_end_at-y[i]+1, y[i]-y[i-1]);
  }
  //return v;

  return confidence_matrix;
}

infra::matrix Classifier::normalizeConfidences(infra::matrix confidences) {
    infra::matrix normalized_confidences(confidences.height(),confidences.width());
    normalized_confidences.zeros();
    for (int row = 0; row < normalized_confidences.height(); row++) {
        double sum = 0;
        for(int col = 0; col < normalized_confidences.width(); col++) {
          sum += exp(confidences(row, col));
        }
        for(int col=0; col < normalized_confidences.width(); col++) {
          normalized_confidences(row, col) = exp(confidences(row, col)) / sum;
        }
    }
    return normalized_confidences;
}

infra::vector_view Classifier::phi_1_per_phoneme(SpeechUtterance& x, 
                                     //int i, // phoneme index
                                     int phoneme,
                                     int t, // phoneme end time
                                     int l) // phoneme length
{
  infra::vector v(phi_size-1);
  v.zeros();
  
#ifdef NORM_SCORES_0_1
  for (int tau = t-l+1; tau <= t; tau++) 
    v(0) +=  x.scores(tau,phoneme);
  // for NORM_SCORES_0_1 the normalization is done at load time in Dataset.cpp
#else
  for (int tau = t-l+1; tau <= t; tau++) 
    v(0) +=  x.scores(tau,phoneme) - x.scores.row(tau).max();
#endif
  
#ifdef NORM_TYPE1 
  v(0) /= l;
#endif
  v(1) = beta1*x.distances(t-l+1,0); 
  v(2) = beta1*x.distances(t-l+1,1);
  v(3) = beta1*x.distances(t-l+1,2);
  v(4) = beta1*x.distances(t-l+1,3);
  v(5) = beta2*gaussian(l, phoneme_length_mean[phoneme], 
                        phoneme_length_std[phoneme]);
  
  return (v/double(x.phonemes.size()));
}

/************************************************************************
 Function:     phi_2
 
 Description:  calculate dynamic part of phi for inference
 Inputs:       SpeechUtterance &x - raw features
 int i - phoneme index
 int t - phoneme end time
 int l1 - phoneme length
 int l2 - previous phoneme length
 Output:       infra::vector_view
 Comments:     none.
 ***********************************************************************/
double Classifier::phi_2_per_phoneme(SpeechUtterance& x, 
                         //int i, // phoneme index
                         int phoneme1,
                         int phoneme2,
                         int t, // phoneme end time
                         int l1, // phoneme length
                         int l2) // previous phoneme len
{
  double v = 0;
  v = (double(l1)/phoneme_length_mean[phoneme1] -
       double(l2)/phoneme_length_mean[phoneme2]);
  v *= v;
  v *= beta3;
  
  return (v/double(x.phonemes.size()));
}

/************************************************************************
 Function:     Classifier::aligned_phoneme_scores
 
 Description:  Compute the score of the pronunced phonemes x given alignment y
 Inputs:       SpeechUtterance &x
 StartTimeSequence &y
 Output:       score
 Comments:     none.
 ***********************************************************************/
double Classifier::aligned_phoneme_scores(SpeechUtterance& x, StartTimeSequence &y)
{
	double score = 0.0;
	
	// run over all phoneme except the last one
	for (int i = 0; i < y.size()-1; i++)
		for (int t = y[i]; t < y[i+1]-1; t++)
			score += x.original_scores(t, x.phonemes[i]);

	// last phoneme
	for (int t = y[y.size()-1]; t < x.original_scores.height(); t++)
		score += x.original_scores(t, x.phonemes[y.size()-1]);

	//return score;
  return score/x.scores.height();
}

/************************************************************************
 Function:     Classifier::align_keyword
 
 Description:  Predict label of instance x
 Inputs:       SpeechUtterance &x 
 StartTimeSequence &y_hat
 Output:       void
 Comments:     none.
 ***********************************************************************/
double Classifier::align_keyword(SpeechUtterance& x, StartTimeSequence &y_hat_best, 
                                 int &end_frame_best)
{	
  // predict label - the argmax operator
  int P = x.phonemes.size();
  int T = x.scores.height();
  int L = _min(T,max_num_frames)+1;
  threeDimArray<int> prev_l(P,T,L); 
  threeDimArray<int> prev_t(P,T,L); 
  threeDimArray<double> D0(P,T,L);  
  double D1, D2, D2_max = MISPAR_KATAN_MEOD, D0_best; // helper variables
  D0_best = MISPAR_KATAN_MEOD;
	std::vector<int> pred_l(P);
  std::vector<int> pred_t(P);
  int best_s = 0;
  StartTimeSequence y_hat;
  y_hat.resize(x.phonemes.size());
  y_hat_best.resize(x.phonemes.size());
  int end_frame;
  
  for (int s = 0; s < T-P*min_num_frames; s++) {
    // Initialization
    for (int i = 0; i < P; i++) 
      for (int t = 0; t < T; t++) 
        for (int l1 = 0; l1 < L; l1++)
          D0(i,t,l1) = MISPAR_KATAN_MEOD;
    
    // Here calculate the calculation for the culculata
    for (int t = s+min_num_frames; t < _min(s+max_num_frames,T); t++) {
      D0(0,t,t-s+1) = w.subvector(0,phi_size-1) * phi_1(x,0,t,t-s+1);
    }
    
    // Recursion
    for	(int i = 1; i < P; i++) {
      for (int t = s+i*min_num_frames; t < _min(s+i*max_num_frames, T); t++) { 
        int stop_l1_at = (t < _min(T,max_num_frames)) ? t : _min(T,max_num_frames);
        for (int l1 = min_num_frames; l1 <= stop_l1_at; l1++) {
          D1 = w.subvector(0,phi_size-1) * phi_1(x,i,t,l1);
          D2_max = MISPAR_KATAN_MEOD;
          for (int l2 = min_num_frames; l2 <= _min(T,max_num_frames); l2++) {
            D2 = D0(i-1,t-l1,l2) + w(phi_size-1) * phi_2(x,i,t,l1,l2);
            if (D2 > D2_max) {
              D2_max = D2;
              prev_l(i,t,l1) = l2;
              prev_t(i,t,l1) = t-l1;
            }
          }
          D0(i,t,l1) = D1 + D2_max;
        }
      }
    }
    
    // Termination
    D2_max = MISPAR_KATAN_MEOD;
    for (int t=s+(P-1)*min_num_frames; t<_min(s+(P-1)*max_num_frames, T); t++)  {
      for (int l = min_num_frames; l <= _min(T,max_num_frames); l++) {
        if (D0(P-1,t,l) > D2_max) {
          D2_max = D0(P-1,t,l);
          pred_l[P-1] = l;
          pred_t[P-1] = t;
        }
      }
    }
    y_hat[P-1] = pred_t[P-1]-pred_l[P-1]+1;
    // Back-tracking
    for (short p = P-2; p >= 0; p--) {
      pred_l[p] = prev_l(p+1,pred_t[p+1],pred_l[p+1]); 
      pred_t[p] = prev_t(p+1,pred_t[p+1],pred_l[p+1]); 
      y_hat[p] = pred_t[p]-pred_l[p]+1;
    }
    y_hat[0] = s;
    end_frame = pred_t[P-1];
    
    // apply normalization
#ifdef NORM_TYPE2
    D2_max /= (end_frame-s+1);
#endif		
    //		std::cout << "y_hat=" << y_hat << " " << pred_t[P-1] << std::endl;
    //		std::cout << "w*phi(predicted)=" << w*phi(x,keyword, y_hat, pred_t[P-1]) << std::endl;
    //		std::cout << "D2_max=" << D2_max << std::endl;
    
    if (D2_max > D0_best) {
      //std::cout << "s=" << s << " D2_max=" << D2_max << " D0_best=" << D0_best << std::endl;
      D0_best = D2_max;
      best_s = s;
      for	(int i = 0; i < P; i++)
        y_hat_best[i] = y_hat[i];
      end_frame_best = end_frame;
    }
  }
  return D0_best;
}



/************************************************************************
 Function:     Classifier::predict_epsilon
 
 Description:  Predict label of instance x
 Inputs:       SpeechUtterance &x 
 StartTimeSequence &y_hat
 StartTimeSequence &y
 double epsilon
 Output:       void
 Comments:     none.
 ***********************************************************************/
double Classifier::predict_epsilon(SpeechUtterance& x, StartTimeSequence &y_hat, 
                                   StartTimeSequence &y, double epsilon)
{
  // predict label - the argmax operator
  int P = x.phonemes.size();
  int T = x.scores.height();
  int L = _min(T,max_num_frames)+1;
  threeDimArray<int> prev_l(P,T,L); // the value of l2 for back-tracking
  threeDimArray<int> prev_t(P,T,L); // the value of t2 for back-tracking
  threeDimArray<double> D0(P,T,L); // phoneme i finished at time t and started at t-l+1
  double D1, D2, D2_max; // helper variables
  
  // Initialization
  for (int i = 0; i < P; i++) 
    for (int t = 0; t < T; t++) 
      for (int l1 = 0; l1 < L; l1++)
        D0(i,t,l1) = MISPAR_KATAN_MEOD;
  
  // Here calculate the calculation for the culculata
  for (int t = min_num_frames; t < _min(T,max_num_frames); t++) {
    D1 = w.subvector(0,phi_size-1) * phi_1(x,0,t,t+1);
    D1 -= epsilon*gamma(0, y[0])/double(P);    
    D0(0,t,t+1) = D1;
  }
  
  // Recursion
  for (int i = 1; i < P; i++) {
    //		for (int t = i*min_num_frames; t < _min(i*max_num_frames,T); t++) { 
    for (int t = min_num_frames; t < T; t++) { 
      int stop_l1_at = (t < _min(T,max_num_frames)) ? t : _min(T,max_num_frames);
      for (int l1 = min_num_frames; l1 <= stop_l1_at; l1++) {
        D1 = w.subvector(0,phi_size-1) * phi_1(x,i,t,l1);
        D1 -= epsilon*gamma(t-l1+1, y[i])/double(P);
        D2_max = MISPAR_KATAN_MEOD;
        for (int l2 = min_num_frames; l2 <= _min(T,max_num_frames); l2++) {
          D2 = D0(i-1,t-l1,l2) + w(phi_size-1) * phi_2(x,i,t,l1,l2);
          if (D2 > D2_max) {
            D2_max = D2;
            prev_l(i,t,l1) = l2;
            prev_t(i,t,l1) = t-l1;
          }
        }
        D0(i,t,l1) = D1 + D2_max;
      }
    }
  }
  
  // Termination
  std::vector<int> pred_l(P);
  std::vector<int>  pred_t(P);
  D2_max = MISPAR_KATAN_MEOD;
  for (int l = min_num_frames; l <= _min(T,max_num_frames); l++) {
    if (D0(P-1,T-1,l) > D2_max) {
      D2_max = D0(P-1,T-1,l);
      pred_l[P-1] = l;
      pred_t[P-1] = T-1;
    }
  }
  y_hat[P-1] = T-1-pred_l[P-1]+1;
  //  y_hat[P-1] = T-pred_l[P-1]+1;
  
  // Back-tracking
  for (short p = P-2; p >= 1; p--) {
    pred_l[p] = prev_l(p+1,pred_t[p+1],pred_l[p+1]); 
    pred_t[p] = prev_t(p+1,pred_t[p+1],pred_l[p+1]); 
    y_hat[p] = pred_t[p]-pred_l[p]+1;
  }
  y_hat[0] = 0;
  
  //	std::cout << "D2_max=" << D2_max << " w*phi(x,y_hat)-epsilon*gamma(y,y_hat)="			
  //	<< w*phi(x,y_hat)-epsilon*gamma(y,y_hat) << std::endl;
  
  return D2_max;
}


/************************************************************************
 Function:     gamma
 
 Description:  Distance between two labels
 Inputs:       StartTimeSequence &y
 StartTimeSequence &y_hat
 Output:       double - the resulted distance
 Comments:     none.
 ***********************************************************************/
double Classifier::gamma(const StartTimeSequence &y, const StartTimeSequence &y_hat)
{
  double loss = 0.0;
  
  if (loss_type == "tau_insensitive_loss")  {
    
    for (unsigned long i=0;i < y.size(); ++i) {
      double loss_i = fabs( double(y_hat[i]) - double(y[i]) ) - GAMMA_EPSILON;
      if (loss_i > 0.0) loss += loss_i;
    }
    
  }
  else if (loss_type == "alignment_loss")  {
    
    for (unsigned long i=0;i < y.size(); ++i) {
      loss += ( fabs( double(y_hat[i]) - double(y[i]) ) > GAMMA_EPSILON ) ? 1.0 : 0.0 ;
    }
    
  }
  else {
    std::cerr << "Error: loss type \"" << loss_type << "\" is undefined." << std::endl;
    exit(-1);
  }
  return loss/double(y.size());
  
}


/************************************************************************
 Function:     gamma
 
 Description:  Distance between two labels
 Inputs:       StartTimeSequence &y
 StartTimeSequence &y_hat
 Output:       double - the resulted distance
 Comments:     none.
 ***********************************************************************/
double Classifier::gamma(const int y, const int y_hat)
{
  if (loss_type == "tau_insensitive_loss")  {
    
    double loss_i = fabs( double(y_hat) - double(y) ) - GAMMA_EPSILON;
    if (loss_i > 0.0) 
      return loss_i;
    
  }
  else if (loss_type == "alignment_loss")  {
    
    double loss_i = ( fabs( double(y_hat) - double(y) ) > GAMMA_EPSILON ) ? 1.0 : 0.0 ;
    return loss_i;
    
  }
  else {
    std::cerr << "Error: loss type \"" << loss_type << "\" is undefined." << std::endl;
    exit(-1);
  }
  
  return 0.0;
}


/************************************************************************
 Function:     gaussian
 
 Description:  Gaussian PDF
 Inputs:       double x, double mean, double std
 Output:       double.
 Comments:     none.
 ***********************************************************************/
double Classifier::gaussian(const double x, const double mean, const double std)
{
  double d = (1/sqrt(2*3.141529)/std * exp(-((x-mean)*(x-mean)) / (2*std*std) ));
  //if (d < 2.0E-22) 
  //	d = 2.0E-22;
  return (d);
}



/************************************************************************
 Function:     Classifier::load
 
 Description:  Loads a classifier 
 Inputs:       string & filename
 Output:       none.
 Comments:     none.
 ***********************************************************************/
void Classifier::load(std::string &filename)
{
  std::ifstream ifs;
  ifs.open(filename.c_str());
  if ( !ifs.good() ) {
    std::cerr << "Unable to open " << filename << std::endl;
    exit(-1);
  }
  
  ifs >> w;
  
  ifs.close();
}


/************************************************************************
 Function:     Classifier::save
 
 Description:  Saves a classifier 
 Inputs:       string & filename
 Output:       none.
 Comments:     none.
 ***********************************************************************/
void Classifier::save(std::string &filename)
{
  std::ofstream ifs;
  ifs.open(filename.c_str());
  if ( !ifs.good() ) {
    std::cerr << "Unable to open " << filename << std::endl;
    exit(-1);
  }
  
  ifs << w;
  
  ifs.close();
}



// --------------------- EOF ------------------------------------//
