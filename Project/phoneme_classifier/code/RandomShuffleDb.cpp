/************************************************************************
 Project:  Phoeneme Frame-Based Classification using Hildreth
 Module:   RandomShuffleDb
 Purpose:  Random shffle of the frames within each utterance
 Date:     28 Jan., 2009
 Programmer: Joseph Keshet
 
 Function List:
 main - Main entry point
 
 **************************** INCLUDE FILES *****************************/
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include <cmdline/cmd_line.h>
#include "Dataset.h"
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
  string random_shuffle_filename;
  bool no_shuffle = false;
  int num_concate_frames;
  
  learning::cmd_line cmdline;
  cmdline.info("Random shffle of the frames within each utterance for online training");
  cmdline.add("-no_shuffle", "no random permutation", &no_shuffle, false);
	cmdline.add("-n", "num frames to concate (odd number)", &num_concate_frames, 1);
  cmdline.add_master_option("input-filelist", &input_filelist);
  cmdline.add_master_option("target-filelist", &target_filelist);
  cmdline.add_master_option("random_shuffle_filename", &random_shuffle_filename);
  int rc = cmdline.parse(argc, argv);
  if (rc < 3) {
    cmdline.print_help();
    return EXIT_FAILURE;
  }
  
  unsigned long frame_index = 0;
  ofstream random_shuffle_ofs(random_shuffle_filename.c_str());
  
  Dataset training_dataset(input_filelist, target_filelist);
  ConcateFeatures concate_features(InstanceType::mfcc_dim, num_concate_frames);
	for (int i=0; i < training_dataset.size(); i++) {
    
    // read the next utterance file
    InstanceType X;
    LabelType Y;
    training_dataset.read(X, Y);
		if (num_concate_frames > 1)
      concate_features.concate_inplace(X);
		
    // random shuffle training set
    int frame_numbers[X.num_frames];
    for (int j=0; j < X.num_frames; j++)
      frame_numbers[j] = j;
    srand(time(NULL));
    if (!no_shuffle) 
      std::random_shuffle(frame_numbers, frame_numbers+X.num_frames);

    // print out
    for (int j=0; j < X.num_frames; j++) {
      random_shuffle_ofs << frame_index << " " << i << " " << frame_numbers[j] << endl;
      frame_index++;
    }

  }

  random_shuffle_ofs.close();
  
  return EXIT_SUCCESS;
}



// ------------------------------- EOF -----------------------------//
