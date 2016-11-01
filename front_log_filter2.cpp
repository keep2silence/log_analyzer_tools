#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <set>

#include <stdlib.h>

class Item
{
public:
	int line_no;
	std::string str;
};

/// 前置日志中由于总线二级订阅机制，一笔定单对应多笔定单应答和成交应答，这
/// 些重复的信息导致日志分析比较困难，本程序试图删除这些重复信息
int
main (int argc, char *argv[])
{
	if (argc != 3) {
		std::cout << "usage: ./front_log_filter logfile clientid" << std::endl;
		return 0;
	}

	std::set<int> sysno_set;
	std::string target_clientid (argv[2]);
	std::list<std::string> line_list;
	std::fstream fs (argv[1]);
	std::string line;
	int line_count = 0;
	while (std::getline (fs, line)) {
		line_list.push_back (line);
		++line_count;
	}
	
	/// 取得特定客户的sysno
	std::list<std::string>::iterator iter = line_list.begin ();
	for (; iter != line_list.end (); ++iter) {
		std::string str = *iter;
		std::string::size_type pos = str.find (target_clientid);
		if (pos == std::string::npos) {
			continue;
		}

		pos = str.find ("定单应答");
		if (pos == std::string::npos) {
			continue;
		}

		pos = str.find ("-S");
		if (pos == std::string::npos) {
			continue;
		}
		std::string str_sysno (str.substr (pos + 2, 8));

		std::pair <std::set<int>::iterator, bool> ret = 
			sysno_set.insert (atoi (str_sysno.c_str ()));
#if 0
		if (ret.second != true) {
			std::cout << str_sysno << " is duplicated." << std::endl;
			return 0;
		}
#endif
	}

	iter = line_list.begin ();
	for (; iter != line_list.end (); ++iter) {
		std::string str = *iter;
		std::string::size_type pos = str.find (target_clientid);
		if (pos == std::string::npos) {
			pos = str.find ("撤单应答");
			if (pos == std::string::npos) {
				continue;
			}
			pos = str.find ("-B");
			if (pos == std::string::npos) {
				std::cout << str << " ERROR" << std::endl;
				return -1;
			}
			std::string str_batchno (str.substr (pos + 2, 8));
			std::set<int>::iterator set_iter = 
				sysno_set.find (atoi (str_batchno.c_str ()));
			if (set_iter != sysno_set.end ()) {
				std::cout << str << std::endl;
			}
		} else {
			std::cout << str << std::endl;
		}
	}
}
