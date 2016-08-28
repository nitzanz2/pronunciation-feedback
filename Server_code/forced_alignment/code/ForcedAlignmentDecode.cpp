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
 Purpose:  Decode using the phonetic segmentation system
 Date:     24 Jan., 2005
 Programmer: Joseph Keshet
 
 Function List:
 main - Main entry point
 
 **************************** INCLUDE FILES *****************************/
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <cmdline/cmd_line.h>
#include "Classifier.h"
#include "Dataset.h"

using namespace std;

#define NUM_CUM_LOSS_RESOLUTIONS 6

int countDigit(double num) {
    string s;
    stringstream out;
    out << num;
    s = out.str();
    return int(s.length());
}

map<string, set<string> > loadSimilarPhonemes_map() {
    typedef map<string, set<string> >::iterator map_it_type;
    typedef set<string>::iterator set_it_type;
    map<string, set<string> > similar_phonemes_map;
    ifstream similar_phonemes_ifs("similar_phonemes_map.txt");
    if (!similar_phonemes_ifs.good()) {
        cerr << "Error: Unable to open phonemes file similar_phonemes_map.txt" << endl;
        return similar_phonemes_map;
    }
    string line;
    while (getline(similar_phonemes_ifs, line)) {
        istringstream iss(line);
        set<string> similar_set;
        do {
            string sub;
            iss >> sub;
            if (!sub.empty()) { similar_set.insert(sub);}
        } while (iss);
        for(set_it_type set_it = similar_set.begin(); set_it != similar_set.end(); set_it++) {
            similar_phonemes_map[(*set_it)] =  similar_set;
        }
    }
    similar_phonemes_ifs.close();
    return similar_phonemes_map;
}
bool areSimilarPhonemes(map<string, set<string> >& similar_phonemes_map, string phone1, string phone2) {
    return (similar_phonemes_map[phone1].find(phone2) != similar_phonemes_map[phone1].end());
}

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
  unsigned int frame_rate;
  double min_phoneme_length, max_phoneme_length;
  string silence_symbol;
  bool remove_silence;
  double C = 0.0;
  double beta1, beta2, beta3;
  string scores_filelist;
  string dists_filelist;
  string phonemes_filelist;
  string start_times_filelist;
  string phonemes_filename;
  string phoneme_stats_filename;
  string classifier_filename;
  string output_align;
  string output_confidence;
	string output_textgrid;
	string loss_type;
  
  learning::cmd_line cmdline;
  cmdline.info("Forced Alignment based on Passive-Aggressive");
  cmdline.add("-frame_rate", "frame rate (shift) in msec [10]",&frame_rate,10);
  cmdline.add("-min_phoneme_length", "min. phoneme duration in msec [20]", &min_phoneme_length, 20);
  cmdline.add("-max_phoneme_length", "max. phoneme duration in msec [330]", &max_phoneme_length, 330);
  cmdline.add("-silence_symbol", "silence symbol [sil]", &silence_symbol, "sil");
  cmdline.add("-remove_silence", "remove pre/post silence from data", &remove_silence, false);  
  cmdline.add("-beta1", "weight of the distance feature", &beta1, 1.0);
  cmdline.add("-beta2", "weight of the duration feature", &beta2, 1.0);
  cmdline.add("-beta3", "weight of the speaking rate feature", &beta3, 1.0);
  cmdline.add("-output_align", "file list where the forced alignemnt is written", &output_align, "");
  cmdline.add("-output_confidence", "single file where the forced alignemnt confidence is written", 
							&output_confidence, "");
	cmdline.add("-output_textgrid", "file list of output TextGrids", &output_textgrid, "");
	cmdline.add("-loss", "shoud be alignment_loss or tau_insensitive_loss (default)", 
							&loss_type, "tau_insensitive_loss");
  cmdline.add_master_option("scores_filelist", &scores_filelist);
  cmdline.add_master_option("dists_filelist", &dists_filelist);
  cmdline.add_master_option("phonemes_filelist", &phonemes_filelist);  
  cmdline.add_master_option("start_times_filelist [null]", &start_times_filelist);
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

  // similar phonemes map, maps a phoneme to the set of other phonemes that are similar to it
  map<string, set<string> > similar_phonemes_map = loadSimilarPhonemes_map();
  
  // Initiate classifier
  Classifier classifier(frame_rate, min_phoneme_length, max_phoneme_length, C, 
												beta1, beta2, beta3, 0.0, loss_type);
  classifier.load(classifier_filename);
  classifier.load_phoneme_stats(phoneme_stats_filename);
  
  // begining of the training set
  Dataset test_dataset(scores_filelist, dists_filelist, phonemes_filelist, start_times_filelist);
  
  int num_boundaries = 0;
  int cummulative_loss = 0;
  double cummulative_loss_eps_alignment = 0.0;
  double cummulative_loss_eps_insensitive = 0.0;
  int cum_loss_less_than[NUM_CUM_LOSS_RESOLUTIONS+1];
  for (uint t=1; t <= NUM_CUM_LOSS_RESOLUTIONS; t++)
	cum_loss_less_than[t] = 0;
  

  StringVector output_align_files;
  if (output_align != "") 
	output_align_files.read(output_align);

  ofstream output_confidence_ofs;
  if (output_confidence != "") {
	output_confidence_ofs.open(output_confidence.c_str());
	if (!output_confidence_ofs.good()) {
	  cerr << "Error: unable to open " << output_confidence << "for writing." << endl;
	}
  }

	StringVector output_textgrid_files;
	if (output_textgrid != "")
		output_textgrid_files.read(output_textgrid);
  
  // Run over all dataset
  for (uint i=0; i <  test_dataset.size(); i++) {

	SpeechUtterance x;
	StartTimeSequence y;
	StartTimeSequence y_hat;

	cout << "==================================================================================" << endl;

	// read next example for dataset
	test_dataset.read(x, y, remove_silence);
	y_hat.resize(x.phonemes.size());

	// predict label
	double confidence = classifier.predict(x, y_hat);

	if (test_dataset.labels_given())
	  cout << "alignment= " << y << endl;
	cout << "predicted start times= " << y_hat << " " << x.scores.height() << endl;
	cout << "confidence= " << confidence << endl;
	cout << "aligned_phoneme_score= " << classifier.aligned_phoneme_scores(x, y_hat) << endl;
	cout << "sentence confidence: " << classifier.confidence_general(x, y_hat) << endl;

	// 1. Get the matrix of the confidences:
	infra::matrix confidences = classifier.confidence_per_phoneme(x, y_hat);

	// 2. Print the desired phonemes, given as input:
	cout << "desired phonemes: " << x.phonemes << endl;
	// 3. Print the maximum confidence of each line (frame):
	cout << "max phonems:     ";
	for (int line=0; line < int(confidences.height()); line++) {
	  cout <<" "<< PhonemeSequence::index2phoneme[int(confidences.row(line).argmax())];
	}
	cout <<"\n";

    // 4. Normalize the confidences matrix:
    infra::matrix normalized_confidences = classifier.normalizeConfidences(confidences);

	// 5. Print the wanted phoneme score against the maximum score of each frame:
	//    The final score for each frame, wanted phoneme, is its difference from the max score.
	infra::vector final_scores(y_hat.size());
	final_scores.zeros();

    cout <<"---Not normalized---\n";
    infra::matrix final_matrix = confidences;
	cout<<"         expected  score     max phone score       dist       ratio\n";
	for (int line=0; line < int(final_matrix.height()); line++) {
	  int wanted_phoneme_index = x.phonemes[line];
	  string wanted_phoneme = PhonemeSequence::index2phoneme[wanted_phoneme_index];
	  double wanted_phoneme_score = final_matrix(line, wanted_phoneme_index);
	  double max_score = final_matrix.row(line).max();
	  int max_index = int(final_matrix.row(line).argmax());
	  string max_phoneme = PhonemeSequence::index2phoneme[max_index];
	  double distance = max_score - wanted_phoneme_score;
      double ratio = wanted_phoneme_score / max_score;
      // If the max and wanted phoneme are similar, e.g. 'eh' and 'ae', ignore thr mistake.
      if (areSimilarPhonemes(similar_phonemes_map, wanted_phoneme, max_phoneme)) {
        ratio = 1.0;
        distance = 0.0;
      }
      final_scores(line) = ratio;
	  string first_tab, second_tab, third_tab;

	  if (countDigit(wanted_phoneme_score) < 3) {first_tab = "\t\t\t\t";}
	  else if (countDigit(wanted_phoneme_score) < 6) {first_tab = "\t\t\t";} else {first_tab = "\t\t";}
	  if (countDigit(max_score) < 3) {second_tab = "\t\t\t\t";}
	  else if (countDigit(max_score) < 6) {second_tab = "\t\t\t";} else {second_tab = "\t\t";}
	  if (countDigit(distance) < 3) {third_tab = "\t\t\t\t";}
	   else if (countDigit(distance) < 6) {third_tab = "\t\t\t";} else {third_tab = "\t\t";}

	  cout << (line+1) << ":(" << wanted_phoneme_index << ")\t\t"
	  << wanted_phoneme << "\t\t" << wanted_phoneme_score << first_tab
	  << max_phoneme << "\t\t" << max_score << second_tab
	  << distance << third_tab << ratio << "\n";
	}

	cout<<"\n---Normarlized---\n";
    final_matrix = normalized_confidences;
	cout<<"         expected  score     max phone score       dist       ratio\n";
	for (int line=0; line < int(final_matrix.height()); line++) {
	  int wanted_phoneme_index = x.phonemes[line];
	  string wanted_phoneme = PhonemeSequence::index2phoneme[wanted_phoneme_index];
	  double wanted_phoneme_score = final_matrix(line, wanted_phoneme_index);
	  double max_score = final_matrix.row(line).max();
	  int max_index = int(final_matrix.row(line).argmax());
	  double distance = max_score - wanted_phoneme_score;
      double ratio = wanted_phoneme_score / max_score;
	  string first_tab, second_tab, third_tab;

	  if (countDigit(wanted_phoneme_score) < 3) {first_tab = "\t\t\t";}
	  else if (countDigit(wanted_phoneme_score) < 6) {first_tab = "\t\t";} else {first_tab = "\t";}
	  if (countDigit(max_score) < 3) {second_tab = "\t\t\t";}
	  else if (countDigit(max_score) < 6) {second_tab = "\t\t";} else {second_tab = "\t";}
	  if (countDigit(distance) < 3) {third_tab = "\t\t\t";}
	   else if (countDigit(distance) < 6) {third_tab = "\t\t";} else {third_tab = "\t";}

	  cout << (line+1) << ":(" << wanted_phoneme_index << ")\t\t"
	  << wanted_phoneme << "\t\t" << wanted_phoneme_score << first_tab
	  << PhonemeSequence::index2phoneme[max_index]<< "\t\t" << max_score << second_tab
	  << distance << third_tab << ratio << "\n";
	}

	cout << "final_scores: " << final_scores << endl;

#if 0    
	int end_frame = 0;
	confidence = classifier.align_keyword(x,y_hat,end_frame);
	cout << "k_predicted= " << y_hat << " " << end_frame << endl;
	cout << "k_confidence= " << confidence << endl;
#endif    

	if (output_align != "") {
	  ofstream output_align_ofs(output_align_files[i].c_str());
	  if (output_align_ofs.good()) {
		for (uint j=0; j < y_hat.size(); j++) {
		  output_align_ofs << y_hat[j] << endl;
		}
		output_align_ofs.close();
	  }
	}
	if (output_confidence != "" && output_confidence_ofs.good())
	  output_confidence_ofs << confidence << endl;

		if (output_textgrid != "") {
			cout << x.scores.height() << endl;
			cout << x.distances.height() << endl;
			WriteTextGrid(output_textgrid_files[i], x.phonemes, y_hat, x.scores.height());
		}
		
	// calculate the error
	if (test_dataset.labels_given()) {
	  int file_loss = 0;
	  int cur_loss;
	  for (unsigned int j=0; j < y.size(); ++j) {
		if (y[j] > y_hat[j]) {
		  cur_loss = y[j] - y_hat[j];
		} else {
		  cur_loss = y_hat[j] - y[j];
		}
		file_loss += cur_loss;
		cummulative_loss += cur_loss;
		for (int t=1; t <= NUM_CUM_LOSS_RESOLUTIONS; t++)
		  if ( cur_loss <= t ) cum_loss_less_than[t]++;
	  }
	  num_boundaries += y.size();
	  cout << "File loss = " << file_loss/double(y.size()) << endl;
	  cout << "Cum loss = " << cummulative_loss/double(num_boundaries) << endl;
	  for (uint t = NUM_CUM_LOSS_RESOLUTIONS; t >= 1; t--) {
		cout << "% Boundaries (t <= " << t*frame_rate << "ms) = "
		<< 100.0*cum_loss_less_than[t]/double(num_boundaries) << "\n";
	  }
	  cout << endl;
	  Classifier::loss_type = "tau_insensitive_loss";
	  cummulative_loss_eps_insensitive += (Classifier::gamma(y, y_hat)*y.size());
//      cout << "Classifier::gamma(y, y_hat)=" << Classifier::gamma(y, y_hat) << " " << num_boundaries << endl;
	  cout << classifier.loss_type << "=" << cummulative_loss_eps_insensitive/double(num_boundaries) << endl;
	  Classifier::loss_type = "alignment_loss";
	  cummulative_loss_eps_alignment += (Classifier::gamma(y, y_hat)*y.size());
//      cout << "Classifier::gamma(y, y_hat)=" << Classifier::gamma(y, y_hat) << " " << num_boundaries << endl;
	  cout << classifier.loss_type << "=" << cummulative_loss_eps_alignment/double(num_boundaries) << endl;

	}
  }
  
  if (output_confidence != "" && output_confidence_ofs.good()) 
	output_confidence_ofs.close();

  cout << "Done." << endl;  
  
  return EXIT_SUCCESS;
  
}

// ------------------------------- EOF -----------------------------//
