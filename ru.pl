#!/usr/bin/perl -w

use strict;

my $UP_SIGNAL_PROB = 7;
my $DOWN_SIGNAL_PROB = -3;

my $UP_OFFSET_PROB = 2.5;
my $DOWN_OFFSET_PROB = -2.5;

my $first_line = 0;
my $net_posi = 0;

my $buy_open_price = 0;
my $sell_open_price = 0;
my $profit = 0;
my $sum_profit = 0;

while (<>) {
	if ($first_line == 0) {
		$first_line = 1;
		print ($_);
		next;
	}

	chomp;
	my @all = split /,/;

	my $line = $_;	

	if ($all[15] >= $UP_OFFSET_PROB) {
		if ($net_posi < 0) {
			$profit = $sell_open_price - $all[11];	
			$sum_profit += $profit;
			$line = $line . ",$all[11]up_offset,$profit,$sum_profit"; 
			++$net_posi;
		}
	}

	if ($all[15] <= $DOWN_OFFSET_PROB) {
		if ($net_posi > 0) {
			$profit = $all [9] - $buy_open_price;	
			$sum_profit += $profit;
			$line = $line . ",$all[9]down_offset,$profit,$sum_profit";
			--$net_posi;
		}
	}

	if ($all[15] >= $UP_SIGNAL_PROB) {
		$line = "$line" . ",1," . "$all[11]";
		++$net_posi;
		$buy_open_price = $all[11];
	}

	if ($all[15] <= $DOWN_SIGNAL_PROB) {
		$line = "$line" . ",-1," . "$all[9]";
		--$net_posi;
		$sell_open_price = $all[9];
	}
	
	printf ("$line\n");
}
