#ifndef __BIZ_FILTER_H__
#define __BIZ_FILTER_H__

#include <vector>
#include <list>
#include <stdint.h>
#include <string>
#include <unordered_map>

#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>

#define TODO \
do {	\
	printf ("%s:%d this function will implement in the future.\n", __FILE__, __LINE__); \
} while (0)

/// 一个table中有N个chain，一个chain有N个rule，一个rule中有N个match，
/// 一个rule中有两个target，匹配后执行的target，不匹配执行的target
enum biz_ret
{
	RET_OK = 1,		/// 处理正确
	RET_ACCEPT = (1 << 2),		/// 处理完成，不需要随后的rule进一步处理
	RET_CONTINUE = (1 << 3),	/// 本rule处理结束，需要随后的rule进一步处理

	/// 当match不匹配后，希望执行错误处理的target
	RET_DO_ERR_TARGET = (1 << 4),
	RET_FAILED = (1 << 31)		/// 处理错误
};

class CFTCPField;

/// biz_match输入输出信息
class biz_data
{
public:
	biz_data ()
	: field (NULL), field_id (UINT_MAX), ret (RET_OK)
	{
		/// 这里不清空error_msg和private_data了，不是所有业务都需要
		/// 使用这两个字段，避免性能消耗
	}

	CFTCPField *field; 
	uint32_t field_id;			/// 业务系统中每个field都有一个唯一的编号

	/// 如果希望后面的规则继续处理ret返回RET_CONTINUE
	/// 如果不希望后面的规则继续处理ret返回RET_ACCEPT
	biz_ret ret;				/// 业务处理返回，指导下一步如何处理

	char error_msg[64];			/// 业务处理返回错误信息
	char private_data[64];		/// 未来业务会更复杂，预留一部分空间，对这部分空间的
								/// 解读由前后两个rule来协商
};

/// 各种match和target都可以有配置参数，理想情况下支持通过配置来改变
/// match和target的行为
/// 参数类型只规定几种
enum biz_param_type
{
	biz_param_type_char = 1,
	biz_param_type_int64,
	biz_param_type_uint64,
	biz_param_type_double
};

class biz_param
{
public:
	std::string param_name;
	biz_param_type param_type;
	uint32_t param_index;	/// biz_param最好放到vector中，此参数为vector下标
	uint32_t param_count;	/// 有可能传递param_type类型的数组
	char value[64]; 		/// 使用者保证sizeof (param_type) * param_count < 64
};

/// 
class biz_match 
{
public:
	virtual ~biz_match ()
	{}

	/// 注册时调用
	virtual bool match_init (std::vector<biz_param>& params)
	{
		return true;
	}
	
	/// 返回匹配true或不匹配false
	virtual bool handle_input (biz_data *data) = 0;
	/// 注销前调用
	virtual void match_exit () 
	{}

	std::string _desc;
	std::string _name;
	std::vector<biz_param> _params;
};

enum biz_target_type
{
	target_type_continue = 1,	/// 继续处理，绝大部分都属于这种类型
	target_type_accept = (1 << 1), 	/// 处理结束后不需要进一步处理，终止随后一切规则处理
	target_type_chain = (1 << 2), 	/// 此target是一个chain
	target_type_err = (1 << 3)	/// 此target是处理错误的target，错误处理结束后，终止随后一切规则处理
};

class biz_target
{
public:
	biz_target () 
	: _type (target_type_continue)
	{}

	biz_target (biz_target_type target_type)
	: _type (target_type)
	{}

	virtual ~biz_target ()
	{}

	virtual bool target_init (std::vector<biz_param>& params)
	{
		return true;
	}
	
	virtual biz_ret handle_input (biz_data *data) = 0;

	virtual void target_exit ()
	{}

	biz_target_type get_target_type ()
	{
		return _type;
	}

	void set_target_type (biz_target_type type)
	{
		_type = type;
	}

public:
	std::string _desc;
	std::string _name;
	std::vector<biz_param> _params;
	biz_target_type _type;
};

/// 一条规则有N个match，match之间是and关系，只有所有match全部匹配
/// 后才能算匹配. 一条规则有两个target，一个是匹配后执行的target
/// 一个是不匹配执行的target，创建rule时都需要进行注册
class biz_rule 
{
public:
	biz_rule (std::string rule_name)
	: _rule_position (UINT_MAX), _ptarget (NULL), _perr_target (NULL), _name (rule_name)
	{}

	/// 在注册rule之前，需要将target和match都安装好
	bool rule_init ()
	{
		if (_ptarget == NULL) {
			printf ("%s:%d no target in this rule.\n", __FILE__, __LINE__);
			return false;
		}

		if (_perr_target == NULL) {
			printf ("%s:%d no err target in this rule.\n", __FILE__, __LINE__);
			return false;
		}

		if (_matches_list.empty () == true) {
			printf ("%s:%d no match in this rule.\n", __FILE__, __LINE__);
			return false;
		}
	}

	biz_ret handle_input (biz_data *pdata) 
	{
		for (auto iter = _matches_list.begin ();
			iter != _matches_list.end (); ++iter) {
			biz_match *pmatch = *iter;
			bool ret = pmatch->handle_input (pdata);
			if (ret == false) { /// 不匹配由match指导具体怎么做
				/// 如果希望后面的规则继续处理pdata->ret返回RET_CONTINUE
				/// 如果不希望后面的规则继续处理pdata->ret返回RET_ACCEPT
				if (pdata->ret != RET_DO_ERR_TARGET) {
					return pdata->ret;/// 一条规则中的match是and关系
				} else {
					break;
				}
			}
		}

		/// 匹配后进行target处理，如果target是一条chain，那么会调用
		/// 这条chain的handle_input
		if (pdata->ret != RET_DO_ERR_TARGET) {
			_ptarget->handle_input (pdata);
			if (_ptarget->get_target_type () == target_type_accept) {
				return RET_ACCEPT;
			}
		} else {
			/// 错误target处理后，后面的规则不再处理
			_perr_target->handle_input (pdata);
			return RET_ACCEPT;
		}
		
		/// 和iptables不同的是一条规则处理结束后默认情况下是继续处理
		/// iptables是终止处理
		return RET_CONTINUE;
	}

	/// 按照执行顺序进行注册，match之间是and关系，只有全部match都
	/// 匹配才认为匹配
	bool register_match (biz_match *pmatch, std::vector<biz_param>& params)
	{
		if (pmatch == NULL) {
			printf ("%s:%d pmatch == NULL\n", __FILE__, __LINE__);
			return false;
		}

		if (pmatch->match_init (params) == false) {
			printf ("%s:%d match: %s init failed\n", 
				__FILE__, __LINE__, pmatch->_name.c_str ());
			return false;
		}

		_matches_list.push_back (pmatch);

		return true;
	}

	bool register_target (biz_target *ptarget, std::vector<biz_param>& params)
    {
        if (ptarget == NULL) {
            printf ("%s:%d ptarget == NULL\n", __FILE__, __LINE__);
            return false;
        }

        if (ptarget->target_init (params) == false) {
            printf ("%s:%d target: %s init failed\n", 
				__FILE__, __LINE__, ptarget->_name.c_str ());
			return false;
		}
		
		if (ptarget->get_target_type () == target_type_err) {
			_perr_target = ptarget;
		} else {
			_ptarget = ptarget;
		}

        return true;
    }

	biz_target* get_target ()
	{
		return _ptarget;
	}

	/// 不是关键路径，不考虑性能
#if 0
	void set_name (std::string name) 
	{
		assert (name.empty () == false);
		_name = name;
	}
#endif
	std::string get_name ()
	{
		return _name;
	}

public:
	uint32_t _rule_position;
	std::list<biz_match *> _matches_list;
	std::string _desc;
	biz_target *_ptarget;
	biz_target *_perr_target;
	std::string _name;   /// 规则名字，由于需要此名字来进行删除操作，需要确保同一链中规则名不冲突
};

/// target可以是一条新的链，所以要继承
class biz_chain : public biz_target
{
public:
	biz_chain ()
	{
		set_target_type (target_type_chain);
	}

	biz_ret handle_input (biz_data *pdata)
	{
		biz_ret ret;
		for (auto iter = rules_list.begin (); 
			iter != rules_list.end (); ++iter) {
			biz_rule *prule = *iter;
			ret = prule->handle_input (pdata);
			if (ret == RET_CONTINUE) {
				continue;
			}

			/// 不是RET_CONTINUE的规则都要返回给父链处理
			/// 有可能在子链调用，需要返回给父链，父链也需要
			/// 这个标志，否则继续会处理父链上的规则
			return ret;
		}

		return RET_CONTINUE;
	}

	/// 规则可以加到任意位置，业务含义有使用人员保证
	/// position == 0，在链表首部加入
	/// position == 1，在第一个规则后加入
	/// position == rules_list.size ()，在链表尾加入
	/// 注意: 连续加入多条规则时，规则链表的位置一直在变，比如以前第一条规则，有可能
	/// 变成第二条，需要仔细操作
	bool register_rule (biz_rule *prule, size_t position) 
	{
		if (position < 0 || position > rules_list.size ()) {
			printf ("%s:%d argument position: %zu is invalid\n", 
				__FILE__, __LINE__, position);
			return false;
		}

		if (prule == NULL) {
			printf ("%s:%d argument prule == NULL is invalid\n", 
				__FILE__, __LINE__);
			return false;
		}

		/// 进行规则名重复性检查
		for (auto iter = rules_list.begin (); iter != rules_list.end (); ++iter) {
			if ((*iter)->get_name () == prule->get_name ()) {
				printf ("%s:%d rule: %s register twice.\n",
						__FILE__, __LINE__,  prule->get_name ().c_str ());
				return false;
			}
		}

		size_t current_position = 0;
		for (auto iter = rules_list.begin (); iter != rules_list.end (); ++iter) {
			if (current_position++ == position) {
				rules_list.insert (iter, prule);
				return true;
			}
		}
		/// 前面已经做了position合理性检查
		rules_list.push_back (prule);
		
		/// 如果此链中有一个rule的target是accept类型的，整个链也是accept类型的
		if (prule->get_target ()->get_target_type () == target_type_accept) {
			set_target_type (target_type_accept);
		}

		return false;
	}

	/// 目前还不想使用id来删除rule，而是通过位置来执行删除
	/// 注意：第一条规则的position == 1，其他顺序编号
	bool unregister_rule (size_t position)
	{
		if (position < 1 || position > rules_list.size ()) {
			printf ("%s:%d argument position: %zu is invalid\n", 
				__FILE__, __LINE__, position);
			return false;
		}

		size_t current_position = 0;
		for (auto iter = rules_list.begin (); iter != rules_list.end (); ++iter) {
			if (++current_position == position) {
				rules_list.erase (iter);
				return true;
			}
		}
		
		return false;
	}

	/// 根据规则名删除rule，方便使用，但需要用户确保规则名的唯一性
	bool unregister_rule (std::string rule_name) 
	{
		for (auto iter = rules_list.begin (); iter != rules_list.end (); ++iter) {
			if ((*iter)->_name == rule_name) {
				rules_list.erase (iter);
				return true;
			}
		}
		
		return false;
	}

	std::string& get_chain_name ()
	{
		return _chain_name;
	}

	int32_t get_chain_id ()
	{
		return _chain_id;
	}

private:
	std::string _chain_name;
	int32_t _chain_id;	/// 每条链全局唯一id，局限于表中，不同表之间有可能有相同chain_id
						/// 不会出现跨表跳到其他表的链这种情况
	std::string _desc;
	/// 一个chain上的rule可以插入新的rule，可以删除
	std::list<biz_rule *> rules_list;
};

class biz_table
{
public:
	biz_table (std::string table_name, int32_t table_id)
	: _table_id (table_id), _table_name (table_name), _chain_id (0)
	{
	}

	/// chain_id是一张表中的某条链，使用时需要注意不要直接跳转到子链，
	/// 业务会错误
	void handle_input (biz_data *pdata, int32_t chain_id)
	{
		auto iter = _chain_map.find (chain_id); 
		if (iter == _chain_map.end ()) {
			printf ("%s:%d chain_id: %d not find.\n", 
				__FILE__, __LINE__, chain_id);
			abort ();
		}

		biz_chain *pchain = iter->second;
		pchain->handle_input (pdata);
	}

	bool delete_chain (std::string& chain_name)
	{
		for (auto iter = _chain_map.begin (); 
			iter != _chain_map.end (); ++iter) {
			if (iter->second->get_chain_name () != chain_name) {
				_chain_map.erase (iter);
				return true;
			}
		}

		printf ("%s:%d chain: %s doesn't exist.\n",
			 __FILE__, __LINE__, chain_name.c_str ());
		return false;
	}

	bool delete_chain (int32_t chain_id) 
	{
		auto iter = _chain_map.find (chain_id);
		if (iter == _chain_map.end ()) {
			printf ("%s:%d chain_id: %d doesn't exist.\n",
				 __FILE__, __LINE__, chain_id);
			return false;
		}

		_chain_map.erase (iter);
		return true;
	}

	/// chain_name 必须唯一，chain不能重复注册
	/// table为每一个chain分配唯一id，当在链间跳转时不牺牲性能
	/// return -1，表示出错
	int32_t register_chain (biz_chain* pchain) 
	{
		for (auto iter = _chain_map.begin (); 
			iter != _chain_map.end (); ++iter) {
			if (iter->second->get_chain_name () == pchain->get_chain_name ()) {
				printf ( "%s has been registered.\n", 
					pchain->get_chain_name ().c_str ());
				return -1;
			}
		}

		++_chain_id;
		_chain_map.insert (std::make_pair (_chain_id, pchain));
		return _chain_id;
	}

private:
	int32_t _table_id; /// 全局唯一的表id
	std::string _table_name;
	int32_t _chain_id;

	/// 有可能将整条chain删除
	/// <chain_id, chain>
	std::unordered_map<int32_t, biz_chain *> _chain_map;
};

/// 定义若干个常用match和target
/// 无条件匹配
class true_match : public biz_match 
{
public:
	bool handle_input (biz_data *pdata) 
	{
		return true;
	}
};

class false_match : public biz_match
{
public:
	bool handle_input (biz_data *pdata) 
	{
		return false;
	}
};

/// 业务上出现严重错误，需要退出时使用，这里不进行日志记录
/// 只在控制台上打印退出原因信息
class severe_err_target : public biz_target
{
public:
	biz_ret handle_input (biz_data *pdata)
	{
		printf ("%s:%d severe err: %s, exit.\n", 
				__FILE__, __LINE__, pdata->error_msg);
		fflush (stdout);
		sync ();
		abort ();
		return RET_FAILED;
	}
};

#endif
