#!/usr/bin/perl -w

my $signal_count = 0;
my $profit = 0;
while (<>) {
	if (/offset.+,(\d+)$/) {
		$profit = $1;
		++$signal_count;
	}
}

printf ("$signal_count, $profit\n");
