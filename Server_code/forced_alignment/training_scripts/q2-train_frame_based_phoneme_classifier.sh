#!/bin/bash

MODELS=/Experiments/phoneme_alignment/timit_16kHz_48/models
BIN=bin

# phoneme frame-based classifier parameters
SIGMA=4.3589
C=1
B=0.8
PAD=1

input_list=config/train_frame_based_classifier_1500.mfc
target_list=config/train_frame_based_classifier_1500.labels
rs_list=config/train_frame_based_classifier_1500.random_shuffle
model=$MODELS/pa_phoeneme_frame_based.C_$C.B_$B.sigma_$SIGMA.model

# train passive-aggressive phoneme frame-based classifier 
mkdir -p $MODELS

if [[ 1 = 1 ]] ; then
	
# create random shuffle of the training data
$BIN/RandomShuffleDb -n $PAD $input_list $target_list $rs_list

$BIN/PhonemeFrameBasedTrain \
	-C $C \
	-B $B \
	-n $PAD \
	-kernel_expansion rbf3 \
	-sigma $SIGMA \
	-mfcc_stats config/mfcc.stats \
	-omap config/timit61.phonemap \
	$input_list \
	$target_list \
	$rs_list \
	config/phonemes_39 \
	$model 

# decode the rest of the training files 
$BIN/PhonemeFrameBasedDecode \
	-n $PAD \
	-kernel_expansion rbf3 \
	-sigma $SIGMA \
	-averaging \
	-mfcc_stats config/mfcc.stats \
	-scores config/train_frame_based_classifier_1500_rest.scores \
	config/train_frame_based_classifier_1500_rest.mfc \
	config/train_frame_based_classifier_1500_rest.labels \
	config/phonemes_39 \
	$model

fi

if [[ 1 = 1 ]] ; then

# decode the all the test files
$BIN/PhonemeFrameBasedDecode \
	-n $PAD \
	-kernel_expansion rbf3 \
	-sigma $SIGMA \
	-averaging \
	-mfcc_stats config/mfcc.stats \
	-scores config/test_core.scores \
	config/test_core.mfc \
	config/test_core.labels \
	config/phonemes_39 \
	$model

fi


if [[ 1 = 1 ]] ; then 

# decode the all the test files
$BIN/PhonemeFrameBasedDecode \
	-n $PAD \
	-kernel_expansion rbf3 \
	-sigma $SIGMA \
	-averaging \
	-mfcc_stats config/mfcc.stats \
	-scores config/test.scores \
	config/test.mfc \
	config/test.labels \
	config/phonemes_39 \
	$model

fi
