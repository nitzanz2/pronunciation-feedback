#!/usr/bin/perl -w

unless ($#ARGV+1 == 3) {
	print "Usage: $0 labels(input) start-times(output) phonemes(output) \n";
	print "     Given input frame-based phoeneme labels, this scripts\n"; 
	print "     generates two files: start-times file, which is the start-\n"; 	
	print "     time of each phoneme (the alignment), and phonemes file, \n";
	print "     which is list of phonemes.\n";
	print "     Labels file is a text file containing the labels of each \n";
	print "     frames, e.g.:\n";
	print "        sil\n";
	print "        sil\n";
	print "        sil\n";
	print "        dh\n";
	print "        dh\n";
	print "        dh\n";
	print "        ...\n";
	print "     The output start-times file is a text file, which after the\n";
	print "     execution will be:\n";
	print "        1\n";
	print "        4\n";
	print "        ...\n";
	print "     Similarly, the output phonemes file is a text file, which\n";
	print "     will be\n";
	print "        sil\n";
	print "        dh\n";
	print "        ...\n";
	print "     \n";
	exit;
}

$labels_file = $ARGV[0];
$align_file = $ARGV[1];
$phoneme_file = $ARGV[2];

$num_phonemes = 0;
$frames = 0;
$last_label = '';
@phonemes = ();
@align = ();
open(LABELSFILE, "$labels_file") or die "Cannot open file $labels_file: $!\n";
while (<LABELSFILE>) { 
	chomp;
	if ($_ ne $last_label) {
		$phonemes[$num_phonemes] = $_;
		$align[$num_phonemes] = $frames;
		$num_phonemes++;
		$last_label = $_;
	}
	$frames++;
}
close(LABELSFILE);

open(PHONEMEFILE, ">$phoneme_file") or die "Cannot open file $phoneme_file: $!\n";
foreach (@phonemes) {
	print PHONEMEFILE $_,"\n";
}
close(PHONEMEFILE);

open(ALIGNFILE, ">$align_file") or die "Cannot open file $align_file: $!\n";
foreach (@align) {
	print ALIGNFILE $_,"\n";
}
close(ALIGNFILE);
