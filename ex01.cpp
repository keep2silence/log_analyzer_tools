#include <vector>
#include <unordered_map>
#include <unordered_set>

class quot_t
{
public:
	quot_t ()
	{
		time[13] = '\0';
	}
	double b1p;
	double s1p;

	int b1v;
	int s1v;
	char time[16];
};

class signal_t
{

};

static std::deque<quot_t> quot_que;

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
		memcpy (quot.time, strvec[1].c_str (), 12);
		quot_que.push_back (quot);
	}
}

bool discard_signal (std::vector<std::string>& vec, signal_t& signal)
{

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
		for (size_t i = current_index; i < quot_que.size (); ++i) {
			if (memcmp (quot_que[i].time, strvec[3].c_str (), 12) == 0) {
				current_index = i;
				found = true;
			}
		}

		if (found == false) {
			printf ("can't find quot: %s\n", strvec[3].c_str ());
			return -1;
		}

		
	}
}
