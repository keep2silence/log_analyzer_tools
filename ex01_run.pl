#!/usr/bin/perl -w

use strict;

my $prob = 0.4;
my $count = 1;

for ($prob = 0.4; $prob <= 0.6; $prob = $prob + 0.02) {
	$count = 1;
	for (; $count <= 5; ++$count) {
		`mkdir out_${count}_${prob}`;
		printf ("./ex01 a.csv $count $prob\n");
		`./ex01 TestCase.170410.172932.log.exe.csv $count $prob > runlog_${count}_${prob}.log`;
		`mv 2016*.csv runlog_${count}_${prob}.log out_${count}_${prob}`;
		`mv out_stat.csv > out_${count}_${prob}/profit.csv`;
	}
}
