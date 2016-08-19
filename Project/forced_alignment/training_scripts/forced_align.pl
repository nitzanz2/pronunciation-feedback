#!/usr/bin/perl -w

use File::Temp qw/ tempfile tempdir /;

# frame-base phoneme classifier parameters
$SIGMA="19";
$C="1";
$B="0.5";
$phoneme_frame_based_model="models/pa_phoeneme_frame_based.C_$C.B_$B.sigma_$SIGMA.model";

# forced-aligned classifier parameters
$beta1=0.01;
$beta2=1.0;
$beta3=1.0;
$min_sqrt_gamma=1.0;
$loss="alignment_loss";
$eps=3;
$rm_silence="";
$forced_alignment_model="models/forced_alignment.beta1_$beta1.beta2_$beta2.beta3_$beta3.gamma_$min_sqrt_gamma.eps_$eps.$loss.model";

unless ($#ARGV+1 == 3) {
	print "Usage: $0 <wavfile> <phonemes> <output>\n";
	print "     Given input TIMIT WAV file and list of phonemes, this scripts\n"; 
	print "     prints a list of phoneme start-times.\n";
	exit;
}

# load the location of HTK installation
open(HTK_LOCATION,"config/htk_location") or 
	die "Unable to open config/htk_location. Please generate a file with the path to HTK tools.\n";
$htk_location=<HTK_LOCATION>;
chomp $htk_location;
close(HTK_LOCATION);


my ($tmp_fh, $tmp_filename) = tempfile();
$mfc_filename = $tmp_filename.".mfc";
$dist_filename = $tmp_filename.".dist";
$scores_filename = $tmp_filename.".scores";

$mfc_filelist = $tmp_filename.".mfc_list";
$cmd="echo $mfc_filename > $mfc_filelist";
system($cmd);
$dist_filelist = $tmp_filename.".dist_list";
$cmd="echo $dist_filename > $dist_filelist";
system($cmd);
$scores_filelist = $tmp_filename.".scores_list";
$cmd="echo $scores_filename > $scores_filelist";
system($cmd);

# first extract features
$cmd="$htk_location/HCopy -C config/htk.config $ARGV[0] $mfc_filename";
print "$cmd\n";
system($cmd);
$cmd="bin/htk_ceps_dist $mfc_filename config/mfcc.stats $dist_filename";
#print "$cmd\n";
system($cmd);
$cmd="bin/PhonemeFrameBasedDecode -n 1 -kernel_expansion rbf3 -sigma $SIGMA -mfcc_stats config/mfcc.stats -averaging -scores $scores_filelist $mfc_filelist null config/phonemes_40 $phoneme_frame_based_model";
#print "$cmd\n";
system($cmd);

$pred_align_filelist=$tmp_filename.".pred_align_list";
#$pred_align_filename=$tmp_filename.".pred_align";
$cmd="echo $ARGV[2] > $pred_align_filelist";
system($cmd);

$phoneme_filelist=$tmp_filename.".phoneme_filelist";
$cmd="echo $ARGV[1] > $phoneme_filelist";
system($cmd);
$cmd="bin/ForcedAlignmentDecode $rm_silence -beta1 $beta1 -beta2 $beta2 -beta3 $beta3 -output_align $pred_align_filelist $scores_filelist $dist_filelist $phoneme_filelist null config/phonemes_39 config/phonemes_39.stats $forced_alignment_model";
print "$cmd\n";
system($cmd);


