
#ifndef _MY_DATASET_H_
#define _MY_DATASET_H_

/************************************************************************
 Project:  Phoeneme Classification
 Module:   Dataset Definitions
 Purpose:  Defines the data structs of instance and label
 Date:     21 Apr., 2005

 *************************** INCLUDE FILES ******************************/
#include <fstream>
#include <vector>
#include <map>
#include <utility>
#include <cstring>
#include <infra.h>

#define MAX_LINE_SIZE 4096

  
/***********************************************************************/
class IntVector : public std::vector<int> {
 public:
  IntVector() : std::vector<int>() {  }

  IntVector(uint size) : std::vector<int>(size) { }
  int read(std::string &filename) {
    std::ifstream ifs(filename.c_str());
    // check input file stream
    if (!ifs.good()) {
      std::cerr << "Unable to read IntVector from " << filename << std::endl;
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
    return size();
  }  
};

std::ostream& operator<< (std::ostream& os, const IntVector& v);


/***********************************************************************/
class StringVector : public std::vector<std::string> {
 public:

  int read(std::string &filename) {
    std::ifstream ifs;
    char line[MAX_LINE_SIZE];
    ifs.open(filename.c_str());
    if (!ifs.is_open()) {
      std::cerr << "Warning: Unable to open file list " << filename << std::endl;
      return 0;
    }    
    while (!ifs.eof()) {
      ifs.getline(line,MAX_LINE_SIZE);
      if (strcmp(line,""))
	push_back(std::string(line));
    }
    return size();
  }
};

std::ostream& operator<< (std::ostream& os, const StringVector& v);


/***********************************************************************/
class InstanceType
{
 public:
  void read(std::string &filename, bool print_header);
  static void load_mfcc_stats(std::string &filename);

 public:
  infra::matrix mfcc;
  int num_frames;
  static int mfcc_dim;
  static int features_dim;
  static infra::vector mfcc_mean;
  static infra::vector mfcc_std;
};

/***********************************************************************/
class LabelType : public IntVector
{
 public:
  void read(std::string &filename);
  static void load_phoneme_map(std::string &phoneme_filename, 
                               std::string &map_output_filename);
  static void load_equivalence_map(std::string &map_filename);
  static void set_silence_symbol(std::string &symbol);
  static bool phonemes_equivalent(int y1, int y2);

 public:
  int num_frames;
  static int num_phonemes;
  static std::map<std::string, int> phoneme2index;
  static std::map<int, std::string> index2phoneme;
  static std::map<int, std::string> index2equiv;
  static std::string silence_symbol;
};

std::ostream& operator<< (std::ostream& os, const LabelType& y);

/***********************************************************************/
class Dataset 
{
 public:
  Dataset(std::string& input_filelist, std::string& target_filelist);
  void initiate(void);
  void read(InstanceType &x, LabelType &y);
  void read_all(infra::matrix &X, IntVector &Y);
  int size() { return input_files.size(); } 
  std::string &filename() { return input_files[current_file]; }
  bool labels_given() { return read_labels; }
  
 private:
  StringVector input_files;
  StringVector target_files;
  int current_file;
  bool read_labels;
};

#endif // _MY_DATASET_H_
