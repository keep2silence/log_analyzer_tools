#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <list>
#include <map>
#include <string>

#include "PriceLevel.h"

/// 前置日志经过其他程序过滤后，已经是单客户单合约的所有交易。
/// 本程序进行进一步的分析，此程序是人工单步执行，不考虑效率
static bool match (std::string &line, std::string key)
{
	std::string::size_type pos = line.find (key);
	if (pos != std::string::npos) {
		return true;
	}

	return false;
}

static void split_to_vector (std::string line, std::vector<std::string> &stdvec)
{
    std::string::size_type pos;
    while ((pos = line.find_first_of ('-')) != std::string::npos) {
        std::string str = line.substr (0, pos);
        stdvec.push_back (str);
        line = line.substr (pos + 1);
    }
    stdvec.push_back (line);
}


static void parse_for_order_rsp (std::string& line, Order *pOrder)
{
	std::vector<std::string> vec;
	vec.reserve (16);
	split_to_vector (line, vec);
	
	std::string sysno_str (vec[3]);
	std::string flag_str  (vec[9]);
	const char *pflag = flag_str.c_str ();
	std::string price_str (vec[10]);
	std::string vol_str (vec[11]);

	pOrder->sysno = atoi (sysno_str.substr (1, 8).c_str ());
	/// T321 ， 3代表了是卖方向（1是买方向）， 2代表了平仓 (1是开仓)
	pOrder->bsflag = (BSFLAG)(*(pflag + 1));
	pOrder->offlag = (OFFLAG)(*(pflag + 2));
	pOrder->contract = vec[5].substr (1); 
	pOrder->price = atof (price_str.substr (1).c_str ());
	pOrder->vol   = atoi (vol_str.substr (1).c_str ());
}


static void parse_for_match_rsp (std::string& line, Match *pMatch)
{
	std::vector<std::string> vec;
	vec.reserve (16);
	split_to_vector (line, vec);
	
	std::string sysno_str (vec[3]);
	std::string flag_str  (vec[8]);
	const char *pflag = flag_str.c_str ();
	std::string price_str (vec[9]);
	std::string vol_str (vec[10]);

	pMatch->sysno = atoi (sysno_str.substr (1, 8).c_str ());
	/// T321 ， 3代表了是卖方向（1是买方向）， 2代表了平仓 (1是开仓)
	pMatch->bsflag = (BSFLAG)(*(pflag + 1));
	pMatch->offlag = (OFFLAG)(*(pflag + 2));
	pMatch->contract = vec[5].substr (1); 
	pMatch->price = atof (price_str.substr (1).c_str ());
	pMatch->vol   = atoi (vol_str.substr (2).c_str ());
}

static void parse_for_cancel_rsp (std::string& line, Cancel *pCancel)
{
	std::vector<std::string> vec;
	vec.reserve (16);
	split_to_vector (line, vec);
	
	std::string batchno_str (vec[3]);
	std::string vol_str (vec[7]);

	pCancel->batchno = atoi (batchno_str.substr (1, 8).c_str ());
	/// T321 ， 3代表了是卖方向（1是买方向）， 2代表了平仓 (1是开仓)
	pCancel->contract = vec[4].substr (1); 
	pCancel->vol   = atoi (vol_str.substr (2).c_str ());
}

int
main (int argc, char *argv[])
{
	if (argc != 2) {
		std::cout << "usage: ./front_log_analyzer logfile" << std::endl;
		return 0;
	}

	PriceLevelMap plm;
	std::fstream fs (argv[1]);
	std::string line, ignored_str;
	int line_count = 0;
	while (std::getline (fs, line)) {
		/// 不分析定单请求，因为没有系统号
		if (match (line, std::string ("定单应答")) == true) {
			Order *pOrder = new Order;
			parse_for_order_rsp (line, pOrder);
			std::map<double, PriceLevel *, price_functor>::iterator iter =
				plm.pricelevel_map.find (pOrder->price);			
			if (iter == plm.pricelevel_map.end ()) {
				PriceLevel *ppl = new PriceLevel;
				ppl->price = pOrder->price;
				if (pOrder->bsflag == BUY) {
					if (pOrder->offlag == OPEN) {
						ppl->buy_vol += pOrder->vol;
					} else if (pOrder->offlag == OFFSET) {
						ppl->buy_vol -= pOrder->vol;
					} else {
						abort ();
					}
				} else if (pOrder->bsflag == SELL) {
					if (pOrder->offlag == OPEN) {
						ppl->sell_vol += pOrder->vol;
					} else if (pOrder->offlag == OFFSET) {
						ppl->sell_vol -= pOrder->vol;
					} else {
						abort ();
					}
				} else {
					abort ();
				}
				plm.pricelevel_map.insert (std::make_pair (pOrder->price, ppl));
			} else {
				PriceLevel *ppl = iter->second;
			}

			std::cin >> ignored_str;
			continue;
		}

		if (match (line, std::string ("撤单应答")) == true) {
			Cancel *pCancel = new Cancel;
			parse_for_cancel_rsp (line, pCancel);

			std::cin >> ignored_str;
			continue;
		}

		if (match (line, std::string ("成交通知")) == true) {
			Match *pMatch = new Match;
			parse_for_match_rsp (line, pMatch);

			std::cin >> ignored_str;
			continue;
		}
	}
	
	std::cout << "删除重复回报前一共有" << line_count << "条日志." << std::endl;
}
