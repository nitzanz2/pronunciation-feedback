/************************************************************************
 Project:  Phoneme sequence classification
 Module:   Classifier
 Purpose:  Alignment discriminative algorithm
 Date:     10 Apr., 2004
 Programmer: Joseph Keshet
 
 
 **************************** INCLUDE FILES *****************************/
#include "Classifier.h"
#include <iostream>
#include <fstream>
#include <vector>

using std::vector;
using std::cout;
using std::cerr;
using std::endl;

#define _max(x,y) ( (x)>(y) ? (x) : (y) )
#define _min(x,y) ( (x)<(y) ? (x) : (y) )


/************************************************************************
 Function:     Classifier::Classifier
 
 Description:  Constructor
 Inputs:       none.
 Output:       none.
 Comments:     none.
 ***********************************************************************/
Classifier::Classifier(double _BB, double _CC, int _num_phonemes, 
                       int _features_dim): 
BB(_BB), CC(_CC), 
W(_num_phonemes,_features_dim), W_averaging(_num_phonemes,_features_dim)
{
  ranks.resize(_num_phonemes);
  averaging_enabled = false;
  W.zeros();
  num_updates = 0;
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
 Function:     Classifier::predict
 
 Description:  Predict label of instance x
 Inputs:       InstanceType &x 
 LabelType &y_hat
 Output:       void
 Comments:     none.
 ***********************************************************************/
infra::vector_view Classifier::predict(infra::vector_view x, int &y_hat)
{
  if (averaging_enabled)
    ranks = W_averaging*x;
  else
    ranks = W*x;
  
  //cout << "ranks=" << ranks << endl;
  
  y_hat =  ranks.argmax();
  return ranks;
}


/************************************************************************
 Function:     Classifier::update
 
 Description:  Train classifier with one example 
 Inputs:       infra::vector& x - example instance 
 int y - label 
 Output:       double - squared loss
 Comments:     Assume the use of predict() before the call to this 
 function, so the ranks vector is up-to-date.
 ***********************************************************************/
double Classifier::update( infra::vector_view x, int y, int y_hat)
{
  double loss = 0.0;
  num_updates++;
  
	//cout << "y=" << y << " y_hat=" << y_hat ;
  if (y_hat == y) {
    double tmp = ranks(y);
    ranks(y) = -99999999.9;
    y_hat = ranks.argmax();
    ranks(y) = tmp;
  }
  
  double margin = ranks(y) - ranks(y_hat);
  if (margin <= BB) {
    loss = 1.0 - margin;
    double tau = _min(CC , loss / (2.0*x*x) );
    //cout << " tau=" << tau << endl ;
    W.row(y) += tau*x;
    W.row(y_hat) -= tau*x;
  }
	//cout << endl;
  //W_averaging = (W_averaging*(num_updates-1.0) + W)/double(num_updates);
  W_averaging *= (num_updates-1.0);
  W_averaging += W;
  W_averaging /= double(num_updates);
  
  return loss;
}


/************************************************************************
 Function:     Classifier::update_crf
 
 Description:  Train classifier with one example 
 Inputs:       infra::vector& x - example instance 
               int y - label 
 Output:       double - squared loss
 Comments:     Assume the use of predict() before the call to this 
 function, so the ranks vector is up-to-date.
 ***********************************************************************/
double Classifier::update_crf( infra::vector_view x, int y, int y_hat)
{
  num_updates++;
  
  infra::vector wx(LabelType::num_phonemes);
	wx = W*x;

  double Z = 0.0;
	for (int p=0; p < LabelType::num_phonemes; p++)
    Z += exp(wx(p));
	double loss = -(wx(y) - log(Z));
	infra::vector delta_phi(LabelType::num_phonemes);
	for (int p=0; p < LabelType::num_phonemes; p++) {
		if (p == y) 
			delta_phi(p) = 1.0 - exp(wx(p))/Z;
		else 
			delta_phi(p) = -exp(wx(p))/Z;
	}	
	double tau = loss/delta_phi.norm2();
//	std::cout  	<< " Pr(y|x)=" << exp(W.row(y)*x - log(Z))
//	<< " Pr(y_hat|x)=" << exp(W.row(y_hat)*x - log(Z))
//	<< " Z=" << Z
//	<< " loss=" << loss
//	<< " tau=" << tau
//	<< std::endl;
	for (int p=0; p < LabelType::num_phonemes; p++) 
			W.row(p) -= (tau*exp(wx(p))/Z)*x;
	W.row(y) += tau*x;
//	std::cout << std::endl;
  //W_averaging = (W_averaging*(num_updates-1.0) + W)/double(num_updates);
  W_averaging *= (num_updates-1.0);
  W_averaging += W;
  W_averaging /= double(num_updates);
  
  return loss;
}

/************************************************************************
 Function:     Classifier::averaging
 
 Description:  Prepare alpha for averaging
 Inputs:       none.
 Output:       none.
 Comments:     none.
 ***********************************************************************/
void Classifier::averaging(void)
{
  averaging_enabled = true;
}

#if 0
/************************************************************************
 Function:     Classifier::load
 
 Description:  Loads a classifier 
 Inputs:       string & filename
 Output:       none.
 Comments:     none.
 ***********************************************************************/
void Classifier::load(std::string &filename)
{
  FILE *pfile;
  pfile = fopen(filename.c_str(), "rb");
  if (pfile == NULL) {
    cerr << "Error: Unable to open " << filename << " for reading. Aborting..." << endl;
    exit(-1);
  }
  W.load_binary(pfile);
  W_averaging.load_binary(pfile);
  fclose(pfile);  
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
  FILE *pfile;
  pfile = fopen(filename.c_str(), "wb");
  if (pfile == NULL) {
    cerr << "Error: Unable to open " << filename << " for writing. Aborting..." << endl;
    exit(-1);
  }
  W.save_binary(pfile);
  W_averaging.save_binary(pfile);
  fclose(pfile);
  
}
#else

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
  

  ifs >> W;
  ifs >> W_averaging;
  
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
  
  ifs << W;
  ifs << W_averaging;

  ifs.close();
}

#endif

// --------------------- EOF ------------------------------------//

