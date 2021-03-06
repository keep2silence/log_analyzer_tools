#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <fstream>

#include <assert.h>

#define UP 1
#define DOWN 2

#define BUY 10
#define SELL 20

#define OPEN 100
#define OFFSET 200

#define MAX_NET_POSI 3
#define PROB 0.48

static int max_net_posi = -1;
static double prob = 0;

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

class posi_t 
{
public:
	int direction;
	int volume;
	double match_price;
};

static std::list<posi_t *> buy_posi_list;
static std::list<posi_t *> sell_posi_list;

static int net_posi = 0;
/// static std::deque<quot_t> quot_que;
/// static std::deque<signal_t> signal_que;
static std::unordered_map<std::string, quot_t> quot_map;

/// 交易日快结束前进行强平的行情
static quot_t force_offset_quot;

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
	quot_map.clear ();
	
	int hh, mm, ss, sss;
    std::vector<std::string> strvec;
    std::string line;
	char quotfile[256];
	snprintf (quotfile, 256, "ru-%d.csv", tradedate);
    std::fstream fs (quotfile);

	quot_t quot;
	force_offset_quot.b1v = -1;
    while (std::getline (fs, line)) {
        strvec.clear ();
        split_to_vector (line, strvec);

		if (strvec[1] != std::string ("ru1709")) {
			continue;
		}

		quot.b1p = atof (strvec[15].c_str ());
		quot.b1v = atoi (strvec[16].c_str ());
		quot.s1p = atof (strvec[17].c_str ());
		quot.s1v = atoi (strvec[18].c_str ());
		sscanf(strvec[2].c_str (), "%d:%d:%d.%d", &hh, &mm, &ss, &sss);
		quot.trade_time = (hh * 3600 + mm * 60 + ss) * 1000 + sss;
		quot.trade_date = tradedate;
		if (hh == 14 && mm == 55 && ss == 0 && force_offset_quot.b1v == -1) {
			force_offset_quot = quot;
			printf ("14:55 force offset, b1p: %f, s1p: %f\n", quot.b1p, quot.s1p);
		}
		/// quot_que.push_back (quot);
		quot_map.insert (std::make_pair (strvec[2], quot));
	}
}

bool discard_signal (std::vector<std::string>& vec, signal_t& signal)
{
	signal.str_trade_time = vec[3];
	/// 当前持仓超过最大允许持仓
	if (abs (net_posi) >= max_net_posi) {
		if (net_posi > 0) {
			/// if (vec[11] == std::string ("up")) {
			if (vec[15].find ('-') == std::string::npos) {
				signal.str_direction = std::string ("up");
				/// printf ("max_net_posi exit\n");
				return true;
			}
		} else {
			/// if (vec[11] == std::string ("down")) {
			if (vec[15].find ('-') != std::string::npos) {
				signal.str_direction = std::string ("down");
				/// printf ("max_net_posi exit\n");
				return true;
			}
		}
	}

#if 0
	/// SD 不用flat信号
	if (vec[11] == std::string ("flat")) {
		/// printf ("flat exit\n");
		signal.str_direction = std::string ("flat");
		return true;
	}
#endif
	int hh, mm, ss, sss;
	/// if (vec[11] == std::string ("up")) {
	if (vec[15].find ('-') == std::string::npos) {
		if (atof (vec[15].c_str ()) + 0.001 < prob) {
			/// printf ("up signal exit\n");
			signal.str_direction = std::string ("up");
			return true; /// 信号强度不够
		}

		sscanf(vec[2].c_str (), "%d:%d:%d.%d", &hh, &mm, &ss, &sss);
		signal.trade_time = (hh * 3600 + mm * 60 + ss) * 1000 + sss;
		signal.direction = UP;
		signal.str_trade_time = vec[2];
		signal.str_direction = std::string ("up");
		return false;
	}

	/// if (vec[11] == std::string ("down")) {
	if (vec[15].find ('-') != std::string::npos) {
		if (atof (vec[12].c_str ()) - 0.001 > prob) {
			signal.str_direction = std::string ("down");
			/// printf ("down signal exit\n");
			return true; /// 信号强度不够
		}

		sscanf(vec[2].c_str (), "%d:%d:%d.%d", &hh, &mm, &ss, &sss);
		signal.trade_time = (hh * 3600 + mm * 60 + ss) * 1000 + sss;
		signal.direction = DOWN;
		signal.str_trade_time = vec[2];
		signal.str_direction = std::string ("down");
		return false;
	}

	abort ();
}

int 
main (int argc, char *argv[])
{
	if (argc != 4) {
		printf ("usage: ./ex01 exe count prob\n");
		return -1;
	}
	int current_tradedate = 0;
	
	max_net_posi = atoi (argv[2]);
	prob = atof (argv[3]);

	char fname[256];
	snprintf (fname, 256, "profit.csv");
	std::ofstream out_stat_fs (fname, std::ios::trunc);
	out_stat_fs << "Date,TradeTimes,Profit\n";

	int trade_times = 0;
	double profit = 0;

	std::ofstream outfs;
	outfs << "Date,Time,SignalDirection,BidPrice,BidQty,AskPrice,AskQty,MatchPrice,NetPosi,PredDirection,down,flat,up\n";

    std::vector<std::string> strvec;
    std::string line;
    std::fstream fs (argv[1]);

	size_t current_index = 0;
	std::getline (fs, line); /// skip title
    while (std::getline (fs, line)) {
        strvec.clear ();
        split_to_vector (line, strvec);

		int tradedate = atoi (strvec[1].c_str ());
		/// printf ("%d, %d\n", tradedate, current_tradedate);

		if (tradedate > current_tradedate) {
			/// 交易日结束，全部进行对价平仓，进行盈亏计算
			if (net_posi > 0) {
				assert (buy_posi_list.empty () == false);
				for (auto iter = buy_posi_list.begin ();
					iter != buy_posi_list.end (); ++iter) {
					posi_t *posi = *iter;
					
					profit += (force_offset_quot.s1p - posi->match_price);	
					delete posi;
				}
				buy_posi_list.clear ();
			} else if (net_posi < 0) {
				assert (sell_posi_list.empty () == false);
				for (auto iter = sell_posi_list.begin ();
					iter != sell_posi_list.end (); ++iter) {
					posi_t *posi = *iter;
					profit += (posi->match_price - force_offset_quot.b1p);	

					delete posi;
				}
				sell_posi_list.clear ();
			}

			if (current_tradedate > 0) {
				out_stat_fs << current_tradedate << "," << trade_times << "," << profit << '\n';
			}

			current_tradedate = tradedate;
			analyze_quot (current_tradedate);
			current_index = 0;
		
			if (outfs.is_open () == true) {
				outfs.close ();
			}			

			snprintf (fname, 256, "%d_out.csv", current_tradedate);
			outfs.open (fname, std::ios::trunc);
			outfs << "Date,Time,SignalDirection,BidPrice,BidQty,AskPrice,AskQty,MatchPrice,NetPosi,PredDirection,down,flat,up\n";

			/// 新交易日，将仓位归零
			net_posi = 0;
	
			profit = 0;
			trade_times = 0;
		}
		
		signal_t signal;
		bool is_discard_signal = discard_signal (strvec, signal);

		/// 从上次结束的地方开始进行遍历，找到对应的时间
		int hh, mm, ss, sss;
		sscanf(strvec[2].c_str (), "%d:%d:%d.%d", &hh, &mm, &ss, &sss);
		int trade_time = (hh * 3600 + mm * 60 + ss) * 1000 + sss;
	
		std::unordered_map<std::string, quot_t>::iterator iter = quot_map.find (strvec[2]);
		if (iter != quot_map.end ()) {
			/// quot_t& quot = quot_que[i];
			quot_t& quot = iter->second;
			if (quot.trade_time == trade_time) {

				if (is_discard_signal == true) {
#if 1
					/// signal.output_info += strvec[11] + "," + strvec[12] + "," + 
					///	strvec[13] + "," + strvec[14];
					signal.output_info += strvec[14] + "," + strvec[15];
					outfs << quot.trade_date << ","  << signal.str_trade_time << "," << 
						signal.str_direction << "," <<
						quot.b1p << "," << quot.b1v << "," << quot.s1p << "," << quot.s1v << 
						"," << signal.match_price << "," << net_posi << "," << 
						signal.output_info << '\n';
#endif
					continue;
				}

				++trade_times;
				/// signal.TICK = current_index;
				int open_or_offset = 0;
				/// 根据净持仓情况决定开平
				if (net_posi < 0) {
					if (signal.direction == UP) {
						/// 买平
						signal.match_price = quot.s1p;
						++net_posi;

						/// 平仓计算盈亏，先开先平
						while (sell_posi_list.empty () == false) {
							posi_t *posi = sell_posi_list.front ();
							sell_posi_list.pop_front ();
							profit += (posi->match_price - signal.match_price);
							delete posi;
							printf ("sell_posi_list size: %zu\n", sell_posi_list.size ());
							printf ("BUY offset price: %f, %f, profit: %f\n", 
								quot.s1p, posi->match_price, profit);
							break;
						}
					} else {
						/// 卖开
						signal.match_price = quot.b1p;
						--net_posi;
						posi_t *posi = new posi_t;
						posi->volume = 1;
						posi->direction = SELL;
						posi->match_price = quot.b1p;
						printf ("SELL open price: %f\n", quot.b1p);
						sell_posi_list.push_back (posi);
					}
				} else if (net_posi > 0) {
					/// 净买仓
					if (signal.direction == UP) {
						/// 买开
						signal.match_price = quot.s1p;
						++net_posi;
						posi_t *posi = new posi_t;
						posi->volume = 1;
						posi->direction = BUY;
						posi->match_price = quot.s1p;
						printf ("BUY open price: %f\n", quot.s1p);
						buy_posi_list.push_back (posi);
					} else {
						/// 卖平
						signal.match_price = quot.b1p;
						--net_posi;
						/// 平仓计算盈亏，先开先平
						while (buy_posi_list.empty () == false) {
							posi_t *posi = buy_posi_list.front ();
							buy_posi_list.pop_front ();
							profit += (signal.match_price - posi->match_price);
							delete posi;
							printf ("SELL offset price: %f, %f, profit: %f\n", 
								quot.b1p, posi->match_price, profit);
							printf ("buy_posi_list size: %zu\n", buy_posi_list.size ());
							break;
						}
					}					
				} else { /// net_posi == 0
					/// 净买仓
					if (signal.direction == UP) {
						/// 买开
						signal.match_price = quot.s1p;
						++net_posi;
						posi_t *posi = new posi_t;
						posi->volume = 1;
						posi->direction = BUY;
						posi->match_price = quot.s1p;
						printf ("BUY open price: %f\n", quot.s1p);
						buy_posi_list.push_back (posi);
					} else {
						/// 卖开
						signal.match_price = quot.b1p;
						--net_posi;
						posi_t *posi = new posi_t;
						posi->volume = 1;
						posi->direction = SELL;
						posi->match_price = quot.b1p;
						printf ("SELL open price: %f\n", quot.b1p);
						sell_posi_list.push_back (posi);
					}					
				}

				
				/// signal.output_info += strvec[11] + "," + strvec[12] + "," + strvec[13] + "," + strvec[14];
				signal.output_info += strvec[14] + "," + strvec[15];
				outfs << quot.trade_date << ","  << signal.str_trade_time << "," << 
					signal.str_direction << "," <<
					quot.b1p << "," << quot.b1v << "," << quot.s1p << "," << quot.s1v << 
					"," << signal.match_price << "," << net_posi << "," << signal.output_info << '\n';
				/// signal_que.push_back (signal);
				continue;
			}
		} else {
			printf ("can't find quot: %s, %s\n", strvec[4].c_str (), strvec[2].c_str ());
			return -1;
		}
	}
}
