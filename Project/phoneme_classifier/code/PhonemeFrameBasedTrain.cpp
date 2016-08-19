/************************************************************************
 Project:  Phoeneme Frame-Based Classification (primal with poly2)
 Module:   PhonemeFrmaeBasedTrain
 Purpose:  Main entry point for training
 Date:     17 June, 2009
 Programmer: Joseph Keshet
 
 **************************** INCLUDE FILES *****************************/
#include <iostream>
#include <fstream>
#include <map>
#include <sys/time.h>
#include <sys/resource.h>
#include <cmdline/cmd_line.h>
#include "Classifier.h"
#include "Dataset.h"
#include "KernelExpansion.h"
#include "ConcateFeatures.h"

#define _min(x,y) ( (x)<(y) ? (x) : (y) )
#define MAX_FRAMES_IN_UTTERANCE 12000

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
  string input_filelist;
  string target_filelist;
  string random_shuffle_filename;
  string phonemes_filename;
  string classifier_filename;
  string mfcc_stats_filename;
  string silence_symbol;
  string classifier_filename_load;
  double B;
  double C;
  int num_epochs;
  int max_num_frames; 
  string kernel_expansion_name;
  double sigma = 1.0;
  int num_concate_frames;
  string map_output_filename;
	bool is_crf_update;
	int max_num_examples;
  
  learning::cmd_line cmdline;
  cmdline.info("Phoeneme Frame-Based Classifier Training (Primal with Poly2 kernel)");
  cmdline.add("-C", "C parameter [1.0]", &C, 1.0);
  cmdline.add("-B", "B parameter [0.5]", &B, 0.5);
  cmdline.add("-mfcc_stats", "feature statstics filename", &mfcc_stats_filename, "");
  cmdline.add("-silence", "silence phoneme symbol []", &silence_symbol, "");
  cmdline.add("-epochs", "num. of epochs for Hildreth's algorithm [1]", &num_epochs, 1);
  cmdline.add("-max_num_frames", "max number of frames for training [1e8]", &max_num_frames, 1e8);
  cmdline.add("-kernel_expansion", "use kernel expansion of type 'poly2' or 'rbf2'", 
              &kernel_expansion_name, "");    
  cmdline.add("-sigma", "if kernel is rbf2 or rbf3 this is the sigma", &sigma, 1.0);
	cmdline.add("-n", "num frames to concate (odd number)", &num_concate_frames, 1);
  cmdline.add("-num_examples", "num. examples to load. Value of 0 loads all of them (the default)", &max_num_examples, 0);
	cmdline.add("-omap", "outut the phoneme mapping to integers", &map_output_filename, "");
	cmdline.add("-crf_update", "CRFs instead of large-margin", &is_crf_update, false);
  cmdline.add("-load", "load classifier as initial set-up", &classifier_filename_load, "");
  cmdline.add_master_option("input-filelist", &input_filelist);
  cmdline.add_master_option("target-filelist", &target_filelist);
  cmdline.add_master_option("random_shuffle_filename", &random_shuffle_filename);
  cmdline.add_master_option("phonemes", &phonemes_filename);
  cmdline.add_master_option("classifier", &classifier_filename);
  int rc = cmdline.parse(argc, argv);
  if (rc < 5) {
    cmdline.print_help();
    return EXIT_FAILURE;
  }
  
  // Set the silence symbol
  LabelType::set_silence_symbol(silence_symbol);
  
  // Load set of phonemes
  LabelType::load_phoneme_map(phonemes_filename, map_output_filename);
  
  // load mfcc statistics (mean and std)
  InstanceType::load_mfcc_stats(mfcc_stats_filename);
  
  Dataset training_dataset(input_filelist, target_filelist);
  ConcateFeatures concate_features(InstanceType::mfcc_dim, num_concate_frames);
  KernelExpansion kernel_expansion(kernel_expansion_name, InstanceType::mfcc_dim, sigma);
  cout << "Info: concate " << num_concate_frames << " frames. Each frame has " 
  << kernel_expansion.features_dim() << " features. " << endl
  << "      Overall " << num_concate_frames*kernel_expansion.features_dim() 
  << " features per example." << endl;
    
  cout << "Loading random shuffle data... " << flush;
  ifstream random_shuffle_ifs;
  random_shuffle_ifs.open(random_shuffle_filename.c_str());
  if (!random_shuffle_ifs.good()) {
    cerr << "Unable to open " << random_shuffle_filename << "for reading." << endl;
    return EXIT_FAILURE;
  }
  uint total_num_frames = 0;
  uint **frame_numbers;
  uint **frame_indices;
  frame_numbers = new uint*[training_dataset.size()];
  frame_indices = new uint*[training_dataset.size()];
  for (int i=0; i < training_dataset.size(); i++) {
    frame_numbers[i] = new uint[MAX_FRAMES_IN_UTTERANCE];
    frame_indices[i] = new uint[MAX_FRAMES_IN_UTTERANCE];
  }
  //  // mapping between file and loop index to the real frame numner in the file
  //  uint frame_numbers[training_dataset.size()][MAX_FRAMES_IN_UTTERANCE];
  //  // mapping between file and loop index to the absolute frame index
  //  uint frame_indices[training_dataset.size()][MAX_FRAMES_IN_UTTERANCE];
  
  uint running_index = 0;
  uint prev_file_number = 999;
  while (!random_shuffle_ifs.eof()) {
    string frame_index_str;
    random_shuffle_ifs >> frame_index_str;
    if (frame_index_str == "") continue;
    uint frame_index = std::strtol(frame_index_str.c_str(), NULL, 10);
    string file_number_str;
    random_shuffle_ifs >> file_number_str;
    if (file_number_str == "") continue;
    uint file_number = std::strtol(file_number_str.c_str(), NULL, 10);
    string frame_number_str;
    random_shuffle_ifs >> frame_number_str;
    if (frame_number_str == "") continue;
    uint frame_number = std::strtol(frame_number_str.c_str(), NULL, 10);
    if (prev_file_number != file_number) {
      running_index = 0;
      prev_file_number = file_number;
    }
    else {
      running_index++;
    }
    frame_numbers[file_number][running_index] = frame_number;
    frame_indices[file_number][running_index] = frame_index;
    if (frame_index > total_num_frames) total_num_frames = frame_index;
  }
  cout << "Dataset containing " << total_num_frames << " frames." << endl;
  
  // Initiate classifier  
  Classifier classifier(B, C, LabelType::num_phonemes, 
                        num_concate_frames*kernel_expansion.features_dim()); 
  
  if (classifier_filename_load != "") {
    cout << "Info: loading classifier from disk... " << flush;
    classifier.load(classifier_filename_load);
    cout << "Done." << endl;
  }
    
  // initiate timer - used to calculate CPU cycles
  struct rusage before, after;
  getrusage(RUSAGE_SELF, &before);
  
  uint cumulative_error = 0;
  uint cumulative_num_frames = 0;
  double cumulative_loss = 0;
  
  // Run several times over the dataset
  for (int epoch=0; epoch < num_epochs; epoch++) {
    
    training_dataset.initiate();
    
    // Run over all dataset
    for (int i=0; i < _min(max_num_examples, training_dataset.size()); i++) {
      // load instance and label
      InstanceType X;
      LabelType Y;
      training_dataset.read(X, Y);
      kernel_expansion.expand_inplace(X);
      if (num_concate_frames > 1)
        concate_features.concate_inplace(X);
      int num_frames = X.num_frames;
      
      // Run over all frames in the utterance
      for (int j=0; j < num_frames; j++) {
        // load instance and label
        int k = frame_numbers[i][j];
        infra::vector_view x = X.mfcc.row(k);
        int y = Y[k];
        int y_hat;
        // don't count unwanted frames in the learning process
        if (y < 0) continue;
        
        // predict label 
        classifier.predict(x, y_hat);    
        if (y != y_hat) cumulative_error++;
        cumulative_num_frames++;
        double loss;
				if (is_crf_update)
					loss = classifier.update_crf(x, y, y_hat);
				else
					loss = classifier.update(x, y, y_hat);
        cumulative_loss += loss;
        
      }
      cout << "Info: epoch= " << epoch 
      << " file= " << i 
      << " cum_error= " << cumulative_error/double(cumulative_num_frames)
      << " cum_loss= " << cumulative_loss/double(cumulative_num_frames) << endl;
    }
    
    
    // print info
    cout << "Epoch " << epoch << ": " 
    << " com_error= " << cumulative_error/double(cumulative_num_frames)
    << " cum_loss= " << cumulative_loss/double(cumulative_num_frames) << endl;
    // save classifier 
    std::ostringstream oss;
    oss << classifier_filename << ".epoch_" << epoch;
    std::string classifier_filename_epoch = oss.str();
    cout <<  "Info: saving classifier " << classifier_filename_epoch << "..." << flush;
    classifier.save(classifier_filename_epoch);
    cout << "Done." << endl;  
  }
  
  // stop the timer
  getrusage(RUSAGE_SELF, &after);
  long msecs_user = ((after.ru_utime.tv_sec - before.ru_utime.tv_sec) * 1000 +
                     (after.ru_utime.tv_usec - before.ru_utime.tv_usec) / 1000);
  long msecs_system = ((after.ru_stime.tv_sec - before.ru_stime.tv_sec) * 1000 +
                       (after.ru_stime.tv_usec -  before.ru_stime.tv_usec) / 1000);
  std::cout << "CPU: utilization " << (msecs_system + msecs_user) 
  << " msec (user: " << msecs_user << " msec, system: " << msecs_system << " msec)."
  << std::endl;
  
  // save classifier 
  cout <<  "Info: saving classifier " << classifier_filename << "..." << flush;
  classifier.save(classifier_filename);
  cout << "Done." << endl;  
  
  std::cout << "Final:" << " cum_error= " << cumulative_error/double(cumulative_num_frames)
  << " loss= " << cumulative_loss/double(cumulative_num_frames) 
  << " cpu_user= " << msecs_user  << endl;
  
  for (int i=0; i < training_dataset.size(); i++) {
    delete [] frame_numbers[i];
    delete [] frame_indices[i];
  }
  delete [] frame_numbers;
  delete [] frame_indices;
  
  return EXIT_SUCCESS;
}



// ------------------------------- EOF -----------------------------//
