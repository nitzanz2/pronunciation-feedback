#ifndef _CLASSIFIER_H
#define _CLASSIFIER_H

/************************************************************************
 Project:  Phoneme Alignment
 Module:   Classifier
 Purpose:  Alignment discriminative algorithm
 Date:     Nov. 24, 2004
 Programmer: Joseph Keshet
 
 *************************** INCLUDE FILES ******************************/
#include <infra.h>
#include "Dataset.h"


class Classifier
{
public:
	Classifier(unsigned int _frame_rate, double _min_phoneme_length, 
						 double _max_phoneme_length, double _PA1_C,
						 double _beta1, double _beta2, double _beta3, 
						 double _min_sqrt_gamma, std::string _loss_type);
	~Classifier();
	void load(std::string &filename);
	void save(std::string &filename);
	void load_phoneme_stats(std::string &filename);
	bool was_changed() { return (w_changed); }
	double update(SpeechUtterance& x, StartTimeSequence &y, 
								StartTimeSequence &y_hat);
	double update_direct_loss(SpeechUtterance& x, StartTimeSequence &y_hat_eps, 
														StartTimeSequence &y_hat, StartTimeSequence &y,
														double epsilon) ;
	double predict(SpeechUtterance& x, StartTimeSequence &y_hat);
	double predict_epsilon(SpeechUtterance& x, StartTimeSequence &y_hat, 
												 StartTimeSequence &y, double epsilon);
  double align_keyword(SpeechUtterance& x, StartTimeSequence &y_hat_best, 
                       int &end_frame_best);
	infra::vector_view phi(SpeechUtterance& x, StartTimeSequence& y);
	infra::vector_view phi_1(SpeechUtterance& x, int i, int t, int l);
	double phi_2(SpeechUtterance& x, int i, int t, int l1, int l2);
	static double gamma(const StartTimeSequence &y, const StartTimeSequence &y_hat);
	static double gamma(const int y, const int y_hat);
	static double gaussian(const double x, const double mean, const double std);
	double aligned_phoneme_scores(SpeechUtterance& x, StartTimeSequence &y);

    // New methods:
	double confidence_general(SpeechUtterance& x, StartTimeSequence& y_hat);
	infra::matrix confidence_per_phoneme(SpeechUtterance& x, StartTimeSequence& y);
	infra::vector_view phi_1_per_phoneme(SpeechUtterance& x, int phoneme, int t, int l);
	double phi_2_per_phoneme(SpeechUtterance& x, int phoneme1, int phoneme2, int t, int l1, int l2);
	infra::matrix normalizeConfidences(infra::matrix confidences);

public:
	static std::string loss_type;

protected:
	infra::vector phoneme_length_mean;
	infra::vector phoneme_length_std;
	static int phi_size;
	unsigned int frame_rate;
	int min_num_frames;
	int max_num_frames;
	infra::vector w;
	infra::vector w_old;
	bool w_changed;
	double PA1_C;
	double beta1;
	double beta2;
	double beta3;
	double min_sqrt_gamma;
};

#endif // _CLASSIFIER_H
