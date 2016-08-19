/************************************************************************
 Project:  Phoneme Alignment
 Module:   Dataset Definition
 Purpose:  Defines the data structs of the instances and the labels
 Date:     25 Jan., 2005
 
 Function List:
 SpeechUtterance
 read - Read Instance from file stream
 
 StartTimeSequence
 read - Read Label from file stream
 
 **************************** INCLUDE FILES *****************************/
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "Dataset.h"

//#define OLD_ALIGNMENT
#define NORM_SCORES_0_1

// PhonemeSequence static memebers definitions
unsigned int PhonemeSequence::num_phonemes;
std::map<std::string, int> PhonemeSequence::phoneme2index;
std::map<int,std::string> PhonemeSequence::index2phoneme;

/************************************************************************
 Function:     SpeechUtterance::read
 
 Description:  Read SpeechUtterance from file stream
 Inputs:       string &filename
 Output:       bool - true if success, otherwise false
 Comments:     none.
 ***********************************************************************/
void SpeechUtterance::read(std::string &scores_filename, std::string &dists_filename, 
                           std::string &phonemes_filename, bool single_file_mode)
{
  // load score matrix
  std::ifstream ifs_scores(scores_filename.c_str());
  if (ifs_scores.good()) {
    std::cerr << "Reading scores from " << scores_filename << std::endl;
    infra::matrix tmp(ifs_scores);
		original_scores.resize(tmp.height(), tmp.width());
		original_scores = tmp;
    scores.resize(tmp.height(), tmp.width());
#ifdef NORM_SCORES_0_1
    // normalize scores to be in [0,1]
    for (uint i = 0; i < tmp.height(); i++) {
      double max_scores_i = tmp.row(i).max();
      double min_scores_i = tmp.row(i).min();
      //std::cout << "tmp.row(i)=" << tmp.row(i) << std::endl;
      //std::cout << "max_scores_i=" << max_scores_i << " min_scores_i=" << min_scores_i << std::endl;
      for (uint j = 0; j < tmp.width(); j++) {
        scores(i,j) = (tmp(i,j)-min_scores_i)/(max_scores_i-min_scores_i);
      }
    }
#else
    scores = tmp;
#endif
  }
  else {
    std::cerr << "Error: Unable to read scores from " << scores_filename << std::endl;
    exit(-1);
  } 
  ifs_scores.close();
  
  // load dist matrix
  std::ifstream ifs_distances(dists_filename.c_str());
  if (ifs_distances.good()) {
    infra::matrix tmp(ifs_distances);
    distances.resize(tmp.height(), tmp.width());
    distances = tmp;
  }
  else {
    std::cerr << "Error: Unable to read distances from " << dists_filename << std::endl;
    exit(-1);
  } 
  ifs_distances.close();
  
  // load phonemes
  if (single_file_mode)
    phonemes.from_string(phonemes_filename);
  else 
    phonemes.read(phonemes_filename);
}

/************************************************************************
 Function:     SpeechUtterance::size()
 
 Description:  Size of features phi
 Inputs:       none.
 Output:       void.
 Comments:     none.
 ***********************************************************************/
unsigned long SpeechUtterance::size() 
{ 
  return ( scores.width() + distances.width() + 1 ); 
} 

/************************************************************************
 Function:     PhonemeSequence::load_phoneme_map
 
 Description:  Load the phonemes file andbuild maps
 Inputs:       string &filename
 Output:       void.
 Comments:     none.
 ***********************************************************************/
void PhonemeSequence::load_phoneme_map(std::string &phonemes_filename, std::string &silence_symbol)
{
  // Generate phoneme mapping
  std::ifstream phonemes_ifs(phonemes_filename.c_str());
  if (!phonemes_ifs.good()) {
    std::cerr << "Error: Unable to open phonemes file " << phonemes_filename << std::endl;
    exit(-1);
  }
  int index = 0;
  bool found_silence_symbol = false;
  while (phonemes_ifs.good()) {
    std::string phoneme;    
    phonemes_ifs >> phoneme;
    if (phoneme == "") continue;
    if (phoneme == silence_symbol) found_silence_symbol = true;
    index2phoneme[index] = phoneme;
    phoneme2index[phoneme] = index;
    index++;
  }
  phonemes_ifs.close();
  if (!found_silence_symbol) {
    std::cerr << "Error: didn't find the silence symbol \"" << silence_symbol 
    << "\" inside phonemes file: " << phonemes_filename << std::endl;
  }
  
  num_phonemes = index;
  
  //	for (uint i = 0; i < num_phonemes; i++)
  //		std::cout << i << " " << index2phoneme[i] << std::endl;
}


/************************************************************************
 Function:     PhonemeSequence::read
 
 Description:  Read PhonemeSequence from file stream
 Inputs:       string &filename
 Output:       void.
 Comments:     none.
 ***********************************************************************/
void PhonemeSequence::read(std::string &filename)
{
  std::ifstream ifs(filename.c_str());
  if (!ifs.good()) {
    std::cerr << "Error: Unable to open phonemes file " << filename << std::endl;
    exit(-1);
  }

#ifdef OLD_ALIGNMENT
  int num_labels; 
  ifs >> num_labels;
#endif
  while (ifs.good()) {
    std::string phoneme;
    ifs >> phoneme;
		//std::cout << phoneme << std::endl;
    if (phoneme == "") break;
#ifdef OLD_ALIGNMENT
    if (phoneme == "-1") phoneme = "34";
#else
    if (phoneme == "del") phoneme = "t";
#endif
    push_back(PhonemeSequence::phoneme2index[phoneme]);
  }
  ifs.close();
}

/************************************************************************
 Function:     PhonemeSequence::from_string
 
 Description:  Read PhonemeSequence from a string
 Inputs:       string &phoneme_string
 Output:       void.
 Comments:     none.
 ***********************************************************************/
void PhonemeSequence::from_string(std::string &phoneme_string)
{
  std::istringstream phoneme_stringstream(phoneme_string);
  std::string phoneme;
  while (phoneme_stringstream >> phoneme) {
    if (phoneme == "") break;
    if (phoneme == "del") phoneme = "t";
    push_back(PhonemeSequence::phoneme2index[phoneme]);
  }
}


/************************************************************************
 Function:     operator << for PhonemeSequence
 
 Description:  Write PhonemeSequence& vector to output stream
 Inputs:       std::ostream&, const StringVector&
 Output:       std::ostream&
 Comments:     none.
 ***********************************************************************/
std::ostream& operator<< (std::ostream& os, const PhonemeSequence& y)
{
  for (uint i=0; i < y.size(); i++)
    os << PhonemeSequence::index2phoneme[ y[i] ] << " ";
  
  return os;
}

#ifndef OLD_ALIGNMENT
/************************************************************************
 Function:     operator << for PhonemeSequence
 
 Description:  Write PhonemeSequence& vector to output stream
 Inputs:       std::ostream&, const StringVector&
 Output:       std::ostream&
 Comments:     none.
 ***********************************************************************/
std::ostream& operator<< (std::ostream& os, const StartTimeSequence& y)
{
  for (uint i=0; i < y.size(); i++)
    os <<  y[i]  << " ";
  
  return os;
}
#endif 

/************************************************************************
 Function:     Dataset::Dataset
 
 Description:  Constructor
 Inputs:       std::string dataset_filename
 Output:       void.
 Comments:     none.
 ***********************************************************************/
Dataset::Dataset(std::string& scores_filename, std::string& dists_filelist, 
                 std::string& phonemes_filelist, std::string&  start_times_filelist)
{
  single_file_mode = false;
  
  // Read list of files into StringVector
  scores_file_list.read(scores_filename);
  dists_file_list.read(dists_filelist);
  phonemes_file_list.read(phonemes_filelist);
  if (start_times_filelist == "null") {
    std::cerr << "Info: no start-times were given for error calculation" << std::endl;
    read_labels = false;
  }
  else {
    read_labels = true;
    start_times_file_list.read(start_times_filelist);
  }
  if (scores_file_list.size() == 1 && dists_file_list.size() == 1 && 
      phonemes_file_list.size() > 1 && !read_labels) {
    single_file_mode = true;
  }
  current_file = 0;
}


/************************************************************************
 Function:     Dataset::read
 
 Description:  Read next instance and label
 Inputs:       SpeechUtterance&
 StartTimeSequence&
 Output:       void.
 Comments:     none.
 ***********************************************************************/
uint Dataset::read(SpeechUtterance &x, StartTimeSequence &y, bool remove_silence, bool enable_printouts)
{
  uint sil_offset = 0;
  
  if (single_file_mode) {
    std::cout << "current file=" << current_file << std::endl;
		std::cout << scores_file_list[0] << std::endl;
		std::cout << dists_file_list[0] << std::endl;
		std::cout << phonemes_file_list[0] << std::endl;

    x.read(scores_file_list[0], dists_file_list[0],
           phonemes_file_list[current_file],single_file_mode);
  }
  else {
		if (enable_printouts) {
    std::cout << "current file=" << current_file << std::endl;
		std::cout << scores_file_list[current_file] << std::endl;
		std::cout << dists_file_list[current_file] << std::endl;
		std::cout << phonemes_file_list[current_file] << std::endl;
		}
    x.read(scores_file_list[current_file], dists_file_list[current_file],
           phonemes_file_list[current_file],single_file_mode);
  }
  
  if (read_labels) {
		std::cout << start_times_file_list[current_file] << std::endl;
    y.read(start_times_file_list[current_file]);
  }
  else {
    y.resize(x.phonemes.size());
  }
    
  ++current_file;
  
  
  if (remove_silence) {
    ///////////////////////////
    // the code below is used to remove the leading silence at the 
    // begining of each utterance and the following silnce at the end

    x.silence_offset = y[1];
    x.last_silence = y[y.size()-1];
    infra::matrix tmp1(x.scores);
    x.scores.resize(y[y.size()-1]-y[1],tmp1.width());
    x.scores = tmp1.submatrix(y[1],0,x.scores.height(),x.scores.width());
    
    // remove silences
    for (uint i=0;i<x.phonemes.size()-2;++i) 
      x.phonemes[i] = x.phonemes[i+1];
    x.phonemes.resize(x.phonemes.size()-2);
    
    infra::matrix tmp2(x.distances);
    x.distances.resize(y[y.size()-1]-y[1],tmp2.width());
    x.distances = tmp2.submatrix(y[1],0,x.distances.height(),x.distances.width());
    
    uint tmp3 = y[1];
    for (uint i=0;i<y.size()-2;++i)
      y[i] = y[i+1]-tmp3;
    y.resize(y.size()-2);
    ///////////////////////////
  }
  return sil_offset;
}

std::ostream& operator<< (std::ostream& os, const IntVector& v)
{
  IntVector::const_iterator iter = v.begin();
  IntVector::const_iterator end = v.end();
  
  while(iter < end) {
    os << *iter << " ";
    ++iter;
  }
  return os;
}


bool WriteTextGrid(std::string filename, PhonemeSequence &p, StartTimeSequence &s, int num_frames)
{
	std::ofstream ofs(filename.c_str());
	if (!ofs.good()) {
		std::cerr << "Error: unable to creat TextGrid " << filename << "." << std::endl;
		return false;
	}
	
	// ASSUMING 10ms frames !!!
	// ASSUMING NO silece was removed!!!!
	
	ofs << "File type = \"ooTextFile\"" << std::endl;
	ofs << "Object class = \"TextGrid\" " << std::endl;
	ofs << std::endl;
	ofs << "xmin = 0" << std::endl;
	ofs << "xmax = " << double(num_frames)/100.0 << std::endl;
	ofs << "tiers? <exists>" << std::endl;
	ofs << "size = 1" << std::endl;
	ofs << "item []:" << std::endl;
	ofs << "\t" << "item [1]:" << std::endl;
	ofs << "\t\t" << "class = \"IntervalTier\"" << std::endl;
	ofs << "\t\t" << "name = \"Forced Alignment\"" << std::endl;
	ofs << "\t\t" << "xmin = 0" << std::endl;
	ofs << "\t\t" << "xmax = " << double(num_frames)/100.0 << std::endl;
	ofs << "\t\t" << "intervals: size = " << p.size() << std::endl;
	
	for (int i = 0; i < p.size()-1; i++) {
		ofs << "\t\t" << "intervals [" << (i+1) << "]:" << std::endl;
		ofs << "\t\t\t" << "xmin = " << double(s[i])/100.0 << std::endl;
		ofs << "\t\t\t" << "xmax = " << double(s[i+1])/100.0 << std::endl;
		ofs << "\t\t\t" << "text = \"" << PhonemeSequence::index2phoneme[ p[i] ] << "\"" << std::endl;
	}
	ofs << "\t\t" << "intervals [" << p.size() << "]:" << std::endl;
	ofs << "\t\t\t" << "xmin = " << double(s[p.size()-1])/100.0 << std::endl;
	ofs << "\t\t\t" << "xmax = " << double(num_frames)/100.0 << std::endl;
	ofs << "\t\t\t" << "text = \"" << PhonemeSequence::index2phoneme[ p[p.size()-1] ] << "\"" << std::endl;
	
	ofs.close();

	return true;
}


// --------------------------  EOF ------------------------------------//
