#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <set>

#include <stdlib.h>
/// 前置日志中由于总线二级订阅机制，一笔定单对应多笔定单应答和成交应答，这
/// 些重复的信息导致日志分析比较困难，本程序试图删除这些重复信息
int
main (int argc, char *argv[])
{
	if (argc != 4) {
		std::cout << "usage: ./front_log_filter logfile clientid contract" << std::endl;
		return 0;
	}

	std::set<int> sysno_set;
	std::set<int> match_matchno_set;
	std::set<int> cancel_sysno_set;
	std::string target_clientid (argv[2]);
	std::string target_contract (argv[3]);
	std::list<std::string> line_list;
	std::fstream fs (argv[1]);
	std::string line;
	int line_count = 0;
	while (std::getline (fs, line)) {
		line_list.push_back (line);
		++line_count;
	}
	
	std::cout << line_count << std::endl;

	/// 取得特定客户的sysno
	std::list<std::string>::iterator iter = line_list.begin ();
	for (; iter != line_list.end ();) {
		std::string str = *iter;

		/// 搜索所有的定单应答，如果在set中有重复的系统号，那么此定单应答就是重复的
		std::string::size_type pos = str.find ("定单应答");
		if (pos == std::string::npos) {
			pos = str.find ("成交通知");
			if (pos == std::string::npos) {
				/// 处理撤单应答
				pos = str.find ("撤单应答");
				if (pos != std::string::npos) {
					pos = str.find ("-B");
					if (pos == std::string::npos) {
						std::cout << str << " 撤单应答没有批次号" << std::endl;
						return -1;
					}
					std::string str_sysno (str.substr (pos + 2, 8));

					std::pair <std::set<int>::iterator, bool> ret = 
						cancel_sysno_set.insert (atoi (str_sysno.c_str ()));
					if (ret.second != true) {
						iter = line_list.erase (iter);
					} else {
						++iter;
					}
					continue;
				} else {
					++iter;
					continue;
				}
			} else { /// 处理成交通知
				pos = str.find ("-M"); /// 成交要用成交号对比，不能用sysno
				if (pos == std::string::npos) {
					std::cout << str << " 成交通知没有成交号" << std::endl;
					return -1;
				}
				std::string str_matchno (str.substr (pos + 2, 8));

				std::pair <std::set<int>::iterator, bool> ret = 
					match_matchno_set.insert (atoi (str_matchno.c_str ()));
				if (ret.second != true) {
					iter = line_list.erase (iter);
				} else {
					++iter;
				}
				continue;
			}
		} else { /// 处理定单应答
			pos = str.find ("-S");
			if (pos == std::string::npos) {
				std::cout << str << " 定单应答没有系统号" << std::endl;
				return -1;
			}
			std::string str_sysno (str.substr (pos + 2, 8));

			std::pair <std::set<int>::iterator, bool> ret = 
				sysno_set.insert (atoi (str_sysno.c_str ()));
			if (ret.second != true) {
				iter = line_list.erase (iter);
				continue;
			} else {
				++iter;
			}
		}
	}

	/// std::cout << line_list.size () << std::endl;
#if 0
	/// 
	line_count = 0;
	iter = line_list.begin ();
	for (; iter != line_list.end (); ++iter) {
		++line_count;
		std::cout << *iter << std::endl;
	}
	std::cout << line_count << std::endl;
#endif
	/// 进行合约过滤
	iter = line_list.begin ();
    for (; iter != line_list.end ();) {
        std::string str = *iter;
        std::string::size_type pos = str.find (target_contract);
        if (pos == std::string::npos) {
			/// 撤单应答不应该被过滤掉
			pos = str.find ("撤单应答");
			if (pos != std::string::npos) {
				++iter;
				continue;
			}
			iter = line_list.erase (iter);
            continue;
		} else {
			++iter;
			continue;
		}
	}
	/// std::cout << line_list.size () << std::endl;

	/// 根据客户号再取一次sysno
	sysno_set.clear ();
	iter = line_list.begin ();
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
	}
	
	/// std::cout << line_list.size () << std::endl;
	/// 重复项目消除后，要进行客户过滤，由于撤单应答没有客户号，需要根据
	/// BatchNo和SysNo对应关系进行过滤
	iter = line_list.begin ();
    for (; iter != line_list.end (); ++iter) {
        std::string str = *iter;
        std::string::size_type pos = str.find (target_clientid);
        if (pos == std::string::npos) {
            pos = str.find ("撤单应答");
            if (pos == std::string::npos) {
				/// std::cout << "EEE " << str << std::endl;
                continue;
			} else {
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
			}
        } else {
            std::cout << str << std::endl;
        }
	}
}
