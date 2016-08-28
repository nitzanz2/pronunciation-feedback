
#ifndef _MY_DATASET_H_
#define _MY_DATASET_H_

/************************************************************************
 Project:  Phoeneme Alignment
 Module:   Dataset Definitions
 Purpose:  Defines the data structs of instance and label
 Date:     25 Jan., 2005
 
 *************************** INCLUDE FILES ******************************/
#include <cstdlib>
#include <fstream>
#include <vector>
#include <map>
#include <infra.h>
#include <string.h>

#define MAX_LINE_SIZE 4096



class IntVector : public std::vector<int> {
public:
  unsigned int read(std::string &filename) {
    std::ifstream ifs(filename.c_str());
    // check input file stream
    if (!ifs.good()) {
      std::cerr << "Error: Unable to read IntVector from " << filename << std::endl;
      exit(-1);
    }
    // delete the vector
    clear();
    // read size from the stream
    int value;
    int num_values;
    if (ifs.good()) 
      ifs >> num_values;
    while (ifs.good() && num_values--) {
      ifs >> value;
      push_back(value);
    }
    ifs.close();
    return size();
  }
  
};

std::ostream& operator<< (std::ostream& os, const IntVector& v);

/***********************************************************************/

class StringVector : public std::vector<std::string> {
public:
  
  unsigned int read(std::string &filename) {
    std::ifstream ifs;
    char line[MAX_LINE_SIZE];
    ifs.open(filename.c_str());
    if (!ifs.is_open()) {
      std::cerr << "Unable to open file list:" << filename << std::endl;
      return 0;
    }    
    while (!ifs.eof()) {
      ifs.getline(line,MAX_LINE_SIZE);
      if (strcmp(line,""))
        push_back(std::string(line));
    }
    ifs.close();
    return size();
  }
};

/***********************************************************************/
class PhonemeSequence : public std::vector<int>
{
public:
  void read(std::string &filename);
  void from_string(std::string &phoneme_string);
  static void load_phoneme_map(std::string &phoneme_filename, std::string &silence_symbol);
  
public:
  static unsigned int num_phonemes;
  static std::map<std::string, int> phoneme2index;
  static std::map<int,std::string> index2phoneme;  
};

std::ostream& operator<< (std::ostream& os, const PhonemeSequence& y);

/***********************************************************************/

class SpeechUtterance
  {
  public:
    void read(std::string &scores_filename, std::string &dists_filename, 
              std::string &phonemes_filename, bool single_file_mode);
    static void load_phoneme_map(std::string &phoneme_filename, std::string &silence_symbol);
    unsigned long size();  
    
  public:
		infra::matrix original_scores;
    infra::matrix scores;
    infra::matrix distances;
    PhonemeSequence phonemes;
    uint silence_offset;
    uint last_silence;
  };

/***********************************************************************/

#ifdef OLD_ALIGNMENT
typedef IntVector StartTimeSequence; 
#else
class StartTimeSequence : public std::vector<int>
{
public:
  unsigned int read(std::string &filename) {
    std::ifstream ifs(filename.c_str());
    // check input file stream
    if (!ifs.good()) {
      std::cerr << "Error: Unable to read StartTimeSequence from " << filename << std::endl;
      exit(-1);
    }
    // delete the vector
    clear();
    // read size from the stream
    while (ifs.good()) {
      std::string value;
      ifs >> value;
      if (value == "") break;
      push_back(int(std::atoi(value.c_str())));
    }
    ifs.close();
    return size();
  }
};

std::ostream& operator<< (std::ostream& os, const StartTimeSequence& y);
#endif

/***********************************************************************/

class Dataset 
  {
  public:
    Dataset(std::string& scores_filelist, std::string& dists_filelist, 
            std::string& phonemes_filelist, std::string&  start_times_filelist);
    unsigned int read(SpeechUtterance &x, StartTimeSequence &y, bool remove_silence, bool enable_printouts = true);
    unsigned long size() { return phonemes_file_list.size(); } 
    bool labels_given() { return read_labels; }
    
  private:
    StringVector scores_file_list;
    StringVector dists_file_list;
    StringVector phonemes_file_list;
    StringVector start_times_file_list;
    int current_file;
    bool read_labels;
    bool single_file_mode;
  };


bool WriteTextGrid(std::string filename, PhonemeSequence &p, StartTimeSequence &s, int num_frames);


#endif // _MY_DATASET_H_
