#!/usr/bin/perl -w

use strict;

my $PROG="./ru.pl";

my $open_valve = 4;
my $offset_valve = 2.5;

my $signal_file = $ARGV[0];
my $BaseName;
if ($ARGV[0] =~ /(\d+)/) {
	$BaseName = $1;
}
mkdir $BaseName;

for (; $open_valve <= 7.02; $open_valve = $open_valve + 0.2) {
	for (; $offset_valve <= 3.52; $offset_valve += 0.1) {
		# printf ("$PROG $open_valve $offset_valve 1 0 $signal_file\n");
		`$PROG $open_valve $offset_valve 1 0 $signal_file > $BaseName/${signal_file}_${open_valve}_${offset_valve}_1.csv`;
		# print "$PROG $open_valve $offset_valve 1 0 $signal_file > $BaseName/${signal_file}_${open_valve}_${offset_valve}_1.csv\n";
	}
}
