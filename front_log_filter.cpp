#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <list>
#include <map>
#include <string>

class Item
{
public:
	int line_no;
	std::string str;
};

/// ǰ����־���������߶������Ļ��ƣ�һ�ʶ�����Ӧ��ʶ���Ӧ��ͳɽ�Ӧ����
/// Щ�ظ�����Ϣ������־�����Ƚ����ѣ���������ͼɾ����Щ�ظ���Ϣ
int
main (int argc, char *argv[])
{
	if (argc != 2) {
		std::cout << "usage: ./front_log_filter logfile" << std::endl;
		return 0;
	}

	std::list<std::string> line_list;
	std::fstream fs (argv[1]);
	std::string line;
	int line_count = 0;
	while (std::getline (fs, line)) {
		line_list.push_back (line);
		++line_count;
	}
	
	std::cout << "ɾ���ظ��ر�ǰһ����" << line_count << "����־." << std::endl;

	/// ��־�����ǳ��࣬��Ҫ�����û��Ž��з������
	std::map<std::string, std::list<Item> *> client_map;
	std::list<std::string>::iterator iter = line_list.begin ();
	int line_no = 0;
	for (; iter != line_list.end (); ++iter) {
		++line_no;
		std::string str = *iter;
		std::string::size_type pos = str.find ("-C");
		if (pos == std::string::npos) {
			continue;
		}
		std::string clientid = str.substr (pos + 1, 8);
		std::map<std::string, std::list<Item> *>::iterator miter =
			client_map.find (clientid);
		if (miter == client_map.end ()) {
			std::list<Item> *p = new std::list<Item>;
			Item item;
			item.line_no = line_no;
			item.str = str;
			p->push_back (item);
			client_map.insert (std::make_pair (clientid, p));
		} else {
			Item item;
			item.line_no = line_no;
			item.str = str;
			miter->second->push_back (item);
		}
	}

	/// ����ÿ���ͻ��ţ�ɾ���ظ���ظ���Ķ����Ǵ���־���ݿ�ʼ�ģ�������
	/// �ļ���ʱ����ֶ�
	std::map<std::string, std::list<Item> *>::iterator miter;
	for (; miter != client_map.end (); ++miter) {
		std::list<Item> *plist = miter->second;

		for (std::list<Item>::iterator iter = plist->begin ();
			iter != plist->end (); ++iter) {
			Item item = *iter;
			std::string::size_type pos = item.str.find_last_of ('|');
			if (pos == std::string::npos) {
				continue;
			}
			std::string content (item.str.substr (pos + 1));
		}
	}

	while (line_list.empty () == false) {
		line = line_list.front ();
		line_list.pop_front ();

		std::list<std::string>::iterator iter = line_list.begin ();
		for (; iter != line_list.end (); ++iter) {
			std::string str;
			
		}
	}
}
