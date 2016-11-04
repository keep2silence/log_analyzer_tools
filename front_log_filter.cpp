#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <set>

#include <stdlib.h>
#include <unistd.h>

/// 前置日志中由于总线二级订阅机制，一笔定单对应多笔定单应答和成交应答，这
/// 些重复的信息导致日志分析比较困难，本程序试图删除这些重复信息
int
main (int argc, char *argv[])
{
	int opt = -1;
	std::string log_file;
	std::string target_clientid;
	std::string target_contract;
	std::string target_member_no;

    while ((opt = getopt(argc, argv, "l:m:c:C:")) != -1) {
        switch (opt) {
            case 'l':
				log_file = std::string (optarg);
                break;
			case 'm':
				target_member_no = std::string (optarg);
            case 'c':
				target_clientid = std::string (optarg);
				break;
			case 'C':
				target_contract = std::string (optarg);
				break;
            default:
				std::cout << 
				"usage: ./front_log_filter -l logfile -m member_no -c clientid -C contract" 
				<< std::endl;
                std::cout << "       -l 原始的前置日志文件" << std::endl;
                std::cout << "       -m 会员号" << std::endl;
                std::cout << "       -c 客户号" << std::endl;
                std::cout << "       -C 合约号，可选项如果不设置全部解析" << std::endl;
                return -1;
        }
    }

	if (log_file.empty ()) {
		std::cout << "log file not set" << std::endl;
		return -1;
	}

	if (target_clientid.empty ()) {
		std::cout << "clientid not set" << std::endl;
		return -1;
	}

	if (target_contract.empty ()) {
		std::cout << "keep all contracts of client: " << target_clientid << std::endl;
	}

	if (target_member_no.empty ()) {
		std::cout << "member_no not set" << std::endl;
		return -1;
    }

	std::set<int> sysno_set;
	std::set<int> match_matchno_set;
	std::set<int> cancel_sysno_set;
	std::list<std::string> line_list;
	std::fstream fs (log_file.c_str ());
	std::string line;
	int line_count = 0;
	while (std::getline (fs, line)) {
		line_list.push_back (line);
		++line_count;
	}
	
	/// std::cout << line_list.size () << std::endl;
	bool is_arb_contract = target_contract.find_first_of ('&') != std::string::npos;

	std::list<std::string>::iterator iter = line_list.begin ();
	if (target_contract.empty () == false) {
		/// 进行合约过滤
		for (; iter != line_list.end ();) {
			std::string str = *iter;
			std::string::size_type pos = str.find (target_contract);
			if (pos == std::string::npos) {
				iter = line_list.erase (iter);
				continue;
			} else {
				if (is_arb_contract == false) {
					/// 删除套利合约
					if (str.find_first_of ('&') != std::string::npos) {
						iter = line_list.erase (iter);
						continue;
					}
				}
				++iter;
				continue;
			}
		}
	}
	/// std::cout << line_list.size () << std::endl;

	/// 进行客户号过滤
	iter = line_list.begin ();
    for (; iter != line_list.end ();) {
        std::string str = *iter;
#if 1
		/// 撤单回报没有客户号信息，特殊处理，全部保留，后面会根据
		/// 批次号进行过滤
		if (str.find ("撤单应答") != std::string::npos) {
			++iter;
			continue;
		}
#endif
        std::string::size_type pos = str.find (target_clientid);
        if (pos == std::string::npos) {
			iter = line_list.erase (iter);
            continue;
		} else {
			++iter;
			continue;
		}
	}
	/// std::cout << line_list.size () << std::endl;

	/// 进行会员号过滤
	iter = line_list.begin ();
    for (; iter != line_list.end ();) {
        std::string str = *iter;
#if 1
		/// 撤单回报没有客户号信息，特殊处理，全部保留，后面会根据
		/// 批次号进行过滤
		if (str.find ("撤单应答") != std::string::npos) {
			++iter;
			continue;
		}
#endif
        std::string::size_type pos = str.find (target_member_no);
        if (pos == std::string::npos) {
			iter = line_list.erase (iter);
            continue;
		} else {
			++iter;
			continue;
		}
	}
	/// std::cout << line_list.size () << std::endl;

	iter = line_list.begin ();
	int order_rsp_count = 0, match_rsp_count = 0, 
		cancel_rsp_count = 0, other_count = 0;
	for (; iter != line_list.end ();) {
		std::string str = *iter;

		/// 搜索所有的定单应答，如果在set中有重复的系统号，那么此定单应答就是重复的
		std::string::size_type pos = str.find ("定单应答");
		if (pos != std::string::npos) {/// 处理定单应答
			++order_rsp_count;
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
			} else {
				++iter;
			}
			continue;
		}

		pos = str.find ("成交通知");
		if (pos != std::string::npos) { /// 处理成交通知
			++match_rsp_count;
			pos = str.find ("-M"); /// 成交要用成交号对比，不能用sysno
			if (pos == std::string::npos) {
				std::cout << str << " 成交通知没有成交号" << std::endl;
				return -1;
			}
			std::string str_matchno (str.substr (pos + 2, 8));
			/// std::cout << str_matchno << std::endl;
			std::pair <std::set<int>::iterator, bool> ret = 
				match_matchno_set.insert (atoi (str_matchno.c_str ()));
			if (ret.second != true) {
				iter = line_list.erase (iter);
			} else {
				++iter;
			}
			continue;
		}

		/// 处理撤单应答
		pos = str.find ("撤单应答");
		if (pos != std::string::npos) {
			++cancel_rsp_count;
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
		}

		++other_count;
		++iter;
		continue;
	}

#if 0
	std::cout << "order_rsp_count: " << order_rsp_count 
		<< "left: " << sysno_set.size () << std::endl;
	std::cout << "match_rsp_count: " << match_rsp_count
		<< "left: " << match_matchno_set.size () << std::endl;
	std::cout << "cancel_rsp_count: " << cancel_rsp_count
		<< "left: " << cancel_sysno_set.size () << std::endl;
	std::cout << "other_count: " << other_count << std::endl;
	
	for (std::set<int>::iterator iter = cancel_sysno_set.begin ();
		iter != cancel_sysno_set.end (); ++iter) {
		std::cout << *iter << std::endl;
	}

	return 0;
#endif
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
	
	/// std::cout << sysno_set.size () << std::endl;
	/// return 0;

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
				std::cout << "必须是撤单应答" << std::endl;
				return -1;
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
