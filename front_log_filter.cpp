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

/// ǰ����־���������߶������Ļ��ƣ�һ�ʶ�����Ӧ��ʶ���Ӧ��ͳɽ�Ӧ����
/// Щ�ظ�����Ϣ������־�����Ƚ����ѣ���������ͼɾ����Щ�ظ���Ϣ
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
                std::cout << "       -l ԭʼ��ǰ����־�ļ�" << std::endl;
                std::cout << "       -m ��Ա��" << std::endl;
                std::cout << "       -c �ͻ���" << std::endl;
                std::cout << "       -C ��Լ�ţ���ѡ�����������ȫ������" << std::endl;
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
		/// ���к�Լ����
		for (; iter != line_list.end ();) {
			std::string str = *iter;
			std::string::size_type pos = str.find (target_contract);
			if (pos == std::string::npos) {
				iter = line_list.erase (iter);
				continue;
			} else {
				if (is_arb_contract == false) {
					/// ɾ��������Լ
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

	/// ���пͻ��Ź���
	iter = line_list.begin ();
    for (; iter != line_list.end ();) {
        std::string str = *iter;
#if 1
		/// �����ر�û�пͻ�����Ϣ�����⴦��ȫ����������������
		/// ���κŽ��й���
		if (str.find ("����Ӧ��") != std::string::npos) {
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

	/// ���л�Ա�Ź���
	iter = line_list.begin ();
    for (; iter != line_list.end ();) {
        std::string str = *iter;
#if 1
		/// �����ر�û�пͻ�����Ϣ�����⴦��ȫ����������������
		/// ���κŽ��й���
		if (str.find ("����Ӧ��") != std::string::npos) {
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

		/// �������еĶ���Ӧ�������set�����ظ���ϵͳ�ţ���ô�˶���Ӧ������ظ���
		std::string::size_type pos = str.find ("����Ӧ��");
		if (pos != std::string::npos) {/// ������Ӧ��
			++order_rsp_count;
			pos = str.find ("-S");
			if (pos == std::string::npos) {
				std::cout << str << " ����Ӧ��û��ϵͳ��" << std::endl;
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

		pos = str.find ("�ɽ�֪ͨ");
		if (pos != std::string::npos) { /// ����ɽ�֪ͨ
			++match_rsp_count;
			pos = str.find ("-M"); /// �ɽ�Ҫ�óɽ��ŶԱȣ�������sysno
			if (pos == std::string::npos) {
				std::cout << str << " �ɽ�֪ͨû�гɽ���" << std::endl;
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

		/// ������Ӧ��
		pos = str.find ("����Ӧ��");
		if (pos != std::string::npos) {
			++cancel_rsp_count;
			pos = str.find ("-B");
			if (pos == std::string::npos) {
				std::cout << str << " ����Ӧ��û�����κ�" << std::endl;
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

	/// ���ݿͻ�����ȡһ��sysno
	sysno_set.clear ();
	iter = line_list.begin ();
    for (; iter != line_list.end (); ++iter) {
        std::string str = *iter;
        std::string::size_type pos = str.find (target_clientid);
        if (pos == std::string::npos) {
            continue;
        }

        pos = str.find ("����Ӧ��");
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
	/// �ظ���Ŀ������Ҫ���пͻ����ˣ����ڳ���Ӧ��û�пͻ��ţ���Ҫ����
	/// BatchNo��SysNo��Ӧ��ϵ���й���
	iter = line_list.begin ();
    for (; iter != line_list.end (); ++iter) {
        std::string str = *iter;
        std::string::size_type pos = str.find (target_clientid);
        if (pos == std::string::npos) {
            pos = str.find ("����Ӧ��");
            if (pos == std::string::npos) {
				std::cout << "�����ǳ���Ӧ��" << std::endl;
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
