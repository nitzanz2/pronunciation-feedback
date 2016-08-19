#!/usr/bin/perl -w                                                                        

# usage : <input pred_align> <input start_times2> <output pred_align2>                                                                                   

# read true start time file and extrcat the begining and end silence times                
open(TRUE_ALIGN_IN,"$ARGV[1]") or die "Unable to open $ARGV[1]: $!\n";
$line = <TRUE_ALIGN_IN>; # read first line                                                
$line = <TRUE_ALIGN_IN>; # read second line                                               
chomp $line;
$first_phone_after_sil = $line;
while (<TRUE_ALIGN_IN>) {
    chomp;
    $line = $_;
}
close(TRUE_ALIGN_IN);
$last_sil = $line;
print "first=$first_phone_after_sil last=$last_sil\n";                         

open(PRED_ALIGN_IN, "$ARGV[0]") or die "Cannot open file $ARGV[0]: $!\n";
open(PRED_ALIGN_OUT, ">$ARGV[2]") or die "Cannot open file $ARGV[2]: $!\n";
$line=<PRED_ALIGN_IN>;
print PRED_ALIGN_OUT $line; # should be 0
print PRED_ALIGN_OUT $first_phone_after_sil."\n"; 
while (<PRED_ALIGN_IN>) {
	chomp;
	print PRED_ALIGN_OUT ($_+$first_phone_after_sil)."\n";
}
print PRED_ALIGN_OUT $last_sil."\n";
close(PRED_ALIGN_OUT);
close(PRED_ALIGN_IN);