#include <vector>
#include <unordered_map>
#include <unordered_set>

#define UP 1
#define DOWN 2

#define BUY 10
#define SELL 20

#define OPEN 100
#define OFFSET 200

#define MAX_NET_POSI 10

class quot_t
{
public:
	double b1p;
	double s1p;

	int b1v;
	int s1v;
	int trade_time;
};

class signal_t
{
public:
	int trade_time;
	int direction;
	int TICK;
	double match_price;
	std::string output_info;
};

#if 0
class posi_t 
{
public:
	int direction;
	int volume;
};

static std::list<posi_t> buy_posi_list;
static std::list<posi_t> sell_posi_list;
#endif

static int net_posi = 0;
static std::deque<quot_t> quot_que;
static std::deque<signal_t> signal_que;

static void split_to_vector (std::string line, std::vector<std::string> &stdvec)
{
    std::string::size_type pos;
    while ((pos = line.find_first_of (',')) != std::string::npos) {
        std::string str = line.substr (0, pos);
        stdvec.push_back (str);
        line = line.substr (pos + 1);
    }
    stdvec.push_back (line);
}

void analyze_quot (int tradedate)
{
	quot_que.clear ();
	
	int hh, mm, ss, sss;
    std::vector<std::string> strvec;
    std::string line;
	char quotfile[256];
	snprintf (quotfile, 256, "m/m-%d.txt", tradedate);
    std::fstream fs (quotfile);

	quot_t quot;
    while (std::getline (fs, line)) {
        strvec.clear ();
        split_to_vector (line, strvec);

		quot.b1p = atof (strvec[13]);		
		quot.b1v = atoi (strvec[14]);		
		quot.s1p = atof (strvec[15]);		
		quot.s1v = atoi (strvec[16]);		
		sscanf(strvec[1].c_str (), "%d:%d:%d.%d", &hh, &mm, &ss, &sss);
		quot.trade_time = (hh * 3600 + mm * 60 + ss) * 1000 + sss;
		quot_que.push_back (quot);
	}
}

bool discard_signal (std::vector<std::string>& vec, signal_t& signal)
{
	/// 当前持仓超过最大允许持仓
	if (abs (net_posi) >= MAX_NET_POSI) {
		return true;
	}

	/// SD 不用flat信号
	if (vec[11] == std::string ("flat")) {
		return true;
	}

	int hh, mm, ss, sss;
	if (vec[11] == std::string ("up")) {
		if (atof (vec[14]) < 0.6) {
			return false; /// 信号强度不够
		}

		sscanf(strvec[3].c_str (), "%d:%d:%d.%d", &hh, &mm, &ss, &sss);
		quot.trade_time = (hh * 3600 + mm * 60 + ss) * 1000 + sss;
		signal.direction = UP;
		return false;
	}

	if (vec[11] == std::string ("down")) {
		if (atof (vec[12]) < 0.6) {
			return false; /// 信号强度不够
		}

		sscanf(strvec[3].c_str (), "%d:%d:%d.%d", &hh, &mm, &ss, &sss);
		quot.trade_time = (hh * 3600 + mm * 60 + ss) * 1000 + sss;
		signal.direction = DOWN;
		return false;
	}

	abort ();
}

int 
main (int argc, char *argv[])
{
	if (argc != 3) {
		printf ("usage: ./ex01 quotfile exe\n");
		return -1;
	}
	int current_trade_date = 0;

    std::vector<std::string> strvec;
    std::string line;
    std::fstream fs (argv[2]);

	size_t current_index = 0;
    while (std::getline (fs, line)) {
        strvec.clear ();
        split_to_vector (line, strvec);

		signal_t signal;
		if (discard_signal (strvec, signal) == true) {
			continue;
		}
#if 0
V1,ContractID,Date,Time,LastPrice,MidP,LastMatchQty,MatchTotQty,STATIC_MomentumA_VOL,STATIC_MomentumB_VOL,Class,predict,down,flat,up
1,m1609,20160704,21:08:13.623,3445,3444.5,2,93914,-2,8,up,flat,0.19,0.61,0.2
27,m1609,20160704,21:08:20.270,3445,3445.5,446,95008,220,80,flat,flat,0.42,0.51,0.07
215,m1609,20160704,21:09:08.375,3448,3448.5,102,105498,24,57,down,flat,0.31,0.57,0.12
2,m1609,20160704,21:08:13.847,3445,3444.5,24,93938,-89,-3,up,flat,0.31,0.55,0.14
3,m1609,20160704,21:08:14.164,3445,3444.5,8,93946,3,100,up,flat,0.2,0.53,0.27
4,m1609,20160704,21:08:14.405,3445,3444.5,4,93950,2,3,up,flat,0.29,0.53,0.18
5,m1609,20160704,21:08:14.608,3445,3444.5,0,93950,0,0,up,flat,0.29,0.52,0.19
6,m1609,20160704,21:08:14.908,3444,3444.5,12,93962,1,-3,up,flat,0.2,0.66,0.14
7,m1609,20160704,21:08:15.163,3445,3444.5,2,93964,0,1,up,flat,0.22,0.64,0.14
8,m1609,20160704,21:08:15.421,3445,3444.5,12,93976,3,0,up,flat,0.22,0.66,0.12
#endif
		int tradedate = atoi (strvec[2].c_str ());
		if (tradedate > current_tradedate) {
			current_tradedate = tradedate;
			analyze_quot (current_tradedate);
			current_index = 0;
		}
		
		/// 从上次结束的地方开始进行遍历，找到对应的时间
		bool found = false;
		int hh, mm, ss, sss;
		sscanf(strvec[3].c_str (), "%d:%d:%d.%d", &hh, &mm, &ss, &sss);
		int trade_time = (hh * 3600 + mm * 60 + ss) * 1000 + sss;

		for (size_t i = current_index; i < quot_que.size (); ++i) {
			quot_t& quot = quot_que[i];
			if (quot.trade_time == trade_time) {
				current_index = i;
				found = true;

				signal.TICK = current_index;

				int open_or_offset = 0;
				/// 根据净持仓情况决定开平
				if (net_posi <= 0) {
					if (signal.direction == UP) {
						/// 买平
						signal.match_price = quot.s1p;
						++net_posi;
					} else {
						/// 卖开
						singal.match_price = quot.b1p;
						--net_posi;
					}
				} else {
					/// 净买仓
					if (signal.direction == UP) {
						/// 买开
						signal.match_price = quot.s1p;
						++net_posi;
					} else {
						/// 卖平
						signal.match_price = quot.b1p;
						--net_posi;
					}					
				}
				
				signal.output_info += strvec[11] + "," + strvec[12] + "," + strvec[13] + "," + strvec[14];
				signal_que.push_back (signal);
			}
		}

		if (found == false) {
			printf ("can't find quot: %s\n", strvec[3].c_str ());
			return -1;
		}

		
	}
}
