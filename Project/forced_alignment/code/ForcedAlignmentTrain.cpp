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
 Project:  Forced Alignment
 Module:   Main entry point
 Purpose:  Train the phonetic segmentation system
 Date:     24 Jan., 2005
 Programmer: Joseph Keshet
 
 Function List:
 main - Main entry point
 
 **************************** INCLUDE FILES *****************************/
#include <iostream>
#include <fstream>
#include <map>
#include <cmdline/cmd_line.h>
#include <string.h>
#include "Classifier.h"
#include "Dataset.h"

using namespace std;

/************************************************************************
 Function:     main
 
 Description:  Main entry point
 Inputs:       int argc, char *argv[] - main input params
 Output:       int - always 0.
 Comments:     none.
 ***********************************************************************/
int main(int argc, char **argv) 
{
  // Parse command line
  string val_scores_filelist;
  string val_dists_filelist;
  string val_phonemes_filelist;
  string val_start_times_filelist;
  unsigned int frame_rate;
  unsigned int num_epochs;
  double min_phoneme_length, max_phoneme_length;
  string silence_symbol;
  bool remove_silence;
  double C;
  double beta1, beta2, beta3;
  double min_sqrt_gamma;
  string scores_filelist;
  string dists_filelist;
  string phonemes_filelist;
  string start_times_filelist;
  string phonemes_filename;
  string phoneme_stats_filename;
  string classifier_filename;
  double epsilon; 
	string loss_type;
  
  learning::cmd_line cmdline;
  cmdline.info("Forced Alignemnt based on Passive-Aggressive");
  cmdline.add("-val_scores_filelist", "validation scores file list", &val_scores_filelist, "");
  cmdline.add("-val_dists_filelist", "validation dists file list",&val_dists_filelist, "");
  cmdline.add("-val_phonemes_filelist", "validation phonemes file list",&val_phonemes_filelist, "");  
  cmdline.add("-val_start_times_filelist", "validation start-times file list", &val_start_times_filelist, "");
  cmdline.add("-epochs", "number of epochs [1]", &num_epochs, 1);
  cmdline.add("-frame_rate", "frame rate (shift) in msec [10]",&frame_rate,10);
  cmdline.add("-min_phoneme_length", "min. phoneme duration in msec [20]", &min_phoneme_length, 20);
  cmdline.add("-max_phoneme_length", "max. phoneme duration in msec [330]", &max_phoneme_length, 330);
  cmdline.add("-silence_symbol", "silence symbol [sil]", &silence_symbol, "sil");
  cmdline.add("-remove_silence", "remove pre/post silence from data", &remove_silence, false);  
  cmdline.add("-PA1_C", "C parameter for PA-I", &C, 5.0);
  cmdline.add("-beta1", "weight of the distance feature", &beta1, 1.0);
  cmdline.add("-beta2", "weight of the duration feature", &beta2, 1.0);
  cmdline.add("-beta3", "weight of the speaking rate feature", &beta3, 1.0);	
  cmdline.add("-min_gamma", "the minimal value of sqrt(gamma) before an update", &min_sqrt_gamma, 1.0);
  cmdline.add("-eps", "epsilon parameter of algorithm", &epsilon, 0.0);
	cmdline.add("-loss", "shoud be alignment_loss or tau_insensitive_loss (default)", 
							&loss_type, "tau_insensitive_loss");
  cmdline.add_master_option("scores_filelist", &scores_filelist);
  cmdline.add_master_option("dists_filelist", &dists_filelist);
  cmdline.add_master_option("phonemes_filelist", &phonemes_filelist);  
  cmdline.add_master_option("start_times_filelist", &start_times_filelist);
  cmdline.add_master_option("phonemes", &phonemes_filename);	
  cmdline.add_master_option("phoneme-stats", &phoneme_stats_filename);
  cmdline.add_master_option("classifier", &classifier_filename);
  int rc = cmdline.parse(argc, argv);
  if (rc < 7) {
    cmdline.print_help();
    return EXIT_FAILURE;
  }
  
  // phoneme symbol to number mapping (Lee and Hon, 89)
  PhonemeSequence::load_phoneme_map(phonemes_filename, silence_symbol);
  
  // Initiate classifier
  Classifier classifier(frame_rate, min_phoneme_length, max_phoneme_length, C, 
												beta1, beta2, beta3, min_sqrt_gamma,loss_type);
  classifier.load_phoneme_stats(phoneme_stats_filename);
  
  double loss;
  double cum_loss = 0.0;
  double best_validation_loss = 1e100;
  
  for (uint epoch = 0; epoch < num_epochs; epoch++) {
    
    // begining of the training set
    Dataset training_dataset(scores_filelist, dists_filelist, phonemes_filelist, start_times_filelist);
    
    double max_loss_in_epoch = 0.0; // maximal loss value in this epoch
    double avg_loss_in_epoch = 0.0; // training loss value in this epoch
    
    // Run over all dataset
    for (uint i=0; i <  training_dataset.size(); i++) {
      
      SpeechUtterance x;
      StartTimeSequence y;
      StartTimeSequence y_hat;
      StartTimeSequence y_hat_eps;
      
      
      cout << "==================================================================================" << endl;
      
      // read next example for dataset
      training_dataset.read(x, y, remove_silence);
      y_hat.resize(y.size());
      y_hat_eps.resize(y.size());
      
      // predict label 
      classifier.predict(x, y_hat);

      cout << "phonemes=" << x.phonemes << endl;
      cout << "alignment= " << y << endl;
      cout << "predicted= " << y_hat << endl;

			if (epsilon > 0.0) {
        classifier.predict_epsilon(x, y_hat_eps, y, epsilon);
				cout << "eps-predicted= " << y_hat_eps << endl;
			}

      // suffer loss and update
      if (epsilon > 0.0) {
        loss = classifier.update_direct_loss(x, y_hat_eps, y_hat, y, epsilon);
			}
      else {
        loss = classifier.update(x, y, y_hat);
			}
      cum_loss += loss;
      
      if (max_loss_in_epoch < loss) max_loss_in_epoch = loss;
      avg_loss_in_epoch += loss;
      
      // now, check the validations error
      if ( val_scores_filelist != "" && classifier.was_changed() ) {
        cout << "Validation...\n";
        Dataset val_dataset(val_scores_filelist, val_dists_filelist, val_phonemes_filelist, val_start_times_filelist);
        double this_w_loss = 0.0;
        for (uint ii=0; ii < val_dataset.size(); ++ii) {
          SpeechUtterance xx;
          StartTimeSequence yy;
          StartTimeSequence yy_hat;
          val_dataset.read(xx, yy, remove_silence, false);
          yy_hat.resize(yy.size());
          classifier.predict(xx, yy_hat);
          double this_loss = 0;
          for (unsigned int jj=0; jj < yy.size(); ++jj) {
            if (yy[jj] > yy_hat[jj]) this_loss += yy[jj] - yy_hat[jj];
            else this_loss += yy_hat[jj] - yy[jj];
          }
          this_loss /= yy.size();
          this_w_loss += this_loss;
        }
        this_w_loss /= val_dataset.size();
        if (this_w_loss < best_validation_loss) {
          best_validation_loss = this_w_loss;
          classifier.save(classifier_filename);
        }
        cout << "i = " << i << ", this validation error = " << this_w_loss 
        << ", best validation loss  = " << best_validation_loss << endl;
        
        // stopping criterion for iterate until convergence
	//        if (best_validation_loss < 1.0)
	//          break;
      }
      
    } // end running over the dataset
    
    avg_loss_in_epoch /=  training_dataset.size();
    
    cout << " average normalized loss = " << avg_loss_in_epoch 
    << " best validation loss  = " << best_validation_loss << endl;
  }
  if (val_scores_filelist == "")
    classifier.save(classifier_filename);
  
  cout << "Done." << endl;  
  
  return EXIT_SUCCESS;
  
}

// ------------------------------- EOF -----------------------------//
