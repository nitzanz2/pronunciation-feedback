/*
 *  ConcateFeatures.cpp
 *  pa_primal
 *
 *  Created by Joseph Keshet on 15/9/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <iostream>
#include "ConcateFeatures.h"

ConcateFeatures::ConcateFeatures(int _d, int _num_concate_frames)
{ 
	d = _d;
	num_concate_frames = _num_concate_frames;
	num_context_frames = (num_concate_frames-1)/2;
	std::cout << "concate " << num_concate_frames << " frames. "
	<< "context size " << num_context_frames << std::endl;
}


void ConcateFeatures::concate_inplace(InstanceType &X)
{
	InstanceType Xnew;
  Xnew.mfcc.resize(X.mfcc.height(),X.mfcc.width()*num_concate_frames);

  // left boundary
  for (int i=0; i < num_context_frames; i++) {
		for (int j=0; j < num_context_frames-i; j++) {
			Xnew.mfcc.row(i).subvector(j*X.mfcc.width(),X.mfcc.width()) = X.mfcc.row(0);
		}
		for (int j=num_context_frames-i; j < num_concate_frames ; j++) {
			Xnew.mfcc.row(i).subvector(j*X.mfcc.width(),X.mfcc.width()) = 
			X.mfcc.row(i + j - num_context_frames);
		}
  }
  // all frames are available
	for (int i=num_context_frames; i < int(X.mfcc.height())-num_context_frames; i++) {      
		for (int j=0; j < num_concate_frames; j++) {
			Xnew.mfcc.row(i).subvector(j*X.mfcc.width(),X.mfcc.width()) = 
			X.mfcc.row(i + j - num_context_frames);
		}
	}
  // right boundary
  for (int i=int(X.mfcc.height())-num_context_frames; i < int(X.mfcc.height()); i++) {
		for (int j=0; j < int(X.mfcc.height()) - i + num_context_frames; j++) {
			Xnew.mfcc.row(i).subvector(j*X.mfcc.width(),X.mfcc.width()) = 
			X.mfcc.row(i + j - num_context_frames);
		}
		for (int j=int(X.mfcc.height()) - i + num_context_frames; j < num_concate_frames; j++) {
			Xnew.mfcc.row(i).subvector(j*X.mfcc.width(),X.mfcc.width()) = X.mfcc.row(X.mfcc.height()-1);
		}
  }
	
  X.mfcc.swap(Xnew.mfcc);
}


#ifdef OLDVERSION
void ConcateFeatures::concate_inplace(InstanceType &X, LabelType &Y)
{
	InstanceType Xnew;
  Xnew.mfcc.resize(X.mfcc.height()-num_concate_frames+1,X.mfcc.width()*num_concate_frames);
  
	LabelType Ynew;	
  Ynew.resize(Y.size()-num_concate_frames+1);
  Ynew.num_frames = Y.num_frames-num_concate_frames+1;
  
	for (int i=num_context_frames; i < int(X.mfcc.height())-num_context_frames; i++) {      
		for (int j=0; j < num_concate_frames; j++) {
			Xnew.mfcc.row(i-num_context_frames).subvector(j*X.mfcc.width(),X.mfcc.width()) = 
			X.mfcc.row(i + j - num_context_frames);
		}
		Ynew[i-num_context_frames] = Y[i];
	}
	
  //  std::cout << "X - " << X.mfcc.height() << " x " << X.mfcc.width() << std::endl;
  //  std::cout << "Xnew - " << Xnew.mfcc.height() << " x " << Xnew.mfcc.width() << std::endl;
  //  std::cout << "Y - " << Y.size() << std::endl;
  X.mfcc.swap(Xnew.mfcc);
	swap(Y, Ynew);
  //  std::cout << "swapping" << std::endl;
  //  std::cout << "X - " << X.mfcc.height() << " x " << X.mfcc.width() << std::endl;
  //  std::cout << "Xnew - " << Xnew.mfcc.height() << " x " << Xnew.mfcc.width() << std::endl;
  //  std::cout << "Y - " << Y.size() << std::endl;
  //  std::cout << "X.mfcc=" << std::endl;
  //  std::cout << X.mfcc;
  X.num_frames = X.num_frames-num_concate_frames+1;
	Y.num_frames = X.num_frames-num_concate_frames+1;
  
}
#endif