#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <list>
#include <map>
#include <string>

#include "PriceLevel.h"

/// ǰ����־��������������˺��Ѿ��ǵ��ͻ�����Լ�����н��ס�
/// ��������н�һ���ķ������˳������˹�����ִ�У�������Ч��
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
	/// T321 �� 3��������������1�����򣩣� 2������ƽ�� (1�ǿ���)
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
	/// T321 �� 3��������������1�����򣩣� 2������ƽ�� (1�ǿ���)
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
	/// T321 �� 3��������������1�����򣩣� 2������ƽ�� (1�ǿ���)
	pCancel->contract = vec[4].substr (1); 
	pCancel->vol   = atoi (vol_str.substr (2).c_str ());
}

static void show_all (std::map<int, Order *>& order_map, PriceLevelMap& plm)
{
	std::cout << "------------ Order Info ------------" << std::endl;
	std::map<double, Order *, price_functor> price_order_map;
	std::map<int, Order *>::iterator iter = order_map.begin ();
	/// ���ռ۸��������
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
		std::cout << "sysno: " << pOrder->sysno << " price: " << pOrder->price 
			<< ", bs: " << dir << ", of: " << of << ", vol: " << pOrder->vol << std::endl;
	}


	std::cout << "------------ Posi Info ------------" << std::endl;
	std::map<double, PriceLevel *, price_functor>::iterator piter = 
		plm.pricelevel_map.begin ();
	for (; piter != plm.pricelevel_map.end (); ++piter) {
		PriceLevel *pl = piter->second;
		std::cout << "price: " << pl->price << ", buy_vol: " << pl->buy_vol 
			<< ", sell_vol: " << pl->sell_vol << std::endl;
	}
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
	
	/// sysno ��Ϊkey
	std::map<int, Order *> order_map;
	/// std::map<int, Order *> buy_offset_order_map;
	/// std::map<int, Order *> sell_open_order_map;
	/// std::map<int, Order *> sell_offset_order_map;
	
	std::list<Order *> hist_order_list;
	std::list<Match *> hist_match_list;

	int line_count = 0;
	while (std::getline (fs, line)) {
		/// ����������������Ϊû��ϵͳ��
		++line_count;
		if (match (line, std::string ("����Ӧ��")) == true) {
			std::cout << "---------- Action ----------" << std::endl; 
			Order *pOrder = new Order;
			parse_for_order_rsp (line, pOrder);
			std::cout << "lno: " << line_count 
				<< line.substr (line.find_last_of ('|')) << std::endl;

			order_map.insert (std::make_pair (pOrder->sysno, pOrder));
			show_all (order_map, plm);
			std::cin >> ignored_str;
			std::cout << 
				"\n\033[1m\033[31m*********** dispose next pkg **********\033[37m" 
				<< std::endl;
			continue;
		}

		if (match (line, std::string ("����Ӧ��")) == true) {
			std::cout << "---------- Action ----------" << std::endl; 
			Cancel *pCancel = new Cancel;
			parse_for_cancel_rsp (line, pCancel);

			/// ����Ӧ���п����ڳɽ�֮�󵽴�������Ҳ���������
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
			std::cin >> ignored_str;
			std::cout << 
				"\n\033[1m\033[31m*********** dispose next pkg **********\033[37m" 
				<< std::endl;
			continue;
		}

		if (match (line, std::string ("�ɽ�֪ͨ")) == true) {
			std::cout << "---------- Action ----------" << std::endl; 
			Match *pMatch = new Match;
			parse_for_match_rsp (line, pMatch);
			std::cout << "lno: " << line_count 
				<< line.substr (line.find_last_of ('|'))
				<< "update posi" << std::endl;
			/// ���³ֲ�
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
			/// ���¶�������
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
			std::cin >> ignored_str;
			std::cout << 
				"\n\033[1m\033[31m*********** dispose next pkg **********\033[37m" 
				<< std::endl;
			continue;
		}
	}
	
	std::cout << "ɾ���ظ��ر�ǰһ����" << line_count << "����־." << std::endl;
}