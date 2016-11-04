#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <list>
#include <map>
#include <string>

#include "PriceLevel.h"
#include <unistd.h>

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

static void split_to_vector (std::string line, std::vector<std::string> &stdvec, char sep)
{
    std::string::size_type pos;
    while ((pos = line.find_first_of (sep)) != std::string::npos) {
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
	split_to_vector (line, vec, '-');
	
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

	std::string s (vec[2]);
	vec.clear ();
	split_to_vector (s, vec, '|');
	pOrder->time  = vec[0].substr (3);
}

static void parse_for_match_rsp (std::string& line, Match *pMatch)
{
	std::vector<std::string> vec;
	vec.reserve (16);
	split_to_vector (line, vec, '-');
	
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
	split_to_vector (line, vec, '-');
	
	std::string batchno_str (vec[3]);
	std::string vol_str (vec[7]);

	pCancel->batchno = atoi (batchno_str.substr (1, 8).c_str ());
	/// T321 ， 3代表了是卖方向（1是买方向）， 2代表了平仓 (1是开仓)
	pCancel->contract = vec[4].substr (1); 
	pCancel->vol   = atoi (vol_str.substr (2).c_str ());
}

static void show_all (std::map<int, Order *>& order_map, PriceLevelMap& plm)
{
	std::cout << "------------ Order Info ------------" << std::endl;
	std::multimap<double, Order *, price_functor> price_order_map;
	std::map<int, Order *>::iterator iter = order_map.begin ();
	/// 按照价格进行排序
	for (; iter != order_map.end (); ++iter) {
		Order *pOrder = iter->second;
		price_order_map.insert (std::make_pair (pOrder->price, pOrder));
	}

	std::map<double, Order *, price_functor>::reverse_iterator riter = 
		price_order_map.rbegin ();
	for (; riter != price_order_map.rend (); ++riter) {
		Order *pOrder = riter->second;
		std::string dir (pOrder->bsflag == BUY ? "BUY" : "SELL");
		std::string of  (pOrder->offlag == OPEN ? "OPEN" : "OFFSET");
		std::cout << pOrder->time << ", sysno: " << pOrder->sysno 
			<< " price: " << pOrder->price 
			<< ", bs: " << dir << ", of: " << of 
			<< ", vol: " << pOrder->vol << std::endl;
	}


	std::cout << "------------ Posi Info ------------" << std::endl;
	std::map<double, PriceLevel *, price_functor>::iterator piter = 
		plm.pricelevel_map.begin ();
#if 0
	for (; piter != plm.pricelevel_map.end (); ++piter) {
		PriceLevel *pl = piter->second;
		std::cout << "price: " << pl->price << ", buy_vol: " << pl->buy_vol 
			<< ", sell_vol: " << pl->sell_vol << std::endl;
	}
#endif
	int buy_vol = 0, sell_vol = 0;
	for (; piter != plm.pricelevel_map.end (); ++piter) {
		PriceLevel *pl = piter->second;
		buy_vol += pl->buy_vol;
		sell_vol += pl->sell_vol;
	}

	std::cout << "buy_vol: " << buy_vol << ", sell_vol: " << sell_vol << std::endl;
	std::cout << std::endl;
}

int
main (int argc, char *argv[])
{
	int interactive_mode = 0;
	int opt = -1;
	std::string log_file;
	while ((opt = getopt(argc, argv, "il:")) != -1) {
		switch (opt) {
			case 'i':
				interactive_mode = 1;
				break;
			case 'l':
				log_file = std::string (optarg);
				break;
			case 'h':
			default:
				std::cout << "usage: ./front_log_analyzer [-i] logfile" << std::endl;
				std::cout << "       -i 可选项，进入交互模式" << std::endl; 
				return -1;
		}
	}

	if (log_file.empty ()) {
		std::cout << "log file lost." << std::endl;
		return -1;
	}

	PriceLevelMap plm;
	std::fstream fs (log_file.c_str ());
	std::string line, ignored_str;
	
	/// sysno 作为key
	std::map<int, Order *> order_map;
	/// std::map<int, Order *> buy_offset_order_map;
	/// std::map<int, Order *> sell_open_order_map;
	/// std::map<int, Order *> sell_offset_order_map;
	
	std::list<Order *> hist_order_list;
	std::list<Match *> hist_match_list;

	int line_count = 0;
	while (std::getline (fs, line)) {
		/// 不分析定单请求，因为没有系统号
		++line_count;
		if (match (line, std::string ("定单应答")) == true) {
			std::cout << "---------- Action " << line.substr (11, 12) 
				<< " ----------" << std::endl; 
			Order *pOrder = new Order;
			parse_for_order_rsp (line, pOrder);
			std::cout << "lno: " << line_count 
				<< line.substr (line.find_last_of ('|')) << std::endl;

			order_map.insert (std::make_pair (pOrder->sysno, pOrder));
			show_all (order_map, plm);
			if (interactive_mode == 1) {
				std::cin >> ignored_str;
				std::cout << 
					"\n\033[1m\033[31m*********** dispose next pkg **********\033[37m" 
					<< std::endl;
			} else {
				std::cout << "*********** dispose next pkg **********" << std::endl;
			}
			continue;
		}

#if 0
		if (match (line, std::string ("撤单应答")) == true) {
			std::cout << "---------- Action " << line.substr (11, 12) 
				<< " ----------" << std::endl; 
			Cancel *pCancel = new Cancel;
			parse_for_cancel_rsp (line, pCancel);

			/// 撤单应答有可能在成交之后到达，这样就找不到定单了
			std::map<int, Order *>::iterator iter =
				order_map.find (pCancel->batchno);
			if (iter != order_map.end ()) {
				/// Order *pOrder = iter->second;
				std::cout << "lno: " << line_count 
					<< line.substr (line.find_last_of ('|'))
					<< " erase order." << std::endl;
				order_map.erase (iter);
			}
			show_all (order_map, plm);
			if (interactive_mode == 1) {
				std::cin >> ignored_str;
				std::cout << 
					"\n\033[1m\033[31m*********** dispose next pkg **********\033[37m" 
					<< std::endl;
			} else {
				std::cout << "*********** dispose next pkg **********" << std::endl;
			}
			continue;
		}
#endif
		if (match (line, std::string ("成交通知")) == true) {
			std::cout << "---------- Action " << line.substr (11, 12) 
				<< " ----------" << std::endl; 
			Match *pMatch = new Match;
			parse_for_match_rsp (line, pMatch);
			std::cout << "lno: " << line_count 
				<< line.substr (line.find_last_of ('|'))
				<< "update posi" << std::endl;
			/// 更新持仓
			std::map<double, PriceLevel *, price_functor>::iterator iter =
				plm.pricelevel_map.find (pMatch->price);			
			if (iter == plm.pricelevel_map.end ()) {
				PriceLevel *ppl = new PriceLevel;
				ppl->price = pMatch->price;
				if (pMatch->bsflag == BUY) {
					if (pMatch->offlag == OPEN) {
						ppl->buy_vol += pMatch->vol;
					} else if (pMatch->offlag == OFFSET) {
						ppl->sell_vol -= pMatch->vol;
					} else {
						abort ();
					}
				} else if (pMatch->bsflag == SELL) {
					if (pMatch->offlag == OPEN) {
						ppl->sell_vol += pMatch->vol;
					} else if (pMatch->offlag == OFFSET) {
						ppl->buy_vol -= pMatch->vol;
					} else {
						abort ();
					}
				} else {
					abort ();
				}
				plm.pricelevel_map.insert (std::make_pair (pMatch->price, ppl));
			} else {
				PriceLevel *ppl = iter->second;
				
				ppl->price = pMatch->price;
				if (pMatch->bsflag == BUY) {
					if (pMatch->offlag == OPEN) {
						ppl->buy_vol += pMatch->vol;
					} else if (pMatch->offlag == OFFSET) {
						ppl->sell_vol -= pMatch->vol;
					} else {
						abort ();
					}
				} else if (pMatch->bsflag == SELL) {
					if (pMatch->offlag == OPEN) {
						ppl->sell_vol += pMatch->vol;
					} else if (pMatch->offlag == OFFSET) {
						ppl->buy_vol -= pMatch->vol;
					} else {
						abort ();
					}
				} else {
					abort ();
				}
			}
			/// 更新定单队列
			std::map<int, Order *>::iterator it = order_map.find (pMatch->sysno);
			if (it == order_map.end ()) {
				std::cout << "can't find order sysno: " << pMatch->sysno << std::endl;
				abort ();
			} else {
				Order *pOrder = it->second;
				pOrder->vol -= pMatch->vol;
				assert (pOrder->vol >= 0);
				if (pOrder->vol == 0) {
					std::cout << "lno: " << line_count 
						<< line.substr (line.find_last_of ('|'))
						<< "erase order" << std::endl;
					order_map.erase (it);
					hist_order_list.push_back (pOrder);
				} else {
					std::cout << "lno: " << line_count 
						<< line.substr (line.find_last_of ('|'))
						<< "update order" << std::endl;
				}
			}
			hist_match_list.push_back (pMatch);

			show_all (order_map, plm);
			if (interactive_mode == 1) {
				std::cin >> ignored_str;
				std::cout << 
					"\n\033[1m\033[31m*********** dispose next pkg **********\033[37m" 
					<< std::endl;
			} else {
				std::cout << "*********** dispose next pkg **********" << std::endl;
			}
			continue;
		}

		if (match (line, std::string ("定单状态确认")) == true) {
			std::cout << "---------- Action " << line.substr (11, 12) 
				<< " ----------" << std::endl; 
			/// 找到批次号和操作码
			std::string::size_type pos = line.find ("-B");
			if (pos == std::string::npos) {
				std::cout << line << " 格式错误" << std::endl;
				abort ();
			}
			int batchno = atoi (line.substr (pos + 2, 8).c_str ());
			
			pos = line.find ("-S");
			if (pos == std::string::npos) {
				std::cout << line << " 格式错误" << std::endl;
				abort ();
			}
			std::string action = line.substr (pos + 2, 1);

#if 1
			/// 通过批次号找到定单
			std::map<int, Order *>::iterator oit = order_map.find (batchno);
			if (oit == order_map.end ()) {
				std::cout << batchno << " not in order_map" << std::endl;
				abort ();
			}
#endif
/*
			#define OS_ORDER            'o'           //已报入
			#define OS_TRIG             'r'           //已触发
			#define OS_COMPLETE         'c'           //完全成交
			#define OS_PART             'p'           //部分成交
			#define OS_CANCEL           'd'           //客户撤单
			#define OS_FOKCANCEL        'k'           //FOK撤单
			#define OS_FAKCANCEL        'a'           //FAK撤单
			#define OS_STRIPCANCEL      's'           //批量定单撤单
			#define OS_FORCECANCEL      'f'           //强平撤单
			#define OS_FORCEUPD         'u'           //强平修改
*/
			if (action == std::string ("d")) { /// 撤单
				std::cout << "lno: " << line_count 
					<< line.substr (line.find_last_of ('|'))
					<< "delete order: " << batchno << std::endl;
				order_map.erase (oit);
			} else {
				std::cout << "暂时不处理其他action: " << action << std::endl;
				abort ();		
			}

			show_all (order_map, plm);
			if (interactive_mode == 1) {
				std::cin >> ignored_str;
				std::cout << 
					"\n\033[1m\033[31m*********** dispose next pkg **********\033[37m" 
					<< std::endl;
			} else {
				std::cout << "*********** dispose next pkg **********" << std::endl;
			}
			continue;
		}
	}
	
	std::cout << "删除重复回报前一共有" << line_count << "条日志." << std::endl;
}
