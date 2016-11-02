#ifndef __PRICE_LEVEL_H__
#define __PRICE_LEVEL_H__

#include <vector>

#include <assert.h>
#include <math.h>

typedef enum
{
	BS_INVALID = '0',
    BUY = '1',
    SELL = '3'
} BSFLAG;

typedef enum
{
	OF_INVALID = '0',
    OPEN = '1',
    OFFSET = '2'
} OFFLAG;

class Order 
{
public:
	Order (): bsflag (BS_INVALID), offlag (OF_INVALID), price (0.0),
		vol (0), sysno (0), batchno (0), clientid (0)
	{}
	
	std::string contract;
	BSFLAG bsflag;
	OFFLAG offlag;
	double price;
	int vol;
	int sysno;
	int batchno;
	int clientid;
};

class Match
{
public:
	Match (): bsflag (BS_INVALID), offlag (OF_INVALID), price (0.0),
		vol (0), sysno (0), batchno (0), clientid (0)
	{}

	std::string contract;
	BSFLAG bsflag;
	OFFLAG offlag;
	double price;
	int vol;
	int sysno;
	int batchno;
	int clientid;
};

class Cancel 
{
public:
	Cancel (): vol (0), batchno (0)
	{}

	std::string contract;
	int vol;
	int batchno;
};

class PriceLevel
{
public:
	PriceLevel ()
	{}

	PriceLevel (double price_, int index_)
	: price (price_), buy_vol (0), sell_vol (0), index (index_)
	{}

	double price;
	int    buy_vol;
	int    sell_vol;
	int    index;
	std::list<Order *> buy_order_list;
	std::list<Order *> sell_order_list;
};


class PriceLevelVec
{
public:
	PriceLevelVec (double lower, double upper, double tick_)
	: lower_limit (lower), upper_limit (upper), tick (tick_)
	{
		pricelevel_count = (int)((upper_limit - lower_limit) / tick) + 1;
	
		pricelevel_vec.reserve (pricelevel_count);
		for (int i = 0; i < pricelevel_count; ++i) {
			pricelevel_vec.push_back (new PriceLevel ((lower_limit + i * tick), i));
		}
	}

	PriceLevel *get_pricelevel_by_price (double price)
	{
		int index = (int)((price - lower_limit) / tick);
		assert (index < pricelevel_count && index >= 0);
		return pricelevel_vec[index];
	}

	std::vector<PriceLevel *> pricelevel_vec;
	double lower_limit;
	double upper_limit;
	double tick;
	int pricelevel_count;
};

int DoubleCompare(double dSource1,double dSource2)
{
	const double DPREC = 0.00001;
	if (fabs (dSource1 - dSource2) < DPREC) {
		return 0;
	} else if (dSource1>dSource2) {
		return 1;
	} else {
		return -1;
	}
}

struct price_functor
{
    bool operator () (const double & a, const double & b) const {
        if (DoubleCompare(a, b) < 0) {
            return true;
        }

        return false;
    }
};

class PriceLevelMap
{
public:
	std::map<double, PriceLevel *, price_functor> pricelevel_map;
	int pricelevel_count;
	
};

#endif
