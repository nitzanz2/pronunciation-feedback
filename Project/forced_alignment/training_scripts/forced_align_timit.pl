#!/usr/bin/perl -w

use File::Temp qw/ :POSIX /;;

unless ($#ARGV+1 == 5) {
	# strip program name to basename
	($progname = $0) =~ s!^.*/!!;
	print "Usage: $progname <speech wav> <coresp. scores> <phonemes sequences> <output confedences> <output best phn>\n";
	print "\n";
	exit;
}

# file names
$tempname = tmpnam();
print "temp_name= $tempname\n";
$fake_start_times = $tempname.".fake_start_times";
$score_filelist = $tempname.".score_filelist";
$dist_filelist = $tempname.".dist_filelist";
$start_time_filelist = $tempname.".start_time_filelist";
$phonemes_filelist = $tempname.".phonemes_filelist";
$pred_align_filelist = $tempname.".pred_align_filelist";

# read true start time file and extrcat the begining and end silence times
($true_start_time_filename = $ARGV[1]) =~ s/.scores/.start_times/;
open(TRUE_ALIGN_IN,"$true_start_time_filename") 
	or die "Unable to open $true_start_time_filename: $!\n";
$line = <TRUE_ALIGN_IN>; # read first line
$line = <TRUE_ALIGN_IN>; # read second line
chomp $line;
$first_after_sil = $line;
while (<TRUE_ALIGN_IN>) {
	chomp;
	$line = $_;
} 
close(TRUE_ALIGN_IN);
$last_sil = $line;
print "first=$first_after_sil last=$last_sil\n";

# generate filelists
open(PRONS,"$ARGV[2]") or die "Unable to open phoneme pronunciations file $ARGV[2]: $!\n";
$pron_num = 0;
while (<PRONS>) { # read all pronunciations
	$phones_filename = $tempname.".phonemes.".$pron_num;
	system("echo $phones_filename >> $phonemes_filelist");
	# print phonemes to a file
	@phones = split(/\s+/, $_);
	open(PHONES, ">$phones_filename") or die "Unable to open $phones_filename for writing: $!\n";
	foreach $p (@phones) {
		print PHONES "$p\n";
	}
	close(PHONES);
	# fake start times for removal of silence
	$fake_start_times_filename = $tempname.".start_times.".$pron_num;
	system("echo $fake_start_times_filename >> $start_time_filelist");
	fake_start_times($fake_start_times_filename, scalar(@phones), $first_after_sil, $last_sil);
	# dist filename
	($dist_filename = $ARGV[1]) =~ s/.scores/.dist/;
	system("echo $dist_filename >> $dist_filelist");
	system("echo $ARGV[1] >> $score_filelist");
	$pred_align_filename = $tempname.".pred_align.".$pron_num;
	system("echo $pred_align_filename >> $pred_align_filelist");
	$pron_num++;
}
close(PRONS);

# Now align with confedence
$beta1='0.01';
$beta2='1.0';
$beta3='1.0';
$min_sqrt_gamma='1.0';
$loss="alignment_loss";
$eps=3;
$rm_silence = " -remove_silence";
$exp_name="forced_alignment.beta1_$beta1.beta2_$beta2.beta3_$beta3.gamma_$min_sqrt_gamma.eps_$eps.$loss";

$confidences = $ARGV[3];
$cmd = "bin/ForcedAlignmentDecode $rm_silence -beta1 $beta1 -beta2 $beta2 -beta3 $beta3 -output_align $pred_align_filelist -output_confidence $confidences $score_filelist $dist_filelist $phonemes_filelist $start_time_filelist config/phonemes_39 config/phonemes_39.stats models/$exp_name.model";

print "$cmd\n";
system($cmd);

# find the largest confidence
open (CONFIDENCES, "$confidences") or die "Problem openning $confidences for reading: $!\n";
$max_value = 0; $max_index = 0;
$index = 0;
foreach (<CONFIDENCES>) {
	chomp;
	if ($_ > $max_value) {
		$max_value = $_;
		$max_index = $index;
	}
	$index++;
}
close(CONFIDENCES);
print "max=$max_value max_index=$max_index\n";

# read best alignment
@best_pred_align = ();
$best_pred_align[0] = 0;
$best_pred_align[1] = $first_after_sil;
$index = 2;
open (BEST_PRED_ALIGN, "$tempname.pred_align.$max_index") 
	or die "Problem openning $tempname.pred_align.$max_index: $!\n";
$line = <BEST_PRED_ALIGN>; # fiest start time is zero
foreach (<BEST_PRED_ALIGN>) {
	chomp;
	$best_pred_align[$index] = $_ + $first_after_sil;
	$index++;
}
$best_pred_align[$index] = $last_sil;
close(BEST_PRED_ALIGN);

# read best phoneme
@best_phonemes = ();
$index = 0;
open (BEST_PHONEMES, "$tempname.phonemes.$max_index") 
or die "Problem openning $tempname.phonemes.$max_index: $!\n";
while (<BEST_PHONEMES>) {
	chomp;
	$best_phonemes[$index] = $_;
	$index++;
}

#oputput phn file
$best_phn_filename = $ARGV[4];
open(OUTPHN, ">$best_phn_filename") or die "Unable to open $best_phn_filename for writing: $!\n";
for ($i=0; $i < scalar(@best_phonemes); $i++) {
	print OUTPHN (160*$best_pred_align[$i]), " ", $best_phonemes[$i], "\n";
}
close(OUTPHN);


sub fake_start_times {
	my ($filename,$num_labels,$first_after_sil,$last_sil) = @_;

	open(FAKE_ALIGN_OUT,">$filename") or die "Unable to open $filename: $!\n";
	for ($i=0; $i < $num_labels; $i++) {
		if ($i == 1) {
			print FAKE_ALIGN_OUT "$first_after_sil\n";
		}
		elsif ($i == ($num_labels-1)) {
			print FAKE_ALIGN_OUT "$last_sil\n";
		}
		else {
			print FAKE_ALIGN_OUT "0\n";
		}
	}
	close(FAKE_ALIGN_OUT);
}