#include <map>
#include <vector>
#include <list>
#include <algorithm>
#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <strings.h>

/// top日志分析，不是所有的数据都需要分析，只分析
/// 我们关注的指标		dht - 20160916
const int max_collect_count = 50;

struct sys_info
{
	std::string time;
	double load_avg[3];

	void show ()
	{
		std::cout << "time: " << time << " load avg: " << load_avg[0] << ", " << load_avg[1] << ", " << load_avg[2] << std::endl;
	}
};

struct task_info 
{
	int ntask;
	int nrunning;
	int nsleeping;
	int nzombie;

	void show ()
	{
		std::cout << "task: " << ntask << " running: " << nrunning << " sleeping: " << nsleeping << " zombie: " << nzombie << std::endl;
	}
};

struct cpu_info 
{
	double us;
	double sys;
	double ni;
	double id;
	double wa;
	double hi;
	double si;

	void show ()
	{
		std::cout << "us: " << us << " sys: " << sys << " ni: " << ni << " id: " << id << " wa: " << wa << " hi: " << hi << " si:" << si << std::endl;
	}
};

struct mem_info
{
	int used;
	int free;
	int buffer;

	void show () 
	{
		std::cout << "mem used: " << used << " mem free: " << free << " buffer: " << buffer << std::endl;
	}
};

struct swap_info
{
	int swap_used;
	int cached;

	void show () 
	{
		std::cout << "swap used: " << swap_used << " cached: " << cached << std::endl;
	}
};

struct proc_info 
{
	int pid;
	std::string user;
	/// int priority;
	/// int nice;
	/// int virt;
	long long res;
	/// int shr;
	float cpu;
	float mem;
	std::string name;
	/// std::string time;
	void show ()
	{
		std::cout << "pid :" << pid << " user: " << user << " resident mem: " << res << " cpu: " << cpu << " mem: " << mem << " proc name: " << name << std::endl;
	}
};

struct top_info 
{
	sys_info sys;
	task_info task;
	cpu_info cpu;
	mem_info mem;
	swap_info swap;
	proc_info proc[10];

	void show () 
	{
		sys.show ();
		task.show ();
		cpu.show ();
		mem.show ();
		swap.show ();

		for (size_t i = 0; i < 10; ++i) {
			proc[i].show ();
		}
	}
};

void split_to_vector (std::string line, std::vector<std::string> &stdvec)
{
    std::string::size_type pos;
    while ((pos = line.find_first_of (' ')) != std::string::npos) {
        std::string str = line.substr (0, pos);
		if (str == std::string ("")) {
			line = line.substr (pos + 1);
			continue;
		}
		
		std::string::size_type pos2;
		if ((pos2 = str.find_first_of (',')) != std::string::npos) {
			std::string::size_type pos3 = str.find_last_of (',');
            if (pos2 != pos3 || str[str.length () - 1] != ',') {
				std::string first_str = str.substr (0, pos2);
				stdvec.push_back (first_str);
				stdvec.push_back (str.substr (pos2 + 1));
			} else {
				stdvec.push_back (str);
			}
		} else {
			stdvec.push_back (str);
		}
        line = line.substr (pos + 1);
    }

	if (line != std::string ("")) {
		stdvec.push_back (line);
	}
}

bool cpu_use_compare (const top_info &b1, const top_info &b2)  
{  
      return b1.proc[0].cpu > b2.proc[0].cpu;  
}  

bool mem_use_compare (const top_info &b1, const top_info &b2)  
{  
      return b1.proc[0].mem > b2.proc[0].mem;  
}  

bool sys_mem_use_compare (const top_info &b1, const top_info &b2)  
{  
      return b1.mem.used > b2.mem.used;  
}  

bool sys_swap_use_compare (const top_info &b1, const top_info &b2)  
{  
      return b1.swap.swap_used > b2.swap.swap_used;  
}  

bool res_use_compare (const top_info &b1, const top_info &b2)  
{  
      return b1.proc[0].res > b2.proc[0].res;  
}  

bool wa_use_compare (const top_info &b1, const top_info &b2)  
{  
      return b1.cpu.wa > b2.cpu.wa;  
}  

bool idle_use_compare (const top_info &b1, const top_info &b2)  
{  
      return b1.cpu.id < b2.cpu.id;  
}  

bool hi_use_compare (const top_info &b1, const top_info &b2)  
{  
      return b1.cpu.hi > b2.cpu.hi;  
}  

int
main (int argc, char *argv[])
{
	if (argc != 2) {
		std::cout << "usage: ./log_analyze log" << std::endl;
		return -1;
	}

	std::vector<top_info> top_info_all;
	std::ifstream log_file (argv[1]);
	std::string line;

	top_info_all.reserve (28000);
	top_info topinfo;
	topinfo.proc[9].pid = -1;
	int proc_count = 0;
	while (std::getline (log_file, line)) {
		if (line[0] != 't') {
			if (topinfo.proc[9].pid > 0) {
				continue;
			}
		}

		std::vector<std::string> vec;
		vec.reserve (16);

		split_to_vector (line, vec);
		if (vec.size () == 0) {
			continue;
		}
		
		if (vec[0] == std::string ("top")) {
			if (top_info_all.empty () == false) {
				top_info_all.push_back (topinfo);
				topinfo.proc[9].pid = -1;
			}
			topinfo.sys.time = vec[2];
			topinfo.sys.load_avg[0] = atof (std::string (vec[11].begin (), 
					--vec[11].end ()).c_str ());
			topinfo.sys.load_avg[1] = atof (std::string (vec[12].begin (), 
					--vec[12].end ()).c_str ());
			topinfo.sys.load_avg[2] = atof (std::string (vec[13].begin (), 
					--vec[13].end ()).c_str ());
		
			continue;
		}

		if (topinfo.proc[9].pid > 0) {
			continue;
		}

		if (vec[0] == std::string ("Tasks:")) {
			topinfo.task.ntask = atoi (vec[1].c_str ());
			topinfo.task.nrunning = atoi (vec[3].c_str ());
			topinfo.task.nsleeping = atoi (vec[5].c_str ());
			topinfo.task.nzombie = atoi (vec[9].c_str ());
			continue;
		}

		if (vec[0] == std::string ("Cpu(s):")) {
			topinfo.cpu.us = atof (vec[1].substr (0, vec[1].find ('%')).c_str ());
			topinfo.cpu.sys = atof (vec[2].substr (0, vec[2].find ('%')).c_str ());
			topinfo.cpu.ni = atof (vec[3].substr (0, vec[3].find ('%')).c_str ());
			topinfo.cpu.id = atof (vec[4].substr (0, vec[4].find ('%')).c_str ());
			topinfo.cpu.wa = atof (vec[5].substr (0, vec[5].find ('%')).c_str ());
			topinfo.cpu.hi = atof (vec[6].substr (0, vec[6].find ('%')).c_str ());
			topinfo.cpu.si = atof (vec[7].substr (0, vec[7].find ('%')).c_str ());
			continue;
		}

		if (vec[0] == std::string ("Mem:")) {
			topinfo.mem.used = atoi (std::string (vec[3].begin (), 
				--vec[3].end ()).c_str ());
			topinfo.mem.free = atoi (std::string (vec[5].begin (), 
				--vec[5].end ()).c_str ());
			topinfo.mem.buffer = atoi (std::string (vec[7].begin (), 
				--vec[7].end ()).c_str ());
			continue;
		}

		if (vec[0] == std::string ("Swap:")) {
			topinfo.swap.swap_used = atoi (std::string (vec[3].begin (), 
				--vec[3].end ()).c_str ());
			topinfo.swap.cached = atoi (std::string (vec[7].begin (), 
				--vec[7].end ()).c_str ());
			continue;
		}

		bool ispid = true;
		for (size_t i = 0; i < vec[0].size (); ++i) {
			if (! isdigit (vec[0].at (i))) {
				ispid = false;
				break;
			}
		}

		if (ispid == false) {
			continue;
		}

		proc_info pinfo;
		pinfo.pid = atoi (vec[0].c_str ());
		pinfo.user = vec[1];
		
		std::string::size_type pos;
		if ((pos = vec[5].find_first_of ('g')) != std::string::npos) {
			pinfo.res = 1024 * 1024 * 1024 * atof (vec[5].substr (0, pos).c_str ());
		} else if ((pos = vec[5].find_first_of ('m')) != std::string::npos) {
			pinfo.res = 1024 * 1024 * atof (vec[5].substr (0, pos).c_str ());
		} else {
			pinfo.res = atoi (vec[5].c_str ());
		}
		/// pinfo.res = vec[5];
		pinfo.cpu  = atof (vec[8].c_str ());
		pinfo.mem  = atof (vec[9].c_str ());
		pinfo.name = vec[11];
		
		topinfo.proc[proc_count++] = pinfo;
		
		if (proc_count == 10) {
			top_info_all.push_back (topinfo);
			/// topinfo.proc[9].pid = -1;
			proc_count = 0;

			/// 验证解析数据是否正确
			/// topinfo.show ();
		}
	} /// 原始数据解析完毕

	/// 对数据进行统计分析
	/// cpu最大使用率统计，即idle最小
	double min_idle = 100.0;
	top_info *pinfo = NULL;
	
	std::cout << "---- cpu info -----" << std::endl;
	std::nth_element (top_info_all.begin (), top_info_all.begin () + max_collect_count, 
		top_info_all.end (), cpu_use_compare);

	for (size_t i = 0; i < max_collect_count; ++i) {
		std::cout << top_info_all[i].sys.time << " " << top_info_all[i].proc[0].name << " " << top_info_all[i].proc[0].cpu << std::endl;
	}

	std::cout << "---- mem info -----" << std::endl;

	std::nth_element (top_info_all.begin (), top_info_all.begin () + max_collect_count, 
		top_info_all.end (), mem_use_compare);

	for (size_t i = 0; i < max_collect_count; ++i) {
		std::cout << top_info_all[i].sys.time << " " << top_info_all[i].proc[0].name << " " << top_info_all[i].proc[0].mem << std::endl;
	}

	std::cout << "--- resident mem ----" << std::endl;

	std::nth_element (top_info_all.begin (), top_info_all.begin () + max_collect_count, 
		top_info_all.end (), res_use_compare);

	for (size_t i = 0; i < max_collect_count; ++i) {
		std::cout << top_info_all[i].sys.time << " " << top_info_all[i].proc[0].name << " " << top_info_all[i].proc[0].res / 1024 / 1024 << "M" << std::endl;
	}

	std::cout << "--- wio ----" << std::endl;

	std::nth_element (top_info_all.begin (), top_info_all.begin () + max_collect_count, 
		top_info_all.end (), wa_use_compare);

	for (size_t i = 0; i < max_collect_count; ++i) {
		std::cout << top_info_all[i].sys.time << " " << top_info_all[i].proc[0].name << " " << top_info_all[i].cpu.wa << std::endl;
	}

	std::cout << "--- sys load ----" << std::endl;

	std::nth_element (top_info_all.begin (), top_info_all.begin () + max_collect_count, 
		top_info_all.end (), idle_use_compare);

	for (size_t i = 0; i < max_collect_count; ++i) {
		std::cout << top_info_all[i].sys.time << " " << top_info_all[i].proc[0].name << " " <<  100.0 - top_info_all[i].cpu.id << std::endl;
	}

	std::cout << "--- hareward interrupt ---" << std::endl;

	std::nth_element (top_info_all.begin (), top_info_all.begin () + max_collect_count, 
		top_info_all.end (), hi_use_compare);

	for (size_t i = 0; i < max_collect_count; ++i) {
		std::cout << top_info_all[i].sys.time << " " << top_info_all[i].proc[0].name << " " <<  top_info_all[i].cpu.hi << std::endl;
	}

	std::cout << "--- max mem use ---" << std::endl;

	std::nth_element (top_info_all.begin (), top_info_all.begin () + max_collect_count, 
		top_info_all.end (), sys_mem_use_compare);

	for (size_t i = 0; i < max_collect_count; ++i) {
		std::cout << top_info_all[i].sys.time << " " << top_info_all[i].proc[0].name << " " <<  top_info_all[i].mem.used << "k" << std::endl;
	}

	std::cout << "--- max swap use ---" << std::endl;

	std::nth_element (top_info_all.begin (), top_info_all.begin () + max_collect_count, 
		top_info_all.end (), sys_swap_use_compare);

	for (size_t i = 0; i < max_collect_count; ++i) {
		std::cout << top_info_all[i].sys.time << " " << top_info_all[i].proc[0].name << " " <<  top_info_all[i].swap.swap_used << "k" << std::endl;
	}

}
