
#include <iostream>
#include "HtkFile.h"

HtkFile::HtkFile(FILE *_stream)
{
  stream = _stream;
  // check endian type of the machine
  unsigned short s = 0x0001;
  big_endian = ! *(unsigned char*)&s;
}

HtkFile::~HtkFile()
{

}

void HtkFile::read_header() 
{
  // This function reads the header into the struct 
  if (fread(&header.nSamples, 4, 1, stream) != 1)
    throw std::bad_alloc();
  if (fread(&header.sampPeriod, 4, 1, stream) != 1)
    throw std::bad_alloc();
  if (fread(&header.sampSize, 2, 1, stream) != 1)
    throw std::bad_alloc();
  if (fread(&header.parmKind, 2, 1, stream) != 1)
    throw std::bad_alloc();

  if (!big_endian) {
    swap4Bytes(&header.nSamples, &header.nSamples);
    swap4Bytes(&header.sampPeriod, &header.sampPeriod);
    swap2Bytes(&header.sampSize, &header.sampSize);
    swap2Bytes(&header.parmKind, &header.parmKind);
  }
  
  // Should never find EOF here since you would be expecting the data in 
  // the MultiFrame too.
  if (feof(stream) != 0 || ferror(stream) != 0) 
    throw std::bad_alloc();
  if (header.sampSize <= 0 || header.sampSize > 5000 || header.nSamples <= 0 ||
      header.sampPeriod <= 0 || header.sampPeriod > 1000000) {
    std::cerr << "Error: HTK header is not readable." << std::endl;
    throw std::exception();
  }
}

void HtkFile::write_header() 
{
	int nSamples;
	int sampPeriod;
	short sampSize;
	short parmKind;
	
  // swap bytes first
  if (!big_endian) {
    swap4Bytes(&header.nSamples, &nSamples);
    swap4Bytes(&header.sampPeriod, &sampPeriod);
    swap2Bytes(&header.sampSize, &sampSize);
    swap2Bytes(&header.parmKind, &parmKind);
  }
	else {
    nSamples = header.nSamples;
    sampPeriod = header.sampPeriod;
    sampSize = header.sampSize;
    parmKind = header.parmKind;
	}		

  // This function reads the header into the struct 
  if (fwrite(&nSamples, 4, 1, stream) != 1)
    throw std::bad_alloc();
  if (fwrite(&sampPeriod, 4, 1, stream) != 1)
    throw std::bad_alloc();
  if (fwrite(&sampSize, 2, 1, stream) != 1)
    throw std::bad_alloc();
  if (fwrite(&parmKind, 2, 1, stream) != 1)
    throw std::bad_alloc();
}


size_t HtkFile::num_coefs()
{
  return header.sampSize/sizeof(float);
}

std::string HtkFile::parmKind2str()
{
/* EXPORT-> ParmKind2Str: convert given parm kind to string */
  static std::string parmKind_map[] = {"WAVEFORM", "LPC", "LPREFC", "LPCEPSTRA", 
				"LPDELCEP", "IREFC", 
				"MFCC", "FBANK", "MELSPEC",
				"USER", "DISCRETE", "PLP",
				"ANON"};
  std::string str;
  str = parmKind_map[header.parmKind & BASEMASK];
  if (header.parmKind & HASENERGY) str += "_E";
  if (header.parmKind & HASDELTA) str += "_D";
  if (header.parmKind & HASACCS) str += "_N";
  if (header.parmKind & HASTHIRD) str += "_A";
  if (header.parmKind & HASNULLE) str += "_T";
  if (header.parmKind & HASCOMPX) str += "_C";
  if (header.parmKind & HASCRCC) str += "_K";
  if (header.parmKind & HASZEROM) str += "_Z";
  if (header.parmKind & HASZEROC) str += "_O";
  if (header.parmKind & HASVQ) str += "_V";
  return str;
}

void HtkFile::print_header()
{
  std::cout << "  nSamples: " << header.nSamples << std::endl;
  std::cout << "  sampPeriod: " << (header.sampPeriod/10.0) << " us" << std::endl;
  std::cout << "  SampSize: " << header.sampSize << std::endl;
  std::cout << "  parmKind: " << parmKind2str() << std::endl;
  std::cout << "  Num Coefs: " << num_coefs() << std::endl;
  std::cout << "  Machine type: " << (big_endian ? "big" : "little") << " endian." << std::endl;
}


size_t HtkFile::read_next_vector(float *data)
{
  size_t rc =  fread(data,sizeof(float),num_coefs(),stream);
  if (!big_endian) {
    for (size_t i = 0; i < rc; i++) {
      swap4Bytes(&(data[i]),&(data[i]));
    }
  }
  return rc;
}

size_t HtkFile::read_next_vector(double *data)
{
  float fdata[num_coefs()];

  size_t rc = fread(fdata,sizeof(float), num_coefs(),stream);
  for (size_t i = 0; i < rc; i++) {
    if (!big_endian) {
      swap4Bytes(&(fdata[i]),&(fdata[i]));
    }
    data[i] = fdata[i];
  }
  return rc;
}

size_t HtkFile::read_next_vector(infra::vector &data)
{
  float fdata[num_coefs()];
  data.resize(num_coefs());
  
  size_t rc = fread(fdata,sizeof(float), num_coefs(),stream);
  for (size_t i = 0; i < rc; i++) {
    if (!big_endian) {
      swap4Bytes(&(fdata[i]),&(fdata[i]));
    }
    data[i] = fdata[i];
  }
  return rc;
}


size_t HtkFile::write_next_vector(float *data)
{
  if (!big_endian) {
    for (size_t i = 0; i < (unsigned int)num_coefs(); i++) {
      swap4Bytes(&(data[i]),&(data[i]));
    }
  }
  size_t rc =  fwrite(data,sizeof(float),num_coefs(),stream);
  return rc;
}

void HtkFile::swap2Bytes(void *src, void *dest)
{
  char *p1,*p2;
  char tmp;
  
  p1 = (char*)src;
  p2 = (char*)dest;
  tmp = p1[1]; /* pour pouvoir utiliser src == dest */
  p2[1] = p1[0];
  p2[0] = tmp;
}

void HtkFile::swap4Bytes(void *src, void *dest)
{
  char *p1,*p2;
  char tmp;
  p1 = (char*)src;
  p2 = (char*)dest;
  tmp = p1[3]; /* pour pouvoir utiliser src == dest */
  p2[3] = p1[0];
  p2[0] = tmp;
  tmp = p1[2]; /* pour pouvoir utiliser src == dest */
  p2[2] = p1[1];
  p2[1] = tmp;
}
