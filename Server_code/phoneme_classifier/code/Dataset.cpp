/************************************************************************
 Project:  Phoneme Classification
 Module:   Dataset Definition
 Purpose:  Defines the data structs of the instances and the labels
 Date:     25 Jan., 2005
 
 Function List:
 InstanceType
 read - Read Instance from file stream
 
 LabelType
 read - Read Label from file stream
 
 **************************** INCLUDE FILES *****************************/
#include <iostream>
#include <fstream>
#include <iomanip>
#include "Dataset.h"
#include "HtkFile.h"

using std::cout;
using std::cerr;
using std::endl;

// InstanceType members definitions
int InstanceType::mfcc_dim;
infra::vector InstanceType::mfcc_mean((unsigned long)0);
infra::vector InstanceType::mfcc_std((unsigned long)0);

// LabelType static memebers definitions
int LabelType::num_phonemes;
std::map<std::string, int> LabelType::phoneme2index;
std::map<int,std::string> LabelType::index2phoneme;
std::map<int, std::string> LabelType::index2equiv;
std::string LabelType::silence_symbol;

/************************************************************************
 Function:     operator << for IntVector
 
 Description:  Write int vector to output stream
 Inputs:       std::ostream&, const IntVector&
 Output:       std::ostream&
 Comments:     none.
 ***********************************************************************/
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

/************************************************************************
 Function:     operator << for StringVector
 
 Description:  Write std::string vector to output stream
 Inputs:       std::ostream&, const StringVector&
 Output:       std::ostream&
 Comments:     none.
 ***********************************************************************/
std::ostream& operator<< (std::ostream& os, const StringVector& v)
{
  StringVector::const_iterator iter = v.begin();
  StringVector::const_iterator end = v.end();
  
  while(iter < end) {
    os << *iter << " ";
    ++iter;
  }
  return os;
}

/************************************************************************
 Function:     InstanceType::load_mfcc_stats
 
 Description:  Read mfcc statistics from an external file
 Inputs:       string &filename
 Output:       void.
 Comments:     none.
 ***********************************************************************/
void InstanceType::load_mfcc_stats(std::string &filename)
{
  std::ifstream ifs(filename.c_str());
  
  if (!ifs.good()) {
    cerr << "Warning: Unable to load mfcc stats from " << filename << endl;
  }
  else {
    infra::matrix tmp1(ifs);
    mfcc_mean.resize(tmp1.width());
    mfcc_mean = tmp1.row(0);
    mfcc_std.resize(tmp1.width());
    mfcc_std = tmp1.row(1);
    ifs.close();
    //    std::cout << "mfcc mean=" << mfcc_mean << endl;
    //    std::cout << "mfcc std=" << mfcc_std << endl;
  }
}



/************************************************************************
 Function:     InstanceType::read
 
 Description:  Read InstanceType from file stream
 Inputs:       string &filename
 Output:       void.
 Comments:     none.
 ***********************************************************************/
void InstanceType::read(std::string &filename, bool print_header)
{
  FILE *instream;
  if ( (instream = fopen(filename.c_str(),"r")) == NULL) {
    std::cerr << "Error: Cannot open input file " << filename  << "." << endl;
    exit(-1);
  }
  
  // construct HTK file
  HtkFile my_htk(instream); 
  my_htk.read_header();
  mfcc.resize(my_htk.nSamples(),  my_htk.num_coefs());
  
  if (print_header)
    my_htk.print_header();
  
  // read HTK file
  double data[my_htk.num_coefs()];    

  for (int i=0; i < my_htk.nSamples(); i++) {
  
    // read next vector from HTK file
    int rc = my_htk.read_next_vector(data);
    if ( rc != int(my_htk.num_coefs()) ) {
      cerr << "Error: Cannot read sample number " << i << " from " 
      << filename  << "." << endl;
      exit(-1);
    }
    
    // normalize vector, if needed
    if (mfcc_mean.size()) {
      for (int j=0; j < int(my_htk.num_coefs()); j++)
        mfcc(i,j) = (data[j] - mfcc_mean[j])/mfcc_std[j];
    }
    else {
      for (int j=0; j < int(my_htk.num_coefs()); j++)
        mfcc(i,j) = data[j];
    }
  
  }
  
  // close file
  fclose(instream);
  
  num_frames = my_htk.nSamples();
}

/************************************************************************
 Function:     LabelType::set_silence_symbol
 
 Description:  Set the silence symbol
 Inputs:       string &symbol
 Output:       void.
 Comments:     none.
 ***********************************************************************/
void LabelType::set_silence_symbol(std::string &symbol)
{
  silence_symbol = symbol;
}

/************************************************************************
 Function:     LabelType::load_phoneme_map
 
 Description:  Load the phonemes file andbuild maps
 Inputs:       string &filename
 Output:       void.
 Comments:     none.
 ***********************************************************************/
void LabelType::load_phoneme_map(std::string &phonemes_filename, 
                                  std::string &map_output_filename)
{
  // Generate phoneme mapping
  std::ifstream phonemes_ifs(phonemes_filename.c_str());
  if (!phonemes_ifs.good()) {
    std::cerr << "Error: Unable to open phonemes file " 
    << phonemes_filename << std::endl;
    exit(-1);
  }
  int index = 0;
  bool silence_symbol_found = false;
  while (phonemes_ifs.good()) {
    std::string phoneme;    
    phonemes_ifs >> phoneme;
    if (phoneme == "") continue;
    if (phoneme == "del") continue;
    if (phoneme == "rej") continue;
    if (phoneme == silence_symbol) silence_symbol_found = true; 
    index2phoneme[index] = phoneme;
    phoneme2index[phoneme] = index;
    index++;
  }
  phonemes_ifs.close();
  
  num_phonemes = index;
  if (!silence_symbol_found) {
    std::cerr << "Warning: Silence symbol " << silence_symbol 
    << " was not found in the phonemes file." 
    << std::endl;
  }
  
  if (map_output_filename != "") {
    std::ofstream map_output(map_output_filename.c_str());
    std::map<std::string, int>::iterator I1;
    for (I1 = phoneme2index.begin(); I1 != phoneme2index.end(); I1++)
      map_output << (*I1).first << " " << (*I1).second << endl;
    map_output.close();
  }
#if 0
  cout << "phoneme2index" << endl;
  std::map<std::string, int>::iterator I1;
  for (I1 = phoneme2index.begin(); I1 != phoneme2index.end(); I1++)
    cout << (*I1).first << "-->" << (*I1).second << endl;
  
  cout << "index2phoneme" << endl;
  std::map<int,std::string>::iterator I2;
  for (I2 = index2phoneme.begin(); I2 != index2phoneme.end(); I2++)
    cout << (*I2).first << "-->" << (*I2).second << endl;
  exit(-1);
#endif
}


/************************************************************************
 Function:     LabelType::load_equivalence_map
 
 Description:  Load the phonemes file andbuild maps
 Inputs:       string &filename
 Output:       void.
 Comments:     none.
 ***********************************************************************/
void LabelType::load_equivalence_map(std::string &map_filename)
{
  // first copy the map index2phoneme to the map index2equiv
  std::map<int,std::string>::iterator I;
  for (I = index2phoneme.begin(); I != index2phoneme.end(); I++) {
//    cout << (*I).first << "-->" << (*I).second << endl;
    index2equiv[(*I).first] = (*I).second;
  }

  // Generate equivalence mapping
  std::ifstream map_ifs(map_filename.c_str());
  if (!map_ifs.good()) {
    std::cerr << "Error: Unable to open phonemes equivalence map file " 
    << map_filename << std::endl;
    exit(-1);
  }
  while (map_ifs.good()) {
    std::string pclass;
    std::string pequiv;
    std::map<std::string,int>::iterator Ip;
    map_ifs >> pclass;
    map_ifs >> pequiv;
    if (pclass == "" || pequiv == "") continue;
    // look for the key in phoneme2index
    Ip = phoneme2index.find(pequiv);
    if (Ip == phoneme2index.end()) {
      std::cerr << "Error: phoneme " << pequiv << " is not found in phonemes file "
      << "while parsing the line (" << pclass << " " << pequiv << ") in the file " 
      << map_filename << std::endl;
      exit(-1);
    }
//    std::cout << "replacing " << (*Ip).second << " with " << pclass << std::endl;
    index2equiv[(*Ip).second] = pclass;
  }
  map_ifs.close();
  
//  for (I = index2equiv.begin(); I != index2equiv.end(); I++)
//    cout << (*I).first << "-->" << (*I).second << endl;
  
}

/************************************************************************
 Function:     LabelType::phonemes_equivalent
 
 Description:  Load the phonemes file andbuild maps
 Inputs:       string &filename
 Output:       void.
 Comments:     none.
 ***********************************************************************/
bool LabelType::phonemes_equivalent(int y1, int y2)
{
  if (LabelType::index2equiv[y1] == LabelType::index2equiv[y2])
    return true;
  return false;
}



/************************************************************************
 Function:     LabelType::read
 
 Description:  Read LabelType from file stream
 Inputs:       string &filename
 Output:       void.
 Comments:     none.
 ***********************************************************************/
void LabelType::read(std::string &filename)
{
  std::ifstream ifs(filename.c_str());
  if (!ifs.good()) {
    std::cerr << "Error: Unable to open phonemes file " << filename << std::endl;
    exit(-1);
  }
  
  std::string phoneme;
  num_frames = 0;
  while (ifs.good()) {
    ifs >> phoneme;
    if (phoneme == "") {
      break;
    }
    else if (phoneme == "del") {
      push_back( -1 );
      num_frames++;
    }
    else if (phoneme == "rej") {
      push_back( -1 );
      num_frames++;
    }
    else {
      push_back( phoneme2index[phoneme] );
      num_frames++;
    }
  }
  
}

#if 0
/************************************************************************
 Function:     LabelType::read
 
 Description:  Read LabelType from file stream
 Inputs:       string &filename
 Output:       void.
 Comments:     none.
 ***********************************************************************/
void LabelType::read(std::string &filename)
{
  int frame_rate = 10*10000; // 10ms in 100ns units
  //int frame_length = 25*10000; // 25ms in 100ns units
  std::vector<int> phonemes, start_times, stop_times;
  
  std::ifstream ifs(filename.c_str());
  if (!ifs.good()) {
    std::cerr << "Error: Unable to open phonemes file " << filename << std::endl;
    exit(-1);
  }
  
  while (ifs.good()) {
    int start_time;
    int stop_time;
    std::string phoneme;
    ifs >> start_time; // 100ns units
    ifs >> stop_time; // 100ns units
    ifs >> phoneme;
    if (phoneme == "") break;
    start_times.push_back(start_time);
    stop_times.push_back(stop_time);
    phonemes.push_back( phoneme2index[phoneme] );
    //    cout << start_time << " " << stop_time << " " 
    // << stop_time-start_time << " "<< phoneme << endl;
  }
  ifs.close();
  
  for (int i=0; i < (int)num_frames; i++) {
    /* search the array of timings to return a phoneme index and context */
    int start = i*frame_rate;
    int stop = start + frame_rate;
    //    int stop = start + frame_length;
    int j;
    
    /* search for the first phoneme that overlaps with this area */
    for (j = 0; j < (int)phonemes.size() && stop_times[j] <= start; j++);
    
    /* if off either end of the record, return the silence index */
    if (j == (int)phonemes.size() || start_times[0] > start || stop_times[phonemes.size()-1] < stop) {
      push_back( phoneme2index[silence_symbol] );
      cout << silence_symbol << endl;
    }
    else {
      /* and search for the last phoneme that overlaps with this area */
      int first = j, last;
      int k;
      for(k = first; k < (int)phonemes.size() && start_times[k] < stop; k++);
      if(start_times[k - 1] >= stop) { /* the last phonemes */
        last = phonemes.size();
      }
      else {
        last = k - 1;
      }
      if (last - first < 0) {
        /* frame of non-existant phoneme symbol in the file (such /q/) */
        push_back( -1 );
        cout << -1 << endl;
      }
      else if (last-first ==  0) {  
        /* there is a single segment that overlaps with this time */
        push_back( phonemes[first] );
        cout << index2phoneme[phonemes[first]] << endl;
      }
      else if (last-first == 1) {
        /* there are two segments which overlap, choose the largest */
        if (stop_times[first] - start > stop - start_times[last]) {
          push_back( phonemes[first] );
          cout << index2phoneme[phonemes[first]] << endl;
        }
        else {
          push_back( phonemes[last] );
          cout << index2phoneme[phonemes[last]] << endl;
        }
      } 
      else {
        push_back( phonemes[first+1] );
        cout << index2phoneme[phonemes[first+1]] << endl;
      }
    }
  }
  
}
#endif



/************************************************************************
 Function:     operator << for LabelType
 
 Description:  Write LabelType& vector to output stream
 Inputs:       std::ostream&, const StringVector&
 Output:       std::ostream&
 Comments:     none.
 ***********************************************************************/
std::ostream& operator<< (std::ostream& os, const LabelType& y)
{
  for (int i=0; i < int(y.size()); i++)
    os << LabelType::index2phoneme[ y[i] ] << " ";
  
  return os;
}

/************************************************************************
 Function:     Dataset::Dataset
 
 Description:  Constructor
 Inputs:       std::string inputs_filelist
 std::string targets_filelist
 Output:       void.
 Comments:     none.
 ***********************************************************************/
Dataset::Dataset(std::string& input_filelist, std::string& target_filelist)

{
 
  // Read list of files into StringVector
  input_files.read(input_filelist);
  
  if (target_filelist == "null") {
    cerr << "Info: no labels were given" << endl;
    read_labels = false;
  }
  else {
    read_labels = true;
    target_files.read(target_filelist);
  }
  
  // read dimention
  InstanceType instance;
  cout << "Loading dimenstions from first file in the dataset" << endl;
  instance.read(input_files[0], true); // true --> print header
  InstanceType::mfcc_dim = instance.mfcc.width();
  
  current_file = 0;
}

/************************************************************************
 Function:     Dataset::initiate
 
 Description:  Initiate reading of the dataset
 Inputs:       void.
 Output:       void.
 Comments:     none.
 ***********************************************************************/
void Dataset::initiate(void)
{
  current_file = 0;
}

/************************************************************************
 Function:     Dataset::read
 
 Description:  Read next instance and label
 Inputs:       InstanceType&
 LabelType&
 Output:       void.
 Comments:     none.
 ***********************************************************************/
void Dataset::read(InstanceType &x, LabelType &y)
{
  int i = current_file;
  //std::cout << "Loading file number " << i << ": ";
  
  x.read(input_files[i], false);
  //cout << input_files[i] << " ";
  
  y.num_frames = x.num_frames;
  
  if (read_labels) {
    y.read(target_files[i]);
    //cout << target_files[i] << endl;
    if (x.num_frames == y.num_frames-1) {
      x.num_frames = y.num_frames-1;
      y.num_frames = y.num_frames-1;
    }
    else if (x.num_frames != y.num_frames) {
      cerr << "Warning: number of labels (" << y.num_frames 
      << ") does not match number of frames (" 
      << x.num_frames << ")" << endl;
      int num_frames = (x.num_frames < y.num_frames) ? x.num_frames : y.num_frames;
      x.num_frames = y.num_frames = num_frames;
      //exit(-1);
    }
  }
  else {
    y.resize(x.num_frames);
  }
  
#ifdef __DEBUG__
  std::cout << "-------------------------" << std::endl;
  for (int i=0; i < x.num_frames; i++) {
    std::cout << x.mfcc.row(i) << " " << y[i];
    if (x.mfcc.row(i)[y[i]] == 0) 
      std::cout << " *";
    std::cout << std::endl;
  }
  std::cout << "-------------------------" << std::endl;
#endif    
  
  current_file++;
}


/************************************************************************
 Function:     Dataset::read_all
 
 Description:  Read all dataset at once
 Inputs:       infra::matrix&, infra::vector&
 Output:       void.
 Comments:     none.
 ***********************************************************************/
void Dataset::read_all(infra::matrix &X, IntVector &Y)
{
  for (int l=0; l < size(); l++) {
    InstanceType x;
    LabelType y;
    // read next example for dataset
    read(x, y);
    X.resize(X.height()+x.num_frames,InstanceType::mfcc_dim);
    Y.resize(Y.size()+x.num_frames);
    for (int j=0; j < x.num_frames; j++) {
      X.row(X.height()-x.num_frames+j) = x.mfcc.row(j);
      Y[Y.size()-x.num_frames+j] = y[j];
    }
  }
}

// --------------------------  EOF ------------------------------------//
