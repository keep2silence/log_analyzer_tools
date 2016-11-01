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

/// 分析前置日志，找到交易数据量最多的客户号
int
main (int argc, char *argv[])
{
	if (argc != 2) {
		std::cout << "usage: ./max_order_count_finder logfile" << std::endl;
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
	
	std::cout << "删除重复回报前一共有" << line_count << "条日志." << std::endl;

	/// 日志行数非常多，需要根据用户号进行分类过滤
	std::map<std::string, std::list<Item> *> client_map;
	std::list<std::string>::iterator iter = line_list.begin ();
	int line_no = 0;
	for (; iter != line_list.end (); ++iter) {
		++line_no;
		std::string str = *iter;
		std::string::size_type pos = str.find ("-C0");
		if (pos == std::string::npos) {
			continue;
		}
		std::string clientid = str.substr (pos + 2, 8);
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

	for (std::map<std::string, std::list<Item> *>::iterator iter = client_map.begin ();
		iter != client_map.end (); ++iter) {
		std::cout << iter->second->size () << " : " << iter->first << std::endl;
	}
	
}
