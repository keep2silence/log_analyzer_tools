#!/usr/bin/perl -w
use strict;

if (@ARGV == 0) {
	printf ("./ru.pl open_valve offset_valve max_posi show_detail signal_file\n");
	printf ("show_detail: 0 -- 只统计最后的信号个数和总盈亏\n");
	exit;
}

my $UP_SIGNAL_PROB = $ARGV[0];
my $DOWN_SIGNAL_PROB = 0 - $UP_SIGNAL_PROB;
my $UP_OFFSET_PROB = $ARGV[1];
my $DOWN_OFFSET_PROB = 0 - $UP_OFFSET_PROB;
my $max_posi = $ARGV[2];
my $show_detail = $ARGV[3];

# printf ("$UP_SIGNAL_PROB, $DOWN_SIGNAL_PROB, $UP_OFFSET_PROB, $DOWN_OFFSET_PROB, $max_posi, $show_detail\n");
# exit;

my $first_line = 0;
my $net_posi = 0;

my @buy_open_prices;
my @sell_open_prices;
my $buy_open_price = 0;
my $sell_open_price = 0;
my $profit = 0;
my $sum_profit = 0;
my $signal_count = 0;

my $signal_file = $ARGV[4];

# printf ("|$signal_file|\n");
open (FILE, "$signal_file") or die open "$signal_file";

while (<FILE>) {
	if ($first_line == 0) {
		$first_line = 1;
		#print ($_);
		next;
	}

	chomp;
	my @all = split /,/;

	my $line = $_;	

	if ($all[15] >= $UP_OFFSET_PROB) {
		if ($net_posi < 0) {
			# first open, first offset
			$sell_open_price = shift @sell_open_prices;
			# printf ("sell_open_price: $sell_open_price\n");
			$profit = $sell_open_price - $all[11];	
			$sum_profit += $profit;
			$line = $line . ",$all[11]up_offset$sell_open_price,$profit,$sum_profit"; 
			++$net_posi;
		}
	}

	if ($all[15] <= $DOWN_OFFSET_PROB) {
		if ($net_posi > 0) {
			$buy_open_price = shift @buy_open_prices;
			#printf ("buy_open_price: $buy_open_price\n");
			$profit = $all [9] - $buy_open_price;	
			$sum_profit += $profit;
			$line = $line . ",$all[9]down_offset$buy_open_price,$profit,$sum_profit";
			--$net_posi;
		}
	}

	if ((@buy_open_prices < $max_posi) && ($all[15] >= $UP_SIGNAL_PROB)) {
		$line = "$line" . ",1," . "$all[11],$sum_profit";
		++$net_posi;
		++$signal_count;
		$buy_open_price = $all[11];
		push @buy_open_prices, $buy_open_price;
	}

	if ((@sell_open_prices < $max_posi) && ($all[15] <= $DOWN_SIGNAL_PROB)) {
		$line = "$line" . ",-1," . "$all[9],$sum_profit";
		--$net_posi;
		++$signal_count;
		$sell_open_price = $all[9];
		push @sell_open_prices, $sell_open_price;
	}
	
	if ($show_detail != 0) {
		printf ("$line\n");
	}
}
close FILE;
printf ("$signal_count,$sum_profit\n");
