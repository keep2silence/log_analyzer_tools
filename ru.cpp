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
static double profit = 0;
static int trade_times = 0;

class posi_t 
{
public:
	int direction;
	int volume;
	double match_price;
};

static std::list<posi_t *> buy_posi_list;
static std::list<posi_t *> sell_posi_list;

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

#define UP_SIGNAL_PROB (7)
#define DOWN_SIGNAL_PROB (-3)
static bool check_matched (std::vector<std::string>& strvec)
{
	double prob = atof (strvec[15].c_str ());
	if (prob > UP_SIGNAL_PROB) {
		posi_t *posi = new posi_t;
		posi->direction = BUY;
		posi->match_price = atof (strvec[11].c_str ()); // 卖价成交
		posi->volume = 1;

		///printf ("%s:%d %s,%s BUY OPEN price: %.2f\n", __FILE__, __LINE__,
		printf ("%s,%s BUY OPEN price: %.2f\n", 
			strvec[1].c_str (), strvec[2].c_str (), posi->match_price);		
		buy_posi_list.push_back (posi);
		return true;
	}

	if (prob < DOWN_SIGNAL_PROB) {
		posi_t *posi = new posi_t;
		posi->direction = SELL;
		posi->match_price = atof (strvec[9].c_str ()); // 买价成交
		posi->volume = 1;
		
		/// printf ("%s:%d %s,%s SELL OPEN price: %.2f\n", __FILE__, __LINE__,
		printf ("%s,%s SELL OPEN price: %.2f\n",
			strvec[1].c_str (), strvec[2].c_str (), posi->match_price);		
		sell_posi_list.push_back (posi);
		return true;
	}

	return false;
}

#define UP_OFFSET_PROB (2.5)
#define DOWN_OFFSET_PROB (-2.5)
static void check_offset (std::vector<std::string>& strvec)
{
	double prob = atof (strvec[15].c_str ());
	if (prob > UP_OFFSET_PROB) {
		if (sell_posi_list.empty () == false) {
			posi_t *posi = sell_posi_list.front ();
			sell_posi_list.pop_front ();
			
			/// 买平
			double offset_price = atof (strvec[11].c_str ());
			profit += (posi->match_price - offset_price);
			printf ("%s,%s BUY OFFSET price: %.2f : %.2f, profit: %.2f, sum_profit: %.2f\n", 
				strvec[1].c_str (), strvec[2].c_str (), posi->match_price, 
				offset_price, posi->match_price - offset_price, profit);		
		}
		return;
	}

	if (prob < DOWN_OFFSET_PROB) {
		if (buy_posi_list.empty () == false) {
			posi_t *posi = buy_posi_list.front ();
			buy_posi_list.pop_front ();

			/// 卖平
			double offset_price = atof (strvec[9].c_str ());
			profit += (offset_price - posi->match_price);
			printf ("%s,%s SELL OFFSET price: %.2f : %.2f, profit: %.2f, sum_profit: %.2f\n", 
				/// __FILE__, __LINE__,
				strvec[1].c_str (), strvec[2].c_str (), offset_price, 
				posi->match_price, offset_price - posi->match_price, profit);
		}
		return;
	}

	return;
}

#define INT_FORCE_OFFSET_TIME  ((14 * 60 + 55) * 60)
static void check_force_offset (std::vector<std::string>& strvec)
{
	int hh, mm, ss, sss;
	sscanf(strvec[2].c_str (), "%d:%d:%d.%d", &hh, &mm, &ss, &sss);
	int trade_time = (hh * 3600 + mm * 60 + ss);
	/// 时间到达14:55将所有持仓强制平仓
	if (trade_time != INT_FORCE_OFFSET_TIME) {
		return;
	}

	while (sell_posi_list.empty () == false) {
		posi_t *posi = sell_posi_list.front ();
		sell_posi_list.pop_front ();

		double offset_price = atof (strvec[11].c_str ());
		profit += (posi->match_price - offset_price);
		printf ("%s,%s BUY FORCE OFFSET price: %.2f : %.2f, profit: %.2f, sum_profit: %.2f\n", 
			/// __FILE__, __LINE__,
			strvec[1].c_str (), strvec[2].c_str (), posi->match_price, 
			offset_price, posi->match_price - offset_price, profit);		
	}

	while (buy_posi_list.empty () == false) {
		posi_t *posi = buy_posi_list.front ();
		buy_posi_list.pop_front ();

		double offset_price = atof (strvec[9].c_str ());
		profit += (offset_price - posi->match_price);
		printf ("%s,%s SELL FORCE OFFSET price: %.2f : %.2f, profit: %.2f, sum_profit: %.2f\n", 
			/// __FILE__, __LINE__,
			strvec[1].c_str (), strvec[2].c_str (), offset_price, 
			posi->match_price, offset_price - posi->match_price, profit);
	}

	return;
}

int 
main (int argc, char *argv[])
{
	if (argc != 2) {
		/// printf ("usage: ./ex01 exe count prob\n");
		printf ("usage: ./ex01 exe\n");
		return -1;
	}
	int current_tradedate = 0;
	
	char fname[256];
	snprintf (fname, 256, "profit.csv");
	std::ofstream out_stat_fs (fname, std::ios::trunc);
	out_stat_fs << "Date,TradeTimes,Profit\n";

	buy_posi_list.clear ();
	sell_posi_list.clear ();

    std::vector<std::string> strvec;
    std::string line;
    std::fstream fs (argv[1]);

	size_t current_index = 0;
	std::getline (fs, line); /// skip title
    while (std::getline (fs, line)) {
        strvec.clear ();
        split_to_vector (line, strvec);

		int tradedate = atoi (strvec[1].c_str ());
		if (tradedate > current_tradedate) {
			/// 切换交易日
			if (current_tradedate != 0) {
				/// 写日志
				out_stat_fs << current_tradedate << "," << 
					trade_times << "," << profit << '\n';
			} 

			/// 清除统计变量，为下一交易日做准备
			current_tradedate = tradedate;
			profit = 0;
			/// 应该被强平了
			/// assert (buy_posi_list.empty () == true);
			/// assert (sell_posi_list.empty () == true);
			buy_posi_list.clear ();
			sell_posi_list.clear ();
			trade_times = 0;
		}

		/// 看看是否需要平仓，如果可以平仓，先平仓再下新单
		check_offset (strvec);

		/// 看看是否需要强平
		check_force_offset (strvec);

		/// 看看是否开仓
		check_matched (strvec);
	}
}

