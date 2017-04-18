#!/usr/bin/perl -w

use strict;

my $sum_profit = 0;
my $sum_signal = 0;

my $first_line = 0;
while (<>) {
	if ($first_line == 0) {
		$first_line = 1;
		next;
	}

	chomp;
	my @all = split /,/;
	$sum_profit += $all[2];
	$sum_signal += $all[1];
}

printf ("signal: $sum_signal, profit: $sum_profit\n");
