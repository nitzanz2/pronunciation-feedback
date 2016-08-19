#!/usr/bin/perl -w

unless ($#ARGV+1 >= 1) {
    print "Usage: $0 phn-filename [-s frame_shift] [-l frame_length] \n";
    print "       [-delta] [-lm label-map-file]\n";
    print " \n";
    print "       Outputs a list of labels from a given TIMIT PHN file.\n";
    exit;
}

# default parameters
$frame_shift = 160;
$frame_length = 400;
$multiframes = 1;
$use_delta = 0;
$label_map_file = "";

# parse command line
$filename = $ARGV[0];
$i = 1;
while ($i < $#ARGV+1) {
	if ($ARGV[$i] eq '-s') {
		$frame_shift = $ARGV[$i+1];
		$i += 2;
	}  elsif ($ARGV[$i] eq '-l') {
	$frame_length = $ARGV[$i+1];
	$i += 2;
    }  elsif ($ARGV[$i] eq '-m') {
	$multiframes = $ARGV[$i+1];
	$i += 2;
    } elsif ($ARGV[$i] eq "-delta") {
	$use_delta = 1;
	$i++;
    } elsif ($ARGV[$i] eq "-lm") {
	$label_map_file = $ARGV[$i+1];
	$i += 2;
    } else {
	die "Unknown parameter $ARGV[$i]\n";
    }
}

# read PHN file
open(PHNFILE, $filename) or die "Cannot open file $filename: $!\n";
$i = 0;
while (<PHNFILE>) {
    chomp;
    ($p_start[$i], $p_stop[$i], $p_code[$i]) = split;
    $i++;
}
close(PHNFILE);
$num_phns = scalar(@p_code);
## print STDERR "num phns = $num_phns \n";

# read label map file into hash array
if (-f $label_map_file) {
    %map = ();
    open(LABELS, $label_map_file) or die "Can't open $label_map_file:$!\n";
    while (<LABELS>) {
	chomp;
	($label1,$label2)=split(" ");
	$map{$label1} = $label2;
    }
    close(LABELS);
}


# determine corresponding WAV file size
$num_samples = wav_file_length($filename);
## print STDERR "num samples = $num_samples\n";

# start working on frames
$multiframe_length = $multiframes*$frame_length;
$num_frames = int(($num_samples-$frame_length)/$frame_shift)+1;
$num_multiframes = $num_frames - ($multiframes - 1);

$first_multiframe_index = 0;
$last_multiframe_index = $num_multiframes - 1;
if ($use_delta) {
    $first_multiframe_index = 4;
    $last_multiframe_index = $num_multiframes - 1 - 4;
}
##  print STDERR "num multiframes = "
##      .($last_multiframe_index-$first_multiframe_index+1)."\n";

for ($i = $first_multiframe_index; $i <= $last_multiframe_index; $i++) {

    $start = $i*$frame_shift;
    $stop = $start + $multiframe_length;

    # search for the first phoneme that overlaps with this area 
    for ($j=0; $j < $num_phns && $p_stop[$j] <= $start; $j++) { };
    ###print "j=$j start=$start stop=$stop mid=".(($stop-$start)/2.0+$start).
    ###      " p_stop[".$j."]=".$p_stop[$j]." ";
 
    # if off either end of the record, return the silence code
    if ($j==$num_phns || $p_start[0]>$start || $p_stop[$num_phns-1] < $stop) {
	$index = "h#";
    } else {
	# and search for the last phoneme that overlaps with this area
	$first = $j;
	for ($k = $first; $k < $num_phns && $p_start[$k] < $stop; $k++) { };
	if ($p_start[$k-1] >= $stop) { # the last phoneme
	    $last = $num_phns;
	} else {
	    $last = $k-1;
	}
	if ($last - $first < 0) {
	    print "On the face :-( \n";
	} elsif ($last - $first == 0) {
	    # there is a single segment that overlaps with this time 
	    $index = $p_code[$first];
	    ###print "1 ";
	} elsif ($last - $first == 1) {
	    # there are two segments which overlap, choose the largest
	    if($p_stop[$first] - $start > $stop - $p_start[$last]) {
		$index = $p_code[$first];
		###print "2 [".($p_stop[$first]-$start)."-".
		###    ($stop-$p_start[$last])."] ";
	    } else {
		$index = $p_code[$last]; 
		###print "2 [".($stop-$p_start[$last])."-".
		###    ($p_stop[$first]-$start)."] ";
	    }
	} else {
	    # there are three or more segments which overlap 
	    $index = $p_code[$first + 1];
	    ###print "3 ";
	}
    }
    if (-f $label_map_file) {
	print $map{$index}."\n";
    } else {
	print "$index\n";
    }
}

sub wav_file_length { # phn-filename
    my ($filename) = @_;
    $filename =~ s/phn/wav/;
    $filename =~ s/PHN/WAV/;
    open(WAVFILE,, $filename) or die "Cannot open file $filename: $!\n";
    $num_bytes = -s WAVFILE;
    close(WAVFILE);
    $num_bytes -= 1024; # remove header size
    return ($num_bytes/2); # return number of samples (shorts) in file
}

