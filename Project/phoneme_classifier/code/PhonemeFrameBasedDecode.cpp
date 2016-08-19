/************************************************************************
 Project:  Phoeneme Frame-Based Classification
 Module:   PhonemeClassify
 Purpose:  Run phoneme classification as a sequences
 Date:     17 June, 2009
 Programmer: Joseph Keshet
 
 **************************** INCLUDE FILES *****************************/
#include <iostream>
#include <fstream>
#include <map>
#include <cmdline/cmd_line.h>
#include "Classifier.h"
#include "Dataset.h"
#include "KernelExpansion.h"
#include "ConcateFeatures.h"

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
  string scores_filelist;
  string phonemes_filename;
  string classifier_filename;
  string mfcc_stats_filename;
  string silence_symbol;
  bool o2b_enabled;
  string kernel_expansion_name;
  double sigma = 1.0;
  string equiv_map_filename;
  int num_concate_frames;
  
  learning::cmd_line cmdline;
  cmdline.info("Phoneme Recognition Sequence Decoder");
  cmdline.add("-mfcc_stats", "feature statstics filename", &mfcc_stats_filename, "");
  cmdline.add("-silence", "silence phoneme symbol [sil]", &silence_symbol, "sil");
  cmdline.add("-averaging", "online-to-batch convergence using averaging", &o2b_enabled, false);  
  cmdline.add("-scores", "output classifier scores filelist", &scores_filelist, "");
  cmdline.add("-kernel_expansion", "use kernel expansion of type 'poly2' or 'rbf2'", 
              &kernel_expansion_name, "");    
  cmdline.add("-sigma", "if kernel is rbf2 or rbf3 this is the sigma", &sigma, 1.0);
  cmdline.add("-equiv_map", "Equivalence map file (similar to HResults -e)", 
              &equiv_map_filename, "");
	cmdline.add("-n", "num frames to concate (odd number)", &num_concate_frames, 1);
  cmdline.add_master_option("input-filelist", &input_filelist);
  cmdline.add_master_option("target-filelist [null]", &target_filelist);
  cmdline.add_master_option("phonemes", &phonemes_filename);
  cmdline.add_master_option("classifier", &classifier_filename);
  int rc = cmdline.parse(argc, argv);
  if (rc < 4) {
    cmdline.print_help();
    return EXIT_FAILURE;
  }
  
  
  // Set the silence symbol
  LabelType::set_silence_symbol(silence_symbol);
  
  // Load set of phonemes
  string empty_string = "";
  LabelType::load_phoneme_map(phonemes_filename, empty_string);
  
  // load mfcc statistics (mean and std)
  InstanceType::load_mfcc_stats(mfcc_stats_filename);

  // Load equivalence map
  if (equiv_map_filename != "")
    LabelType::load_equivalence_map(equiv_map_filename);
  
  Dataset test_dataset(input_filelist, target_filelist);
  ConcateFeatures concate_features(InstanceType::mfcc_dim, num_concate_frames);
  KernelExpansion kernel_expansion(kernel_expansion_name, InstanceType::mfcc_dim, sigma);
    
  // Initiate classifier  
  Classifier classifier(0, 1.0, LabelType::num_phonemes, 
												num_concate_frames*kernel_expansion.features_dim()); 
  cout << "Info: loading classifier from disk... " << flush;
  classifier.load(classifier_filename);
  if (o2b_enabled) classifier.averaging();
  cout << "Done." << endl;
  
  // loading confidences file list
  StringVector scores_files;
  if (scores_filelist != "")
    scores_files.read(scores_filelist);
  
  // begining of the test set
  test_dataset.initiate();
  
  uint total_frame_error = 0;
  uint total_num_frames = 0;
  // Run over all dataset
  for (int l=0; l <  test_dataset.size(); l++) {
    
    InstanceType x;
    LabelType y;
    LabelType y_hat;
    infra::matrix scores;
    
    // read next example for dataset
    cout << "Loading " << test_dataset.filename() << " " << flush;
    test_dataset.read(x, y);
    kernel_expansion.expand_inplace(x);
		if (num_concate_frames > 1)
      concate_features.concate_inplace(x);
    y_hat.resize(x.num_frames);
    scores.resize(x.num_frames, LabelType::num_phonemes);
    
    uint this_file_error = 0;
    for (int i=0; i < x.num_frames; i++) {
      
      // read next instance vector
      infra::vector instance(x.mfcc.row(i));
      //infra::vector instance = kernel_expansion.expand(x.mfcc.row(i));
			
      // predict label 
      scores.row(i) = classifier.predict(instance, y_hat[i]);
      
      // computer error
      if (test_dataset.labels_given()) {
				if (equiv_map_filename != "") {
					if (!LabelType::phonemes_equivalent(y[i],y_hat[i]) ) {
						this_file_error++;
						total_frame_error++;
					}
				}
				else {
					if (y[i] != y_hat[i]) {
						this_file_error++;
						total_frame_error++;
					}
				}
			}
		}
		total_num_frames += x.num_frames;
		
		// output error
		if (test_dataset.labels_given())
			cout << "Info: frame_error= " << this_file_error/double(x.num_frames) << endl;
		else
			cout << endl;
		
		// write out to a prediction file
		if (scores_filelist != "" && scores_files.size() > 0) {
			ofstream ofs(scores_files[l].c_str());
			if (ofs.good()) ofs << scores;
			ofs.close();
		}
	}
	
	// output error
	if (test_dataset.labels_given())
		cout << "Info: total_frame_error= " << total_frame_error/double(total_num_frames) << endl;
	else
		cout << endl;
	
	cout << "Done." << endl;  
	
	return EXIT_SUCCESS;
}



// ------------------------------- EOF -----------------------------//
