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

/// ����ÿ��ǰ�õ�¼��ϯλ
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
				/// ����ϯλ��¼���п����˳���Ȼ���ܵ�����ǰ���ٴε�¼���������
				/// �涨һ��ʱ����㣬Ȼ��������Ժ�1�������ڵ�ϯλ��¼���������
				/// ���ǹ涨09:00����ô�Ϳ�09:00��09:01���ڼ�ϯλ��¼�����
				std::cout << 
					"       -t ϯλ�ʱ�䣬09:00����ʾ��09:00��ʼͳ��" << 
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
