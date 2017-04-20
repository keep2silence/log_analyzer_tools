#include "biz_filter.h"
#include <string.h>

class t1_match : public biz_match
{
public:
	t1_match (std::string who)
	: _who (who)
	{}

	bool handle_input (biz_data *data)
	{
		printf ("|%s| match handle_input\n", _who.c_str ());
		return true;
	}
private:
	std::string _who;
};

class t1_target : public biz_target 
{
public:
	t1_target (std::string who) 
	: _who (who)
	{}

	biz_ret handle_input (biz_data *data)
	{
		printf ("|%s| target handle_input\n", _who.c_str ());
		return RET_OK;
	}

private:
	std::string _who;
};

/// 最简单的单链表测试
static void simple_chain_test ()
{
	biz_table table ("biz_filter_test_table", 1);
	
	biz_chain* pchain = new biz_chain;

#if 1
	std::vector<biz_param> param_vec;
	biz_rule *prule = new biz_rule;
	t1_match *pt1_match = new t1_match ("pt1_match");
	prule->register_match (pt1_match, param_vec);
	t1_target *pt1_target = new t1_target ("pt1_target");
	pt1_target->set_target_type (target_type_continue);
	prule->register_target (pt1_target, param_vec);

	biz_rule *prule2 = new biz_rule;
	t1_match *pt1_match2 = new t1_match ("pt1_match2");
	prule2->register_match (pt1_match2, param_vec);
	t1_target *pt1_target2 = new t1_target ("pt1_target2");
	pt1_target2->set_target_type (target_type_accept);
	prule2->register_target (pt1_target2, param_vec);

	biz_rule *prule3 = new biz_rule;
	t1_match *pt1_match3 = new t1_match ("pt1_match3");
	prule3->register_match (pt1_match3, param_vec);
	t1_target *pt1_target3 = new t1_target ("pt1_target3");
	pt1_target3->set_target_type (target_type_continue);
	prule3->register_target (pt1_target3, param_vec);

	pchain->register_rule (prule, 0);
	pchain->register_rule (prule2, 0);
	pchain->register_rule (prule3, 0);
	int32_t chain_id = table.register_chain (pchain);
	if (chain_id == -1) {
		printf ("register error\n");
		abort ();
	}

	biz_data data;
	memset (data.private_data, 'a', 64);
	data.private_data[63] = '\0';

	table.handle_input (&data, chain_id);
#endif
}

int
main ()
{
	simple_chain_test ();
}
