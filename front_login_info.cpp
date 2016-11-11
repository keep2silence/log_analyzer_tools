#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <list>
#include <unordered_map>
#include <string>
#include <set>

#include <stdlib.h>
#include <unistd.h>

class trader 
{
public:
	int member_no;
	int trader_no;
};

/// 分析每个前置登录的席位
int
main (int argc, char *argv[])
{
	int opt = -1;
	std::string log_file;
	std::string log_time;

	while ((opt = getopt(argc, argv, "l:t:")) != -1) {
        switch (opt) {
            case 'l':
                log_file = std::string (optarg);
                break;
			case 't':
				log_time = std::string (optarg);
				log_time = " " + log_time;
				break;
			default:
				std::cout << "usage: front_login_info -l log_file -t time" << std::endl;
				std::cout << "       -l front log file" << std::endl;
				/// 由于席位登录后有可能退出，然后跑到其他前置再次登录，因此我们
				/// 规定一个时间起点，然后分析这以后1分钟以内的席位登录情况，比如
				/// 我们规定09:00，那么就看09:00－09:01这期间席位登录情况。
				std::cout << 
					"       -t 席位活动时间，09:00，表示从09:00开始统计" << 
					std::endl;
				return -1;
		}
	}

	std::unordered_map<int, std::set<int> * > trader_info;
	std::fstream fs (log_file);
	std::string line;
	bool log_time_matched = false;
	std::vector<std::string> vec;
	while (std::getline (fs, line)) {
		if (line.find (log_time) == std::string::npos) {
			if (log_time_matched == true) {
				break;
			}
			continue;
		}

		log_time_matched = true;

		std::string::size_type pos = line.find ("Mem");
		if (pos == std::string::npos) {
			continue;
		}
		int member_no = atoi (line.substr (pos + 3, 4).c_str ());

		pos = line.find ("-C");
		if (pos == std::string::npos) {
			continue;
		}
		int trader_no = atoi (line.substr (pos + 2, 8).c_str ());
		
		std::unordered_map<int, std::set<int> * >::iterator iter =
			trader_info.find (member_no);
		if (iter == trader_info.end ()) {
			std::set<int> *pset = new std::set<int>;
			pset->insert (trader_no);
			trader_info.insert (std::make_pair (member_no, pset));
		} else {
			iter->second->insert (trader_no);
		}
	}

	std::unordered_map<int, std::set<int> * >::iterator iter = trader_info.begin ();
	for (; iter != trader_info.end (); ++iter) {
		std::cout << "member: " << iter->first << " ";
		std::set<int>::iterator siter = iter->second->begin ();
		for (; siter != iter->second->end (); ++siter) {
			std::cout << *siter << ", ";
		}
		std::cout << std::endl;
	}
}
