#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <set>

#include <stdlib.h>
/// ǰ����־���������߶������Ļ��ƣ�һ�ʶ�����Ӧ��ʶ���Ӧ��ͳɽ�Ӧ����
/// Щ�ظ�����Ϣ������־�����Ƚ����ѣ���������ͼɾ����Щ�ظ���Ϣ
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

	/// ȡ���ض��ͻ���sysno
	std::list<std::string>::iterator iter = line_list.begin ();
	for (; iter != line_list.end ();) {
		std::string str = *iter;

		/// �������еĶ���Ӧ�������set�����ظ���ϵͳ�ţ���ô�˶���Ӧ������ظ���
		std::string::size_type pos = str.find ("����Ӧ��");
		if (pos == std::string::npos) {
			pos = str.find ("�ɽ�֪ͨ");
			if (pos == std::string::npos) {
				/// ������Ӧ��
				pos = str.find ("����Ӧ��");
				if (pos != std::string::npos) {
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
				} else {
					++iter;
					continue;
				}
			} else { /// ����ɽ�֪ͨ
				pos = str.find ("-M"); /// �ɽ�Ҫ�óɽ��ŶԱȣ�������sysno
				if (pos == std::string::npos) {
					std::cout << str << " �ɽ�֪ͨû�гɽ���" << std::endl;
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
		} else { /// ������Ӧ��
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
	/// ���к�Լ����
	iter = line_list.begin ();
    for (; iter != line_list.end ();) {
        std::string str = *iter;
        std::string::size_type pos = str.find (target_contract);
        if (pos == std::string::npos) {
			/// ����Ӧ��Ӧ�ñ����˵�
			pos = str.find ("����Ӧ��");
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
