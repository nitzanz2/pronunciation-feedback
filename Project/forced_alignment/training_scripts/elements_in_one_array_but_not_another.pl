#!/usr/bin/perl -w

unless (scalar(@ARGV) == 2) {
    print "Usage: $0 [array A] [array B]\n";
    print "       find elements in A that aren't in B\n";
    exit;
}

@A = ();
open(FILE_A,"$ARGV[0]") or die "Error: unable to open $ARGV[0]: $!\n";
while (<FILE_A>) {
    chomp;
    push @A, $_;
}

@B = ();
open(FILE_B,"$ARGV[1]") or die "Error: unable to open $ARGV[1]: $!\n";
while (<FILE_B>) {
    chomp;
    push @B,$_;
}

my %seen; # lookup table
my @aonly;# answer

# build lookup table
    @seen{@B} = ();

foreach $item (@A) {
    push(@aonly, $item) unless exists $seen{$item};
}

foreach $item (@aonly) {
    print "$item\n";
}
