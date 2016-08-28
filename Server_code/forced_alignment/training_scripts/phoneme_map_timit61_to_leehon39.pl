#!/usr/bin/perl -w

# load map
$map_filename = "config/phoneme_map_timit61_to_leehon39";
%map = ();
open(PHONEMES_MAP, $map_filename) or die "Can't open $map_filename:$!\n";
while (<PHONEMES_MAP>) {
	chomp;
	($timit61_code,$leehon39_code) = split;
	$map{$timit61_code} = $leehon39_code;
}
close(PHONEMES_MAP);

while (<>) {
	chomp;
	@phonemes = split;
	foreach (@phonemes) {
		print $map{$_}," ";
	}
}