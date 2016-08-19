#!/usr/bin/perl -w

unless ($#ARGV+1 == 1) {
    print "Usage: $0 <input file>\n";
    print "       randomly reorder lines of file.\n";
    exit;
}

$input_file = $ARGV[0];

open(INPUT,"$input_file") or die "Can't open $input_file: $!\n";
@lines = <INPUT>;
close(INPUT);

fisher_yates_shuffle(\@lines);

print @lines;


# fisher_yates_shuffle( \@array ) : generate a random permutation
# of @array in place
sub fisher_yates_shuffle {
    my $array = shift;
    my $i;
    for ($i = @$array; --$i; ) {
        my $j = int rand ($i+1);
        next if $i == $j;
        @$array[$i,$j] = @$array[$j,$i];
    }
}

