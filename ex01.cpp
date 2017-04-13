#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <fstream>

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
	int trade_date;
};

class signal_t
{
public:
	signal_t () :
		trade_time (0), direction (0), TICK (0), match_price (0)
	{}

	int trade_time;
	int direction;
	int TICK;
#if 0
    int b1v;
    int s1v;

	double b1p;
    double s1p;
#endif
	double match_price;
	std::string output_info;
	std::string str_trade_time;
	std::string str_direction;
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

		quot.b1p = atof (strvec[13].c_str ());
		quot.b1v = atoi (strvec[14].c_str ());
		quot.s1p = atof (strvec[15].c_str ());
		quot.s1v = atoi (strvec[16].c_str ());
		sscanf(strvec[1].c_str (), "%d:%d:%d.%d", &hh, &mm, &ss, &sss);
		quot.trade_time = (hh * 3600 + mm * 60 + ss) * 1000 + sss;
		quot.trade_date = tradedate;
		quot_que.push_back (quot);
	}
}

bool discard_signal (std::vector<std::string>& vec, signal_t& signal)
{
	/// 当前持仓超过最大允许持仓
	if (abs (net_posi) >= MAX_NET_POSI) {
		if (net_posi > 0) {
			if (vec[11] == std::string ("up")) {
				/// printf ("max_net_posi exit\n");
				return true;
			}
		} else {
			if (vec[11] == std::string ("down")) {
				/// printf ("max_net_posi exit\n");
				return true;
			}
		}
	}

	/// SD 不用flat信号
	if (vec[11] == std::string ("flat")) {
		/// printf ("flat exit\n");
		return true;
	}

	int hh, mm, ss, sss;
	if (vec[11] == std::string ("up")) {
		if (atof (vec[14].c_str ()) < 0.6) {
			/// printf ("up signal exit\n");
			return true; /// 信号强度不够
		}

		sscanf(vec[3].c_str (), "%d:%d:%d.%d", &hh, &mm, &ss, &sss);
		signal.trade_time = (hh * 3600 + mm * 60 + ss) * 1000 + sss;
		signal.direction = UP;
		signal.str_trade_time = vec[3];
		signal.str_direction = std::string ("up");
		return false;
	}

	if (vec[11] == std::string ("down")) {
		if (atof (vec[12].c_str ()) < 0.6) {
			/// printf ("down signal exit\n");
			return true; /// 信号强度不够
		}

		sscanf(vec[3].c_str (), "%d:%d:%d.%d", &hh, &mm, &ss, &sss);
		signal.trade_time = (hh * 3600 + mm * 60 + ss) * 1000 + sss;
		signal.direction = DOWN;
		signal.str_trade_time = vec[3];
		signal.str_direction = std::string ("down");
		return false;
	}

	abort ();
}

int 
main (int argc, char *argv[])
{
	if (argc != 2) {
		printf ("usage: ./ex01 exe\n");
		return -1;
	}
	int current_tradedate = 0;

	std::ofstream outfs ("out.log", std::ios::trunc);
	outfs << "Date,Time,SignalDirection,BidPrice,BidQty,AskPrice,AskQty,MatchPrice,NetPosi,PredDirection,down,flat,up\n";

    std::vector<std::string> strvec;
    std::string line;
    std::fstream fs (argv[1]);

	size_t current_index = 0;
	std::getline (fs, line); /// skip title
    while (std::getline (fs, line)) {
        strvec.clear ();
        split_to_vector (line, strvec);

		int tradedate = atoi (strvec[2].c_str ());
		/// printf ("%d, %d\n", tradedate, current_tradedate);

		if (tradedate > current_tradedate) {
			current_tradedate = tradedate;
			analyze_quot (current_tradedate);
			current_index = 0;
			
			/// 新交易日，将仓位归零
			net_posi = 0;
		}
		
		signal_t signal;
		
		bool is_discard_signal = discard_signal (strvec, signal);

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

				if (is_discard_signal == true) {
					signal.output_info += strvec[11] + "," + strvec[12] + "," + 
						strvec[13] + "," + strvec[14];
					outfs << quot.trade_date << ","  << signal.str_trade_time << "," << 
						signal.str_direction << "," <<
						quot.b1p << "," << quot.b1v << "," << quot.s1p << "," << quot.s1v << 
						"," << signal.match_price << "," << net_posi << "," << 
						signal.output_info << '\n';

					break;
				}

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
						signal.match_price = quot.b1p;
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
				outfs << quot.trade_date << ","  << signal.str_trade_time << "," << 
					signal.str_direction << "," <<
					quot.b1p << "," << quot.b1v << "," << quot.s1p << "," << quot.s1v << 
					"," << signal.match_price << "," << net_posi << "," << signal.output_info << '\n';
				/// signal_que.push_back (signal);
				break;
			}
		}

		if (found == false) {
			printf ("can't find quot: %s\n", strvec[3].c_str ());
			return -1;
		}
	}
}
