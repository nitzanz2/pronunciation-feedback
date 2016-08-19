#!/bin/bash

# local setup
TIMIT=/Datasets/timit
DATA=/Experiments/phoneme_alignment/timit_16kHz_48/data
HTK=/Users/jkeshet/Packages/htk/HTKTools
BIN=bin

# keep the location of TIMIT and HTK for further use
echo $TIMIT > config/timit_location
echo $HTK > config/htk_location

if [[ 1 = 1 ]] ; then 

# read the TIMIT disk and encode into acoutic features and labels
mkdir -p $DATA
for DIR in train test ; do

	# create a mirror of the TIMIT directory structure
	(cd $TIMIT ; find $DIR -type d) | sed "s:^:$DATA/:" | xargs mkdir -p

	# generate lists of files with random shuffle of the files
	(cd $TIMIT ; find $DIR -type f -name "s[ix]*wav") | scripts/random_file_lines.pl - > config/$DIR.wav
	sed "s/wav$/mfc/" config/$DIR.wav | sed "s:^:$DATA/:" > config/$DIR.mfc

	# generate the acoutic feature vectors
	paste config/$DIR.wav config/$DIR.mfc | sed "s:^:$TIMIT/:" > config/$DIR.htk_scp
	$HTK/HCopy -C config/htk.config -S config/$DIR.htk_scp
	rm -f config/$DIR.htk_scp

	# generate frame labels from PHN files
	sed "s/wav$/phn/" config/$DIR.wav | while read infile ; do
		outfile=`echo $infile | sed 's/phn$/labels/'`
		#echo $TIMIT/$infile "-->" $DATA/$outfile
		scripts/phn2labels.pl $TIMIT/$infile -lm config/phoneme_map_timit61_to_leehon39 > $DATA/$outfile
	done
	sed "s/wav$/labels/" config/$DIR.wav | sed "s:^:$DATA/:" > config/$DIR.labels
	
done

# first 500 utterances are used for training the frame-based phoneme classifier
head -n 500 config/train.mfc > config/train_frame_based_classifier_500.mfc
head -n 500 config/train.labels > config/train_frame_based_classifier_500.labels

# alternatively, the first 1500 utterances can be used for training the
# frame-based phoneme classifier
head -n 1500 config/train.mfc > config/train_frame_based_classifier_1500.mfc
head -n 1500 config/train.labels > config/train_frame_based_classifier_1500.labels

# there are 3696 training files in TIMIT, so the rest are 2196 utterances
tail -n 2196 config/train.mfc > config/train_frame_based_classifier_1500_rest.mfc
tail -n 2196 config/train.labels > config/train_frame_based_classifier_1500_rest.labels

# scores are files which will contain the posterios, that is confidence of each
# phoneme per each frame (10 mec). Here we generate a list of scores file, and
# the script q2-train_frame_based_phoneme_classifier.sh will create each of the
# files.
sed "s/labels$/scores/" config/train_frame_based_classifier_1500_rest.labels > config/train_frame_based_classifier_1500_rest.scores
sed "s/labels$/scores/" config/test.labels > config/test.scores

# The rest of the 2196 training files are splitted as follows:

# the next 1796 files are used as a training set for forced alignment
tail -n 2196 config/train.mfc | head -n 1796 > config/train_forced_alignment_1796.mfc
tail -n 2196 config/train.labels | head -n 1796 > config/train_forced_alignment_1796.labels

# the next 400 files are used as a validation set for forced alignment
tail -n 400 config/train.mfc > config/train_val_forced_alignment_400.mfc
tail -n 400 config/train.labels  > config/train_val_forced_alignment_400.labels

# the next 150 files are used as a training set for forced alignment
tail -n 2196 config/train.mfc | head -n 150 > config/train_forced_alignment_150.mfc
tail -n 2196 config/train.labels | head -n 150 > config/train_forced_alignment_150.labels

# the next 100 files are used as a validation set for forced alignment
tail -n 2046 config/train.mfc | head -n 100 > config/train_val_forced_alignment_100.mfc
tail -n 2046 config/train.labels | head -n 100 > config/train_val_forced_alignment_100.labels

# the rest used for keyword spotting
tail -n 1946 config/train.mfc > config/train_keyword_spotting.mfc

# filter the main test set to get the core test set
FILTER='data/test/dr./[mf](DAB0|WBT0|ELC0|TAS1|WEW0|PAS0|JMP0|LNT0|PKT0|LLL0|TLS0|JLM0|BPM0|KLT0|NLP0|CMJ0|JDH0|MGD0|GRT0|NJM0|DHC0|JLN0|PAM0|MLD0)/s[ix]'
egrep -i $FILTER config/test.mfc > config/test_core.mfc
egrep -i $FILTER config/test.labels > config/test_core.labels

# compute feature statistics: mean and variance. The statistics is needed to 
# normalize the features to have mean of 0 and stdvar of 1.
$BIN/htk_stats config/train_frame_based_classifier_500.mfc > config/mfcc.stats


# generate cepstral distance files
cat config/train_frame_based_classifier_1500_rest.mfc | while read infile ; do
	outfile=`echo $infile | sed "s/mfc$/dist/"`
	$BIN/htk_ceps_dist $infile config/mfcc.stats $outfile
done

cat config/test.mfc | while read infile ; do
	outfile=`echo $infile | sed "s/mfc$/dist/"`
	$BIN/htk_ceps_dist $infile config/mfcc.stats $outfile
done

# generate the true alignment for forced alignemnt execution
cat config/train_frame_based_classifier_1500_rest.labels | while read infile ; do
	alignfile=`echo $infile | sed "s/labels$/start_times/"` 
	phonemefile=`echo $infile | sed "s/labels$/phonemes/"` 
	scripts/generate_true_alignment.pl $infile $alignfile $phonemefile
done

cat config/test.labels | while read infile ; do
	alignfile=`echo $infile | sed "s/labels$/start_times/"` 
	phonemefile=`echo $infile | sed "s/labels$/phonemes/"` 
	scripts/generate_true_alignment.pl $infile $alignfile $phonemefile
done

# generate statistics of phoneme duration (mean and std-dev)
sed -e "s:wav$:phn:" -e "s:^:$TIMIT/:" config/train.wav > config/train.phn
scripts/timit_phoneme_stats.pl config/train.phn config/phonemes_39.map config/phoneme_map_timit61_to_leehon39 > config/phonemes_39.stats

fi

