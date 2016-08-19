#!/usr/bin/perl -w

# This scripts is implemented for the TIMIT dictionary

# TIMIT location
$timit_dir="/Databases/timit";
#$timit_dir="/share/data/speech/Datasets/timit";

# dictionary Parameters
$dict_sil_symbol = "h#";
$dict_filename = "${timit_dir}/doc/timitdic.txt";
$dict_comment = ";";
$dict_phone_sep = "/";
$phone_map = "config/phoneme_map_timit61_to_leehon39";

# parse command line parameters
unless ($#ARGV+1 == 2) {
	# strip program name to basename
	($progname = $0) =~ s!^.*/!!;
	print "Usage: $progname <input word sequence> <phone pronunciations sequences>\n";
	exit;
}
$input_word_filename=$ARGV[0];
$output_phonemes_filename=$ARGV[1];

# read dictionary
%dict = ();
open(DICT, "$dict_filename") or die "Cannot open file $dict_filename: $!\n";
while (<DICT>) {
	chomp;                  # no newline
	s/${dict_comment}.*//;  # no comments
	s/^\s+//;               # no leading white
	s/\s+$//;               # no trailing white
	s/\d//g;                # ignore stress indication
	next unless length;     # anything left?
	($word,$phoneme_str) = split(/${dict_phone_sep}/, $_);
	my ($word,$rest) = split(/~/,$word); # ignore part-of-speech tags
	$word =~ s/^\s+//;               # no leading white
	$word =~ s/\s+$//;               # no trailing white
	#print "$word /$phoneme_str/\n";
	push ( @{$dict{$word}}, $phoneme_str);
}
close(DICT);
push ( @{$dict{"--"}}, "h#");
# test dictionary for multiple values
if (0) {
	foreach $pron (sort @{$dict{"project"}}) {
		print "-->>$pron\n";
	}
	foreach $word (sort keys %dict) {
		print "--$word-- @{$dict{$word}}\n";
	}
}

# read phoneme symbol mapping if the phoneme map file can be found
%map = ();
if (-e $phone_map) {
	open(LABELS_MAP, $phone_map) or die "Can't open $phone_map:$!\n";
	while (<LABELS_MAP>) {
		chomp;
		($label1,$label2)=split(" ");
		$map{$label1} = $label2;
	}
	close(LABELS_MAP);
	$map_file_read = 1;
}
else {
	print "Could not find phoneme map file.\n";
	$map_file_read = 0;
}
# add deleted symbol
$map{"#"} = "";

# Read text line
open(TIMIT_TXT_IN,"$input_word_filename") or die "Unable to open $input_word_filename: $!\n";
$line = <TIMIT_TXT_IN>;
close(TIMIT_TXT_IN);
chomp $line;
my @words = split(/ /, $line);
if ($words[0] =~ /\D/ && $words[1] =~ /\D/) {
	$timit_format = 0;
} 
else {
	$timit_format = 1 ;
	splice(@words, 0, 2);
}

@prons = (); $prons[0] = "";
# run over all words, and extract all entries in the dictionary
foreach $word (@words) {
	# convert word to lowercase 
	$word = lc($word);
	# remove . and ,
	$word =~ s/(\,)|(\.)//;
	# look up in the dictionary
	if (!defined($dict{$word})) {
		print "The word /$word/ could not be found in the dictionary.\n";
	} 
	else {
		# extract all pronunciation of the word from the dictionary
		@word_prons = @{$dict{$word}};
		@new_prons = (); 
		foreach $word_pron (@word_prons) {
			foreach $pron (@prons) {
				$new_pron = $pron.$word_pron." ";
				push (@new_prons, $new_pron);
			}
		}
		@prons = @new_prons;
		# IMPLEMET HERE: add optional silence between words???
	}	
}

# phonetical rules
foreach $pron (@prons) {
	# add silence symbol before and after each pronunciation
	$pron = $dict_sil_symbol." ".$pron.$dict_sil_symbol;
	# add closure before plosive	
	$pron =~ s/\b(b|d|g|k|p|t)\b/$1cl $1/g;
	$pron =~ s/(jh)/dcl $1/g;
	$pron =~ s/(ch)/tcl $1/g;
	#print $pron,"\n";
}

# rewrite rules: Morgan's rule:	xcl x ycl y --> xcl y
@new_prons = ();
foreach $pron (@prons){
	@phones = split(/ /,$pron);
	@matches_start_phone = ();
	$num_matches = 0;
	for ($i = 0; $i < scalar(@phones)-3; $i++) {
		if ($phones[$i] =~ /\wcl/ && 
			$phones[$i+1] =~/\w/ && 
			$phones[$i+2] =~ /\wcl/ && 
			$phones[$i+3] =~ /\w/) {
			$matches_start_phone[$num_matches] = $i;
			#print $matches_start_phone[$num_matches],"\n";
			$num_matches++;
		}
	}
	for ($i=0; $i < 2**$num_matches ; $i++) {
		#print "** $i ",dec2bin($i,$num_matches),"\n";
		@which_match_is_on = split(//, dec2bin($i,$num_matches));
		@new_phones = @phones;
		for ($j=0; $j < $num_matches; $j++) {
			if ($which_match_is_on[$j]) {
				# xcl x ycl y --> xcl # # y
				$new_phones[$matches_start_phone[$j]] = $new_phones[$matches_start_phone[$j]];
				$new_phones[$matches_start_phone[$j]+1] = "#"; # to be deleted
				$new_phones[$matches_start_phone[$j]+2] = "#"; # to be deleted
				$new_phones[$matches_start_phone[$j]+3] = $new_phones[$matches_start_phone[$j]+3];
			}
		}
		push(@new_prons, join(" ", @new_phones));
	}

}
@prons = @new_prons;

# phoneme mapping and printing to file
open(PHONEMES_OUT,">$output_phonemes_filename") or die "Unable to open $output_phonemes_filename: $!\n";
foreach $pron (@prons){
	if ($map_file_read) {
		# split pronunciation string into phones
		@in_phones = split(/ /,$pron);
		@out_phones = ();
		foreach $in_phone (@in_phones) {
			if (!defined($map{$in_phone})) {
				print "The phone /$in_phone/ could not be found in the map file.\n";
			} 
			else {
				push (@out_phones, $map{$in_phone});
			}
		}
		# join mapped phones back to a string
		print PHONEMES_OUT join(' ', @out_phones),"\n";	
	}	
	else {
		# no mapping is available - just print using the dictionary phoneme symbols
		print PHONEMES_OUT	$pron,"\n";
	}

}

sub dec2bin {
	my ($dec, $size) = @_;
  my $str = unpack("B32", pack("N", $dec));
  #$str =~ s/^0+(?=\d)//;   # otherwise you'll get leading zeros
  $end = substr($str, -$size);
	return $end;
}
