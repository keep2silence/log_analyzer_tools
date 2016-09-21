#include <map>
#include <vector>
#include <list>
#include <algorithm>
#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

/// sar日志分析，不是所有的数据都需要分析，只分析
/// 我们关注的指标		dht - 20160921
/// sar日志根据输入选项不同，日志输出也不同，这里采用和sar相同的
/// 命令行选项，方便使用

static void usage ()
{
	std::cout << "以下选项拷贝自sar，现在还不是所有的数据都分析，慢慢完善" << std::endl;
	std::cout << "-B	Paging statistics " << std::endl;
	std::cout << "-b	I/O and transfer rate statistics " << std::endl;
	std::cout << "-d	Block devices statistics " << std::endl;
	std::cout << "-F	Filesystems statistics " << std::endl;
	std::cout << "-H	Hugepages utilization statistics " << std::endl;
	std::cout << "-I { <int> | SUM | ALL | XALL } " << std::endl;
	std::cout << "	Interrupts statistics	" << std::endl;
	std::cout << "-m { <keyword> [,...] | ALL } " << std::endl;
	std::cout << "	Power management statistics " << std::endl;
	std::cout << "	Keywords are: " << std::endl;
	std::cout << "	CPU	CPU instantaneous clock frequency  " << std::endl;
	std::cout << "	FAN	Fans speed " << std::endl;
	std::cout << "	FREQ	CPU average clock frequency " << std::endl;
	std::cout << "	IN	Voltage inputs " << std::endl;
	std::cout << "	TEMP	Devices temperature " << std::endl;
	std::cout << "	USB	USB devices plugged into the system " << std::endl;
	std::cout << "-n { <keyword> [,...] | ALL } " << std::endl;
	std::cout << "	Network statistics " << std::endl;
	std::cout << "	Keywords are: " << std::endl;
	std::cout << "	DEV	Network interfaces " << std::endl;
	std::cout << "	EDEV	Network interfaces (errors) " << std::endl;
	std::cout << "	NFS	NFS client " << std::endl;
	std::cout << "	NFSD	NFS server " << std::endl;
	std::cout << "	SOCK	Sockets	(v4) " << std::endl;
	std::cout << "	IP	IP traffic	(v4) " << std::endl;
	std::cout << "	EIP	IP traffic	(v4) (errors) " << std::endl;
	std::cout << "	ICMP	ICMP traffic	(v4) " << std::endl;
	std::cout << "	EICMP	ICMP traffic	(v4) (errors) " << std::endl;
	std::cout << "	TCP	TCP traffic	(v4) " << std::endl;
	std::cout << "	ETCP	TCP traffic	(v4) (errors) " << std::endl;
	std::cout << "	UDP	UDP traffic	(v4) " << std::endl;
	std::cout << "	SOCK6	Sockets	(v6) " << std::endl;
	std::cout << "	IP6	IP traffic	(v6) " << std::endl;
	std::cout << "	EIP6	IP traffic	(v6) (errors) " << std::endl;
	std::cout << "	ICMP6	ICMP traffic	(v6) " << std::endl;
	std::cout << "	EICMP6	ICMP traffic	(v6) (errors) " << std::endl;
	std::cout << "	UDP6	UDP traffic	(v6) " << std::endl;
	std::cout << "-q	Queue length and load average statistics " << std::endl;
	std::cout << "-R	Memory statistics " << std::endl;
	std::cout << "-r	Memory utilization statistics " << std::endl;
	std::cout << "-S	Swap space utilization statistics " << std::endl;
	std::cout << "-u [ ALL ] " << std::endl;
	std::cout << "	CPU utilization statistics " << std::endl;
	std::cout << "-v	Kernel tables statistics " << std::endl;
	std::cout << "-W	Swapping statistics " << std::endl;
	std::cout << "-w	Task creation and system switching statistics " << std::endl;
	std::cout << "-y	TTY devices statistics "  << std::endl;
}

int
main (int argc, char *argv[])
{
	int opt;

	if (argc == 1) {
		usage ();
		return -1;
	}

	while ((opt = getopt(argc, argv, "bdF")) != -1) {
		switch (opt) {
			case 'b':
				break;
			case 'B':
				break;
		}
	}
}
